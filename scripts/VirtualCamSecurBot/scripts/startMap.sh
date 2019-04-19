#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

echo "Starting Virtual Camera with ffmpeg..."
sudo modprobe v4l2loopback video_nr=5 card_label="virtual_map" exclusive_caps=1

ffmpeg -loop 1 -re -i $DIR/VirtualCamSecurBot/images/Map.png -f v4l2 -vcodec rawvideo -pix_fmt yuv420p /dev/video5

sudo rmmod v4l2loopback
echo "Virtual camera stopped..."
