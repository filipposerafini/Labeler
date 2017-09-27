/**
 * @file gui.h
 * @brief User interface header file
 * @author Filippo Serafini
 */

#ifndef GUI_H
#define GUI_H

#include "label.h"
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
 * Stores all GUI elements to interact with
 */
typedef struct {
    GtkWidget *main_window;
    GtkMenuItem *mi_open;
    GtkMenuItem *mi_save;
    GtkMenuItem *mi_reset;
    GtkMenuItem *mi_quit;
    GtkMenuItem *mi_edit;
    GtkMenuItem *mi_move;
    GtkMenuItem *mi_copy;
    GtkMenuItem *mi_paste;
    GtkMenuItem *mi_delete;
    GtkMenuItem *mi_classes;
    GtkMenu *menu_classes;
    GtkMenuItem *mi_view;
    GtkMenuItem *mi_print;
    GtkWidget *event_box;
    GtkImage *image;
    GtkButton *btn_quit;
    GtkButton *btn_next;
    GtkWidget *open_dialog;
    GtkButton *btn_cancel;
    GtkButton *btn_open;
    GtkListBox *class_list;
    GtkEntry *entry_class;
    GtkButton *btn_add_class;
    GtkWidget *save_dialog;
} gui_elements;

/**
 * Stores all data used by the program
 */
typedef struct {
    gui_elements elements;      /**< User interface elements */
	labels labels;              /**< Labels struct */
    gchar *selected_folder;     /**< Currently selected source folder */
    gchar *selected_file;       /**< Currently selected source file */
    struct dirent **dirlist;    /**< Dirent struct array of currenty selected source folder content*/
    int dir_count;              /**< Dirlist logical dimention */ 
    int dir_position;           /**< Current position in the array */
    char *name;                 /**< Current image name */
    IplImage *img;              /**< Current image struct */
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
 * Search for the next valid image in selected directory,
 * and shows it calling show_image().
 *
 * @param data
 *
 * @return true on success, false if there isn't any other valid image in current directory
 */
bool open_next_image(data *data);

/**
 * Update image content.
 * Refresh the image redrawing all the labels, and shows it calling show_image().
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
 * If exists, rename the temporary file used to store labels information to given filename,
 *
 * @param tmpfile Temporary file name
 * @param filename Selected file name
 */
void save(char *tmpfile, char *filename);

#endif
