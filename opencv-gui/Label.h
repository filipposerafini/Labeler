/**
 * @file Label.h
 * @brief Header File
 * @author Filippo Serafini
 */

#ifndef LABEL_H
#define LABEL_H

#include <stdio.h>
#include <opencv/cv.h>

#define MAX_LABELS 64
#define ALPHA 0.4

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
} label;

/**
 * Struct to represent a group of labels.
 */
typedef struct {
    label label[MAX_LABELS];    /**< Array of labels */
    int count;                  /**< Array logical dimention */
    int selected;               /**< Index of currently selected label in the array. -1 if no one is selected */
    int copied;                 /**< Index of currenntly copied label in the array. -1 if no one is selected */
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
 * Check if a point of coordinates (x, y) is inside any label in labels
 * and eventually select it, saving the index in labels.selected.
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
 * background (ALPHA) of the same color of the border.
 * @param img Destination image
 * @param corner1 Corner of the label
 * @param corner2 Opposite corner of the label
 * @param color Color of the label
 * @param fill Fill option
 */
void draw_label(IplImage *img, CvPoint corner1, CvPoint corner2, CvScalar color, bool fill);

/**
 * Save a new label in the labels array with given arguments.
 * @param labels
 * @param center Central coordinates of the new label
 * @param width Width of the new label
 * @param height Height of the new label
 * @return true in case of success, false when labels array is full
 */
bool create_label(labels *labels, CvPoint center, int width, int height);

/**
 * Save a copy of currently copied label in the labels array.
 * @param labels
 * @return true in case of success, false when labels array is full
 */
bool paste_label(labels *labels);

/**
 * Delete currently selected labels from the labels array.
 * @param labels
 */
bool delete_label(labels *labels);

/**
 * Reset struct labels parameters
 * @param labels
 */
void reset(labels *labels);

void load_labels(char *filename, char *imagename, labels *labels);

void save_labels(char *filename, char *imagename, labels labels);

#endif

