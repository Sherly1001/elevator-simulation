#include <gtk/gtk.h>

#include "shared.h"
#include "sigs.h"

static shared_mem *shm  = NULL;
static GtkWidget  *_lab = NULL;
static GtkWidget  *_buts[5];
static int         _self_clicked = 0;

static void tox(int sig) {
    int floor = sig - SIG_OPE_REQ_TO_2 + 2;
    if (!_self_clicked) set_but_img(_buts[floor - 1], floor, 0);
    _self_clicked = 0;
}

static void move_done_handle(int sig) {
    int floor = sig - SIG_OPE_REQ_TO_2_DONE + 2;
    set_but_img(_buts[floor - 1], floor, 1);
}

static void update_light() {
    int cur_floor = lift_at_floor(shm->lift_pos);
    set_label_text(_lab, cur_floor, cur_floor == 1 ? "green" : "red");
}

static void but_clicked(GtkWidget *but, gpointer data) {
    opx_args_t *args = data;
    if (opx_send_sig(args->shm, args->id) == 0) {
        _self_clicked = 1;
        set_but_img(but, args->id, 0);
    } else {
        perror("opx_send_sig");
    }
}

static void activate(GtkApplication *app, gpointer data) {
    GtkWidget *win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), "floor 1");
    gtk_window_set_icon_from_file(GTK_WINDOW(win), "icons/elevator.png", NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(win), grid);

    PangoAttrList *attr = pango_attr_list_new();
    pango_attr_list_insert(attr, pango_attr_size_new(36 * PANGO_SCALE));

    GtkWidget *label = gtk_label_new("");
    gtk_label_set_attributes(GTK_LABEL(label), attr);
    set_label_text(label, 1, "green");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    _lab = label;

    for (int i = 2; i <= 5; ++i) {
        GtkWidget *but = gtk_button_new();
        set_but_img(but, i, 1);
        gtk_grid_attach(GTK_GRID(grid), but, 0, i, 1, 1);

        opx_args_t *args = malloc(sizeof(opx_args_t));

        args->shm = data;
        args->id  = i;
        g_signal_connect(but, "clicked", G_CALLBACK(but_clicked), args);
        _buts[i - 1] = but;
    }

    gtk_window_set_wmclass(GTK_WINDOW(win), "esopx", "esopx");
    gtk_widget_show_all(win);
}

int op1_main(int ac, char **av) {
    signal(SIG_OPE_REQ_TO_2, tox);
    signal(SIG_OPE_REQ_TO_3, tox);
    signal(SIG_OPE_REQ_TO_4, tox);
    signal(SIG_OPE_REQ_TO_5, tox);
    signal(SIG_OPE_REQ_TO_2_DONE, move_done_handle);
    signal(SIG_OPE_REQ_TO_3_DONE, move_done_handle);
    signal(SIG_OPE_REQ_TO_4_DONE, move_done_handle);
    signal(SIG_OPE_REQ_TO_5_DONE, move_done_handle);
    signal(SIG_OPX_UPDATE_LIGHT, update_light);

    shm = get_shared_mem();

    GtkApplication *app = gtk_application_new(
        "org.gtk.elevator-simulation", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), shm);
    int res = g_application_run(G_APPLICATION(app), ac, av);
    g_object_unref(app);
    return res;
}
