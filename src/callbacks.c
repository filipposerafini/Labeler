#include "gui.h"

void show_open_dialog(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    g_debug("Showing open dialog\n");
    gtk_window_present(GTK_WINDOW(data->elements.open_dialog));
}

void on_folder_chooser_file_set(GtkFileChooser *folder_chooser, gpointer user_data) {
    data *data = user_data;
    DIR *dir;
    if ((data->selected_folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(folder_chooser))) != NULL) {
        if ((dir = opendir(data->selected_folder)) == NULL) {
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), FALSE);
            g_critical("Failed to open %s. Please select another folder", data->selected_folder);
        } else {
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), TRUE);
            closedir(dir);
            g_message("Selected folder %s", data->selected_folder);
        }
    }
}

void on_btn_cancel_clicked(GtkButton *button, gpointer user_data) {
    gui_elements *elements = user_data;
    g_debug("Hiding open dialog");
    gtk_widget_hide(elements->open_dialog);
}

void on_btn_open_clicked(GtkButton *button, gpointer user_data)
{
    data *data = user_data;
    if (data->dir != NULL) 
        closedir(data->dir);
    
    if ((data->dir = opendir(data->selected_folder)) == NULL) {
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_next), FALSE);
        g_critical("Failed to open %s. Please select another folder", data->selected_folder);
    } else {
        save(data->tmpfile, data->selected_folder);
        reset(&data->labels);
        gtk_widget_hide(data->elements.open_dialog);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_next), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_edit), TRUE); 
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.event_box), TRUE);
        g_message("Opening first image in %s", data->selected_folder);
        if (!open_next_image(data)) {
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_next), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.event_box), FALSE);
            g_warning("Selected directory has no image");
        } else {
            update_image(data);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.event_box), TRUE);
        }
    }
}

void show_save_dialog(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    FILE *file;
    g_debug("Showing save dialog\n");
    if ((file = fopen(data->tmpfile, "r")) != NULL) {
        fclose(file);
        gtk_window_present(GTK_WINDOW(data->elements.save_dialog));
    } else
        destroy(GTK_WINDOW(data->elements.main_window), data);
}

void on_file_chooser_file_set(GtkFileChooser *file_chooser, gpointer user_data) {
    data *data = user_data;
    char *p;
    if ((data->selected_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser))) != NULL) {
        if ((p = strrchr(data->selected_file, '.')) != NULL) {
            if (strcmp(p, ".csv")) {
                g_warning("File format incompatible. Select .csv file");
                gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), FALSE);
            } else {
                g_message("Selected file %s", data->selected_file);
                gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), TRUE);
            }
        } else {
            g_warning("File format incompatible. Select .csv file");
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), FALSE);
        }
    }
}


void on_btn_save_clicked(GtkButton *button, gpointer user_data) {
    data *data = user_data;
    save(data->tmpfile, data->selected_folder);
    destroy(GTK_WINDOW(data->elements.main_window), data);
}

void on_mi_reset_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    reset(&data->labels);
    update_image(data);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_copy), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_paste), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_delete), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_reset), FALSE);
    g_message("Image cleared");
}

void on_mi_copy_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    data->labels.copied = data->labels.selected;
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_paste), TRUE);
    g_message("Label %d copied\n", data->labels.copied);
}

void on_mi_paste_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    if (paste_label(&data->labels)) {
        g_message("Label %d pasted", data->labels.count - 1);
        update_image(data);
    } else
        g_warning("Label not pasted: too many labels\n");
}

void on_mi_delete_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    if (delete_label(&data->labels))
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_paste), FALSE);
    update_image(data);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_copy), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_delete), FALSE);
    if (data->labels.count == 0)
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_reset), FALSE);
    g_message("Deleted selected label");
}

void on_mi_print_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    debug_print(data->labels);
}

gboolean on_key_press_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    data *data = user_data;
    if (event->key.keyval == GDK_KEY_Control_L) 
        data->control = true;
    if (data->control && event->key.keyval == GDK_KEY_c && gtk_widget_is_sensitive(GTK_WIDGET(data->elements.mi_copy)))
        on_mi_copy_activate(data->elements.mi_copy, data);
    if (data->control && event->key.keyval == GDK_KEY_v && gtk_widget_is_sensitive(GTK_WIDGET(data->elements.mi_paste)))
        on_mi_paste_activate(data->elements.mi_paste, data);
    if (event->key.keyval == GDK_KEY_BackSpace && gtk_widget_is_sensitive(GTK_WIDGET(data->elements.mi_delete)))
        on_mi_delete_activate(data->elements.mi_delete, data);
    if (data->control && event->key.keyval == GDK_KEY_r && gtk_widget_is_sensitive(GTK_WIDGET(data->elements.mi_reset)))
        on_mi_reset_activate(data->elements.mi_reset, data);
    if (event->key.keyval == GDK_KEY_n && gtk_widget_is_sensitive(GTK_WIDGET(data->elements.btn_next)))
        on_btn_next_clicked(data->elements.btn_next, data);
}

gboolean on_key_release_event(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    data *data = user_data;
    data->control = false;
}

gboolean on_button_press_event(GtkWidget *image, GdkEvent *event, gpointer user_data) {
    data *data = user_data;
    int x, y;
    bool inside = convert_coordinates(event->button.x, event->button.y, data->elements.event_box, &x, &y, data->img, data->elements.image);
    if (event->button.button == 1) {
        g_debug("Left click triggered");
        if (!data->moving) {
            data->drawing = true;
            if (data->labels.selected >= 0) {
                data->labels.label[data->labels.selected].selected = false;
                data->labels.selected = -1;
            }
            if (inside) {
                data->corner = cvPoint(x, y);
                data->opposite_corner = cvPoint(x, y);
            } else {
                data->corner = cvPoint(-1, -1); 
                data->opposite_corner = cvPoint(-1, -1);
            }
        }
    }
    else if (event->button.button == 3 && inside) {
        g_debug("Right click triggered");
        if (select_label(x, y, &data->labels)) {
            data->moving = true;
            g_message("Selected label %d\n", data->labels.selected);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_copy), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_delete), TRUE);
        } else {
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_copy), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_delete), FALSE);
        }
    }
}

gboolean on_button_release_event(GtkWidget *image, GdkEvent *event, gpointer user_data) {
    data *data = user_data;
    int x, y;
    bool inside = convert_coordinates(event->button.x, event->button.y, data->elements.event_box, &x, &y, data->img, data->elements.image);
    if (event->button.button == 1) {
        g_debug("Left release triggered");
        data->drawing = false;
        if (data->corner.x >= 0 && data->corner.y >= 0 && inside)
            data->opposite_corner = cvPoint(x, y);
        // Save new label
        if (data->opposite_corner.x != data->corner.x && data->opposite_corner.y != data->corner.y) {
            CvPoint center;
            int width, height;
            label *label = data->labels.label;
            center = cvPoint((data->corner.x + data->opposite_corner.x)/2, (data->corner.y + data->opposite_corner.y)/2);
            width = abs(center.x - data->corner.x);
            height = abs(center.y - data->corner.y);
            if (create_label(&data->labels, center, width, height)) {
                g_message("New label");
                printf("Label %d -> ", data->labels.count - 1);
                print_label(data->labels.label[data->labels.count - 1]);
                update_image(data);
            } else
                g_warning("Label not saved: Too many labels\n");
            if (data->labels.count == 1)
                gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_reset), TRUE);
        }
    }
    else if (event->button.button == 3) {
        g_debug("Right release triggered");
        data->moving = false;
        update_image(data);
    }
}

gboolean on_mouse_move_event(GtkWidget *image, GdkEvent *event, gpointer user_data) {
    data *data = user_data;
    int x, y;
    bool inside = convert_coordinates(event->button.x, event->button.y, data->elements.event_box, &x, &y, data->img, data->elements.image);
    if (data->drawing) {
        if (data->corner.x >= 0 && data->corner.y >= 0 && inside)
            data->opposite_corner = cvPoint(x, y);
        update_image(data);
    } else if (data->moving && inside) {
        data->labels.label[data->labels.selected].center = cvPoint(x, y);
        update_image(data);
    }
}

void on_btn_next_clicked(GtkButton *button, gpointer user_data) {
    data *data = user_data;
    save_labels(data->tmpfile, data->name, data->labels);
    if (!open_next_image(data)) {
        g_warning("No more image in %s", data->selected_folder);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_next), FALSE);
    } else {
        g_message("Opening next image in %s", data->selected_folder);
        update_image(data);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_reset), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_copy), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_paste), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_delete), FALSE);
    }
}

void resize_image(GtkWidget *widget, GdkRectangle *rectangle, gpointer user_data) {
    data *data = user_data;
    if (data->img != NULL) {
        update_image(data);
    }
}

// called when window is closed
void destroy(GtkWindow *self, gpointer user_data) {
    data *data = user_data;
    if (data->img != NULL) 
        cvReleaseImage(&data->img);
    if (data->tmp != NULL) 
        cvReleaseImage(&data->tmp);
    closedir(data->dir);
    g_free(data->selected_folder);
    free(data->name);
    FILE *file;
    if ((file = fopen(data->tmpfile, "r")) != NULL) {
        fclose(file);
        if (remove(data->tmpfile)) {
            g_error("Error removing file %s\n", data->tmpfile);
            exit(EXIT_FAILURE);
        }
    }
    gtk_main_quit();
}

