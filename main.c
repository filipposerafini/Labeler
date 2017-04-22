#include "Label.h"

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
                printf("Appending output to file %s\n", outfile);
                append = true;
                fclose(file);
                break;
            }
        } while (true);
        getchar();
    } else
        printf("Writing output to new file %s\n", outfile);

    static labels_data labelsdata;

    labelsdata.count = 0;
    labelsdata.drawing = false;
    labelsdata.moving = false;
    labelsdata.max = false;
    labelsdata.selected = -1;
    labelsdata.copied = -1;

    cvNamedWindow(WINDOW_NAME, CV_WINDOW_NORMAL);
    cvSetMouseCallback(WINDOW_NAME, on_mouse, &labelsdata);
    
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
        IplImage *img = cvLoadImage(src_img, CV_LOAD_IMAGE_COLOR);
        if (!img) {
            printf("Could not load image file: %s\n", src_img);
            continue;
        }
        printf("Loaded %s: %dx%d, depth: %d, nChannels: %d\n", src_img, img->width, img->height, img->depth, img->nChannels);
        free(src_img);
        
        cvShowImage(WINDOW_NAME, img);
        
        // Search and load labels from file
        if (append) {
            load_labels(outfile, dd->d_name, &labelsdata);
            update_image(img, &labelsdata);
        }
        
        // Work on current image
        bool end = false;
        labelsdata.copied = -1;
        while (!end) {
            //update_image(img, labels, count);
            if (labelsdata.drawing || labelsdata.moving)
                update_image(img, &labelsdata);
            else if (labelsdata.max) {
                update_image(img, &labelsdata);
                labelsdata.max = false;
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
                    if (labelsdata.selected >= 0) {
                        labelsdata.copied = labelsdata.selected; 
                        printf("Label %d copied\n", labelsdata.copied);
                    }
                    break;
                // p = Paste label
                case 'p':
                    if (labelsdata.copied >= 0) {
                        if (labelsdata.count < MAX_LABELS) {
                            memcpy(&labelsdata.l[labelsdata.count], &labelsdata.l[labelsdata.copied], sizeof(label));
                            labelsdata.l[labelsdata.count].selected = false;
                            printf("Label %d pasted -> ", labelsdata.count);
                            print_label(labelsdata.l[labelsdata.count]);
                            labelsdata.count++;
                            update_image(img, &labelsdata);
                        } else
                            printf("Label not pasted: too many labels\n");
                    }
                    break;
                // d = Debug print
                case 'd':
                    debug_print(labelsdata);
                    break;
                // ARROWS //
                // Up
                case 82:
                    if (labelsdata.selected >= 0) {
                        labelsdata.l[labelsdata.selected].center.y--;
                        update_image(img, &labelsdata);
                    }
                    break;
                // Down
                case 84:
                    if (labelsdata.selected >= 0) {
                        labelsdata.l[labelsdata.selected].center.y++;
                        update_image(img, &labelsdata);
                    }
                    break;
                // Left
                case 81:
                    if (labelsdata.selected >= 0) {
                        labelsdata.l[labelsdata.selected].center.x--;
                        update_image(img, &labelsdata);
                    }
                    break;
                // Right
                case 83:
                    if (labelsdata.selected >= 0) {
                        labelsdata.l[labelsdata.selected].center.x++;
                        update_image(img, &labelsdata);
                    }
                    break;
                // Backspace: Delete selected label
                case 8 :
                    if (labelsdata.selected >= 0) {
                        if (labelsdata.selected == labelsdata.copied)
                            labelsdata.copied = -1;
                        for (int i = labelsdata.selected; i < labelsdata.count; i++)
                            labelsdata.l[i] = labelsdata.l[i + 1];
                        printf("Deleted label %d\n", labelsdata.selected);
                        labelsdata.selected = -1;
                        labelsdata.count--;
                        update_image(img, &labelsdata);
                    }
                    break;
                // r: Reset current image
                case 'r':
                    labelsdata.count = 0;
                    labelsdata.selected = -1;
                    labelsdata.copied = -1;
                    cvShowImage(WINDOW_NAME, img);
                    break;
                default:
                    break;
            }
        }
        
        // Save labels to file
        if (labelsdata.count > 0) {
            if (mkdir(dest_dir, 0755) == 0)
                printf("Created destination folder for labelled image: %s\n", dest_dir);
            save_labels(tmpfile, dd->d_name, dest_dir, labelsdata, img);
        }
        cvReleaseImage(&img);
        
        // Reset label count
        labelsdata.count = 0;
        labelsdata.selected = -1;
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

