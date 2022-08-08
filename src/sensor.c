#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sigs.h"
#include "shared.h"

static shared_mem *shm   = NULL;
static int         id    = 0;
static int         is_on = 0;

static int sen_is_on(double lift_pos, int sen_id) {
    double min = 3 * (sen_id - 1) + 0.5;
    double max = min + 1;
    return lift_pos >= min && lift_pos <= max;
}

static void sensor_loop() {
    if (id == 0) return;
    if (sen_is_on(shm->lift_pos, id)) {
        if (kill(shm->ctrl_id, SIG_SEN_ON) < 0) {
            perror("sen_on");
        }
        // printf("sen %d: on\n", id);
        is_on = 1;
    } else {
        if (is_on && kill(shm->ctrl_id, SIG_SEN_OFF) < 0) {
            perror("sen_off");
        }
        // if (is_on) printf("sen %d: off\n", id);
        is_on = 0;
    }
}

int sensor_main(int ag, char **av) {
    if (ag < 1) {
        fprintf(stderr, "missing sensor id\n");
        return 1;
    }

    shm = get_shared_mem();

    id = atoi(av[0]);
    signal(SIGALRM, sensor_loop);
    ualarm(250000, 250000);

    while (1) pause();
    return 0;
}
