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

uninstall:
	rm -f /usr/local/bin/$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)
