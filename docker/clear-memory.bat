@echo off
docker exec -it esphome-builder-usb esptool.py --port /dev/ttyUSB0 erase_flash
