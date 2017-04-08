#include "Label.h"

void draw_label(IplImage* image, CvPoint corner1, CvPoint corner2, CvScalar color) {
    IplImage* rect = cvCreateImage(cvSize(image->width, image->height),image->depth, image->nChannels);
    cvCopy(image, rect, NULL);
    cvRectangle(rect, corner1, corner2, color, -1, 8, 0);
    double alpha = 0.5;
    cvAddWeighted(rect, alpha, image, 1 - alpha, 0, image);
    cvReleaseImage(&rect);
    cvRectangle(image, corner1, corner2, color, 2, 8, 0);
    return;
}

void load_labels(char* filename, char* imagename, label* labels, int* count) {
    FILE* file;
    char name[64];
    int i = 0, x, y, h, w;

    if ((file = fopen(filename, "r")) == NULL) {
        printf("Could not open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    while (fscanf(file, "%s %d;%d;%d;%d\n", name, &x, &y, &h, &w) != EOF) {
        if (!strcmp(name, imagename)) {
            if (*count < MAX_LABELS) {
                printf("Label %d loaded from file: Center: %d,%d, height: %d, width: %d\n", *count, x, y, h, w);
                CvPoint center = cvPoint(x, y);
                labels[*count].center = center;
                labels[*count].height = h;
                labels[*count].width = w;
                (*count)++;
            } else
                printf("Label not loaded: too many labels\n");
        }
    }

    fclose(file);
    return;
}

