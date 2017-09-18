#include "gui.h"

int main(int argc, char *argv[]) {
    data data;

    gtk_init(&argc, &argv);

    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "src/labeler.ui", NULL);

    init_data(&data, builder);

    gtk_builder_connect_signals(builder, &data);

    g_object_unref(builder);

    gtk_widget_show(data.elements.main_window);
    gtk_main();

    return 0;
}
