#ifndef LABEL_H
#define LABEL_H

#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#define MAX_LABELS 64
#define WINDOW_NAME "Image"
#define ALPHA 0.4

typedef enum { false, true } bool;

typedef struct {
    CvPoint center;
    int width;
    int height;
    bool selected;
} label;

typedef struct {
    label label[MAX_LABELS];
    int count;
    int selected;
    int copied;
} labels;

typedef struct {
    labels *labels;
    CvPoint corner;
    CvPoint opposite_corner;
    bool drawing;
    bool moving;
    IplImage *img;
} data;

void on_mouse(int event, int x, int y, int, void *data); 
void update_image(data *data);

void print_label(label l);
void debug_print(labels labels);
void draw_label(IplImage *img, CvPoint corner1, CvPoint corner2, CvScalar color, bool fill);
bool create_label(labels *labels, CvPoint center, int width, int height);
bool paste_label(labels *labels, int copied);
void delete_label(labels *labels, int *selected);
void load_labels(char *filename, char *imagename, labels *labels);
void save_labels(char *filename, char *imagename, char *dest_dir, labels labels, IplImage *img);

#endif

