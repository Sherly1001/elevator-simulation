#include <stdlib.h>
#include "shared.h"

shared_mem *get_shared_mem() {
    int shmid = shmget(SHM_KEY, sizeof(shared_mem), IPC_CREAT | 0660);
    if (shmid < 0) {
        perror("shmget");
        exit(-1);
    }
    shared_mem *shm = shmat(shmid, NULL, 0);
    if (shm == (shared_mem *)-1) {
        perror("shmat");
        exit(-1);
    }
    return shm;
}

void free_shared_mem() {
    int shmid = shmget(SHM_KEY, sizeof(shared_mem), IPC_CREAT | 0660);
    shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
}

void set_label_text(GtkWidget *label, int id, char *color) {
    char label_t[100];
    sprintf(label_t, "<span foreground=\"%s\">%d</span>", color, id);
    gtk_label_set_markup(GTK_LABEL(label), label_t);
}

GtkWidget *but_imgs[5]      = {NULL};
GtkWidget *but_pend_imgs[5] = {NULL};

void set_but_img(GtkWidget *but, int id, int activate) {
    GtkWidget *but_img = activate ? but_imgs[id - 1] : but_pend_imgs[id - 1];

    if (!but_img) {
        char file_path[50];
        sprintf(
            file_path, "icons/%s-%d.png", activate ? "floor" : "pending", id);
        but_img = gtk_image_new_from_file(file_path);
        (activate ? but_imgs : but_pend_imgs)[id - 1] = but_img;
        g_object_ref(but_img);
    }

    gtk_button_set_image(GTK_BUTTON(but), but_img);
}

int opx_send_sig(shared_mem *shm, int id) {
    int sig = 0;
    switch (id) {
        case 2:
            sig = SIG_OPE_REQ_TO_2;
            break;
        case 3:
            sig = SIG_OPE_REQ_TO_3;
            break;
        case 4:
            sig = SIG_OPE_REQ_TO_4;
            break;
        case 5:
            sig = SIG_OPE_REQ_TO_5;
            break;
        default:
            return -1;
    }

    return kill(shm->mng_id, sig);
}

int lift_at_floor(double lift_pos) {
    return (int)(lift_pos / 3) + 1;
}

int lift_arrived(double lift_pos) {
    double intval = (int)lift_pos;
    if (intval != lift_pos) {
        return 0;
    }
    double floor = (lift_pos - 1) / 3 + 1;
    intval       = (int)floor;
    return intval == floor && intval >= 1 && intval <= 5;
}
