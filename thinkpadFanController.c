#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>  // For CPU utilization
#include <cairo.h>   // For simple graphing

#define GRAPH_WIDTH 240
#define GRAPH_HEIGHT 120
#define GRAPH_POINTS 60  // 60 data points = 2 minutes history with 2sec updates

GtkWidget *status_item;  // Stores fan level
GtkWidget *rpm_item;     // Stores fan RPM
GtkWidget *temp_item;    // Stores CPU temperature
GtkWidget *cpu_item;     // Stores CPU utilization
GtkWidget *fan_viz_item; // Visual fan level indicator
GtkWidget *graph_window = NULL; // Temperature history graph window
GtkWidget *graph_drawing_area;

// History data for temperature graph
double temp_history[GRAPH_POINTS] = {0};
double cpu_history[GRAPH_POINTS] = {0};
int history_index = 0;
gboolean graph_visible = FALSE;

struct {
    long user;
    long nice;
    long system;
    long idle;
    long iowait;
    long irq;
    long softirq;
    long steal;
} cpu_stats, cpu_stats_prev;

// Function prototypes
gboolean on_fan_viz_draw(GtkWidget *widget, cairo_t *cr, gpointer data);
gboolean on_graph_draw(GtkWidget *widget, cairo_t *cr, gpointer data);
gboolean on_graph_window_close(GtkWidget *widget, GdkEvent *event, gpointer data);
void show_graph_window();

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

// Reads CPU utilization from /proc/stat
double get_cpu_usage() {
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        return -1.0;
    }

    // Save previous values
    cpu_stats_prev = cpu_stats;

    // Read current CPU stats
    char line[256];
    if (fgets(line, sizeof(line), fp)) {
        sscanf(line, "cpu %ld %ld %ld %ld %ld %ld %ld %ld",
               &cpu_stats.user, &cpu_stats.nice, &cpu_stats.system, &cpu_stats.idle,
               &cpu_stats.iowait, &cpu_stats.irq, &cpu_stats.softirq, &cpu_stats.steal);
    }
    fclose(fp);

    // Calculate the delta between current and previous
    long prev_idle = cpu_stats_prev.idle + cpu_stats_prev.iowait;
    long idle = cpu_stats.idle + cpu_stats.iowait;

    long prev_non_idle = cpu_stats_prev.user + cpu_stats_prev.nice + 
                        cpu_stats_prev.system + cpu_stats_prev.irq + 
                        cpu_stats_prev.softirq + cpu_stats_prev.steal;
    long non_idle = cpu_stats.user + cpu_stats.nice + 
                   cpu_stats.system + cpu_stats.irq + 
                   cpu_stats.softirq + cpu_stats.steal;

    long prev_total = prev_idle + prev_non_idle;
    long total = idle + non_idle;

    // Calculate the CPU usage percentage
    double totald = (double)total - (double)prev_total;
    double idled = (double)idle - (double)prev_idle;
    
    if (totald == 0)
        return 0.0;
        
    return 100.0 * (totald - idled) / totald;
}

// Create a visual representation of fan level
GtkWidget* create_fan_visualization(const char* level_str) {
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 140, 20);
    
    // Store the fan level as data on the widget
    int level = 0;
    
    if (strstr(level_str, "auto"))
        level = -1; // Auto mode
    else if (strstr(level_str, "disengaged"))
        level = 8;  // Max
    else if (isdigit(level_str[0]))
        level = level_str[0] - '0';
        
    g_object_set_data(G_OBJECT(drawing_area), "fan-level", GINT_TO_POINTER(level));
    
    // Connect draw signal
    g_signal_connect(G_OBJECT(drawing_area), "draw", 
                     G_CALLBACK(on_fan_viz_draw), NULL);
                     
    return drawing_area;
}

// Drawing function for fan visualization
gboolean on_fan_viz_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int level = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "fan-level"));
    
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    int width = allocation.width;
    int height = allocation.height;
    
    // Clear background
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
    
    // Draw fan level indicator
    int segments = 7;  // Regular levels 0-7
    int bar_width = width / (segments + 1);  // +1 for max level
    
    for (int i = 0; i <= segments; i++) {
        // Color gradient from blue to red
        double r, g, b;
        if (i <= 2) {
            r = 0.0; g = 0.4 + (i * 0.2); b = 0.7 - (i * 0.2);
        } else if (i <= 5) {
            r = (i - 2) * 0.25; g = 0.8; b = 0.3 - ((i - 2) * 0.1);
        } else {
            r = 0.8 + ((i - 5) * 0.1); g = 0.8 - ((i - 5) * 0.4); b = 0.0;
        }
        
        // Determine if segment should be lit
        if ((level == -1 && i == 0) ||  // Auto - light only first segment in blue
            (level >= i && level != -1)) {  // Current level and below
            cairo_set_source_rgb(cr, r, g, b);
        } else {
            // Dim version for inactive segments
            cairo_set_source_rgb(cr, r * 0.3, g * 0.3, b * 0.3);
        }
        
        // Draw the segment
        cairo_rectangle(cr, i * bar_width, 0, bar_width - 2, height);
        cairo_fill(cr);
    }
    
    return FALSE;
}

// Create and display a temperature history graph
void show_graph_window() {
    if (graph_window != NULL) {
        // Window already exists, just show it
        gtk_widget_show_all(graph_window);
        graph_visible = TRUE;
        return;
    }
    
    // Create a new window
    graph_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(graph_window), "ThinkPad Temperature History");
    gtk_window_set_default_size(GTK_WINDOW(graph_window), GRAPH_WIDTH, GRAPH_HEIGHT);
    g_signal_connect(G_OBJECT(graph_window), "delete-event", 
                     G_CALLBACK(on_graph_window_close), NULL);
    
    // Create drawing area for the graph
    graph_drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(graph_window), graph_drawing_area);
    g_signal_connect(G_OBJECT(graph_drawing_area), "draw", 
                     G_CALLBACK(on_graph_draw), NULL);
    
    gtk_widget_show_all(graph_window);
    graph_visible = TRUE;
}

// Handle graph window close
gboolean on_graph_window_close(GtkWidget *widget, GdkEvent *event, gpointer data) {
    graph_visible = FALSE;
    // Don't destroy, just hide
    gtk_widget_hide(widget);
    return TRUE;  // Prevent destruction
}

// Drawing function for the temperature graph
gboolean on_graph_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    int width = allocation.width;
    int height = allocation.height;
    
    // Clear background
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
    
    // Draw grid
    cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 0.5);
    cairo_set_line_width(cr, 0.5);
    
    // Vertical grid lines - every 10 seconds
    for (int i = 0; i < GRAPH_POINTS; i += 5) {
        int x = i * width / GRAPH_POINTS;
        cairo_move_to(cr, x, 0);
        cairo_line_to(cr, x, height);
    }
    
    // Horizontal grid lines - every 10 degrees
    for (int temp = 30; temp <= 90; temp += 10) {
        int y = height - (temp * height / 100);
        cairo_move_to(cr, 0, y);
        cairo_line_to(cr, width, y);
    }
    cairo_stroke(cr);
    
    // Draw temperature axis labels
    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_set_font_size(cr, 10);
    for (int temp = 30; temp <= 90; temp += 10) {
        int y = height - (temp * height / 100);
        cairo_move_to(cr, 2, y - 2);
        char text[10];
        snprintf(text, sizeof(text), "%d°C", temp);
        cairo_show_text(cr, text);
    }
    
    // Draw CPU usage line
    cairo_set_source_rgb(cr, 0.2, 0.7, 0.2);
    cairo_set_line_width(cr, 1.5);
    
    int start_idx = (history_index + 1) % GRAPH_POINTS;
    int x = 0;
    
    for (int i = 0; i < GRAPH_POINTS; i++) {
        int idx = (start_idx + i) % GRAPH_POINTS;
        int y = height - (cpu_history[idx] * height / 100);
        
        if (i == 0)
            cairo_move_to(cr, x, y);
        else
            cairo_line_to(cr, x, y);
            
        x += width / GRAPH_POINTS;
    }
    cairo_stroke(cr);
    
    // Draw temperature line
    cairo_set_source_rgb(cr, 0.9, 0.3, 0.2);
    cairo_set_line_width(cr, 2.0);
    
    x = 0;
    for (int i = 0; i < GRAPH_POINTS; i++) {
        int idx = (start_idx + i) % GRAPH_POINTS;
        int y = height - (temp_history[idx] * height / 100);
        
        if (i == 0)
            cairo_move_to(cr, x, y);
        else
            cairo_line_to(cr, x, y);
            
        x += width / GRAPH_POINTS;
    }
    cairo_stroke(cr);
    
    // Draw legends
    cairo_set_font_size(cr, 12);
    
    // Temperature legend
    cairo_set_source_rgb(cr, 0.9, 0.3, 0.2);
    cairo_rectangle(cr, width - 80, 5, 12, 12);
    cairo_fill(cr);
    cairo_move_to(cr, width - 65, 15);
    cairo_show_text(cr, "Temp");
    
    // CPU usage legend
    cairo_set_source_rgb(cr, 0.2, 0.7, 0.2);
    cairo_rectangle(cr, width - 80, 25, 12, 12);
    cairo_fill(cr);
    cairo_move_to(cr, width - 65, 35);
    cairo_show_text(cr, "CPU");
    
    return FALSE;
}

// Gets formatted CPU utilization string
char* get_cpu_utilization() {
    static char cpu_str[64];
    double usage = get_cpu_usage();
    
    if (usage < 0) {
        snprintf(cpu_str, sizeof(cpu_str), "CPU Usage: Error");
    } else {
        snprintf(cpu_str, sizeof(cpu_str), "CPU Usage: %.1f%%", usage);
        
        // Update history data
        cpu_history[history_index] = usage;
    }
    
    return cpu_str;
}

// Override get_cpu_temp() to update history array
char* get_cpu_temp() {
    static char temp[64];
    FILE *fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r"); // May need adjustment on some systems

    if (fp == NULL) {
        snprintf(temp, sizeof(temp), "CPU Temp: Error");
        return temp;
    }

    int temp_millideg;
    if (fscanf(fp, "%d", &temp_millideg) == 1) {
        double temp_c = temp_millideg / 1000.0;
        snprintf(temp, sizeof(temp), "CPU Temp: %.1f°C", temp_c);
        
        // Update history data
        temp_history[history_index] = temp_c;
    } else {
        snprintf(temp, sizeof(temp), "CPU Temp: Unknown");
    }
    
    fclose(fp);
    return temp;
}

// Updates fan level, RPM, and temperature in the menu
void update_status() {
    // Get fan level string
    char* level_str = get_fan_level();
    
    // Update labels
    gtk_menu_item_set_label(GTK_MENU_ITEM(status_item), level_str);
    gtk_menu_item_set_label(GTK_MENU_ITEM(rpm_item), get_fan_rpm());
    gtk_menu_item_set_label(GTK_MENU_ITEM(temp_item), get_cpu_temp());
    gtk_menu_item_set_label(GTK_MENU_ITEM(cpu_item), get_cpu_utilization());
    
    // Update fan visualization
    GtkWidget* drawing_area = create_fan_visualization(level_str + 11); // Skip "Fan Level: "
    
    // Replace the old fan visualization
    if (fan_viz_item != NULL) {
        GtkWidget* old_viz = gtk_bin_get_child(GTK_BIN(fan_viz_item));
        if (old_viz != NULL) {
            gtk_container_remove(GTK_CONTAINER(fan_viz_item), old_viz);
        }
        gtk_container_add(GTK_CONTAINER(fan_viz_item), drawing_area);
        gtk_widget_show_all(fan_viz_item);
    }
    
    // Update the history index and graph if visible
    history_index = (history_index + 1) % GRAPH_POINTS;
    
    if (graph_visible && graph_drawing_area != NULL) {
        gtk_widget_queue_draw(graph_drawing_area);
    }
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

// Open the graph window
void on_show_graph(GtkMenuItem *item, gpointer data) {
    show_graph_window();
}

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
    cpu_item = gtk_menu_item_new_with_label(get_cpu_utilization());
    
    // Fan visualization
    fan_viz_item = gtk_menu_item_new();
    GtkWidget* drawing_area = create_fan_visualization(get_fan_level() + 11);
    gtk_container_add(GTK_CONTAINER(fan_viz_item), drawing_area);

    GtkWidget *auto_item = gtk_menu_item_new_with_label("Auto");
    GtkWidget *level1_item = gtk_menu_item_new_with_label("Low (1)");
    GtkWidget *level3_item = gtk_menu_item_new_with_label("Mid (3)");
    GtkWidget *level5_item = gtk_menu_item_new_with_label("High (5)");
    GtkWidget *level7_item = gtk_menu_item_new_with_label("Full (7)");
    GtkWidget *max_item = gtk_menu_item_new_with_label("Unlimited (Max)");
    GtkWidget *graph_item = gtk_menu_item_new_with_label("Show Graph");
    GtkWidget *exit_item = gtk_menu_item_new_with_label("Exit");

    // Connect signals
    g_signal_connect(auto_item, "activate", G_CALLBACK(on_fan_auto), NULL);
    g_signal_connect(level1_item, "activate", G_CALLBACK(on_fan_1), NULL);
    g_signal_connect(level3_item, "activate", G_CALLBACK(on_fan_3), NULL);
    g_signal_connect(level5_item, "activate", G_CALLBACK(on_fan_5), NULL);
    g_signal_connect(level7_item, "activate", G_CALLBACK(on_fan_7), NULL);
    g_signal_connect(max_item, "activate", G_CALLBACK(on_fan_max), NULL);
    g_signal_connect(graph_item, "activate", G_CALLBACK(on_show_graph), NULL);
    g_signal_connect(exit_item, "activate", G_CALLBACK(exit_program), NULL);

    // Add items to menu
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), status_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), fan_viz_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), rpm_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), temp_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), cpu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), auto_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), level1_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), level3_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), level5_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), level7_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), max_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), graph_item);
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
