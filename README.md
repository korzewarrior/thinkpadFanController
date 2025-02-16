# ThinkPad Fan Controller

## Overview
This is a simple **GTK-based system tray application** for controlling and monitoring the **fan speed on ThinkPad laptops**.  
It retrieves **fan speed, CPU temperature**, and allows for **fan level adjustments**.

## Features
✅ Displays **fan speed, CPU temperature**, and current fan level  
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

---

## 📜 License
This project is licensed under the **MIT License**.  
See the [`LICENSE`](./LICENSE) file for details.

---

## 🤝 Contributing
Feel free to **submit issues** or **pull requests** on **GitHub**!  

**⭐ If you find this useful, consider starring the repo!**  
