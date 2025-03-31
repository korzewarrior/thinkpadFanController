#!/bin/bash
# Launcher script for ThinkPad Fan Controller on Hyprland

# Check if already running
if pgrep -f "^thinkpadFanController" > /dev/null; then
    echo "ThinkPad Fan Controller is already running."
    exit 0
fi

# Set environment variable to help with system tray integration
export XDG_CURRENT_DESKTOP=Unity

# Launch the application
exec thinkpadFanController "$@" 