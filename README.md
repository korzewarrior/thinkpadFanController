ThinkPad Fan Controller

Overview

This is a simple GTK-based system tray application for controlling and monitoring the fan speed on ThinkPad laptops. It retrieves fan speed, CPU temperature, and allows for fan level adjustments.

Features

Displays fan speed, CPU temperature, and current fan level.

Integrates with the system tray using libappindicator.

Reads data from /proc/acpi/ibm/fan (ThinkPad-specific ACPI interface).

Requirements

This program is designed for Linux systems, specifically Arch Linux with ThinkPad hardware. It may work on other distributions with minor adjustments.

Dependencies

Install the required dependencies using the package manager:

sudo pacman -S gtk3 libappindicator-gtk3 thinkfan acpi_call-dkms glibc

Additional Kernel Module

Ensure the thinkpad_acpi kernel module is loaded:

sudo modprobe thinkpad_acpi

If necessary, add it to /etc/modules-load.d/thinkpad_acpi.conf for auto-loading at startup.

Installation

Clone the repository:

git clone https://github.com/yourusername/thinkpad-fan-controller.git
cd thinkpad-fan-controller

Compile the program:

make

Run the application:

./thinkpadFanController

Installation (System-wide)

To install it as a system-wide application:

sudo make install

To remove it:

sudo make uninstall

Usage

Run the program from the terminal:

./thinkpadFanController

Or add it to your startup applications if using a window manager like i3wm or LXQt.

License

[Specify your preferred license, e.g., MIT, GPL, etc.]

Feel free to contribute to this project by submitting issues or pull requests on GitHub!

