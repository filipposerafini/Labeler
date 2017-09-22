/**
 * @file label.h
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

// Classes Colors
#define COLOR_SELECTED cvScalar(255, 0, 0, 0)   /**< Red */
#define COLOR_0 cvScalar(0, 128, 255, 0)        /**< Cyan */
#define COLOR_1 cvScalar(255, 128, 0, 0)        /**< Orange */
#define COLOR_2 cvScalar(0, 255, 0, 0)          /**< Green */
#define COLOR_3 cvScalar(127, 0, 255, 0)        /**< Purple */
#define COLOR_4 cvScalar(255, 255, 0, 0)        /**< Yellow */
#define COLOR_5 cvScalar(0, 0, 255, 0)          /**< Blue */
#define COLOR_6 cvScalar(255, 0, 255, 0)        /**< Magenta */
#define COLOR_7 cvScalar(0, 255, 255, 0)        /**< Light blue */
#define COLOR_8 cvScalar(255, 0, 127, 0)        /**< Pink */
#define COLOR_9 cvScalar(0, 0, 0, 0)            /**< Black */

/**
 * Define boolean type.
 */
typedef enum { false, true } bool;

/**
 * Store information for a labeled bounding box
 */
typedef struct {
    CvPoint center;     /**< Central point of the bounding box */
    int width;          /**< Distance from center to left/right edge */
    int height;         /**< Distance from center to top/bottom edge */
    bool selected;      /**< true if bounding box is currently selected, false otherwise */
    int class;          /**< Indicate witch class the label belongs to */
} label;

/**
 * Store the information for all labels in an image
 */
typedef struct {
    label label[MAX_LABELS];        /**< Array of labels */
    int count;                      /**< Array logical dimention */
    int selected;                   /**< Index of currently selected label in the array. -1 if no one is selected */
    int copied;                     /**< Index of currenntly copied label in the array. -1 if no one is selected */
    char classes[MAX_CLASSES][64];  /**< Array of classes names */
    int classes_count;              /**< Classes array logical dimention */
    int selected_class;             /**< Index of currently selected class in the array */
} labels;

/**
 * Save a new label.
 * Save in labels array a new label with given parameters,
 * incrementing array's logical dimention.
 * @param dest
 * @param center Central coordinates of the new label
 * @param width Width of the new label
 * @param height Height of the new label
 * @param class Class of the new label
 * @return true in case of success, false if maximum number of labels is reached
 */
bool create_label(labels *dest, CvPoint center, int width, int height, int class);

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
 * Select color for the label
 *
 * @param index class index
 *
 * @return class relative color
 */
CvScalar select_color(int index);

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
 * Show information for struct label.
 * Print center coordinates, width and height of a label to stdout.
 * @param label label struct to print information
 */
void print_label(label label);

/**
 * Show informations for struct labels.
 * Calls print_label() functions for every label in the array 
 * displaying which one is selected and/or copied, and then show 
 * all classes displaying which one is selected, for debug purpose.
 * @param labels labels struct to print information
 */
void debug_print(labels labels);

/**
 * Save labels to file.
 * Write in given file the information about every label and class
 * from source labels struct.
 * Write in the first line of the file the number of classes followed 
 * by, once per line, all class's names.
 * Then write a row per label, including image name,
 * center coordinates, width, height and class.
 * All the information are separated by semicolon.
 *
 * @param filename file to write in
 * @param imagename name of image that labels belong to
 * @param src source labels struct
 */
void save_labels(char *filename, char *imagename, labels src);

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
 * Load classes from file
 * Read classes count and names from given file, saving
 * them to dest labels struct
 *
 * @param filename file to read from
 * @param dest destination labels struct
 *
 * @return true in case of success, false if the file is not
 * well formatted
 */
bool load_classes(char *filename, labels *dest);

/**
 * Add new class.
 * Add new class to labels struct with given name,
 * incrementing classes counter.
 *
 * @param name name of the new class
 * @param labels
 *
 * @return true in case of success, false if a class with
 * the same name already exists or maximum number of classes is reached
 */
bool add_class(char *name, labels* labels);

/**
 * Remove a class.
 * Remove an existing class with given name from labels struct.
 *
 * @param name name of the class to remove
 * @param labels
 *
 * @return the index of removed class, -1 if name does not exists
 */
int remove_class(char *name, labels* labels);

/**
 * Reset classes.
 * Re-initialize classes parameters in labels struct,
 * setting logical dimention and selected index to 0
 * @param labels
 */
void reset_classes(labels *labels);

#endif
