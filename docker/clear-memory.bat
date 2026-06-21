@echo off
setlocal

set MODE=%1
if "%MODE%"=="" (
  echo Usage: clear-memory.bat usb ^| usb-c3c6
  exit /b 1
)

if "%MODE%"=="usb" (
  docker exec -it esphome-builder-usb esptool --port /dev/ttyUSB0 erase-flash
) else if "%MODE%"=="usb-c3c6" (
  docker exec -it esphome-builder-usb-c3c6 esptool --port /dev/ttyACM0 erase-flash
) else (
  echo Usage: clear-memory.bat usb ^| usb-c3c6
  exit /b 1
)

