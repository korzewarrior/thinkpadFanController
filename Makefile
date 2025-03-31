CC = gcc
CFLAGS = -Wall `pkg-config --cflags gtk+-3.0 appindicator3-0.1`
LDFLAGS = `pkg-config --libs gtk+-3.0 appindicator3-0.1`
TARGET = thinkpadFanController

SRC = thinkpadFanController.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install:
	install -m 755 $(TARGET) /usr/local/bin/
	# Install Hyprland launcher script
	install -m 755 thinkpadFanController-hyprland.sh /usr/local/bin/
	# Create icon directories if they don't exist
	mkdir -p /usr/local/share/icons/hicolor/scalable/apps/
	# Install icon
	install -m 644 icon.png /usr/local/share/icons/hicolor/scalable/apps/thinkpad-fan-controller.png
	# Install desktop file
	mkdir -p /usr/local/share/applications/
	install -m 644 thinkpadFanController.desktop /usr/local/share/applications/

uninstall:
	rm -f /usr/local/bin/$(TARGET)
	rm -f /usr/local/bin/thinkpadFanController-hyprland.sh
	rm -f /usr/local/share/icons/hicolor/scalable/apps/thinkpad-fan-controller.png
	rm -f /usr/local/share/applications/thinkpadFanController.desktop

clean:
	rm -f $(OBJ) $(TARGET)
