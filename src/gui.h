#ifndef GUI_H
#define GUI_H

#include "Label.h"
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdkkeysyms.h>
#include <opencv/highgui.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct {
    GtkWidget *main_window;
    GtkMenuItem *mi_open;
    GtkMenuItem *mi_reset;
    GtkMenuItem *mi_quit;
    GtkMenuItem *mi_edit;
    GtkMenuItem *mi_copy;
    GtkMenuItem *mi_paste;
    GtkMenuItem *mi_delete;
    GtkMenuItem *mi_print;
    GtkWidget *event_box;
    GtkImage *image;
    GtkButton *btn_quit;
    GtkButton *btn_next;
    GtkWidget *open_dialog;
    GtkButton *btn_cancel;
    GtkButton *btn_open;
    GtkWidget *save_dialog;
} gui_elements;

typedef struct {
    gui_elements elements;
	labels labels;
    IplImage *img;
    IplImage *tmp;
    gchar *selected_folder;
    gchar *selected_file;
    DIR *dir;
    CvPoint corner;
    CvPoint opposite_corner;
    bool drawing;
    bool moving;
    bool control;
    char *tmpfile;
    char *name;
} data;

void init_gui_elements(gui_elements *elements, GtkBuilder *builder);
void init_data(data *data, GtkBuilder *builder);
void show_image(IplImage *img, GtkImage *image, GtkWidget *widget);
bool open_next_image(data *data);
void update_image(data *data);
bool convert_coordinates(float pointer_x, float pointer_y, GtkWidget *widget, int *x, int *y, IplImage *img, GtkImage *image); 
void save(char *tmpfile, char *folder_name);

void show_select_folder_dialog(GtkMenuItem *image_menu_item, gpointer user_data);
void on_mi_reset_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_copy_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_paste_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_delete_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_print_activate(GtkMenuItem *menu_item, gpointer user_data);
gboolean on_key_press_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_key_release_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_button_press_event(GtkWidget *image, GdkEvent *event, gpointer user_data);
gboolean on_button_press_event(GtkWidget *image, GdkEvent *event, gpointer user_data);
gboolean on_mouse_move_event(GtkWidget *image, GdkEvent *event, gpointer user_data);
void on_folder_chooser_file_set(GtkFileChooser *file_chooser, gpointer user_data); 
void on_file_chooser_file_set(GtkFileChooser *file_chooser, gpointer user_data);
void on_btn_next_clicked(GtkButton *button, gpointer user_data);
void on_btn_open_clicked(GtkButton *button, gpointer user_data);
void on_btn_cancel_clicked(GtkButton *button, gpointer user_data);
void resize_image(GtkWidget *widget, GdkRectangle *rectangle, gpointer user_data);
void destroy(GtkWindow *window, gpointer pointer);

#endif

