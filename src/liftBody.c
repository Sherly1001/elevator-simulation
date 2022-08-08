#include <unistd.h>

#include "shared.h"

static shared_mem *shm    = NULL;
static int         moving = 0;

static void sig_handle(int sig) {
    if (sig == SIG_CTRL_MOVE_LIFT_UP) {
        moving = 1;
    } else if (sig == SIG_CTRL_MOVE_LIFT_DOWN) {
        moving = -1;
    } else if (sig == SIG_CTRL_MOVE_LIFT_STOP) {
        moving = 0;
    }
    // printf("body: %d\n", moving);
}

static void moving_loop() {
    shm->lift_pos += moving * 0.5;
    int arrived   = lift_arrived(shm->lift_pos);
    int cur_floor = lift_at_floor(shm->lift_pos);

    if (moving) {
        if (arrived) {
            printf("Current position: %3.1lfm, current floor: %d\n", shm->lift_pos,
                cur_floor);
        } else {
            printf("Current position: %3.1lfm\n", shm->lift_pos);
        }
    }
    alarm(1);
}

int body_main() {
    shm           = get_shared_mem();
    shm->lift_pos = 1;

    signal(SIGALRM, moving_loop);
    signal(SIG_CTRL_MOVE_LIFT_UP, sig_handle);
    signal(SIG_CTRL_MOVE_LIFT_DOWN, sig_handle);
    signal(SIG_CTRL_MOVE_LIFT_STOP, sig_handle);

    moving_loop();

    while (1) pause();
    return 0;
}
