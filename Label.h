#ifndef LABEL_H
#define LABEL_H

#include <stdio.h>
#include <dirent.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#define MAX_LABELS 64
#define MAX_FILE_NAME_LENGTH 128
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
    label l[MAX_LABELS];
    int count;
    CvPoint corner;
    CvPoint opposite_corner;
    bool drawing;
    bool moving;
    bool max;
    int selected;
    int copied;
} labels_data;

void on_mouse(int event, int x, int y, int, void *labels); 
void update_image(IplImage *img, labels_data *data);
void print_label(label l);
void debug_print(labels_data data);
void draw_label(IplImage *img, CvPoint corner1, CvPoint corner2, CvScalar color, bool fill);
void load_labels(char *filename, char *imagename, labels_data *data);
void save_labels(char *filename, char *imagename, char *dest_dir, labels_data data, IplImage *img);

static CvScalar color;
static CvScalar color_selected;

#endif

