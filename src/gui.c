#include "gui.h"

// Initialize struct 'data'
void init_data(data *data, GtkBuilder *builder) {
    init_gui_elements(&data->elements, builder);
    reset(&data->labels);
    data->drawing = false;
    data->moving = false;
    data->control = false;
    data->img = NULL;
    data->tmp = NULL;
    data->selected_file = NULL;
    data->selected_folder = NULL;
    data->dir = NULL;
    data->name = NULL;
    data->tmpfile = "out/tmpfile";
}

// Initialize all elements in 'elements'
void init_gui_elements(gui_elements *elements, GtkBuilder *builder) {
    elements->main_window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    elements->mi_reset = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_reset"));
    elements->mi_edit = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_edit"));
    elements->mi_copy = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_copy"));
    elements->mi_paste = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_paste"));
    elements->mi_delete = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_delete"));
    elements->mi_print = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_print"));
    elements->event_box = GTK_WIDGET(gtk_builder_get_object(builder, "event_box"));
    elements->image = GTK_IMAGE(gtk_builder_get_object(builder, "image"));
    elements->btn_next = GTK_BUTTON(gtk_builder_get_object(builder, "btn_next"));
    elements->open_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "open_dialog"));
    elements->btn_cancel = GTK_BUTTON(gtk_builder_get_object(builder, "btn_cancel"));
    elements->btn_open = GTK_BUTTON(gtk_builder_get_object(builder, "btn_open"));
    elements->save_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "save_dialog"));

    gtk_widget_add_events(GTK_WIDGET(elements->event_box), GDK_KEY_PRESS_MASK);
}

// Show 'img' in 'image', resizing it to fit in container 'widget' keeping original ratio.
void show_image(IplImage *img, GtkImage *image, GtkWidget *widget) {
    float ratio;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data((guchar *)img->imageData,GDK_COLORSPACE_RGB,FALSE,img->depth,img->width,img->height,img->widthStep,NULL,NULL);
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    if (width > (allocation.width - 10)) {
        ratio = ((float)allocation.width - 10) / width;
        width = width * ratio;
        height = height * ratio;
    }
    if (height > (allocation.height - 90)) {
        ratio = ((float) allocation.height - 90) / height;
        width = width * ratio;
        height = height * ratio;
    }
    // Scale pixbuf to fit in container
    pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(image, pixbuf);
    g_object_unref(pixbuf);
}

// Open and show next image in current directory, accepting only .jpg/.png formats.
// Returns true on success, false when there isn't any other valid image in current directory
bool open_next_image(data *data) {
    struct dirent *dd;
    char *p, *src_img;

    // Cycle through directory looking for valid image
    while ((dd = readdir(data->dir)) != NULL) {
        if ((p = strrchr(dd->d_name, '.')) == NULL) {
            g_debug("Skipped file: %s\n", dd->d_name);
            continue;
        }
        else {
            if (!strcmp(p, ".jpg") || !strcmp(p, ".JPG") || !strcmp(p, ".png") || !strcmp(p, ".PNG")) {
                data->name = (char*)realloc(data->name, strlen(dd->d_name) - 4);
                strncpy(data->name, dd->d_name, strlen(dd->d_name) - 4);
                src_img = (char*)malloc(strlen(dd->d_name) + strlen(data->selected_folder) + 1);
                sprintf(src_img, "%s/%s", data->selected_folder, dd->d_name);
                g_message("Opening file: %s\n", data->name);
                break;
            }
            else {
                g_debug("Skipped file: %s\n", dd->d_name);
                continue;
            }
        }
    }
    
    if (dd == NULL)
        // No valid image found
        return false;
    else {
        if (data->img != NULL)
            cvReleaseImage(&data->img);
            
        reset(&data->labels);
        // Load image
        data->img = cvLoadImage(src_img, CV_LOAD_IMAGE_COLOR);
        if (!data->img) {
            g_error("Failed to load image %s", src_img);
            exit(EXIT_FAILURE);
        }
        data->tmp = cvCreateImage(cvSize(data->img->width, data->img->height), data->img->depth, data->img->nChannels);
        // Convert from OPENCV BGR to GTK RGB
        cvCvtColor(data->img, data->img, CV_BGR2RGB);
        if (data->selected_file != NULL)
            load_labels(data->selected_file, data->name, &data->labels);
        free(src_img);
        return true;
    }
}

// Refresh the image drawing current and previous labels
void update_image(data *data) {
    CvScalar color = cvScalar(0, 0, 255, 0);
    CvScalar color_selected = cvScalar(255, 0, 0, 0);

    // Create a copy of original image
    cvCopy(data->img, data->tmp, NULL);

    // Draw all labels
    if (data->drawing && data->labels.count < MAX_LABELS)
        draw_label(data->tmp, data->corner, data->opposite_corner, color, true);
    for (int i = 0; i < data->labels.count; i++) {
        CvPoint corner1 = cvPoint(data->labels.label[i].center.x + data->labels.label[i].width, data->labels.label[i].center.y + data->labels.label[i].height);
        CvPoint corner2 = cvPoint(2 * data->labels.label[i].center.x - corner1.x, 2 * data->labels.label[i].center.y - corner1.y);
        if (data->labels.label[i].selected)
            draw_label(data->tmp, corner1, corner2, color_selected, true);
        else
            draw_label(data->tmp, corner1, corner2, color, false);
    }
    show_image(data->tmp, data->elements.image, data->elements.main_window);
}

// Convert from coordinates ('pointer_x', 'pointer_y') relative to the container 'widget' 
// to coordinates ('*x', '*y') relative to 'img'. 
// The function use 'image' to get effective dimention of pixbuf if it was resized.
bool convert_coordinates(float pointer_x, float pointer_y, GtkWidget *widget, int *x, int *y, IplImage *img, GtkImage *image) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(image);
    float width = gdk_pixbuf_get_width(pixbuf);
    float height = gdk_pixbuf_get_height(pixbuf);
    float xrap = width / img->width;    
    float yrap = height / img->height;

    *x = (pointer_x - (allocation.width - width)/2) / xrap;
    *y = (pointer_y - (allocation.height - height)/2) / yrap;
    if (*x < 0 || *x > img->width ||*y < 0 || *y > img->height)
        return false;
    else
        return true;
}

// Create a folder in out/ named 'folder_name' and place in it a .csv file
// with all label imformations
void save(char *tmpfile, char *folder_name) {
    FILE *file;
    char *p = strrchr(folder_name, '/');
    char *destfolder = (char*)malloc(sizeof(p) + 4);
    sprintf(destfolder, "out/%s",++p);
    char *outfile = (char*)malloc(strlen(destfolder) + sizeof(p) + 5);
    sprintf(outfile, "%s/%s.csv", destfolder, p);
    if ((file = fopen(tmpfile, "r")) != NULL) {
        fclose(file);
        if (mkdir(destfolder, 0755) == 0)
            g_message("Created folder %s", destfolder);
        if (rename(tmpfile, outfile) != 0) {
            g_error("Error renaming the temporary file\n");
            exit(EXIT_FAILURE);
        } else
            g_message("%s saved correctly\n", outfile);
    }
    free(destfolder);
    free(outfile);
}

