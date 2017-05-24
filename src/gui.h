/**
 * @file gui.h
 * @brief User interface header file
 * @author Filippo Serafini
 */

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

#define TMPFILE "out/.tmpfile"

/**
 * Stuct that contains all gui elements
 */
typedef struct {
    GtkWidget *main_window;
    GtkMenuItem *mi_open;
    GtkMenuItem *mi_save;
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

/**
 * Struct that contains all data used by the program
 */
typedef struct {
    gui_elements elements;      /**< User interface elements */
	labels labels;              /**< Labels struct */
    gchar *selected_folder;     /**< Currently selected source folder */
    gchar *selected_file;       /**< Currently selected source file */
    DIR *dir;                   /**< DIR struct of currenty selected source folder*/
    char *name;                 /**< Image name */
    IplImage *img;              /**< Current image */
    IplImage *tmp;              /**< Temporary image */
    CvPoint corner;             /**< Currently drawing label corner point */
    CvPoint opposite_corner;    /**< Currently drawing label opposite_corner point */
    bool drawing;               /**< Drawing label flag */
    bool moving;                /**< Moving label flag */
} data;

void init_data(data *data, GtkBuilder *builder);
void init_gui_elements(gui_elements *elements, GtkBuilder *builder);

/**
 * Show image in the image container.
 * Show given IplImage image in GtkImage container, resizing to fit
 * in widget dimention keeping original ratio.
 *
 * @param img Image to show
 * @param image Image container
 * @param widget Window
 */
void show_image(IplImage *img, GtkImage *image, GtkWidget *widget);

/**
 * Open next image in selected directory.
 * Search for the next valid image in selected directory accepting only .jpg/.png files,
 * and show it calling show_image().
 *
 * @param data
 *
 * @return true on success, false if there isn't any other valid image in current directory
 */
bool open_next_image(data *data);

/**
 * Update image content.
 * Refresh the image redrawing all the labels, and show it calling show_image().
 *
 * @param data
 */
void update_image(data *data);

/**
 * Convert coordinates.
 * Convert from container relative coordinates to image relative coordinates.
 *
 * @param pointer_x Container relative x coordinate
 * @param pointer_y Container relative y coordinate
 * @param widget Event box
 * @param x Image relative x coordinate
 * @param y Image relative y coordinate
 * @param img Current image
 * @param image Image container
 *
 * @return true if given point is inside the image, false otherwise
 */
bool convert_coordinates(float pointer_x, float pointer_y, GtkWidget *widget, int *x, int *y, IplImage *img, GtkImage *image);

/**
 * Save labels to file.
 * If exists, rename the temporary file used to store labels information to .csv output file,
 * placing it in out/ in a folder named as give folder_name
 *
 * @param tmpfile Temporary file name
 * @param folder_name Selected folder
 */
void save(char *tmpfile, char *folder_name);

// CALLBACKS
void resize_image(GtkWidget *widget, GdkRectangle *rectangle, gpointer user_data);

void show_open_dialog(GtkMenuItem *image_menu_item, gpointer user_data);
void on_folder_chooser_file_set(GtkFileChooser *file_chooser, gpointer user_data);
void on_file_chooser_file_set(GtkFileChooser *file_chooser, gpointer user_data);
void on_btn_open_cancel_clicked(GtkButton *button, gpointer user_data);
void on_btn_open_clicked(GtkButton *button, gpointer user_data);

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
