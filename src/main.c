#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "shared.h"

int main() {
    int pid = 0;
    shared_mem *shm = get_shared_mem();

    int (*fp[])(int, char **) = {mng_main, ctrl_main, body_main};

    for (unsigned long i = 0; i < sizeof(fp) / sizeof(fp[0]); ++i) {
        if ((pid = fork()) == 0) {
            int resp = fp[i](0, NULL);
            exit(resp);
        } else if (pid > 0) {
            if (i == 0) {
                shm->mng_id = pid;
            } else if (i == 1) {
                shm->ctrl_id = pid;
            } else if (i == 2) {
                shm->lift_id = pid;
            }
        } else {
            perror("fork in main");
        }
    }

    if ((pid = fork()) == 0) {
        int resp = op1_main(0, NULL);
        exit(resp);
    } else if (pid > 0) {
        shm->opx_id[0] = pid;
    } else {
        perror("fork op1");
    }

    for (int i = 2; i <= 5; ++i) {
        if ((pid = fork()) == 0) {
            char id[10];
            sprintf(id, "%d", i);
            char *av   = {id};
            int   resp = opx_main(1, &av);
            exit(resp);
        } else if (pid) {
            shm->opx_id[i - 1] = pid;
            printf("main: %d: %d\n", i - 1, pid);
        } else {
            perror("fork opx");
        }
    }

    int res = 0;
    while (wait(&res) > 0);
    free_shared_mem();
}
