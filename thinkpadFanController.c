#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

GtkWidget *status_item;  // Stores fan level
GtkWidget *rpm_item;     // Stores fan RPM
GtkWidget *temp_item;    // Stores CPU temperature

// Reads fan status from /proc/acpi/ibm/fan
char* get_fan_level() {
    static char level[64];
    FILE *fp = fopen("/proc/acpi/ibm/fan", "r");

    if (fp == NULL) {
        snprintf(level, sizeof(level), "Fan Level: Error");
        return level;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "level", 5) == 0) {
            snprintf(level, sizeof(level), "Fan Level: %s", line + 6);
            break;
        }
    }
    fclose(fp);
    return level;
}

// Reads fan RPM from /proc/acpi/ibm/fan
char* get_fan_rpm() {
    static char rpm[64];
    FILE *fp = fopen("/proc/acpi/ibm/fan", "r");

    if (fp == NULL) {
        snprintf(rpm, sizeof(rpm), "Fan Speed: Error");
        return rpm;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "speed", 5) == 0) {
            snprintf(rpm, sizeof(rpm), "Fan Speed: %s RPM", line + 6);
            break;
        }
    }
    fclose(fp);
    return rpm;
}

// Reads CPU temperature from /sys/class/thermal/thermal_zone*/temp
char* get_cpu_temp() {
    static char temp[64];
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r"); // May need adjustment on some systems

    if (fp == NULL) {
        snprintf(temp, sizeof(temp), "CPU Temp: Error");
        return temp;
    }

    int temp_millideg;
    if (fscanf(fp, "%d", &temp_millideg) == 1) {
        snprintf(temp, sizeof(temp), "CPU Temp: %.1f°C", temp_millideg / 1000.0);
    } else {
        snprintf(temp, sizeof(temp), "CPU Temp: Unknown");
    }
    
    fclose(fp);
    return temp;
}

// Updates fan level, RPM, and temperature in the menu
void update_status() {
    gtk_menu_item_set_label(GTK_MENU_ITEM(status_item), get_fan_level());
    gtk_menu_item_set_label(GTK_MENU_ITEM(rpm_item), get_fan_rpm());
    gtk_menu_item_set_label(GTK_MENU_ITEM(temp_item), get_cpu_temp());
}

// Sets fan level
void set_fan_level(const char *level) {
    char command[128];
    snprintf(command, sizeof(command), "echo 'level %s' > /proc/acpi/ibm/fan", level);
    system(command);
    update_status();
}

// Fan control functions
void on_fan_auto(GtkMenuItem *item, gpointer data) { set_fan_level("auto"); }
void on_fan_1(GtkMenuItem *item, gpointer data) { set_fan_level("1"); }
void on_fan_3(GtkMenuItem *item, gpointer data) { set_fan_level("3"); }
void on_fan_5(GtkMenuItem *item, gpointer data) { set_fan_level("5"); }
void on_fan_7(GtkMenuItem *item, gpointer data) { set_fan_level("7"); }
void on_fan_max(GtkMenuItem *item, gpointer data) { set_fan_level("disengaged"); }
void exit_program(GtkMenuItem *item, gpointer data) { gtk_main_quit(); }

// Refresh status every 2 seconds
gboolean refresh_status(gpointer data) {
    update_status();
    return TRUE;
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppIndicator *indicator = app_indicator_new(
        "thinkpad-fan-controller",
        "thinkpad-fan-controller",
        APP_INDICATOR_CATEGORY_HARDWARE
    );

    GtkWidget *menu = gtk_menu_new();

    // Status items
    status_item = gtk_menu_item_new_with_label(get_fan_level());
    rpm_item = gtk_menu_item_new_with_label(get_fan_rpm());
    temp_item = gtk_menu_item_new_with_label(get_cpu_temp());

    GtkWidget *auto_item = gtk_menu_item_new_with_label("Auto");
    GtkWidget *level1_item = gtk_menu_item_new_with_label("Low (1)");
    GtkWidget *level3_item = gtk_menu_item_new_with_label("Mid (3)");
    GtkWidget *level5_item = gtk_menu_item_new_with_label("High (5)");
    GtkWidget *level7_item = gtk_menu_item_new_with_label("Full (7)");
    GtkWidget *max_item = gtk_menu_item_new_with_label("Unlimited (Max)");
    GtkWidget *exit_item = gtk_menu_item_new_with_label("Exit");

    // Connect signals
    g_signal_connect(auto_item, "activate", G_CALLBACK(on_fan_auto), NULL);
    g_signal_connect(level1_item, "activate", G_CALLBACK(on_fan_1), NULL);
    g_signal_connect(level3_item, "activate", G_CALLBACK(on_fan_3), NULL);
    g_signal_connect(level5_item, "activate", G_CALLBACK(on_fan_5), NULL);
    g_signal_connect(level7_item, "activate", G_CALLBACK(on_fan_7), NULL);
    g_signal_connect(max_item, "activate", G_CALLBACK(on_fan_max), NULL);
    g_signal_connect(exit_item, "activate", G_CALLBACK(exit_program), NULL);

    // Add items to menu
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), status_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), rpm_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), temp_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
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

    // Periodically refresh fan status every 2 seconds
    g_timeout_add_seconds(2, refresh_status, NULL);

    gtk_main();
    return 0;
}
