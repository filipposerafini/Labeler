#include <dirent.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <opencv/highgui.h>
#include "Label.h"

#define MAX_FILE_NAME_LENGTH 128
#define WINDOW_NAME "Image"
 
typedef struct {
	labels labels;
    CvPoint corner;
    CvPoint opposite_corner;
    bool drawing;
    bool moving;
    IplImage *img;
} labels_data;

void on_mouse(int event, int x, int y, int, void *data); 
void update_image(labels_data *data);

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
    sprintf(dest_dir, "%s/Labelled", src_dir);

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
                printf("Appending output to file %s\n", outfile);                append = true;
                fclose(file);
                break;
            }
        } while (true);
        getchar();
    } else
        printf("Writing output to new file %s\n", outfile);

    static labels_data data;

    data.labels.count = 0;
    data.labels.selected = -1;
    data.labels.copied = -1;

    cvNamedWindow(WINDOW_NAME, CV_WINDOW_NORMAL);
    cvSetMouseCallback(WINDOW_NAME, on_mouse, &data);
    
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
        data.img = cvLoadImage(src_img, CV_LOAD_IMAGE_COLOR);
        if (!data.img) {
            printf("Could not load image file: %s\n", src_img);
            continue;
        }
        printf("Loaded %s: %dx%d, depth: %d, nChannels: %d\n", src_img, data.img->width, data.img->height, data.img->depth, data.img->nChannels);
        free(src_img);
        
        cvShowImage(WINDOW_NAME, data.img);
        
        // Search and load labels from file
        if (append) {
            load_labels(outfile, dd->d_name, &data.labels);
            update_image(&data);
        }
        
        // Work on current image
        bool end = false;
        while (!end) {
            if (data.drawing || data.moving) {
                update_image(&data);
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
                    if (data.labels.selected >= 0) {
                        data.labels.copied = data.labels.selected; 
                        printf("Label %d copied\n", data.labels.copied);
                    }
                    break;
                // p = Paste label
                case 'p':
                    if (data.labels.copied >= 0) {
                        if (paste_label(&data.labels) == true) {
                            printf("Label %d pasted -> ", data.labels.count - 1);
                            print_label(data.labels.label[data.labels.count - 1]);
                            update_image(&data);
                        } else
                            printf("Label not pasted: too many labels\n");
                    }
                    break;
                // d = Debug print
                case 'd':
                    debug_print(data.labels);
                    break;
                // ARROWS //
                // Up
                case 82:
                    if (data.labels.selected >= 0) {
                        data.labels.label[data.labels.selected].center.y--;
                        update_image(&data);
                    }
                    break;
                // Down
                case 84:
                    if (data.labels.selected >= 0) {
                        data.labels.label[data.labels.selected].center.y++;
                        update_image(&data);
                    }
                    break;
                // Left
                case 81:
                    if (data.labels.selected >= 0) {
                        data.labels.label[data.labels.selected].center.x--;
                        update_image(&data);
                    }
                    break;
                // Right
                case 83:
                    if (data.labels.selected >= 0) {
                        data.labels.label[data.labels.selected].center.x++;
                        update_image(&data);
                    }
                    break;
                // Backspace: Delete selected label
                case 8 :
                    if (data.labels.selected >= 0) {
                        delete_label(&data.labels);
                        update_image(&data);
                    }
                    break;
                // r: Reset current image
                case 'r':
                    data.labels.count = 0;
                    data.labels.selected = -1;
                    data.labels.copied = -1;
                    update_image(&data);
                    /*cvShowImage(WINDOW_NAME, data.img);*/
                    break;
                default:
                    break;
            }
        }
        
        // Save labels to file
        if (data.labels.count > 0) {
            if (mkdir(dest_dir, 0755) == 0)
                printf("Created destination folder for labelled image: %s\n", dest_dir);
            save_labels(tmpfile, dd->d_name, data.labels);
        }
        cvReleaseImage(&data.img);
        
        // Reset label count
        data.labels.count = 0;
        data.labels.selected = -1;
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

// Mouse callback handler
void on_mouse(int event, int x, int y, int i, void *param) {
    labels_data* data = param;
    switch (event) {
        case CV_EVENT_LBUTTONDOWN:
            if (!data->moving) {
                if (data->labels.selected >= 0) {
                    data->labels.label[data->labels.selected].selected = false;
                    data->labels.selected = -1;
                }
                data->corner = cvPoint(x, y);
                data->opposite_corner = cvPoint(x, y);
                data->drawing = true;
            }
            break;
        case CV_EVENT_LBUTTONUP:
            data->drawing = false;
            data->opposite_corner = cvPoint(x, y);
            // Save new label
            if (data->opposite_corner.x != data->corner.x && data->opposite_corner.y != data->corner.y) {
                CvPoint center;
                int width, height;
                center = cvPoint((data->corner.x + data->opposite_corner.x)/2, (data->corner.y + data->opposite_corner.y)/2);
                width = abs(center.x - data->corner.x);
                height = abs(center.y - data->corner.y);
                if (create_label(&data->labels, center, width, height)) {
                    printf("Label %d -> ", data->labels.count - 1);
                    print_label(data->labels.label[data->labels.count - 1]);
                } else
                    printf("Label not saved: Too many labels\n");
            }
            break;
        case CV_EVENT_RBUTTONDOWN:
            if (select_label(x, y, &data->labels)) {
                printf("Selected label %d\n", data->labels.selected);
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
            else if (data->moving && data->labels.selected >= 0)
                data->labels.label[data->labels.selected].center = cvPoint(x, y);
            break;
        default: break;
    }
    return;
}

// Refresh the image 'img' redrawing the lables in 'labels'
void update_image(labels_data *data) {

    CvScalar color = cvScalar(255, 0, 0, 0);
    CvScalar color_selected = cvScalar(0, 0, 255, 0);

    IplImage *tmp = cvCreateImage(cvSize(data->img->width, data->img->height), data->img->depth, data->img->nChannels);
    cvCopy(data->img, tmp, NULL);

    if (data->drawing && data->labels.count < MAX_LABELS)
        draw_label(tmp, data->corner, data->opposite_corner, color, true);
    for (int i = 0; i < data->labels.count; i++) {
        CvPoint corner1 = cvPoint(data->labels.label[i].center.x + data->labels.label[i].width, data->labels.label[i].center.y + data->labels.label[i].height);
        CvPoint corner2 = cvPoint(2 * data->labels.label[i].center.x - corner1.x, 2 * data->labels.label[i].center.y - corner1.y);
        data->labels.label[i].selected ? draw_label(tmp, corner1, corner2, color_selected, true) : draw_label(tmp, corner1, corner2, color, false);
    }
    cvShowImage(WINDOW_NAME, tmp);
    cvReleaseImage(&tmp);
    return;
}

