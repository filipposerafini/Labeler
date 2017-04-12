#include "Label.h"

#define ALPHA 0.4

// Print informations about label 'label':
// Center coordinates (x,y), width and height
void print_label(label label) {
    printf("Center: %dx%d, width: %d, height: %d\n", label.center.x, label.center.y, label.width, label.height);
}

// Print information for 'count' element in 'labels', 
// also displaying witch is 'selected' and/or 'copied'
void debug_print(label *labels, int count, int selected, int copied) {
    if (count > 0) {
        printf("\n**********************************************************************\n");
        for (int i = 0; i < count; i++) {
            printf("Label %d -> ", i);
            if (selected == i) printf("(selected) ");
            if (copied == i) printf("(copied) ");
            print_label(labels[i]);
        }
        printf("**********************************************************************\n\n");
    }
}

// Draw a rectangle in image 'image' from corner 'corner1' to corner 'corner2'
// If 'fill' is true the rectangle is filled with a transparent background with alpha='ALPHA'
void draw_label(IplImage *image, CvPoint corner1, CvPoint corner2, CvScalar color, bool fill) {
    if (fill) {
        IplImage *rect = cvCreateImage(cvSize(image->width, image->height),image->depth, image->nChannels);
        cvCopy(image, rect, NULL);
        cvRectangle(rect, corner1, corner2, color, -1, 8, 0);
        cvAddWeighted(rect, ALPHA, image, 1 - ALPHA, 0, image);
        cvReleaseImage(&rect);
    }
    cvRectangle(image, corner1, corner2, color, 3, 8, 0);
    return;
}

// Load labels for image 'imagename' from file 'filename' 
// and save that to 'labels'
void load_labels(char *filename, char *imagename, label *labels, int *count) {
    FILE *file;
    char name[64];
    int i = 0, x, y, h, w;

    // Check file existance
    if ((file = fopen(filename, "r")) == NULL) {
        printf("Could not open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Read line
    while (fscanf(file, "%[^;];%d;%d;%d;%d\n", name, &x, &y, &h, &w) != EOF) {
        if (!strcmp(name, imagename)) {
            // Save label for current image
            if (*count < MAX_LABELS) {
                CvPoint center = cvPoint(x, y);
                labels[*count].center = center;
                labels[*count].height = h;
                labels[*count].width = w;
                printf("Label %d loaded from file -> ", *count);
                print_label(labels[*count]);
                (*count)++;
            } else
                printf("Label not loaded: too many labels\n");
        }
    }

    fclose(file);
    return;
}

// Save 'count' labels, stored in 'labels', for image 'imagename' to file file 'filename'
// and save an image in 'dest_dir' with the drawed labels
void save_labels(char *filename, char *imagename, char *dest_dir, label *labels, int count, IplImage *img, CvScalar color) {
    FILE *file;
    
    // Check file existence
    if ((file = fopen(filename,"a")) == NULL) {
        printf("Error writing on file\n");
        exit(EXIT_FAILURE);
    }

    IplImage *tmp = cvCreateImage(cvSize(img->width, img->height), img->depth, img->nChannels);
    cvCopy(img, tmp, NULL);

    // Write labels to file
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s;%d;%d;%d;%d\n", imagename, labels[i].center.x, labels[i].center.y, labels[i].width, labels[i].height);
        CvPoint corner1 = cvPoint(labels[i].center.x + labels[i].width, labels[i].center.y + labels[i].height);
        CvPoint corner2 = cvPoint(2 * labels[i].center.x - corner1.x, 2 * labels[i].center.y - corner1.y);
        draw_label(tmp, corner1, corner2, color, false);
    }
    fclose(file);

    char *save_file = (char*)malloc(strlen(dest_dir) + strlen(imagename) + 1);
    sprintf(save_file, "%s/%s", dest_dir, imagename);

    // Save image
    cvSaveImage(save_file, tmp);

    cvReleaseImage(&tmp);
    free(save_file);
}

