#include <stdio.h>
#include <dirent.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#define MAX_LABELS 64
#define WINDOW_NAME "Image"
#define ALPHA 0.5

typedef struct {
    CvPoint center;
    int width;
    int height;
    bool selected;
} label;

void draw_label(IplImage* img, CvPoint corner1, CvPoint corner2, CvScalar color);
void load_labels(char* filename, char* imagename, label* labels, int* count);
void save_labels(char* filename, char* imagename, char* dest_dir, label* labels, int count, IplImage *img, CvScalar color);
