/**
 * @file Label.h
 * @brief Label managment header File
 * @author Filippo Serafini
 */

#ifndef LABEL_H
#define LABEL_H

#include <stdio.h>
#include <opencv/cv.h>

#define MAX_LABELS 64   /**< Maximum number of allowed label per image */
#define MAX_CLASSES 10  /**< Maximum number of allowed classes per project */
#define ALPHA 0.4       /**< Transparency value for label with filled background */
#define COLOR_SELECTED cvScalar(255, 0, 0, 0)
#define COLOR_0 cvScalar(0, 128, 255, 0)
#define COLOR_1 cvScalar(255, 128, 0, 0)
#define COLOR_2 cvScalar(0, 255, 0, 0)
#define COLOR_3 cvScalar(127, 0, 255, 0)
#define COLOR_4 cvScalar(255, 255, 0, 0)
#define COLOR_5 cvScalar(0, 0, 255, 0)
#define COLOR_6 cvScalar(255, 0, 255, 0)
#define COLOR_7 cvScalar(0, 255, 255, 0)
#define COLOR_8 cvScalar(255, 0, 127, 0)
#define COLOR_9 cvScalar(0, 0, 0, 0)

/**
 * Define boolean type.
 */
typedef enum { false, true } bool;

/**
 * Store coordinates of a rectangle.
 */
typedef struct {
    CvPoint center;     /**< Central point of the label */
    int width;          /**< Distance from center to left/right edge */
    int height;         /**< Distance from center to top/bottom edge */
    bool selected;      /**< true if label is currently selected, false otherwise */
    int class;          /**< Indicate witch class the label belongs to */
} label;

/**
 * Struct to represent a group of labels.
 */
typedef struct {
    label label[MAX_LABELS];    /**< Array of labels */
    int count;                  /**< Array logical dimention */
    int selected;               /**< Index of currently selected label in the array. -1 if no one is selected */
    int copied;                 /**< Index of currenntly copied label in the array. -1 if no one is selected */
    char classes[MAX_CLASSES][64];
    int classes_count;
    int selected_class;
} labels;

/**
 * Show information for struct label.
 * Print center coordinates, width and height of a label to stdout.
 * @param label label struct to print information
 */
void print_label(label label);

/**
 * Show informations for struct labels.
 * Calls print_label() functions for every label in the array and
 * display also which one is selected and/or copied for debug purpose.
 * @param labels labels struct to print information
 */
void debug_print(labels labels);

/**
 * Check if a point is over a label.
 * Check if the point of coordinates (x,y) is inside the label.
 * @param x x coordinate
 * @param y y coordinate
 * @param label label to check
 * @return true if point is inside the label, false otherwise
 */
bool over_label(int x, int y, label label);

/**
 * Select a label from a group of labels.
 * Check if a point of coordinates x and y is inside any label in labels
 * and eventually select one, saving the index in 'selected' structure's field.
 * @param x x coordinate
 * @param y y coordinate
 * @param labels The group of labels to check
 * @return true if a label is seleted, false otherwise
 */
bool select_label(int x, int y, labels *labels);

/**
 * Draw a label.
 * Draw a colored rectangle in an image with given corners.
 * If fill is true the label will have a transparent
 * background of the same color of the border.
 * @param img Destination image
 * @param corner1 Corner of the label
 * @param corner2 Opposite corner of the label
 * @param color Color of label borders
 * @param fill Fill option
 */
void draw_label(IplImage *img, CvPoint corner1, CvPoint corner2, CvScalar color, bool fill);

/**
 * Save a new label.
 * Save in labels array a new label with given parameters,
 * incrementing array's logical dimention.
 * @param labels
 * @param center Central coordinates of the new label
 * @param width Width of the new label
 * @param height Height of the new label
 * @param class Class of the new label
 * @return true in case of success, false if maximum number of labels is reached
 */
bool create_label(labels *dest, CvPoint center, int width, int height, int class);

/**
 * Paste copied label.
 * Save a copy of currently copied label in labels array,
 * incrementing array's logical dimention.
 * @param labels
 * @return true in case of success, false if maximum number of labels is reached
 */
bool paste_label(labels *labels);

/**
 * Delete selected label.
 * Delete currently selected label from the labels array,
 * decrementing array's logical dimention and re-initializing
 * 'selected' structure's field to -1.
 * @param labels
 * @return true if deleted label was also the copied one,
 * in this case 'copied' structure's field is re-initialized,
 * false otherwise
 */
bool delete_label(labels *labels);

/**
 * Reset labels.
 * Re-initialize labels struct parameters,
 * setting logical dimention to 0, copied and selected index to -1
 * @param labels
 */
void reset(labels *labels);

/**
 * Reset classes.
 * Re-initialize classes parameters in labels struct,
 * setting logical dimention to 0 and selected index to -1
 * @param labels
 */
void reset_classes(labels *labels);

/**
 * Load labels from file.
 * Read given file searching for given imagename labels,
 * and save them to dest labels struct.
 *
 * @param filename file to read from
 * @param imagename name of the image to load labels
 * @param dest destination labels struct
 */
void load_labels(char *filename, char *imagename, labels *dest);

/**
 * Save labels to file.
 * Write in given file the information about every label
 * from source labels struct.
 * The function write a row per label, including image name,
 * center coordinates, width and height.
 * All the information are separated by semicolon.
 *
 * @param filename file to write in
 * @param imagename name of image that labels belong to
 * @param src source labels struct
 */
void save_labels(char *filename, char *imagename, labels src);

bool add_class(char *name, labels* labels);

int remove_class(char *name, labels* labels);

#endif
