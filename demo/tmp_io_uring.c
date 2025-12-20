#include <unistd.h>
#include <errno.h>
#include <liburing.h>

static struct io_uring g_ring;
static char g_stdin_buf[1024];

#define USE_CURRENT_OFFSET (-1)
#define FILE_OFFSET_START  (0)
#define NO_FLAGS           (0)

static void emit_stdin_event(void)
{
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;

    // submit one read
    sqe = io_uring_get_sqe(&g_ring);
    io_uring_prep_read(sqe, STDIN_FILENO, g_stdin_buf, sizeof(g_stdin_buf) - 1, 0);

    io_uring_submit(&g_ring);

    // wait completion (this is the "event")
    int ret = io_uring_wait_cqe(&g_ring, &cqe);
    if (ret < 0) {
        fprintf(stderr, "io_uring_wait_cqe: %s\n", strerror(-ret));
        return;
    }

    int nread = cqe->res;
    io_uring_cqe_seen(&g_ring, cqe);

    if (nread <= 0) {
        if (nread == 0) {
            printf("[EOF]\n");
        } else {
            fprintf(stderr, "read: %s\n", strerror(-nread));
        }
        return;
    }

    g_stdin_buf[nread] = '\0';

    size_t n = strlen(g_stdin_buf);
    if (n > 0 && g_stdin_buf[n - 1] == '\n') {
        g_stdin_buf[n - 1] = '\0';
    }

    // semantic layer stays the same
    emit_stdin_line_message(g_stdin_buf);
}


if (io_uring_queue_init(8, &g_ring, 0) < 0) {
    perror("io_uring_queue_init");
    return 1;
}

io_uring_queue_exit(&g_ring);







