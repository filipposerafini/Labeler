#include "Label.h"

// Print informations about label 'label':
// Center coordinates (x,y), width and height
void print_label(label label) {
    printf("Center: %dx%d, width: %d, height: %d, class: %d\n", label.center.x, label.center.y, label.width, label.height, label.class);
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
        printf("**********************************************************************\n");
        for (int i = 0; i < labels.classes_count; i++) {
            printf("Class %d -> %s", i, labels.classes[i]);
            if (labels.selected_class == i) printf(" (selected)");
            printf("\n");
        }
        printf("**********************************************************************\n\n");
    }
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
}

// Save a new label with given arguments in 'labels'
// Return true in case of success and false when labels array is full
bool create_label(labels *labels, CvPoint center, int width, int height, int class) {
    if (labels->count < MAX_LABELS) {
        labels->label[labels->count].center = center;
        labels->label[labels->count].width = width;
        labels->label[labels->count].height = height;
        labels->label[labels->count].selected = false;
        labels->label[labels->count].class = class;
        labels->count++;
        return true;
    } else
        return false;
}

// Save a copy of currently copied label in 'labels'
// Return true in case of success and false when labels array is full
bool paste_label(labels *labels) {
    return create_label(labels, labels->label[labels->copied].center, labels->label[labels->copied].width, labels->label[labels->copied].height, labels->label[labels->copied].class);
}

// Delete currently selected label from 'labels'
// Return true if deleted label was the currently copied label, false otherwise
bool delete_label(labels *labels) {
    for (int i = labels->selected; i < labels->count; i++)
        labels->label[i] = labels->label[i + 1];
    labels->count--;
    if (labels->selected == labels->copied) {
        labels->selected = -1;
        labels->copied = -1;
        return true;
    } else {
        labels->selected = -1;
        return false;
    }
}

// Reset 'labels' parameters
void reset(labels *labels) {
    labels->count = 0;
    labels->selected = -1;
    labels->copied = -1;
}

// Reset classes parameters
void reset_classes(labels *labels) {
    labels->classes_count = 0;
    labels->selected_class = 0;
}

// Load labels for image 'imagename' from file 'filename'
// and save them to 'dest'
void load_labels(char *filename, char *imagename, labels *dest) {
    FILE *file;
    char name[64], class[64];
    int i = 0, x, y, h, w, c, num;

    // Check file existance
    if ((file = fopen(filename, "r")) == NULL) {
        printf("Could not open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    if (fscanf(file, "%d\n", &num) == EOF) {
        printf("Error reading file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num; i++) {
        if (fscanf(file, "%s\n", class) == EOF) {
            printf("Error reading file %s\n", filename);
            exit(EXIT_FAILURE);
        }
        add_class(class, dest);
    }

    // Read line
    while (fscanf(file, "%[^;];%d;%d;%d;%d;%d\n", name, &x, &y, &w, &h, &c) != EOF) {
        if (!strcmp(name, imagename)) {
            // Save label for current image
            if (create_label(dest, cvPoint(x,y), w, h, c)) {
                printf("Label %d loaded from file -> ", dest->count - 1);
                print_label(dest->label[dest->count - 1]);
            } else
                printf("Label not loaded: too many labels\n");
        }
    }

    fclose(file);
}

// Save every element stored in 'src' to file 'filename'
// (and save an image in 'label_dir' with the drawed labels)
// FILE FORMAT: 'imagename';x;y;width;height;class
void save_labels(char *filename, char *imagename, labels src) {
    FILE *file;
    bool new_file = false;

    if (src.count > 0) {
        // Check if is new
        if ((file = fopen(filename, "r")) == NULL)
            new_file = true;
        else
            fclose(file);

        // Open file in append mode
        if ((file = fopen(filename, "a")) == NULL) {
            printf("Error writing on file\n");
            exit(EXIT_FAILURE);
        }

        // Write classes on top of new file
        if (new_file) {
            fprintf(file, "%d\n", src.classes_count);
            for (int i = 0; i < src.classes_count; i++)
                fprintf(file, "%s\n", src.classes[i]);
        }

        // Write labels to file
        for (int i = 0; i < src.count; i++)
            fprintf(file, "%s;%d;%d;%d;%d;%d\n", imagename, src.label[i].center.x, src.label[i].center.y, src.label[i].width, src.label[i].height, src.label[i].class);
        fclose(file);
    }
}

bool add_class(char *name, labels* labels) {
    if (labels->classes_count < MAX_CLASSES) {
        for (int i = 0; i < labels->classes_count; i++) {
            if (!strcmp(labels->classes[i], name))
                return false;
        }
        strcpy(labels->classes[labels->classes_count], name);
        labels->classes_count++;
        
        printf("Class %d -> %s\n", labels->classes_count - 1, name);
        
        return true;
    } else
        return false;
}

int remove_class(char *name, labels* labels) {
    for (int i = 0; i < labels->classes_count; i++) {
        if (!strcmp(labels->classes[i], name)) {
            for (int j = i; j < labels->classes_count; j++) 
                strcpy(labels->classes[j], labels->classes[j+1]);
            labels->classes_count--;
            return i;
        }
    }
    return -1;
}
