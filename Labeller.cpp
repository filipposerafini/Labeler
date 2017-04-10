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

    // Check arguments
    if (argc < 3) {
        printf("Usage: %s directory outfile\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Check source image directory existence
    DIR *dir;
    if ((dir = opendir(argv[1])) == NULL) {
        printf("Could not open directory %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    char *dest_dir = (char*)malloc(sizeof(char) * strlen(argv[1]) + 9);
    sprintf(dest_dir, "%s/Labelled", argv[1]);
    if (mkdir(dest_dir, 0755) == 0)
        printf("Created destination folder for labelled image\n");

    // Check output file existence
    char *outfile = (char*)malloc(sizeof(char) * strlen(argv[2]) + 4);
    sprintf(outfile, "%s.csv", argv[2]);

    FILE *file;
    bool append = false;

    if ((file = fopen(outfile, "r")) != NULL) {
        printf("File %s already exists. Do you want to Overwrite it or to Append the output? (O/A) ", outfile);
        while (char ch = getchar()) {
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
            } else {
                printf("File %s already exists. Do you want to Overwrite it or to Append the output? (O/A) ", outfile);
            }
        }
    } else
        printf("Writing output to file %s\n", outfile);

    cvNamedWindow(WINDOW_NAME, CV_WINDOW_NORMAL);
    cvSetMouseCallback(WINDOW_NAME, on_mouse);

    struct dirent *dd;
    char tmpfile[8] = "tmpfile";
    bool next = true;

    // Cycle trought image in source directory
    while ((dd = readdir(dir)) != NULL && next) {
        
        char *p, *src_img;
        // Analize only .jpg files
        if ((p = strrchr(dd->d_name, '.')) == NULL) {
            printf("Skipped file: %s\n", dd->d_name);
            continue;
        }
        else {
            if (!strcmp(p, ".jpg") || !strcmp(p, ".JPG")) {
                src_img = (char*)malloc(strlen(dd->d_name) + strlen(argv[1]) + 1);
                sprintf(src_img, "%s/%s", argv[1], dd->d_name);
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
            if (drawing || moving || max) {
                update_image(img, labels, label_count);
                if (max)
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
                    print_all(labels, label_count, selected, copied);
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
        if (label_count > 0)
            save_labels(tmpfile, dd->d_name, dest_dir, labels, label_count, img, color);
        cvReleaseImage(&img);
        
        // Reset label count
        label_count = 0;
        selected = -1;
    }

    if (rename(tmpfile, outfile) != 0)
        printf("Error renaming the temporary file\n");

    // Relese used resources
    closedir(dir);
    free(outfile);
    free(dest_dir);
    cvDestroyWindow(WINDOW_NAME);

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
            //if (selected >= 0)
                //labels[selected].selected = false;
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
        draw_label(tmp, corner, opposite_corner, color);
    for (int i = 0; i < count; i++) {
        CvPoint corner1 = cvPoint(list[i].center.x + list[i].width, list[i].center.y + list[i].height);
        CvPoint corner2 = cvPoint(2 * list[i].center.x - corner1.x, 2 * list[i].center.y - corner1.y);
        list[i].selected ? draw_label(tmp, corner1, corner2, colorSelected) : draw_label(tmp, corner1, corner2, color);
    }
    cvShowImage(WINDOW_NAME, tmp);
    cvReleaseImage(&tmp);
    return;
}

