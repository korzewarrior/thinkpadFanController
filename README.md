# ThinkPad Fan Controller

## Overview
This is a simple **GTK-based system tray application** for controlling and monitoring the **fan speed on ThinkPad laptops**.  
It retrieves **fan speed, CPU temperature**, and allows for **fan level adjustments**.

## Features
✅ Displays **fan speed, CPU temperature, and current fan level**  
✅ Integrates with the **system tray** using `libappindicator`  
✅ Reads data from `/proc/acpi/ibm/fan` (ThinkPad-specific ACPI interface)  

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

---

## 📥 Installation

### 1️⃣ Clone the repository:
```sh
git clone https://github.com/korzewarrior/thinkpadFanController.git
cd thinkpad-fan-controller
```

### 2️⃣ Compile the program:
```sh
make
```

### 3️⃣ Run the application:
```sh
./thinkpadFanController
```

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
./thinkpadFanController
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
