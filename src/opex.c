#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "sigs.h"
#include "shared.h"

static shared_mem *shm           = NULL;
static GtkWidget  *_but          = NULL;
static GtkWidget  *_lab          = NULL;
static int         _id           = 0;
static int         _self_clicked = 0;

static void tome() {
    if (!_self_clicked) set_but_img(_but, _id, 0);
    _self_clicked = 0;
}

static void but_clicked(GtkWidget *but, gpointer data) {
    opx_args_t *args = data;
    if (opx_send_sig(args->shm, args->id) == 0) {
        _but          = but;
        _self_clicked = 1;
        set_but_img(but, args->id, 0);
    } else {
        perror("opx_send_sig");
    }
}

static void move_done_handle() {
    set_but_img(_but, _id, 1);
}

static void update_light() {
    int cur_floor = lift_at_floor(shm->lift_pos);
    set_label_text(_lab, cur_floor, cur_floor == _id ? "green" : "red");
}

static void activate(GtkApplication *app, gpointer data) {
    opx_args_t *args = data;

    char title[10];
    sprintf(title, "floor %d", args->id);

    GtkWidget *win = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(win), title);
    gtk_window_set_icon_from_file(GTK_WINDOW(win), "icons/elevator.png", NULL);

    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(win), grid);

    PangoAttrList *attr = pango_attr_list_new();
    pango_attr_list_insert(attr, pango_attr_size_new(36 * PANGO_SCALE));

    GtkWidget *label = gtk_label_new("");
    gtk_label_set_attributes(GTK_LABEL(label), attr);
    set_label_text(label, 1, "red");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    _lab = label;

    GtkWidget *but = gtk_button_new();
    set_but_img(but, args->id, 1);
    gtk_grid_attach(GTK_GRID(grid), but, 0, 2, 1, 1);
    _but = but;

    g_signal_connect(but, "clicked", G_CALLBACK(but_clicked), data);

    gtk_window_set_wmclass(GTK_WINDOW(win), "esopx", "esopx");
    gtk_widget_show_all(win);
}

int opx_main(int ac, char **av) {
    if (ac < 1) {
        fprintf(stderr, "missing opx number\n");
        return 1;
    }

    int  id = atoi(av[0]);
    char gtk_id[100];

    _id = id;
    signal(SIG_OPE_REQ_TO_2 + id - 2, tome);
    signal(SIG_OPE_REQ_TO_2_DONE + id - 2, move_done_handle);
    signal(SIG_OPX_UPDATE_LIGHT, update_light);

    shm             = get_shared_mem();
    opx_args_t args = {.shm = shm, .id = id};

    sprintf(gtk_id, "org.gtk.elevator-simulation-sub-%d", id);
    GtkApplication *app = gtk_application_new(gtk_id, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), &args);
    int res = g_application_run(G_APPLICATION(app), ac, av);
    g_object_unref(app);

    return res;
}
