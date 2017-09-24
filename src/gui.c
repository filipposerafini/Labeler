#include "gui.h"

// Initialize struct 'data'
void init_data(data *data, GtkBuilder *builder) {
    init_gui_elements(&data->elements, builder);
    reset(&data->labels);
    reset_classes(&data->labels);
    data->drawing = false;
    data->moving = false;
    data->img = NULL;
    data->tmp = NULL;
    data->selected_file = NULL;
    data->selected_folder = NULL;
    data->dirlist = NULL;
    data->name = NULL;
    data->dir_count = 0;
    data->dir_position = -1;
}

// Initialize all elements in 'elements'
void init_gui_elements(gui_elements *elements, GtkBuilder *builder) {
    elements->main_window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    elements->mi_open = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_open"));
    elements->mi_save = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_save"));
    elements->mi_reset = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_reset"));
    elements->mi_edit = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_edit"));
    elements->mi_move = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_move"));
    elements->mi_copy = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_copy"));
    elements->mi_paste = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_paste"));
    elements->mi_delete = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_delete"));
    elements->mi_classes = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_classes"));
    elements->menu_classes = GTK_MENU(gtk_builder_get_object(builder, "menu_classes"));
    elements->mi_view = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_view"));
    elements->mi_print = GTK_MENU_ITEM(gtk_builder_get_object(builder, "mi_print"));
    elements->event_box = GTK_WIDGET(gtk_builder_get_object(builder, "event_box"));
    elements->image = GTK_IMAGE(gtk_builder_get_object(builder, "image"));
    elements->btn_next = GTK_BUTTON(gtk_builder_get_object(builder, "btn_next"));
    elements->open_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "open_dialog"));
    elements->btn_cancel = GTK_BUTTON(gtk_builder_get_object(builder, "btn_cancel"));
    elements->btn_open = GTK_BUTTON(gtk_builder_get_object(builder, "btn_open"));
    elements->class_list = GTK_LIST_BOX(gtk_builder_get_object(builder, "class_list"));
    elements->entry_class = GTK_ENTRY(gtk_builder_get_object(builder, "entry_class"));
    elements->btn_add_class = GTK_BUTTON(gtk_builder_get_object(builder, "btn_add_class"));
    elements->save_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "save_dialog"));
}

// Show 'img' in 'image', resizing it to fit in container 'widget' keeping original ratio.
void show_image(IplImage *img, GtkImage *image, GtkWidget *widget) {
    float ratio;
    // Create pixbuf from IplImage data
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data((guchar *)img->imageData,
            GDK_COLORSPACE_RGB,FALSE,img->depth,img->width,img->height,img->widthStep,NULL,NULL);
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    // Scale pixbuf to fit in container
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
    pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(image, pixbuf);
    g_object_unref(pixbuf);
}

// Open and show next image in current directory, accepting only .jpg/.png formats.
// Returns true on success, false when there isn't any other valid image in current directory
bool open_next_image(data *data) {
    struct dirent *dd = NULL;
    char *p, *src_img;

    // Cycle through directory looking for valid image
    while (data->dir_position < data->dir_count - 1) {
        if (data->dir_position >= 0)
            free(data->dirlist[data->dir_position]);
        // Move to next dirent
        data->dir_position++;
        dd = data->dirlist[data->dir_position];
        src_img = (char*)malloc(strlen(dd->d_name) + strlen(data->selected_folder) + 2);
        sprintf(src_img, "%s/%s", data->selected_folder, dd->d_name);
        IplImage *test = cvLoadImage(src_img, CV_LOAD_IMAGE_COLOR);
        // Check if is valid image
        if (test != NULL) {
            cvReleaseImage(&test);
            g_message("Opening file: %s", dd->d_name);
            break;
        }
        else {
            g_debug("Skipped file: %s", dd->d_name);
            dd = NULL;
            continue;
        }
    }

    if (dd == NULL)
        // No valid image found
        return false;
    else {
        // Handle file name
        data->name = (char*)realloc(data->name, strlen(dd->d_name) - 3);
        strncpy(data->name, dd->d_name, strlen(dd->d_name) - 4);
        data->name[strlen(dd->d_name) - 4] = '\0';
        // Release previous img allocation
        if (data->img != NULL)
            cvReleaseImage(&data->img);
        // Re-initialize labels informations
        reset(&data->labels);
        // Load new image
        data->img = cvLoadImage(src_img, CV_LOAD_IMAGE_COLOR); 
        // Release previous tmp allocation
        if (data->tmp != NULL)
            cvReleaseImage(&data->tmp);
        data->tmp = cvCreateImage(cvSize(data->img->width, data->img->height), data->img->depth, data->img->nChannels);
        // Convert from BGR (opencv) to RGB (gtk)
        cvCvtColor(data->img, data->img, CV_BGR2RGB);
        // Load labels from file
        if (data->selected_file != NULL)
            load_labels(data->selected_file, data->name, &data->labels);
        free(src_img);
        return true;
    }
}

// Refresh the image drawing current and previous labels
void update_image(data *data) {

    // Create a copy of original image
    cvCopy(data->img, data->tmp, NULL);

    // Draw all labels
    if (data->drawing && data->labels.count < MAX_LABELS)
        draw_label(data->tmp, data->corner, data->opposite_corner, select_color(data->labels.selected_class), true);
    for (int i = 0; i < data->labels.count; i++) {
        CvPoint corner1 = cvPoint(data->labels.label[i].center.x + data->labels.label[i].width, data->labels.label[i].center.y + data->labels.label[i].height);
        CvPoint corner2 = cvPoint(2 * data->labels.label[i].center.x - corner1.x, 2 * data->labels.label[i].center.y - corner1.y);
        if (data->labels.label[i].selected)
            draw_label(data->tmp, corner1, corner2, select_color(-1), true);
        else
            draw_label(data->tmp, corner1, corner2, select_color(data->labels.label[i].class), false);
    }
    // Show the updated image
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

    // Calculate image relative coordinates
    *x = (pointer_x - (allocation.width - width)/2) / xrap;
    *y = (pointer_y - (allocation.height - height)/2) / yrap;

    // Check if point is inside the image
    if (*x < 0 || *x > img->width ||*y < 0 || *y > img->height)
        return false;
    else
        return true;
}

// Rename 'tmpfile' to 'filename'
void save(char *tmpfile, char *filename) {
    FILE *file;
    if ((file = fopen(tmpfile, "r")) != NULL) {
        fclose(file);
        // File will have '.csv' extension
        char *file;
        char *p = strrchr(filename, '.');
        if (p != NULL && !strcmp(p, ".csv"))
            file = filename;
        else
        {
            file = (char*)malloc(sizeof(filename) + 4);
            sprintf(file, "%s.csv", filename);
        }
        // Rename tempfile that becomes the output file
        if (rename(tmpfile, file) != 0) {
            g_error("Error renaming the temporary file\n");
            exit(EXIT_FAILURE);
        } else
            g_message("%s saved correctly", file);
    }
}

