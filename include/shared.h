#ifndef __SHARED_H__
#define __SHARED_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <gtk/gtk.h>

#include "sigs.h"

#define SHM_KEY 1001

typedef struct {
    int cur_req;
    
    // bottom of elevator
    double lift_pos;

    pthread_mutex_t mutex;

    pid_t mng_id;
    pid_t ctrl_id;
    pid_t lift_id;
    pid_t sensor_id[6];
    pid_t opx_id[5];
} shared_mem;

shared_mem *get_shared_mem();
void        free_shared_mem();

typedef struct {
    shared_mem *shm;
    int         id;
} opx_args_t;

typedef int main_func(int, char **);

int op1_main(int ac, char **av);
int opx_main(int ac, char **av);
int mng_main();
int ctrl_main();
int body_main();
int sensor_main(int ac, char **av);

void set_monitor_img(GtkWidget *monitor, int id, char *color);
void set_but_img(GtkWidget *but, int id, int status);
int  opx_send_sig(shared_mem *shm, int id);
int  lift_at_floor(double lift_pos);
int  lift_arrived(double lift_pos);

#endif
