#include "Label.h"

// Print informations about label 'label':
// Center coordinates (x,y), width and height
void print_label(label label) {
    printf("Center: %dx%d, width: %d, height: %d\n", label.center.x, label.center.y, label.width, label.height);
    return;
}

// Print information for every element in 'labels', 
// also displaying witch is selected and/or copied
void debug_print(labels labels) {
    if (labels.count > 0) {
        printf("\n**********************************************************************\n");
        for (int i = 0; i < labels.count; i++) {
            printf("Label %d -> ", i);
            if (labels.selected == i) printf("(selected) ");
            if (labels.copied == i) printf("(copied) ");
            print_label(labels.label[i]);
        }
        printf("**********************************************************************\n\n");
    }
    return;
}

// Check if a point of coorinates 'x', 'y' is the inside the label 'label'
// Retrun true if the point is inside the label, false otherwise
bool over_label(int x, int y, label label) {
    return ((x >= label.center.x - label.width && x <= label.center.x + label.width) && (y >= label.center.y - label.height && y <= label.center.y + label.height));
}

// Check if a point of coordinates 'x', 'y' is inside any a label in 'labels',
// eventually selecting it and saving the index
// Return true if a label is selected, false otherwise
bool select_label(int x, int y, labels *labels) {
    labels->selected = -1;
    for (int i = 0; i < labels->count; i++) {
        if (over_label(x, y, labels->label[i])) {                   
            if (labels->label[i].selected) {
                labels->selected = i;
            }
            else if (!labels->label[i].selected && labels->selected < 0) {
                labels->selected = i;
            }
        } else
            labels->label[i].selected = false;
    }
    if (labels->selected >= 0) {
        labels->label[labels->selected].selected = true;
        return true;
    } else 
        return false;
}

// Draw a rectangle in image 'image' from corner 'corner1' to corner 'corner2' with a colored border.
// If 'fill' is true the rectangle has a transparent background with alpha='ALPHA'
void draw_label(IplImage *image, CvPoint corner1, CvPoint corner2, CvScalar color, bool fill) {
    if (fill) {
        IplImage *rect = cvCreateImage(cvSize(image->width, image->height),image->depth, image->nChannels);
        cvCopy(image, rect, NULL);
        cvRectangle(rect, corner1, corner2, color, CV_FILLED, 8, 0);
        cvAddWeighted(rect, ALPHA, image, 1 - ALPHA, 0, image);
        cvReleaseImage(&rect);
    }
    cvRectangle(image, corner1, corner2, color, 2, 8, 0);
    return;
}

// Save a new label with given arguments in 'labels'
// Return true in case of success and false when labels array is full
bool create_label(labels *labels, CvPoint center, int width, int height) {
    if (labels->count < MAX_LABELS) {
        labels->label[labels->count].center = center;
        labels->label[labels->count].width = width;
        labels->label[labels->count].height = height;
        labels->label[labels->count].selected = false;
        labels->count++;
        return true;
    } else 
        return false;
}

// Save a copy of currently copied label in 'labels'
// Return true in case of success and false when labels array is full
bool paste_label(labels *labels) {
    return create_label(labels, labels->label[labels->copied].center, labels->label[labels->copied].width, labels->label[labels->copied].height);
}

// Delete currently selected label from 'labels'
// Return true if deleted label was the currently copied label, false otherwise
bool delete_label(labels *labels) {
    for (int i = labels->selected; i < labels->count; i++)
        labels->label[i] = labels->label[i + 1];
    labels->selected = -1;
    labels->count--;
    if (labels->selected == labels->copied) {
        labels->copied = -1;
        return true;
    } else
        return false;
}

// Reset 'labels' parameters
void reset(labels *labels) {
    labels->count = 0;
    labels->selected = -1;
    labels->copied = -1;
}

// Load labels for image 'imagename' from file 'filename' 
// and save them to 'labels'
void load_labels(char *filename, char *imagename, labels *labels) {
    FILE *file;
    char name[64];
    int i = 0, x, y, h, w;

    // Check file existance
    if ((file = fopen(filename, "r")) == NULL) {
        printf("Could not open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Read line
    while (fscanf(file, "%[^;];%d;%d;%d;%d\n", name, &x, &y, &w, &h) != EOF) {
        if (!strcmp(name, imagename)) {
            // Save label for current image
            if (create_label(labels, cvPoint(x,y), w, h)) {
                printf("Label %d loaded from file -> ", labels->count - 1);
                print_label(labels->label[labels->count - 1]);
            } else
                printf("Label not loaded: too many labels\n");
        }
    }

    fclose(file);
    return;
}

// Save every element stored in 'labels' to file 'filename'
// (and save an image in 'label_dir' with the drawed labels)
// FILE FORMAT: 'imagename';x;y;width;height
void save_labels(char *filename, char *imagename, labels labels) {
    FILE *file;

    // Open file in append mode
    if (labels.count > 0) {
        if ((file = fopen(filename, "a")) == NULL) {
            printf("Error writing on file\n");
            exit(EXIT_FAILURE);
        }

        // Write labels to file
        for (int i = 0; i < labels.count; i++) {
            fprintf(file, "%s;%d;%d;%d;%d\n", imagename, labels.label[i].center.x, labels.label[i].center.y, labels.label[i].width, labels.label[i].height);
        }
        fclose(file);
    }
}

