#include "Label.h"

// Mouse callback handler
void on_mouse(int event, int x, int y, int i, void *param) {
    labels_data* labelsdata = param;
    switch (event) {
        case CV_EVENT_LBUTTONDOWN:
            if (!labelsdata->moving) {
                if (labelsdata->selected >= 0) {
                    labelsdata->l[labelsdata->selected].selected = false;
                    labelsdata->selected = -1;
                }
                labelsdata->corner = cvPoint(x, y);
                labelsdata->opposite_corner = cvPoint(x, y);
                labelsdata->drawing = true;
            }
            break;
        case CV_EVENT_LBUTTONUP:
            labelsdata->drawing = false;
            // Save new label
            if (labelsdata->opposite_corner.x != labelsdata->corner.x && labelsdata->opposite_corner.y != labelsdata->corner.y) {
                if (labelsdata->count < MAX_LABELS) {
                    CvPoint center = cvPoint((labelsdata->corner.x + labelsdata->opposite_corner.x)/2, (labelsdata->corner.y + labelsdata->opposite_corner.y)/2);
                    labelsdata->l[labelsdata->count].center = center;
                    labelsdata->l[labelsdata->count].width = abs(center.x - labelsdata->corner.x);
                    labelsdata->l[labelsdata->count].height = abs(center.y - labelsdata->corner.y);
                    labelsdata->l[labelsdata->count].selected = false;
                    printf("Label %d -> ", labelsdata->count);
                    print_label(labelsdata->l[labelsdata->count]);
                    labelsdata->count++;
                } else {
                    printf("Label not saved: Too many labels\n");
                    labelsdata->max = true;
                }
            }
            break;
        case CV_EVENT_RBUTTONDOWN:
            labelsdata->selected = -1;
            for (int i = 0; i < labelsdata->count; i++) {
                if ((x >= labelsdata->l[i].center.x - labelsdata->l[i].width && x <= labelsdata->l[i].center.x + labelsdata->l[i].width) && 
                        (y >= labelsdata->l[i].center.y - labelsdata->l[i].height && y <= labelsdata->l[i].center.y + labelsdata->l[i].height)) {
                    if (labelsdata->l[i].selected) {
                        labelsdata->selected = i;
                    }
                    else if (!labelsdata->l[i].selected && labelsdata->selected < 0) {
                        labelsdata->selected = i;
                    }
                } else
                    labelsdata->l[i].selected = false;
            }
            if (labelsdata->selected >= 0) {
                labelsdata->l[labelsdata->selected].selected = true;
                printf("Selected label %d\n", labelsdata->selected);
            }
            if (!labelsdata->drawing) {
                labelsdata->moving = true;
            }
            break;
        case CV_EVENT_RBUTTONUP:
            labelsdata->moving = false;
            break;
        case CV_EVENT_MOUSEMOVE:
            if (labelsdata->drawing)
                labelsdata->opposite_corner = cvPoint(x, y);
            else if (labelsdata->moving && labelsdata->selected >= 0) {
                labelsdata->l[labelsdata->selected].center.x = x;
                labelsdata->l[labelsdata->selected].center.y = y;
            }
            break;
        default: break;
    }
    return;
}

// Refresh the image 'img' redrawing the lables in 'labelsdata'
void update_image(IplImage *img, labels_data *labelsdata) {

    CvScalar color = cvScalar(255, 0, 0, 0);
    CvScalar color_selected = cvScalar(0, 0, 255, 0);

    IplImage *tmp = cvCreateImage(cvSize(img->width, img->height), img->depth, img->nChannels);
    cvCopy(img, tmp, NULL);

    if (labelsdata->drawing)
        draw_label(tmp, labelsdata->corner, labelsdata->opposite_corner, color, true);
    for (int i = 0; i < labelsdata->count; i++) {
        CvPoint corner1 = cvPoint(labelsdata->l[i].center.x + labelsdata->l[i].width, labelsdata->l[i].center.y + labelsdata->l[i].height);
        CvPoint corner2 = cvPoint(2 * labelsdata->l[i].center.x - corner1.x, 2 * labelsdata->l[i].center.y - corner1.y);
        labelsdata->l[i].selected ? draw_label(tmp, corner1, corner2, color_selected, true) : draw_label(tmp, corner1, corner2, color, false);
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
void debug_print(labels_data labelsdata) {
    if (labelsdata.count > 0) {
        printf("\n**********************************************************************\n");
        for (int i = 0; i < labelsdata.count; i++) {
            printf("Label %d -> ", i);
            if (labelsdata.selected == i) printf("(selected) ");
            if (labelsdata.copied == i) printf("(copied) ");
            print_label(labelsdata.l[i]);
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

// Load labels for image 'imagename' from file 'filename' 
// and save them to 'labels'
void load_labels(char *filename, char *imagename, labels_data *labelsdata) {
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
            if (labelsdata->count < MAX_LABELS) {
                CvPoint center = cvPoint(x, y);
                labelsdata->l[labelsdata->count].center = center;
                labelsdata->l[labelsdata->count].height = h;
                labelsdata->l[labelsdata->count].width = w;
                printf("Label %d loaded from file -> ", labelsdata->count);
                print_label(labelsdata->l[labelsdata->count]);
                labelsdata->count++;
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
void save_labels(char *filename, char *imagename, char *dest_dir, labels_data labelsdata, IplImage *img) {
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
    for (int i = 0; i < labelsdata.count; i++) {
        fprintf(file, "%s;%d;%d;%d;%d\n", imagename, labelsdata.l[i].center.x, labelsdata.l[i].center.y, labelsdata.l[i].width, labelsdata.l[i].height);
        CvPoint corner1 = cvPoint(labelsdata.l[i].center.x + labelsdata.l[i].width, labelsdata.l[i].center.y + labelsdata.l[i].height);
        CvPoint corner2 = cvPoint(2 * labelsdata.l[i].center.x - corner1.x, 2 * labelsdata.l[i].center.y - corner1.y);
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

