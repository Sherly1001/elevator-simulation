#include <stdio.h>
#include <unistd.h>

#include "sigs.h"
#include "shared.h"

static shared_mem *shm = NULL;

static void to2() {
    kill(shm->opx_id[1], SIG_OPE_REQ_TO_2);
    kill(shm->opx_id[0], SIG_OPE_REQ_TO_2);
}

static void to3() {
    kill(shm->opx_id[2], SIG_OPE_REQ_TO_3);
    kill(shm->opx_id[0], SIG_OPE_REQ_TO_3);
}

static void to4() {
    kill(shm->opx_id[3], SIG_OPE_REQ_TO_4);
    kill(shm->opx_id[0], SIG_OPE_REQ_TO_4);
}

static void to5() {
    kill(shm->opx_id[4], SIG_OPE_REQ_TO_5);
    kill(shm->opx_id[0], SIG_OPE_REQ_TO_5);
}

int mng_main(int ag, char **av) {
    shm = get_shared_mem();

    signal(SIG_OPE_REQ_TO_2, to2);
    signal(SIG_OPE_REQ_TO_3, to3);
    signal(SIG_OPE_REQ_TO_4, to4);
    signal(SIG_OPE_REQ_TO_5, to5);
    while (1) pause();
    return 0;
}
