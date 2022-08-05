#include <gtk/gtk.h>

#include "shared.h"
#include "sigs.h"

static GtkWidget *_buts[5];
static int _self_clicked = 0;

static void to2() {
    if (!_self_clicked) set_but_img(_buts[1], 2, 0);
    _self_clicked = 0;
}

static void to3() {
    if (!_self_clicked) set_but_img(_buts[2], 3, 0);
    _self_clicked = 0;
}

static void to4() {
    if (!_self_clicked) set_but_img(_buts[3], 4, 0);
    _self_clicked = 0;
}

static void to5() {
    if (!_self_clicked) set_but_img(_buts[4], 5, 0);
    _self_clicked = 0;
}

static void but_clicked(GtkWidget *but, gpointer data) {
    opx_args_t *args = data;
    if (opx_send_sig(args->shm, args->id) == 0) {
        printf("sub clicked %d: ", args->id);
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

    for (int i = 2; i <= 5; ++i) {
        GtkWidget *but = gtk_button_new();
        printf("sub %d: ", i);
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
    signal(SIG_OPE_REQ_TO_2, to2);
    signal(SIG_OPE_REQ_TO_3, to3);
    signal(SIG_OPE_REQ_TO_4, to4);
    signal(SIG_OPE_REQ_TO_5, to5);

    shared_mem     *shm = get_shared_mem();
    GtkApplication *app = gtk_application_new(
        "org.gtk.elevator-simulation", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), shm);
    int res = g_application_run(G_APPLICATION(app), ac, av);
    g_object_unref(app);
    return res;
}
