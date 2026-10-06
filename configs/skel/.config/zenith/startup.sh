#!/bin/bash
# Zenith OS — Session Startup Script
# Executed by zenith-compositor after it initializes Wayland.

# Wait for Wayland display to be ready
sleep 1

# Start the background
zenith-wallpaper &

# Start the top panel
zenith-panel &

# Start the bottom dock
zenith-dock &

