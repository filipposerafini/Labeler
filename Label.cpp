#include "Label.h"

// Print label informations
void print_label(label l) {
    printf("Center: %dx%d, width: %d, height: %d\n", l.center.x, l.center.y, l.width, l.height);
}

// Print information for all labels
void print_all(label* labels, int count, int selected, int copied) {
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

// Draw a transparent label on the image
void draw_label(IplImage *image, CvPoint corner1, CvPoint corner2, CvScalar color) {
    IplImage *rect = cvCreateImage(cvSize(image->width, image->height),image->depth, image->nChannels);
    cvCopy(image, rect, NULL);
    cvRectangle(rect, corner1, corner2, color, -1, 8, 0);
    cvAddWeighted(rect, ALPHA, image, 1 - ALPHA, 0, image);
    cvReleaseImage(&rect);
    cvRectangle(image, corner1, corner2, color, 2, 8, 0);
    return;
}

// Load labels from files
void load_labels(char *filename, char *imagename, label *labels, int *count) {
    FILE *file;
    char name[64];
    int i = 0, x, y, h, w;

    if ((file = fopen(filename, "r")) == NULL) {
        printf("Could not open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Read line
    while (fscanf(file, "%s %d;%d;%d;%d\n", name, &x, &y, &h, &w) != EOF) {
        if (!strcmp(name, imagename)) {
            // Save label for current image
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

void save_labels(char *filename, char *imagename, char* dest_dir, label *labels, int count, IplImage *img, CvScalar color) {
    FILE *file;
    if ((file = fopen(filename,"a")) == NULL) {
        printf("Error writing on file\n");
        exit(EXIT_FAILURE);
    }

    IplImage *tmp = cvCreateImage(cvSize(img->width, img->height), img->depth, img->nChannels);
    cvCopy(img, tmp, NULL);

    for (int i = 0; i < count; i++) {
        fprintf(file, "%s %d;%d;%d;%d\n", imagename, labels[i].center.x, labels[i].center.y, labels[i].width, labels[i].height);
        CvPoint corner1 = cvPoint(labels[i].center.x + labels[i].width, labels[i].center.y + labels[i].height);
        CvPoint corner2 = cvPoint(2 * labels[i].center.x - corner1.x, 2 * labels[i].center.y - corner1.y);
        draw_label(tmp, corner1, corner2, color);
    }
    fclose(file);

    char *save_file = (char*)malloc(strlen(dest_dir) + strlen(imagename) + 1);
    sprintf(save_file, "%s/%s", dest_dir, imagename);

    cvSaveImage(save_file, tmp);

    cvReleaseImage(&tmp);
    free(save_file);
}
