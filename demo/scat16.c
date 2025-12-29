/** SCAT16 — Structured Reaction (SSR) with OP (instruction) **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <liburing.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <inttypes.h>

/* =========================
 *  Basic types / counters
 * ========================= */
typedef int      cap_id_t;
typedef uint64_t msg_id_t;
typedef uint64_t trace_id_t;

static unsigned long g_msg_created  = 0;
static unsigned long g_msg_handled  = 0;
static unsigned long g_msg_dropped  = 0;

static int g_running = 1;

/* =========================
 *  io_uring stdin
 * ========================= */
static struct io_uring g_ring;
static char g_stdin_buf[1024];

#define FILE_OFFSET_START (0)
#define NO_FLAGS (0)

/* =========================
 *  CAP — pure capability
 * ========================= */
typedef uint32_t cap_rights_t;

enum { CAP_NONE = 0 };
enum { CAP_TABLE_SIZE = 64 };

enum {
    CAP_R_READ  = 1u << 0,
    CAP_R_WRITE = 1u << 1,
};

typedef struct {
    int fd;              /* anchor resource */
    cap_rights_t rights; /* permissions */
    int in_use;          /* slot occupied */
} CapEntry;

static CapEntry g_caps[CAP_TABLE_SIZE];

static void cap_init(void)
{
    for (int i = 0; i < CAP_TABLE_SIZE; i++) {
        g_caps[i].fd = -1;
        g_caps[i].rights = 0;
        g_caps[i].in_use = 0;
    }
}

static int cap_is_valid(cap_id_t cap)
{
    if (cap == CAP_NONE) return 0;
    if (cap < 0) return 0;
    if (cap >= CAP_TABLE_SIZE) return 0;
    return g_caps[cap].in_use == 1;
}

static cap_id_t mint_cap(int fd, cap_rights_t rights)
{
    if (fd < 0) return CAP_NONE;
    for (int i = 1; i < CAP_TABLE_SIZE; i++) {
        if (!g_caps[i].in_use) {
            g_caps[i].fd = fd;
            g_caps[i].rights = rights;
            g_caps[i].in_use = 1;
            return (cap_id_t)i;
        }
    }
    return CAP_NONE;
}

static void cap_revoke(cap_id_t cap)
{
    if (cap <= 0 || cap >= CAP_TABLE_SIZE) return;
    g_caps[cap].in_use = 0;
    g_caps[cap].fd = -1;
    g_caps[cap].rights = 0;
}

static int cap_write_all(cap_id_t cap, const void *buf, size_t len)
{
    if (!cap_is_valid(cap)) return -1;
    if ((g_caps[cap].rights & CAP_R_WRITE) == 0) return -2;

    int fd = g_caps[cap].fd;

    const char *p = (const char *)buf;
    size_t left = len;
    while (left > 0) {
        ssize_t n = write(fd, p, left);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -3;
        }
        p += (size_t)n;
        left -= (size_t)n;
    }
    return 0;
}

/* =========================
 *  SSR core: Work / Signal
 * ========================= */
typedef enum {
    WORK_OK = 0,
    WORK_ERROR = 1,
    WORK_STOP = 2,   /* explicit stop (OP_DROP etc.) */
} WorkResult;

typedef enum {
    TARGET_A = 0,
    TARGET_B = 1,
    TARGET_BOTH = 2, /* must not enter runtime directly */
} Target;

typedef enum {
    MSGK_APP = 0,
    MSGK_STDIN_LINE = 1,
} MessageKind;

/* =========================
 *  OP instruction set v1
 * ========================= */
typedef enum {
    OP_NOOP = 0,
    OP_VALIDATE = 1,
    OP_PRINT_LINE = 2,
    OP_WRITE_ALL = 3,
    OP_DROP = 4,
    /* OP_FANOUT reserved (keep for future) */
} OpKind;

static const char* op_name(OpKind op)
{
    switch (op) {
        case OP_NOOP:       return "NOOP";
        case OP_VALIDATE:   return "VALIDATE";
        case OP_PRINT_LINE: return "PRINT_LINE";
        case OP_WRITE_ALL:  return "WRITE_ALL";
        case OP_DROP:       return "DROP";
        default:            return "UNKNOWN";
    }
}

/* =========================
 *  Steps and Message
 * ========================= */
#define MAX_STEPS 8

typedef struct {
    Target  target;
    cap_id_t cap;
    OpKind  op;
} Step;

typedef struct {
    Step steps[MAX_STEPS];
    int count;
} TargetSteps;

typedef struct {
    msg_id_t     id;
    msg_id_t     parent_msg_id;
    trace_id_t   trace_id;
    MessageKind  kind;

    const TargetSteps *steps;
    int step_idx;

    Target to;          /* current step target (redundant but handy) */
    char  *payload;     /* owned by trace lifecycle (see below) */
} Message;

/* WorkSignal carries enough to build next step message */
typedef struct {
    msg_id_t     msg_id;
    trace_id_t   trace_id;
    msg_id_t     parent_msg_id;
    const TargetSteps *steps;
    int step_idx;
    MessageKind  kind;
    char *payload;
    WorkResult result;
} WorkSignal;

/* =========================
 *  Trace edges (observability)
 * ========================= */
#define MAX_EDGES 1024
typedef struct {
    trace_id_t trace_id;
    msg_id_t   msg_id;
    msg_id_t   parent_msg_id;
    const char *from;
    const char *to;
    const char *note;
} MsgEdge;

static MsgEdge g_edges[MAX_EDGES];
static int g_edge_count = 0;
static trace_id_t g_trace_max = 0;

static void record_edge(trace_id_t trace_id,
                        msg_id_t msg_id,
                        msg_id_t parent_msg_id,
                        const char *from,
                        const char *to,
                        const char *note)
{
    if (g_edge_count >= MAX_EDGES) return;
    g_edges[g_edge_count++] = (MsgEdge){
        .trace_id = trace_id,
        .msg_id = msg_id,
        .parent_msg_id = parent_msg_id,
        .from = from,
        .to = to,
        .note = note
    };
}

/* =========================
 *  SIGQ (signal queue)
 * ========================= */
#define SIGQ_CAP 64
static WorkSignal g_sigq[SIGQ_CAP];
static int g_sig_h = 0, g_sig_t = 0;

static int sigq_empty(void) { return g_sig_h == g_sig_t; }
static int sigq_full(void)  { return ((g_sig_t + 1) % SIGQ_CAP) == g_sig_h; }

static int sigq_push(const WorkSignal *s)
{
    if (sigq_full()) return -1;
    g_sigq[g_sig_t] = *s;
    g_sig_t = (g_sig_t + 1) % SIGQ_CAP;
    return 0;
}

static int sigq_pop(WorkSignal *out)
{
    if (sigq_empty()) return -1;
    *out = g_sigq[g_sig_h];
    g_sig_h = (g_sig_h + 1) % SIGQ_CAP;
    return 0;
}

static void signal_emit(const Message *m, WorkResult r)
{
    WorkSignal s = {
        .msg_id       = m->id,
        .trace_id     = m->trace_id,
        .parent_msg_id= m->parent_msg_id,
        .steps        = m->steps,
        .step_idx     = m->step_idx,
        .kind         = m->kind,
        .payload      = m->payload,
        .result       = r,
    };

    if (sigq_push(&s) != 0) {
        fprintf(stderr, "[SIGQ_FULL] drop signal for msg=%"PRIu64"\n", m->id);
    }
}

/* =========================
 *  Trace / Message minting
 * ========================= */
enum { TRACE_NONE = 0 };

static trace_id_t mint_trace(void)
{
    static trace_id_t mint_trace_id = 0;
    trace_id_t t = ++mint_trace_id;
    g_trace_max = t;
    return t;
}

static void mint_message(Message *m)
{
    static msg_id_t mint_msg_id = 0;
    if (m->trace_id == TRACE_NONE) {
        m->trace_id = mint_trace();
    }
    m->id = ++mint_msg_id;
}

/* =========================
 *  Inbox
 * ========================= */
#define INBOX_CAP 16
typedef struct {
    Message msgs[INBOX_CAP];
    int head;
    int tail;
} Inbox;

static void inbox_init(Inbox *q) { q->head = q->tail = 0; }
static int inbox_empty(Inbox *q) { return q->head == q->tail; }
static int inbox_full(Inbox *q)  { return ((q->tail + 1) % INBOX_CAP) == q->head; }

static int inbox_push(Inbox *q, const Message *m)
{
    if (inbox_full(q)) return -1;
    q->msgs[q->tail] = *m;
    q->tail = (q->tail + 1) % INBOX_CAP;
    return 0;
}

static int inbox_pop(Inbox *q, Message *out)
{
    if (inbox_empty(q)) return -1;
    *out = q->msgs[q->head];
    q->head = (q->head + 1) % INBOX_CAP;
    return 0;
}

/* =========================
 *  Doer (homogeneous)
 * ========================= */
typedef struct Doer {
    const char *name; /* for observability only */
    Inbox inbox;
} Doer;

static Doer g_doer_a;
static Doer g_doer_b;

/* =========================
 *  Doer registry / scheduler
 * ========================= */
#define MAX_DOERS 8
typedef struct {
    Doer *list[MAX_DOERS];
    int count;
} DoerRegistry;

static void registry_init(DoerRegistry *r) { r->count = 0; }
static int registry_add(DoerRegistry *r, Doer *d)
{
    if (r->count >= MAX_DOERS) return -1;
    r->list[r->count++] = d;
    return 0;
}

typedef struct {
    DoerRegistry *reg;
} Scheduler;

static int scheduler_has_work(const Scheduler *s)
{
    for (int i = 0; i < s->reg->count; i++) {
        Doer *d = s->reg->list[i];
        if (!inbox_empty(&d->inbox)) return 1;
    }
    return 0;
}

/* =========================
 *  Runtime routing (no permission checks)
 * ========================= */
static void runtime_record_drop(const Message *m, Doer *d)
{
    record_edge(m->trace_id, m->id, m->parent_msg_id, d->name, "dropped", "inbox_full");
    g_msg_dropped++;
    fprintf(stderr,
        "[DROP] msg=%"PRIu64" (p %"PRIu64") to=%s payload=\"%s\"\n",
        m->id, m->parent_msg_id, d->name,
        m->payload ? m->payload : "");
}

static void runtime_emit(const Message *src, Doer *d)
{
    Message m = *src;
    g_msg_created++;
    record_edge(m.trace_id, m.id, m.parent_msg_id, "runtime", d->name, "emit");

    if (inbox_push(&d->inbox, &m) != 0) {
        runtime_record_drop(&m, d);
    }
}

static void runtime_route(const Message *msg)
{
    switch (msg->to) {
        case TARGET_A: runtime_emit(msg, &g_doer_a); break;
        case TARGET_B: runtime_emit(msg, &g_doer_b); break;
        case TARGET_BOTH:
            fprintf(stderr, "[BUG] TARGET_BOTH must not enter runtime; producer must fan-out\n");
            break;
    }
}

/* =========================
 *  Steps validation / assembly
 * ========================= */
static int validate_steps(const TargetSteps *s, const DoerRegistry *reg)
{
    (void)reg;
    if (!s) return -1;
    if (s->count <= 0 || s->count > MAX_STEPS) return -1;

    for (int i = 0; i < s->count; i++) {
        Target t = s->steps[i].target;
        if (t != TARGET_A && t != TARGET_B) return -1;

        if (s->steps[i].cap < 0) return -1;

        /* op must be known */
        OpKind op = s->steps[i].op;
        if (op != OP_NOOP &&
            op != OP_VALIDATE &&
            op != OP_PRINT_LINE &&
            op != OP_WRITE_ALL &&
            op != OP_DROP) {
            return -1;
        }
    }
    return 0;
}

static const TargetSteps* assemble_steps(const TargetSteps *s, const DoerRegistry *reg)
{
    if (validate_steps(s, reg) != 0) {
        fprintf(stderr, "[STEPS_INVALID]\n");
        return NULL;
    }
    return s;
}

/* =========================
 *  Trace payload lifecycle (minimal & robust)
 *  - for stdin lines we strdup once per trace
 *  - freed when trace finishes (no more steps)
 * ========================= */
typedef struct {
    trace_id_t trace_id;
    char *payload;
    int in_use;
} TracePayload;

#define TRACE_PAYLOAD_CAP 64
static TracePayload g_trace_payloads[TRACE_PAYLOAD_CAP];

static void trace_payload_init(void)
{
    for (int i = 0; i < TRACE_PAYLOAD_CAP; i++) {
        g_trace_payloads[i].trace_id = 0;
        g_trace_payloads[i].payload = NULL;
        g_trace_payloads[i].in_use = 0;
    }
}

static void trace_payload_bind(trace_id_t tid, char *p)
{
    if (!p) return;
    for (int i = 0; i < TRACE_PAYLOAD_CAP; i++) {
        if (!g_trace_payloads[i].in_use) {
            g_trace_payloads[i].in_use = 1;
            g_trace_payloads[i].trace_id = tid;
            g_trace_payloads[i].payload = p;
            return;
        }
    }
    /* fallback: leak-safe-ish (rare) */
}

static void trace_payload_release(trace_id_t tid)
{
    for (int i = 0; i < TRACE_PAYLOAD_CAP; i++) {
        if (g_trace_payloads[i].in_use && g_trace_payloads[i].trace_id == tid) {
            free(g_trace_payloads[i].payload);
            g_trace_payloads[i].payload = NULL;
            g_trace_payloads[i].trace_id = 0;
            g_trace_payloads[i].in_use = 0;
            return;
        }
    }
}

/* =========================
 *  OP execution (the only place with "how")
 * ========================= */
static WorkResult op_exec(const Doer *self, const Message *msg)
{
    if (!msg || !msg->steps) return WORK_ERROR;
    if (msg->step_idx < 0 || msg->step_idx >= msg->steps->count) return WORK_ERROR;

    const Step *st = &msg->steps->steps[msg->step_idx];

    /* Optional: strong invariant check */
    if (msg->to != st->target) {
        /* structure mismatch */
        return WORK_ERROR;
    }

    switch (st->op) {

        case OP_NOOP:
            return WORK_OK;

        case OP_DROP:
            return WORK_STOP;

        case OP_VALIDATE: {
            /* minimal structure checks only (expand later) */
            if (!msg->payload) return WORK_ERROR;
            if (st->cap < 0) return WORK_ERROR;
            return WORK_OK;
        }

        case OP_WRITE_ALL: {
            if (!msg->payload) return WORK_ERROR;
            size_t len = strlen(msg->payload);
            if (len == 0) return WORK_OK;
            return (cap_write_all(st->cap, msg->payload, len) == 0) ? WORK_OK : WORK_ERROR;
        }

        case OP_PRINT_LINE: {
            char line[2048];
            int n = snprintf(line, sizeof(line),
                "[%s] op=%s msg=%"PRIu64" (p %"PRIu64") cap=%d payload=\"%s\"\n",
                self->name,
                op_name(st->op),
                msg->id,
                msg->parent_msg_id,
                st->cap,
                msg->payload ? msg->payload : "");

            if (n <= 0 || (size_t)n >= sizeof(line)) return WORK_ERROR;
            return (cap_write_all(st->cap, line, (size_t)n) == 0) ? WORK_OK : WORK_ERROR;
        }

        default:
            return WORK_ERROR;
    }
}

static void doer_handle(Doer *self, const Message *msg)
{
    WorkResult r = op_exec(self, msg);
    signal_emit(msg, r);
}

/* =========================
 *  Scheduler: one message per doer per round
 * ========================= */
static void scheduler_round(Scheduler *s)
{
    for (int i = 0; i < s->reg->count; i++) {
        Doer *d = s->reg->list[i];
        Message m;

        if (inbox_pop(&d->inbox, &m) == 0) {
            record_edge(m.trace_id, m.id, m.parent_msg_id, d->name, "handled", "doer_handle");
            doer_handle(d, &m);
            g_msg_handled++;
        }
    }
}

/* =========================
 *  Reactor: signals drive next step
 * ========================= */
static void reactor_run(void)
{
    WorkSignal s;
    while (sigq_pop(&s) == 0) {

        if (s.result != WORK_OK) {
            /* STOP/ERROR ends the chain; release trace payload if this was the end */
            /* NOTE: If you later add retries or branches, handle here. */
            trace_payload_release(s.trace_id);
            continue;
        }

        int next = s.step_idx + 1;
        if (!s.steps || next >= s.steps->count) {
            /* chain complete */
            trace_payload_release(s.trace_id);
            continue;
        }

        Message next_msg = {
            .parent_msg_id = s.msg_id,
            .trace_id      = s.trace_id,
            .kind          = s.kind,
            .payload       = s.payload,

            .steps    = s.steps,
            .step_idx = next,
            .to       = s.steps->steps[next].target,
        };

        mint_message(&next_msg);
        runtime_route(&next_msg);
    }
}

/* =========================
 *  Stdin event -> Message (WORLD -> runtime boundary)
 * ========================= */
static void emit_stdin_line_message(const char *line, const TargetSteps *steps)
{
    if (!steps) return;

    /* trim leading spaces */
    const char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0') return;

    /* payload ownership: strdup once, bind to trace */
    char *dup = strdup(p);
    if (!dup) return;

    Message msg = {
        .parent_msg_id = 0,
        .trace_id      = TRACE_NONE,
        .kind          = MSGK_STDIN_LINE,
        .payload       = dup,

        .steps    = steps,
        .step_idx = 0,
        .to       = steps->steps[0].target,
    };

    mint_message(&msg);
    trace_payload_bind(msg.trace_id, dup);

    record_edge(msg.trace_id, msg.id, msg.parent_msg_id, "WORLD", "stdin", "emit_line");
    runtime_route(&msg);
}

static void emit_stdin_event(const TargetSteps *stdin_steps)
{
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;

    sqe = io_uring_get_sqe(&g_ring);
    io_uring_prep_read(sqe, STDIN_FILENO, g_stdin_buf, sizeof(g_stdin_buf) - 1, FILE_OFFSET_START);

    int sret = io_uring_submit(&g_ring);
    if (sret < 0) {
        fprintf(stderr, "submit: %s\n", strerror(-sret));
        g_running = 0;
        return;
    }

    int ret = io_uring_wait_cqe(&g_ring, &cqe);
    if (ret == -EINTR) return;
    if (ret < 0) {
        fprintf(stderr, "io_uring_wait_cqe failed: %s\n", strerror(-ret));
        g_running = 0;
        return;
    }

    int res = cqe->res;
    io_uring_cqe_seen(&g_ring, cqe);

    if (res == 0) {
        printf("EOF, exit.\n");
        g_running = 0;
        return;
    }
    if (res < 0) {
        fprintf(stderr, "read failed: %s\n", strerror(-res));
        g_running = 0;
        return;
    }

    g_stdin_buf[res] = '\0';
    if (res > 0 && g_stdin_buf[res - 1] == '\n') {
        g_stdin_buf[res - 1] = '\0';
    }

    emit_stdin_line_message(g_stdin_buf, stdin_steps);
}

/* =========================
 *  Diagnostics
 * ========================= */
static unsigned long runtime_pending_messages(const DoerRegistry *reg)
{
    unsigned long n = 0;
    for (int i = 0; i < reg->count; i++) {
        Inbox *q = &reg->list[i]->inbox;
        if (q->tail >= q->head) n += (unsigned long)(q->tail - q->head);
        else n += (unsigned long)(INBOX_CAP - q->head + q->tail);
    }
    return n;
}

static void runtime_print_message_balance(const DoerRegistry *reg)
{
    unsigned long pending = runtime_pending_messages(reg);
    long balance = (long)g_msg_created
                 - (long)g_msg_handled
                 - (long)g_msg_dropped
                 - (long)pending;

    printf("[MSG_BALANCE] created=%lu handled=%lu dropped=%lu pending=%lu balance=%ld\n",
           g_msg_created, g_msg_handled, g_msg_dropped, pending, balance);
}

static void dump_trace(trace_id_t trace_id)
{
    for (int i = 0; i < g_edge_count; i++) {
        if (g_edges[i].trace_id == trace_id) {
            printf("msg %"PRIu64" (p %"PRIu64") %s -> %s  [%s]\n",
                   g_edges[i].msg_id,
                   g_edges[i].parent_msg_id,
                   g_edges[i].from,
                   g_edges[i].to,
                   g_edges[i].note ? g_edges[i].note : "");
        }
    }
}

/* =========================
 *  SIGINT
 * ========================= */
static void on_sigint(int signo)
{
    (void)signo;
    g_running = 0;
}

/* =========================
 *  Runtime init
 * ========================= */
static void runtime_init(void)
{
    g_doer_a.name = "A";
    inbox_init(&g_doer_a.inbox);

    g_doer_b.name = "B";
    inbox_init(&g_doer_b.inbox);
}

/* =========================
 *  Example steps
 *  stdin -> A then B
 * ========================= */
static TargetSteps make_stdin_steps(cap_id_t cap_out)
{
    TargetSteps s = {0};
    s.steps[0] = (Step){ .target = TARGET_A, .cap = cap_out, .op = OP_PRINT_LINE };
    s.steps[1] = (Step){ .target = TARGET_B, .cap = cap_out, .op = OP_PRINT_LINE };
    s.count = 2;
    return s;
}

/* =========================
 *  main
 * ========================= */
int main(void)
{
    printf("=== SCAT16: SSR + OP ===\n");
    signal(SIGINT, on_sigint);

    /* io_uring */
    int ret = io_uring_queue_init(8, &g_ring, NO_FLAGS);
    if (ret < 0) {
        fprintf(stderr, "io_uring_queue_init failed: %s\n", strerror(-ret));
        return 1;
    }

    /* cap */
    cap_init();
    trace_payload_init();

    cap_id_t cap_out = mint_cap(STDOUT_FILENO, CAP_R_WRITE);
    if (cap_out == CAP_NONE) {
        fprintf(stderr, "failed to mint stdout cap\n");
        return 1;
    }

    /* runtime */
    runtime_init();

    DoerRegistry reg;
    registry_init(&reg);
    registry_add(&reg, &g_doer_a);
    registry_add(&reg, &g_doer_b);

    Scheduler sched = { .reg = &reg };

    /* steps */
    TargetSteps stdin_steps_storage = make_stdin_steps(cap_out);
    const TargetSteps *stdin_steps = assemble_steps(&stdin_steps_storage, &reg);
    if (!stdin_steps) {
        fprintf(stderr, "stdin steps invalid\n");
        return 1;
    }

    /* demo: app message (not from stdin) */
    {
        trace_id_t tid = mint_trace();
        Message m = {
            .parent_msg_id = 0,
            .trace_id = tid,
            .kind = MSGK_APP,
            .payload = "hi (app payload - not owned)",
            .steps = stdin_steps,
            .step_idx = 0,
            .to = stdin_steps->steps[0].target,
        };
        mint_message(&m);
        record_edge(m.trace_id, m.id, m.parent_msg_id, "APP", "runtime", "seed");
        runtime_route(&m);
    }

    while (g_running) {
        emit_stdin_event(stdin_steps);

        while (scheduler_has_work(&sched) || !sigq_empty()) {
            while (scheduler_has_work(&sched)) {
                scheduler_round(&sched);
            }
            reactor_run();
        }

        runtime_print_message_balance(&reg);
    }

    io_uring_queue_exit(&g_ring);
    printf("io_uring cleaned.\n");

    for (trace_id_t t = 1; t <= g_trace_max; t++) {
        printf("\n");
        dump_trace(t);
    }

    /* cleanup: revoke caps if you want */
    cap_revoke(cap_out);
    return 0;
}
