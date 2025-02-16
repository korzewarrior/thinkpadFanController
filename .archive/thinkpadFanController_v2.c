#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <stdlib.h>

void set_fan_level(const char *level) {
    char command[128];
    snprintf(command, sizeof(command), "echo 'level %s' | sudo tee /proc/acpi/ibm/fan", level);
    system(command);
}

void on_fan_auto(GtkMenuItem *item, gpointer data) { set_fan_level("auto"); }
void on_fan_1(GtkMenuItem *item, gpointer data) { set_fan_level("1"); }
void on_fan_3(GtkMenuItem *item, gpointer data) { set_fan_level("3"); }
void on_fan_5(GtkMenuItem *item, gpointer data) { set_fan_level("5"); }
void on_fan_7(GtkMenuItem *item, gpointer data) { set_fan_level("7"); }
void on_fan_max(GtkMenuItem *item, gpointer data) { set_fan_level("disengaged"); }
void exit_program(GtkMenuItem *item, gpointer data) { gtk_main_quit(); }  // Renamed function

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppIndicator *indicator = app_indicator_new(
        "thinkpad-fan-controller",
        "utilities-terminal",
        APP_INDICATOR_CATEGORY_HARDWARE
    );

    GtkWidget *menu = gtk_menu_new();

    GtkWidget *auto_item = gtk_menu_item_new_with_label("Auto");
    GtkWidget *level1_item = gtk_menu_item_new_with_label("Low (1)");
    GtkWidget *level3_item = gtk_menu_item_new_with_label("Mid (3)");
    GtkWidget *level5_item = gtk_menu_item_new_with_label("High (5)");
    GtkWidget *level7_item = gtk_menu_item_new_with_label("Full (7)");
    GtkWidget *max_item = gtk_menu_item_new_with_label("Unlimited (Max)");
    GtkWidget *exit_item = gtk_menu_item_new_with_label("Exit");

    g_signal_connect(auto_item, "activate", G_CALLBACK(on_fan_auto), NULL);
    g_signal_connect(level1_item, "activate", G_CALLBACK(on_fan_1), NULL);
    g_signal_connect(level3_item, "activate", G_CALLBACK(on_fan_3), NULL);
    g_signal_connect(level5_item, "activate", G_CALLBACK(on_fan_5), NULL);
    g_signal_connect(level7_item, "activate", G_CALLBACK(on_fan_7), NULL);
    g_signal_connect(max_item, "activate", G_CALLBACK(on_fan_max), NULL);
    g_signal_connect(exit_item, "activate", G_CALLBACK(exit_program), NULL); // Updated here

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), auto_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), level1_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), level3_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), level5_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), level7_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), max_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), exit_item);

    gtk_widget_show_all(menu);
    app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_menu(indicator, GTK_MENU(menu));

    gtk_main();
    return 0;
}
