// fake_ebpf.c
// gcc -O2 -std=c11 fake_ebpf.c -o fake_ebpf && ./fake_ebpf 40
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#define MAX_HOOKS 8
#define ABI_VER 1

typedef enum { EV_PKT=1, EV_SYSCALL=2 } ev_type_t;

typedef struct {
    int abi_ver;
    ev_type_t type;
    uint32_t id;
    uint32_t aux;
} event_t;

// ---- map（共享状态）----
typedef struct {
    uint64_t pkt_total;
    uint64_t pkt_dropped;
    uint64_t sys_total;
} map_t;

enum { ACT_OK=0, ACT_DROP=1<<0, ACT_LOG=1<<1 };
typedef int (*hook_fn)(const event_t*, map_t*);

// ---- helpers ----
static inline void map_inc_u64(uint64_t *p){ assert(p); (*p)++; }
static inline void helper_log(const char *msg, const event_t *ev){
    printf("[LOG] %s (type=%d id=%u aux=%u)\n", msg, ev->type, ev->id, ev->aux);
}

// ---- verifier ----
static void verify_hook(const event_t *probe_ev, map_t *m, hook_fn fn){
    assert(fn && probe_ev && m);
    assert(probe_ev->abi_ver == ABI_VER);
    assert(probe_ev->type==EV_PKT || probe_ev->type==EV_SYSCALL);
    (void)fn(probe_ev, m);
}

// ---- hooks ----
int hook_noop(const event_t *ev, map_t *m){ (void)ev; (void)m; return ACT_OK; }

int hook_count(const event_t *ev, map_t *m){
    assert(ev && m);
    if(ev->type==EV_PKT) map_inc_u64(&m->pkt_total);
    else                 map_inc_u64(&m->sys_total);
    return ACT_OK;
}

int hook_drop_even_packets(const event_t *ev, map_t *m){
    (void)m;
    if(ev->type==EV_PKT && (ev->id % 2 == 0)) return ACT_DROP;
    return ACT_OK;
}

int hook_rate_limit_port_80(const event_t *ev, map_t *m){
    (void)m;
    if(ev->type==EV_PKT && ev->aux==80 && (ev->id % 5 != 0)) return ACT_DROP;
    return ACT_OK;
}

int hook_logger(const event_t *ev, map_t *m){
    (void)m;
    if(ev->type==EV_SYSCALL && (ev->aux==60 || ev->aux==39))
        return ACT_LOG;
    return ACT_OK;
}

// ---- 微内核调度 ----
typedef struct {
    hook_fn f[MAX_HOOKS];
    int n;
} hookchain_t;

static int run_hooks(hookchain_t *hc, const event_t *ev, map_t *m){
    int act = ACT_OK;
    for(int i=0;i<hc->n;i++){
        if(!hc->f[i]) continue;          // 防空指针
        int a = hc->f[i](ev,m);
        act |= a;
    }
    return act;
}

static void hot_swap(hookchain_t *hc, int idx, hook_fn fn, map_t *m){
    event_t probe = {.abi_ver=ABI_VER, .type=EV_PKT, .id=0, .aux=0};
    verify_hook(&probe, m, fn);
    assert(idx>=0 && idx<MAX_HOOKS);
    hc->f[idx] = fn;
    if(idx >= hc->n) hc->n = idx+1;
}

// ---- 事件源 ----
static event_t next_event(uint32_t i){
    event_t ev = {.abi_ver=ABI_VER};
    if(rand()%3){
        ev.type = EV_PKT;
        ev.id   = i;
        ev.aux  = (rand()%5==0)?80:443;
    }else{
        ev.type = EV_SYSCALL;
        ev.id   = i;
        ev.aux  = (rand()%2)?60:39;
    }
    return ev;
}

// ---- main ----
int main(int argc, char **argv){
    int N = (argc>1)?atoi(argv[1]):30;
    srand((unsigned)time(NULL));

    map_t m = {0};
    hookchain_t hc = { .n=0 };

    // 初始化：填充 no-op
    for(int i=0;i<MAX_HOOKS;i++) hc.f[i] = hook_noop;

    // 安装初始 hook
    hot_swap(&hc, 0, hook_count, &m);
    hot_swap(&hc, 1, hook_drop_even_packets, &m);
    hot_swap(&hc, 3, hook_logger, &m);

    for(int i=1;i<=N;i++){
        event_t ev = next_event((uint32_t)i);

        if(i == N/2){
            printf("=== HOT-SWAP @%d: drop_even_packets -> rate_limit_port_80 ===\n", i);
            hot_swap(&hc, 1, hook_rate_limit_port_80, &m);
        }

        int act = run_hooks(&hc, &ev, &m);

        if((act & ACT_LOG)) helper_log("interesting syscall", &ev);
        if(ev.type==EV_PKT && (act & ACT_DROP)){
            map_inc_u64(&m.pkt_dropped);
            printf("DROP pkt id=%u port=%u\n", ev.id, ev.aux);
        }else{
            if(ev.type==EV_PKT)
                printf("PASS pkt id=%u port=%u\n", ev.id, ev.aux);
            else
                printf("PASS syscall id=%u no=%u\n", ev.id, ev.aux);
        }
    }

    printf("\n=== STATS ===\n");
    printf("pkt_total=%llu, pkt_dropped=%llu, sys_total=%llu\n",
           (unsigned long long)m.pkt_total,
           (unsigned long long)m.pkt_dropped,
           (unsigned long long)m.sys_total);
    return 0;
}
