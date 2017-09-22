#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "gui.h"

// CALLBACKS
void resize_image(GtkWidget *widget, GdkRectangle *rectangle, gpointer user_data);

void show_open_dialog(GtkMenuItem *image_menu_item, gpointer user_data);
void on_folder_chooser_file_set(GtkFileChooser *file_chooser, gpointer user_data);
void on_file_chooser_file_set(GtkFileChooser *file_chooser, gpointer user_data);
void on_btn_add_class_clicked(GtkButton *button, gpointer user_data);
void on_btn_remove_class_clicked(GtkButton *button, gpointer user_data);
void on_btn_open_cancel_clicked(GtkButton *button, gpointer user_data);
void on_btn_open_clicked(GtkButton *button, gpointer user_data);

void on_mi_class_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_up_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_down_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_left_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_right_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_copy_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_paste_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_delete_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_reset_activate(GtkMenuItem *menu_item, gpointer user_data);
void on_mi_print_activate(GtkMenuItem *menu_item, gpointer user_data);

gboolean on_button_press_event(GtkWidget *image, GdkEvent *event, gpointer user_data);
gboolean on_button_press_event(GtkWidget *image, GdkEvent *event, gpointer user_data);
gboolean on_mouse_move_event(GtkWidget *image, GdkEvent *event, gpointer user_data);

void on_btn_next_clicked(GtkButton *button, gpointer user_data);

void show_save_dialog(GtkMenuItem *menu_item, gpointer user_data);
void on_btn_save_cancel_clicked(GtkButton *button, gpointer user_data);
void on_btn_save_clicked(GtkButton *button, gpointer user_data);
void destroy(GtkWindow *window, gpointer pointer);

#endif
