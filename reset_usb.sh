#!/bin/bash

# Find the USB serial device
DEVICE=$(ls /dev/cu.usbserial* 2>/dev/null | head -n 1)

if [ -z "$DEVICE" ]; then
    echo "No USB serial device found"
    exit 1
fi

echo "Resetting USB serial adapter..."

# Kill any processes using it
lsof -t "$DEVICE" | xargs -r kill -9 2>/dev/null

# Wait a moment
sleep 0.5

echo "Ready"
