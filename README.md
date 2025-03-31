# ThinkPad Fan Controller

## Overview
This is a simple **GTK-based system tray application** for controlling and monitoring the **fan speed on ThinkPad laptops**.  
It retrieves **fan speed, CPU temperature**, and allows for **fan level adjustments** with real-time monitoring.

![Screenshot of ThinkPad Fan Controller](screenshot.png)
*You can replace this with your own screenshot by taking one and naming it 'screenshot.png'*

## Features
✅ Displays **fan speed, CPU temperature, and CPU utilization**  
✅ **Visual fan level indicator** with color-coded levels  
✅ **Temperature and CPU usage history graph** for monitoring trends  
✅ Integrates with the **system tray** using `libappindicator`  
✅ Reads data from `/proc/acpi/ibm/fan` (ThinkPad-specific ACPI interface)  
✅ No longer requires `sudo` for fan control after proper setup  

---

## 🛠 Requirements
This program is designed for **Linux systems**, specifically **Arch Linux** with **ThinkPad hardware**.  
It may work on other distributions with minor adjustments.

### 🔧 Dependencies
Install the required dependencies using **pacman**:

```sh
sudo pacman -S gtk3 libappindicator-gtk3 thinkfan acpi_call-dkms glibc
```

### ⚙️ Additional Kernel Module
Ensure the `thinkpad_acpi` kernel module is loaded:

```sh
sudo modprobe thinkpad_acpi
```

If necessary, **enable auto-loading** at startup:

```sh
echo "thinkpad_acpi" | sudo tee /etc/modules-load.d/thinkpad_acpi.conf
```

### 🔑 Fixing Permissions for `/proc/acpi/ibm/fan`
To allow the program to control fan speeds **without requiring sudo**, set proper permissions:

```sh
sudo groupadd thinkpad
sudo chown :thinkpad /proc/acpi/ibm/fan
sudo chmod 664 /proc/acpi/ibm/fan
sudo usermod -aG thinkpad $USER
```

Make this permission persist across reboots:
```sh
echo 'f /proc/acpi/ibm/fan 0664 root thinkpad' | sudo tee /etc/tmpfiles.d/thinkpadfan.conf
sudo systemd-tmpfiles --create
```

Reboot your system to apply the changes.

---

## 📥 Installation

### 1️⃣ Clone the repository:
```sh
git clone https://github.com/korzewarrior/thinkpadFanController.git
cd thinkpadFanController
```

### 2️⃣ Compile the program:
```sh
make clean
make
```

### 3️⃣ Install the application system-wide:
```sh
sudo make install
```

### 4️⃣ Run the application:
```sh
thinkpadFanController
```

✅ **No need for `sudo` if permissions were set correctly!**

---

## 🔧 System-wide Installation
To install it as a system-wide application:
```sh
sudo make install
```

To remove it:
```sh
sudo make uninstall
```

---

## 🚀 Usage
Run the program from the terminal:
```sh
thinkpadFanController
```
Or add it to your **startup applications** if using a window manager like **i3wm** or **LXQt**.

### Key Features
- **Fan Level Control**: Set your fan speed to Auto, Low (1), Mid (3), High (5), Full (7), or Unlimited (Max)
- **Visual Fan Indicator**: See your current fan level with an intuitive color-coded visualization
- **CPU Monitoring**: Track both CPU temperature and utilization in real-time
- **Temperature History**: Click "Show Graph" to view a history graph of temperature and CPU usage over time

### 🌡️ Fan Levels and Performance Impact

Different fan levels have varying effects on system performance, noise, and battery life:

| Fan Level | Noise Level | Cooling Efficiency | Battery Impact | Recommended Use Case |
|-----------|-------------|-------------------|----------------|----------------------|
| Auto      | Variable    | Adaptive          | Best           | Everyday use, balanced performance |
| Low (1)   | Very quiet  | Minimal           | Good           | Light tasks, quiet environments |
| Mid (3)   | Moderate    | Adequate          | Moderate       | Regular work, mild workloads |
| High (5)  | Noticeable  | Good              | Higher         | Gaming, video editing, sustained loads |
| Full (7)  | Loud        | Excellent         | High           | Heavy workloads, hot environments |
| Max       | Very loud   | Maximum           | Highest        | Emergency cooling, extreme situations |

**Note**: Higher fan speeds will consume more battery but will keep your CPU cooler, potentially allowing it to maintain higher performance for longer periods before thermal throttling occurs.

### 🚀 Using with Hyprland
To use this application with Hyprland:

1. Ensure your Hyprland configuration supports system tray applications.

2. Add this application to your Hyprland autostart by editing your Hyprland config file (typically `~/.config/hypr/hyprland.conf`):

```
# Start ThinkPad Fan Controller
exec-once = thinkpadFanController-hyprland.sh
```

3. Make sure you have a status bar that supports XDG system tray (like Waybar) and it's configured correctly:

If using Waybar, make sure your Waybar config (typically `~/.config/waybar/config`) includes the "tray" module:

```json
{
    "modules-right": ["tray", "clock", ...],
    "tray": {
        "icon-size": 21,
        "spacing": 10
    }
}
```

4. If the tray icon doesn't appear, you may need to install `xdg-desktop-portal-wlr` and `xdg-desktop-portal-gtk`:

```sh
sudo pacman -S xdg-desktop-portal-wlr xdg-desktop-portal-gtk
```

5. The included `thinkpadFanController-hyprland.sh` script sets necessary environment variables for better compatibility with Hyprland. It's automatically installed by `make install`.

6. If you're migrating from i3wm to Hyprland, make sure to remove any i3-specific autostart entries for thinkpadFanController and use the Hyprland approach described above.

---

## 🔧 Troubleshooting

### Common Issues

#### Fan Control Not Working
- **Issue**: Fan speed doesn't change when selecting different levels
- **Solution**: 
  - Check that the `thinkpad_acpi` module is loaded: `lsmod | grep thinkpad_acpi`
  - Verify permissions on `/proc/acpi/ibm/fan` with `ls -l /proc/acpi/ibm/fan`
  - Make sure your user is in the `thinkpad` group with `groups $USER`
  - Some newer ThinkPad models may have different fan control interfaces

#### Fan Control Permission Denied
- **Issue**: "Permission denied" errors when changing fan levels
- **Solution**: Follow the permission setup steps in the Requirements section

#### Application Not Appearing in System Tray
- **Issue**: Icon doesn't appear in your system tray
- **Solution**:
  - For Waybar: Ensure you have the "tray" module enabled in your Waybar config
  - For other DEs/WMs: Make sure you have a system tray implementation running
  - Try restarting your status bar after launching the application

#### Compatibility
This application is specifically designed for ThinkPad laptops and relies on ThinkPad-specific interfaces. It will not work on other laptop brands. It has been tested on various ThinkPad models including:
- ThinkPad X series (X220, X230, X1 Carbon, etc.)
- ThinkPad T series (T420, T430, T480, etc.)
- ThinkPad L series and others with standard ThinkPad ACPI interfaces

## 📜 License
This project is licensed under the **MIT License**.  
See the [`LICENSE`](./LICENSE) file for details.

## 🤝 Contributing
Feel free to **submit issues** or **pull requests** on **GitHub**!  

**⭐ If you find this useful, consider starring the repo!**  
