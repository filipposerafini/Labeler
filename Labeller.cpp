#include "Label.h"

bool drawing = false;
bool moving = false;
bool max = false;
int label_count = 0;
int selected = -1;
CvPoint corner;
CvPoint opposite_corner;
CvScalar color = cvScalar(255, 0, 0);
CvScalar colorSelected = cvScalar(0, 0, 255);
label labels[MAX_LABELS];

void on_mouse(int event, int x, int y, int, void*); 
void update_image(IplImage *img, label *list, int count);

int main(int argc, char *argv[]) {
    
    char src_dir[MAX_FILE_NAME_LENGTH];
    // Check arguments:
    if (argc > 2) {
        printf("Usage: %s [source folder] \n", argv[0]);
        exit(EXIT_FAILURE);
    } else if (argc == 2) {
        strcpy(src_dir, argv[1]);
    } else {
        do {
            printf("Type source folder: ");
            if (fgets(src_dir, sizeof(src_dir), stdin) == NULL)
                printf("Error while reading from stdin\n");
        } while (strlen(src_dir) <= 1);
        src_dir[strlen(src_dir) - 1] = '\0';
    }
    
    // Check source image directory existence
    DIR *dir;
    if ((dir = opendir(src_dir)) == NULL) {
        printf("Could not open directory %s\n", src_dir);
        exit(EXIT_FAILURE);
    }

    // Create destination folder for labelled images
    char *dest_dir = (char*)malloc(strlen(src_dir) + 9);
    strcat(dest_dir, "/Labelled");

    // Ask for output file name
    char outfile[MAX_FILE_NAME_LENGTH];
    do {
        printf("Type output file name: ");
        if (fgets(outfile, sizeof(outfile) - 4, stdin) == NULL)
            printf("Error while reading from stdin\n");
    } while (strlen(outfile) <= 1);

    outfile[strlen(outfile) - 1] = '\0';
    strcat(outfile, ".csv");

    FILE *file;
    bool append = false;
    char ch;

    // Manage already existing file
    if ((file = fopen(outfile, "r")) != NULL) {
        do {
            printf("File %s already exists. Do you want to Overwrite it or to Append the output? (O/A) ", outfile);
            ch = getchar();
            if (ch == 'O') {
                fclose(file);
                if (remove(outfile) != 0) {
                    printf("Error removing file %s\n", outfile);
                    exit(EXIT_FAILURE);
                } else
                    printf("Overwriting file %s\n", outfile);
                break;
            } else if (ch == 'A') {
                printf("Appending output to file %s\n", outfile);
                append = true;
                fclose(file);
                break;
            }
        } while (true);
        getchar();
    } else
        printf("Writing output to new file %s\n", outfile);

    cvNamedWindow(WINDOW_NAME, CV_WINDOW_NORMAL);
    cvSetMouseCallback(WINDOW_NAME, on_mouse);
    
    struct dirent *dd;
    char tmpfile[8] = "tmpfile";
    bool next = true;

    // Cycle trought image in source directory
    while ((dd = readdir(dir)) != NULL && next) {
        
        char *p, *src_img;
        // Analize only .jpg/.png files
        if ((p = strrchr(dd->d_name, '.')) == NULL) {
            printf("Skipped file: %s\n", dd->d_name);
            continue;
        }
        else {
            if (!strcmp(p, ".jpg") || !strcmp(p, ".JPG") || !strcmp(p, ".png") || !strcmp(p, ".PNG")) {
                src_img = (char*)malloc(strlen(dd->d_name) + strlen(src_dir) + 1);
                sprintf(src_img, "%s/%s", src_dir, dd->d_name);
                printf("Opening file: %s\n", dd->d_name);
            }
            else {
                printf("Skipped file: %s\n", dd->d_name);
                continue;
            }
        }
        
        // Load Image
        IplImage *img = cvLoadImage(src_img);
        if (!img) {
            printf("Could not load image file: %s\n", src_img);
            continue;
        }
        printf("Loaded %s: %dx%d, depth: %d, nChannels: %d\n", src_img, img->width, img->height, img->depth, img->nChannels);
        free(src_img);
        
        cvShowImage(WINDOW_NAME, img);
        
        // Search and load labels from file
        if (append) {
            load_labels(outfile, dd->d_name, labels, &label_count);
            update_image(img, labels, label_count);
        }
        
        // Work on current image
        bool end = false;
        int copied = -1;
        while (!end) {
            //update_image(img, labels, label_count);
            if (drawing || moving)
                update_image(img, labels, label_count);
            else if (max) {
                update_image(img, labels, label_count);
                max = false;
            }
            char ch = cvWaitKey(10);
            switch (ch) {
                // q = Terminate program
                case 'q':
                    end = true;
                    next = false;
                    break;
                // Enter: Next image
                case 10:
                    end = true;
                    break;
                // c = Copy selected label
                case 'c':
                    if (selected >= 0) {
                        copied = selected; 
                        printf("Label %d copied\n", copied);
                    }
                    break;
                // p = Paste label
                case 'p':
                    if (copied >= 0) {
                        if (label_count < MAX_LABELS) {
                            memcpy(&labels[label_count], &labels[copied], sizeof(label));
                            labels[label_count].selected = false;
                            printf("Label %d pasted -> ", label_count);
                            print_label(labels[label_count]);
                            label_count++;
                            update_image(img, labels, label_count);
                        } else
                            printf("Label not pasted: too many labels\n");
                    }
                    break;
                // d = Debug print
                case 'd':
                    debug_print(labels, label_count, selected, copied);
                    break;
                // ARROWS //
                // Up
                case 82:
                    if (selected >= 0) {
                        labels[selected].center.y--;
                        update_image(img, labels, label_count);
                    }
                    break;
                // Down
                case 84:
                    if (selected >= 0) {
                        labels[selected].center.y++;
                        update_image(img, labels, label_count);
                    }
                    break;
                // Left
                case 81:
                    if (selected >= 0) {
                        labels[selected].center.x--;
                        update_image(img, labels, label_count);
                    }
                    break;
                // Right
                case 83:
                    if (selected >= 0) {
                        labels[selected].center.x++;
                        update_image(img, labels, label_count);
                    }
                    break;
                // Backspace: Delete selected label
                case 8 :
                    if (selected >= 0) {
                        if (selected == copied)
                            copied = -1;
                        for (int i = selected; i < label_count; i++)
                            labels[i] = labels[i + 1];
                        printf("Deleted label %d\n", selected);
                        selected = -1;
                        label_count--;
                        update_image(img, labels, label_count);
                    }
                    break;
                // r: Reset current image
                case 'r':
                    label_count = 0;
                    copied = -1;
                    selected = -1;
                    cvShowImage(WINDOW_NAME, img);
                    break;
                default:
                    break;
            }
        }
        
        // Save labels to file
        if (label_count > 0) {
            if (mkdir(dest_dir, 0755) == 0)
                printf("Created destination folder for labelled image: %s\n", dest_dir);
            save_labels(tmpfile, dd->d_name, dest_dir, labels, label_count, img, color);
        }
        cvReleaseImage(&img);
        
        // Reset label count
        label_count = 0;
        selected = -1;
    }
    cvDestroyWindow(WINDOW_NAME);
    
    // Ask to save output
    if ((file = fopen(tmpfile, "r")) != NULL) {
        fclose(file);
        do {
            printf("Do you want to save %s? (Y/n) ", outfile);
            ch = getchar();
            if (ch == 'y' || ch == 'Y') {
                if (rename(tmpfile, outfile) != 0) {
                    printf("Error renaming the temporary file\n");
                    exit(EXIT_FAILURE);
                } else
                    printf("%s saved correctly\n", outfile);
                break;
            } else if (ch == 'n' || ch == 'N') {
                if (remove(tmpfile)) {
                    printf("Error removing file %s\n", tmpfile);
                    exit(EXIT_FAILURE);
                } else
                    printf("%s not saved\n", outfile);
                break;
            }
        } while(true);
    }

    // Relese used resources
    closedir(dir);
    free(dest_dir);

    return 0;
}

// Mouse event handler
void on_mouse(int event, int x, int y, int, void*) {
    switch (event) {
        case CV_EVENT_LBUTTONDOWN:
            if (!moving) {
                if (selected >= 0) {
                    labels[selected].selected = false;
                    selected = -1;
                }
                corner = cvPoint(x, y);
                opposite_corner = cvPoint(x, y);
                drawing = true;
            }
            break;
        case CV_EVENT_LBUTTONUP:
            drawing = false;
            // Save new label
            if (opposite_corner.x != corner.x && opposite_corner.y != corner.y) {
                if (label_count < MAX_LABELS) {
                    CvPoint center = cvPoint((corner.x + opposite_corner.x)/2, (corner.y + opposite_corner.y)/2);
                    labels[label_count].center = center;
                    labels[label_count].width = abs(center.x - corner.x);
                    labels[label_count].height = abs(center.y - corner.y);
                    labels[label_count].selected = false;
                    printf("Label %d -> ", label_count);
                    print_label(labels[label_count]);
                    label_count++;
                } else {
                    printf("Label not saved: Too many labels\n");
                    max = true;
                }
            }
            break;
        case CV_EVENT_RBUTTONDOWN:
            selected = -1;
            for (int i = 0; i < label_count; i++) {
                if ((x >= labels[i].center.x - labels[i].width && x <= labels[i].center.x + labels[i].width) && 
                        (y >= labels[i].center.y - labels[i].height && y <= labels[i].center.y + labels[i].height)) {
                    if (labels[i].selected) {
                        selected = i;
                    }
                    else if (!labels[i].selected && selected < 0) {
                        selected = i;
                    }
                } else
                    labels[i].selected = false;
            }
            if (selected >= 0) {
                labels[selected].selected = true;
                printf("Selected label %d\n", selected);
            }
            if (!drawing) {
                moving = true;
            }
            break;
        case CV_EVENT_RBUTTONUP:
            moving = false;
            break;
        case CV_EVENT_MOUSEMOVE:
            if (drawing)
                opposite_corner = cvPoint(x, y);
            else if (moving && selected >= 0) {
                labels[selected].center.x = x;
                labels[selected].center.y = y;
            }
            break;
        default: break;
    }
    return;
}

//Refresh the image redrawing the lables
void update_image(IplImage *img, label *list, int count) {
    IplImage *tmp = cvCreateImage(cvSize(img->width, img->height), img->depth, img->nChannels);
    cvCopy(img, tmp, NULL);

    if (drawing)
        draw_label(tmp, corner, opposite_corner, color, true);
    for (int i = 0; i < count; i++) {
        CvPoint corner1 = cvPoint(list[i].center.x + list[i].width, list[i].center.y + list[i].height);
        CvPoint corner2 = cvPoint(2 * list[i].center.x - corner1.x, 2 * list[i].center.y - corner1.y);
        list[i].selected ? draw_label(tmp, corner1, corner2, colorSelected, true) : draw_label(tmp, corner1, corner2, color, false);
    }
    cvShowImage(WINDOW_NAME, tmp);
    cvReleaseImage(&tmp);
    return;
}

