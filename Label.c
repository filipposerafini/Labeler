#include "Label.h"

// Mouse callback handler
void on_mouse(int event, int x, int y, int i, void *param) {
    data* data = param;
    switch (event) {
        case CV_EVENT_LBUTTONDOWN:
            if (!data->moving) {
                if (data->labels->selected >= 0) {
                    data->labels->label[data->labels->selected].selected = false;
                    data->labels->selected = -1;
                }
                data->corner = cvPoint(x, y);
                data->opposite_corner = cvPoint(x, y);
                data->drawing = true;
            }
            break;
        case CV_EVENT_LBUTTONUP:
            data->drawing = false;
            // Save new label
            if (data->opposite_corner.x != data->corner.x && data->opposite_corner.y != data->corner.y) {
                CvPoint center;
                int width, height;
                center = cvPoint((data->corner.x + data->opposite_corner.x)/2, (data->corner.y + data->opposite_corner.y)/2);
                width = abs(center.x - data->corner.x);
                height = abs(center.y - data->corner.y);
                if (create_label(data->labels, center, width, height)) {
                    printf("Label %d -> ", data->labels->count - 1);
                    print_label(data->labels->label[data->labels->count - 1]);
                } else
                    printf("Label not saved: Too many data\n");
            }
            break;
        case CV_EVENT_RBUTTONDOWN:
            data->labels->selected = -1;
            for (int i = 0; i < data->labels->count; i++) {
                if ((x >= data->labels->label[i].center.x - data->labels->label[i].width && x <= data->labels->label[i].center.x + data->labels->label[i].width) && 
                        (y >= data->labels->label[i].center.y - data->labels->label[i].height && y <= data->labels->label[i].center.y + data->labels->label[i].height)) {
                    if (data->labels->label[i].selected) {
                        data->labels->selected = i;
                    }
                    else if (!data->labels->label[i].selected && data->labels->selected < 0) {
                        data->labels->selected = i;
                    }
                } else
                    data->labels->label[i].selected = false;
            }
            if (data->labels->selected >= 0) {
                data->labels->label[data->labels->selected].selected = true;
                printf("Selected label %d\n", data->labels->selected);
            }
            if (!data->drawing) {
                data->moving = true;
            }
            break;
        case CV_EVENT_RBUTTONUP:
            data->moving = false;
            break;
        case CV_EVENT_MOUSEMOVE:
            if (data->drawing)
                data->opposite_corner = cvPoint(x, y);
            else if (data->moving && data->labels->selected >= 0) {
                data->labels->label[data->labels->selected].center.x = x;
                data->labels->label[data->labels->selected].center.y = y;
            }
            break;
        default: break;
    }
    return;
}

// Refresh the image 'img' redrawing the lables in 'labels'
void update_image(data *data) {

    CvScalar color = cvScalar(255, 0, 0, 0);
    CvScalar color_selected = cvScalar(0, 0, 255, 0);

    IplImage *tmp = cvCreateImage(cvSize(data->img->width, data->img->height), data->img->depth, data->img->nChannels);
    cvCopy(data->img, tmp, NULL);

    if (data->drawing && data->labels->count < MAX_LABELS)
        draw_label(tmp, data->corner, data->opposite_corner, color, true);
    for (int i = 0; i < data->labels->count; i++) {
        CvPoint corner1 = cvPoint(data->labels->label[i].center.x + data->labels->label[i].width, data->labels->label[i].center.y + data->labels->label[i].height);
        CvPoint corner2 = cvPoint(2 * data->labels->label[i].center.x - corner1.x, 2 * data->labels->label[i].center.y - corner1.y);
        data->labels->label[i].selected ? draw_label(tmp, corner1, corner2, color_selected, true) : draw_label(tmp, corner1, corner2, color, false);
    }
    cvShowImage(WINDOW_NAME, tmp);
    cvReleaseImage(&tmp);
    return;
}

// Print informations about label 'label':
// Center coordinates (x,y), width and height
void print_label(label label) {
    printf("Center: %dx%d, width: %d, height: %d\n", label.center.x, label.center.y, label.width, label.height);
    return;
}

// Print information for every element in 'labels', 
// also displaying witch is 'selected' and/or 'copied'
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

// Draw a rectangle in image 'image' from corner 'corner1' to corner 'corner2'
// If 'fill' is true the rectangle is filled with a transparent background with alpha='ALPHA'
void draw_label(IplImage *image, CvPoint corner1, CvPoint corner2, CvScalar color, bool fill) {
    if (fill) {
        IplImage *rect = cvCreateImage(cvSize(image->width, image->height),image->depth, image->nChannels);
        cvCopy(image, rect, NULL);
        cvRectangle(rect, corner1, corner2, color, CV_FILLED, 8, 0);
        cvAddWeighted(rect, ALPHA, image, 1 - ALPHA, 0, image);
        cvReleaseImage(&rect);
    }
    cvRectangle(image, corner1, corner2, color, 3, 8, 0);
    return;
}

// Save a new label ('center', 'width', 'height') in 'labels'
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

// Save a copy of 'copied' labels in 'labels'
// Return true in case of success and false when labels array is full
bool paste_label(labels *labels, int copied) {
    return create_label(labels, labels->label[copied].center, labels->label[copied].width, labels->label[copied].height);
}

// Delete currently 'selected' labels from 'labels'
void delete_label(labels *labels, int *selected) {
    if (*selected == labels->copied)
        labels->copied = -1;
    for (int i = *selected; i < labels->count; i++)
        labels->label[i] = labels->label[i + 1];
    printf("Deleted label %d\n", labels->selected);
    *selected = -1;
    labels->count--;
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
// and save an image in 'dest_dir' with the drawed labels
// FILE FORMAT: 'imagename';x;y;width;height
void save_labels(char *filename, char *imagename, char *dest_dir, labels labels, IplImage *img) {
    FILE *file;
    CvScalar color = cvScalar(255, 0, 0, 0);

    // Check file existence
    if ((file = fopen(filename,"a")) == NULL) {
        printf("Error writing on file\n");
        exit(EXIT_FAILURE);
    }

    IplImage *tmp = cvCreateImage(cvSize(img->width, img->height), img->depth, img->nChannels);
    cvCopy(img, tmp, NULL);

    // Write labels to file
    for (int i = 0; i < labels.count; i++) {
        fprintf(file, "%s;%d;%d;%d;%d\n", imagename, labels.label[i].center.x, labels.label[i].center.y, labels.label[i].width, labels.label[i].height);
        CvPoint corner1 = cvPoint(labels.label[i].center.x + labels.label[i].width, labels.label[i].center.y + labels.label[i].height);
        CvPoint corner2 = cvPoint(2 * labels.label[i].center.x - corner1.x, 2 * labels.label[i].center.y - corner1.y);
        draw_label(tmp, corner1, corner2, color, false);
    }
    fclose(file);

    char *save_file = (char*)malloc(strlen(dest_dir) + strlen(imagename) + 1);
    sprintf(save_file, "%s/%s", dest_dir, imagename);

    // Save image
    cvSaveImage(save_file, tmp, 0);

    cvReleaseImage(&tmp);
    free(save_file);
}

