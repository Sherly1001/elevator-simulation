#include <stdio.h>
#include <unistd.h>

#include "sigs.h"
#include "shared.h"

#include "vec.h"

static shared_mem *shm = NULL;
// vec<int>
static vec_t *queue    = NULL;
static int    waiting  = 0;
static int    sleeping = 0;

static void print_queue(char *color) {
    printf("%sRequest queue: ", color);
    if (queue->len > 0) {
        for (size_t idx = 0; idx < queue->len - 1; ++idx) {
            printf("%d, ", vec_get_r(int, queue, idx));
        }
        printf("%d\e[0m\n", vec_get_r(int, queue, queue->len - 1));
    } else {
        printf("Empty\e[0m\n");
    }
}

static void tox(int sig) {
    int floor = sig - SIG_OPE_REQ_TO_2 + 2;

    kill(shm->opx_id[floor - 1], sig);
    kill(shm->opx_id[0], sig);

    size_t idx = vec_index_of_r(int, queue, floor);
    if (idx == -1lu) {
        vec_add_r(int, queue, floor);
        printf("\e[32;1mAdd new request: Floor %d\e[0m\n", floor);
    } else {
        printf("\e[31;1mFloor %d already in the queue\e[0m\n", floor);
    }

    print_queue("\e[32;1m");
}

static void arrive_handle() {
    waiting = 0;
}

static void wait_move_done() {
    waiting = 1;
    while (waiting) pause();
}

static void update_light() {
    for (int i = 0; i < 5; ++i) {
        if (kill(shm->opx_id[i], SIG_OPX_UPDATE_LIGHT) < 0) {
            char tmp[20];
            sprintf(tmp, "mng_update_light %d", i + 1);
            perror(tmp);
        }
    }
}

static int move_lift_to(int floor) {
    if (floor < 1 || floor > 5) return -1;
    int sig = SIG_MNG_MOVE_LIFT_TO_1 + floor - 1;
    return kill(shm->ctrl_id, sig);
}

static int move_done_in_floor(int floor) {
    int sig = SIG_OPE_REQ_TO_2_DONE + floor - 2;
    int res = kill(shm->opx_id[0], sig);
    if (res < 0) return res;
    return kill(shm->opx_id[floor - 1], sig);
}

static int move_ready_in_floor(int floor) {
    int sig = SIG_OPE_REQ_TO_2_READY + floor - 2;
    int res = kill(shm->opx_id[0], sig);
    if (res < 0) return res;
    return kill(shm->opx_id[floor - 1], sig);
}

static void wake_handle() {
    sleeping = 0;
}

static void sleep_in(int sec) {
    sleeping = 1;
    alarm(sec);
    while (sleeping) pause();
}

int mng_main() {
    shm   = get_shared_mem();
    queue = vec_new_r(int, NULL, NULL, NULL);

    signal(SIGALRM, wake_handle);
    signal(SIG_OPE_REQ_TO_2, tox);
    signal(SIG_OPE_REQ_TO_3, tox);
    signal(SIG_OPE_REQ_TO_4, tox);
    signal(SIG_OPE_REQ_TO_5, tox);
    signal(SIG_CTRL_MOVE_LIFT_ARRIVED, arrive_handle);
    signal(SIG_OPX_UPDATE_LIGHT, update_light);

    while (1) {
        if (queue->len > 0) {
            int next_floor = vec_get_r(int, queue, 0);
            printf("\e[33;1mOpen the door\e[0m\n");
            sleep_in(3);
            printf("\e[33;1mClose the door\e[0m\n");

            if (move_lift_to(next_floor) < 0) {
                char tmp[20];
                sprintf(tmp, "move_lift_to %d", next_floor);
                perror(tmp);
                continue;
            }

            wait_move_done();
            printf("\e[33;1mOpen the door\e[0m\n");
            sleep_in(3);
            printf("\e[33;1mClose the door\e[0m\n");

            if (queue->len > 1) {
                shm->cur_req = vec_get_r(int, queue, 1);
                move_ready_in_floor(vec_get_r(int, queue, 1));
            }
            else
                shm->cur_req = 0;

            move_done_in_floor(next_floor);
            vec_remove(queue, 0);
            print_queue("\e[31;1m");

            if (move_lift_to(1) < 0) {
                perror("move_lift_to");
                continue;
            }

            wait_move_done();
        }
        pause();
    }

    return 0;
}
