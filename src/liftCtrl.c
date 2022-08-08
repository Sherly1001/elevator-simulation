#include <stdio.h>
#include <unistd.h>

#include "sigs.h"
#include "shared.h"

static shared_mem *shm = NULL;

static int dest_floor = 0;

static int sig_to_floor(int sig) {
    return sig - SIG_MNG_MOVE_LIFT_TO_1 + 1;
}

static void sig_handle(int sig) {
    int next_floor = sig_to_floor(sig);
    int cur_floor  = lift_at_floor(shm->lift_pos);
    int arrived    = lift_arrived(shm->lift_pos);
    dest_floor     = next_floor;

    // printf("ctrl: %d, %d, %d\n", cur_floor, next_floor, arrived);

    if (next_floor > cur_floor) {
        if (kill(shm->lift_id, SIG_CTRL_MOVE_LIFT_UP) < 0) {
            perror("ctrl_move_up");
        }
    } else if (next_floor < cur_floor) {
        if (kill(shm->lift_id, SIG_CTRL_MOVE_LIFT_DOWN) < 0) {
            perror("ctrl_move_down");
        }
    } else if (arrived) {
        if (kill(shm->lift_id, SIG_CTRL_MOVE_LIFT_STOP) < 0) {
            perror("ctrl_stop_move");
        }
        if (kill(shm->mng_id, SIG_CTRL_MOVE_LIFT_ARRIVED) < 0) {
            perror("ctrl_move_arrived");
        }
    }
}

static void sen_handle() {
    int cur_floor = lift_at_floor(shm->lift_pos);
    int arrived   = lift_arrived(shm->lift_pos);

    if (dest_floor && cur_floor == dest_floor && arrived) {
        if (kill(shm->lift_id, SIG_CTRL_MOVE_LIFT_STOP) < 0) {
            perror("ctrl_sen_stop_move");
        }
        if (kill(shm->mng_id, SIG_CTRL_MOVE_LIFT_ARRIVED) < 0) {
            perror("ctrl_sen_move_arrived");
        }
    }

    if (kill(shm->mng_id, SIG_OPX_UPDATE_LIGHT) < 0) {
        perror("ctrl_sen_update_light");
    }
}

int ctrl_main() {
    shm = get_shared_mem();

    signal(SIG_MNG_MOVE_LIFT_TO_1, sig_handle);
    signal(SIG_MNG_MOVE_LIFT_TO_2, sig_handle);
    signal(SIG_MNG_MOVE_LIFT_TO_3, sig_handle);
    signal(SIG_MNG_MOVE_LIFT_TO_4, sig_handle);
    signal(SIG_MNG_MOVE_LIFT_TO_5, sig_handle);
    signal(SIG_SEN_ON, sen_handle);
    signal(SIG_SEN_OFF, sen_handle);

    while (1) pause();
    return 0;
}
