#include "callbacks.h"

// Add class row to classes list box
void add_class_row(data *data, char *class_name, bool loaded) {

    // Get row from builder
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "src/labeler.ui", NULL);
    GtkListBoxRow *row = GTK_LIST_BOX_ROW(gtk_builder_get_object(builder, "class_row"));
    GtkLabel *label = GTK_LABEL(gtk_builder_get_object(builder, "class_label"));
    GtkButton *button = GTK_BUTTON(gtk_builder_get_object(builder, "btn_remove_class"));
    g_signal_connect(button, "clicked", G_CALLBACK(on_btn_remove_class_clicked), data);
    if (loaded)
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);
    gtk_label_set_text(label, class_name);
    gtk_list_box_insert(data->elements.class_list, GTK_WIDGET(row), -1);
    g_object_unref(builder);
}

// Resize image
void resize_image(GtkWidget *widget, GdkRectangle *rectangle, gpointer user_data) {
    data *data = user_data;
    if (data->img != NULL) {
        update_image(data);
    }
}

// Show open dialog
void show_open_dialog(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    g_debug("Showing open dialog");
    gtk_window_present(GTK_WINDOW(data->elements.open_dialog));
}

// Select image source folder
void on_folder_chooser_file_set(GtkFileChooser *folder_chooser, gpointer user_data) {
    data *data = user_data;
    DIR *dir;
    // Read folder name
    if ((data->selected_folder = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(folder_chooser))) != NULL) {
        // Try to open folder
        if ((dir = opendir(data->selected_folder)) == NULL) {
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), FALSE);
            g_critical("Failed to open %s. Please select another folder", data->selected_folder);
        } else {
            if (data->labels.classes_count > 0)
                gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), TRUE);
            closedir(dir);
            g_message("Selected folder %s", data->selected_folder);
        }
    }
}

// Select file to load informations
void on_file_chooser_file_set(GtkFileChooser *file_chooser, gpointer user_data) {
    data *data = user_data;
    char *p;
    FILE *file;
    // Read file name
    if ((data->selected_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser))) != NULL) {
        // Check file extension
        if ((p = strrchr(data->selected_file, '.')) != NULL) {
            if (strcmp(p, ".csv")) {
                data->selected_file = NULL;
                g_warning("File format incompatible. Select .csv file");
                gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), FALSE);
            } else {
                if (load_classes(data->selected_file, &data->labels)) {
                    for (int i = 0; i < data->labels.classes_count; i++)
                        add_class_row(data, data->labels.classes[i], true);
                    g_message("Selected file %s", data->selected_file);

                    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_add_class), FALSE);

                    if (data->selected_folder != NULL)
                        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), TRUE);
                } else {
                    g_error("Error reading file %s", data->selected_file);
                    data->selected_file = NULL;
                    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), FALSE);
                }
            }
        } else {
            data->selected_file = NULL;
            g_warning("File format incompatible. Select .csv file");
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), FALSE);
        }
    }
}

// Add a new class
void on_btn_add_class_clicked(GtkButton *button, gpointer user_data) {
    data *data = user_data;
    if (gtk_entry_get_text_length(data->elements.entry_class) > 0) {
        char *class_name = gtk_entry_get_text(data->elements.entry_class);
        // Try to add new class
        g_message("New Class");
        if (add_class(class_name, &data->labels)) {
            // Add class row to list box
            add_class_row(data, class_name, false);
            // Reset entry
            gtk_entry_set_text(data->elements.entry_class, "");
            // Eventually enable open button and disable add class button 
            if (data->labels.classes_count > 0 && data->selected_folder != NULL)
                gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), TRUE);
            if (data->labels.classes_count == MAX_CLASSES)
                gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_add_class), FALSE);
        }
        else
            g_warning("Class cannot be added");
    }
}

// Remove a specific class and destroy related widget
void on_btn_remove_class_clicked(GtkButton *button, gpointer user_data) {
    data* data = user_data;
    // Get class to remove
    GtkBox *box = GTK_BOX(gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(button)))); 
    GList *children = gtk_container_get_children(GTK_CONTAINER(box));
    char *class = gtk_label_get_text(GTK_LABEL(children->data));
    g_list_free(children);
    // Remove class
    int class_index = remove_class(class, &data->labels);
    if (class_index >= 0) {
        g_message("Removed class %d", class_index);
        // Destroy related widget
        gtk_widget_destroy(gtk_widget_get_parent(GTK_WIDGET(box)));
        // Eventually disable open button and enable add class button
        if (data->labels.classes_count == 0)
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), FALSE);
        if (data->labels.classes_count < MAX_CLASSES)
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_add_class), TRUE);
    }
}

// Hide the open dialog window
void on_btn_open_cancel_clicked(GtkButton *button, gpointer user_data) {
    data *data = user_data;
    g_debug("Hiding open dialog");
    gtk_widget_hide(data->elements.open_dialog);
}

// Start cycling image from selected folder
void on_btn_open_clicked(GtkButton *button, gpointer user_data) {
    data *data = user_data;
    // Close previous dir
    closedir(data->dir);
    // Try to open selected directory
    if ((data->dir = opendir(data->selected_folder)) == NULL) {
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_next), FALSE);
        g_critical("Failed to open %s. Please select another folder", data->selected_folder);
    } else {
        // Initialize labels
        reset(&data->labels);
        // Hide open dialog
        gtk_widget_hide(data->elements.open_dialog);
        g_message("Opening first image in %s", data->selected_folder);
        // Open next image
        if (!open_next_image(data))
            g_warning("Selected directory has no image");
        else {
            update_image(data);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_open), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_save), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_edit), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_classes), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_view), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_print), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_next), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.event_box), TRUE);
            // Populate classes menu item
            GtkAccelGroup *accel_group = gtk_accel_group_new();
            gtk_window_add_accel_group(GTK_WINDOW(data->elements.main_window), accel_group);
            GtkRadioMenuItem *group = NULL;
            for (int i = 0; i < data->labels.classes_count; i++) {
                GtkWidget *mi_class = gtk_radio_menu_item_new_with_label_from_widget(group, data->labels.classes[i]);
                gtk_widget_show(mi_class);
                gtk_container_add(GTK_CONTAINER(data->elements.menu_classes), mi_class);
                g_signal_connect(mi_class, "activate", G_CALLBACK(on_mi_class_activate), &data->labels);
                gtk_widget_add_accelerator(mi_class, "activate", accel_group, GDK_KEY_0 + i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
                group = GTK_RADIO_MENU_ITEM(mi_class);
            }
        }
    }
}

// Select a specific class
void on_mi_class_activate(GtkMenuItem *menu_item, gpointer user_data) {
    labels *labels = user_data;
    for (int i = 0; i < labels->classes_count; i++) {
        if(!strcmp(gtk_menu_item_get_label(menu_item), labels->classes[i])) {
            labels->selected_class = i;
            g_message("Selected class %d", i);
            // Change class of selected label
            if (labels->selected >= 0) {
                labels->label[labels->selected].class = i;
                g_message("Label %d -> Class changed to %d", labels->selected, i);
            }
            break;
        }
    }
}

// Move up currently selected label
void on_mi_up_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    data->labels.label[data->labels.selected].center.y--;
    update_image(data);
}

// Move down currently selected label
void on_mi_down_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    data->labels.label[data->labels.selected].center.y++;
    update_image(data);
}

// Move left currently selected label
void on_mi_left_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    data->labels.label[data->labels.selected].center.x--;
    update_image(data);
}

// Move right currently selected label
void on_mi_right_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    data->labels.label[data->labels.selected].center.x++;
    update_image(data);
}

// Copy currently selected label
void on_mi_copy_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    data->labels.copied = data->labels.selected;
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_paste), TRUE);
    g_message("Label %d copied", data->labels.copied);
}

// Paste currently copied label
void on_mi_paste_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    if (paste_label(&data->labels)) {
        g_message("Label %d pasted", data->labels.count - 1);
        update_image(data);
    } else
        g_warning("Label not pasted: too many labels");
}

// Delete currently selected label
void on_mi_delete_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    if (delete_label(&data->labels))
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_paste), FALSE);
    update_image(data);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_copy), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_move), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_delete), FALSE);
    if (data->labels.count == 0) {
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_reset), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_print), FALSE);
    }
    g_message("Deleted selected label");
}

// Reset labels for current image
void on_mi_reset_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    reset(&data->labels);
    update_image(data);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_copy), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_move), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_paste), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_delete), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_reset), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_print), FALSE);
    g_message("Image cleared");
}

// Print useful info to stdout
void on_mi_print_activate(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    debug_print(data->labels);
}

gboolean on_button_press_event(GtkWidget *image, GdkEvent *event, gpointer user_data) {
    data *data = user_data;
    int x, y;
    bool inside = convert_coordinates(event->button.x, event->button.y, data->elements.event_box, &x, &y, data->img, data->elements.image);
    // Left mouse button
    if (event->button.button == 1) {
        g_debug("Left click triggered");
        if (!data->moving) {
            data->drawing = true;
            // Deselect selected label
            if (data->labels.selected >= 0) {
                data->labels.label[data->labels.selected].selected = false;
                data->labels.selected = -1;
            }
            // Start drawing
            if (inside) {
                data->corner = cvPoint(x, y);
                data->opposite_corner = cvPoint(x, y);
            } else {
                data->corner = cvPoint(-1, -1);
                data->opposite_corner = cvPoint(-1, -1);
            }
        }
    }
    // Right mouse button
    else if (event->button.button == 3 && inside) {
        g_debug("Right click triggered");
        // Select label
        if (select_label(x, y, &data->labels)) {
            data->moving = true;
            g_message("Selected label %d", data->labels.selected);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_copy), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_move), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_delete), TRUE);
        } else {
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_copy), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_move), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_delete), FALSE);
        }
    }
}

gboolean on_button_release_event(GtkWidget *image, GdkEvent *event, gpointer user_data) {
    data *data = user_data;
    int x, y;
    bool inside = convert_coordinates(event->button.x, event->button.y, data->elements.event_box, &x, &y, data->img, data->elements.image);
    // Left mouse button
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
            if (create_label(&data->labels, center, width, height, data->labels.selected_class)) {
                g_message("New label");
                printf("Label %d -> ", data->labels.count - 1);
                print_label(data->labels.label[data->labels.count - 1]);
                update_image(data);
            } else
                g_warning("Label not saved: Too many labels");
            if (data->labels.count == 1) {
                gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_reset), TRUE);
                gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_print), TRUE);
            }
        }
    }
    // Right mouse button
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
        // Update opposite corner coordinates
        if (data->corner.x >= 0 && data->corner.y >= 0 && inside)
            data->opposite_corner = cvPoint(x, y);
        update_image(data);
    } else if (data->moving && inside) {
        // Update center coordinates
        data->labels.label[data->labels.selected].center = cvPoint(x, y);
        update_image(data);
    }
}

// Try to open next image in working directory
void on_btn_next_clicked(GtkButton *button, gpointer user_data) {
    data *data = user_data;
    save_labels(TMPFILE, data->name, data->labels);
    // Open next image
    if (!open_next_image(data)) {
        g_warning("No more image in %s", data->selected_folder);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_next), FALSE);
    } else {
        g_message("Opening next image in %s", data->selected_folder);
        update_image(data);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_reset), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_copy), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_move), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_paste), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_delete), FALSE);
    }
}

// Save data and reset workspace
void on_mi_save_clicked(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;

    GtkWidget *dialog = gtk_file_chooser_dialog_new("Labeler - Save",
            GTK_WINDOW(data->elements.main_window),
            action,
            "_Cancel",
            GTK_RESPONSE_CANCEL,
            "_Save",
            GTK_RESPONSE_ACCEPT,
            NULL);
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    char *foldername = strrchr(data->selected_folder, '/') + 1;
    char *p = (char*)malloc(sizeof(foldername) + 4);
    sprintf(p, "%s.csv", foldername);
    gtk_file_chooser_set_current_name(chooser, p);
    free(p);

    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename(chooser);
        
        // Save
        save_labels(TMPFILE, data->name, data->labels);
        save(TMPFILE, filename);
        free(filename);
        
        // Reset Classes
        reset_classes(&data->labels);
        
        // Force to open a new project (or quit)
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.event_box), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_open), TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_save), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_next), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_edit), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_view), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.mi_classes), FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(data->elements.btn_open), FALSE);

        // Destroy list box & classes menu children
        GList *children, *iter;
        children = gtk_container_get_children(GTK_CONTAINER(data->elements.class_list));
        for(iter = children; iter != NULL; iter = g_list_next(iter))
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        children = gtk_container_get_children(GTK_CONTAINER(data->elements.menu_classes));
        for(iter = children; iter != NULL; iter = g_list_next(iter))
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        g_list_free(children);
        g_list_free(iter);
    }
    gtk_widget_destroy (dialog);
}

// Show save dialog if any unsaved changes are present
void show_save_dialog(GtkMenuItem *menu_item, gpointer user_data) {
    data *data = user_data;
    FILE *file;
    g_debug("Showing save dialog");
    if ((file = fopen(TMPFILE, "r")) != NULL) {
        fclose(file);
        gtk_window_present(GTK_WINDOW(data->elements.save_dialog));
    } else
        destroy(GTK_WINDOW(data->elements.main_window), data);
}

// Save data and calls destroy function
/*void on_btn_save_clicked(GtkButton *button, gpointer user_data) {*/
    /*data *data = user_data;*/
    /*show_save_file_dialog(data);*/
    /*[>save(TMPFILE, data->selected_folder);<]*/
    /*destroy(GTK_WINDOW(data->elements.main_window), data);*/
/*}*/

// Free all allocated memory and destroy all windows
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
    if ((file = fopen(TMPFILE, "r")) != NULL) {
        fclose(file);
        if (remove(TMPFILE)) {
            g_error("Error removing file %s", TMPFILE);
            exit(EXIT_FAILURE);
        }
    }
    gtk_main_quit();
}
