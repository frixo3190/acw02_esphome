@echo off
setlocal

set MODE=%1
if "%MODE%"=="" (
  echo Usage: open-container.bat ota ^| usb ^| usb-c3c6
  exit /b 1
)

if "%MODE%"=="usb" (
  docker exec -it esphome-builder-usb bash
) else if "%MODE%"=="usb-c3c6" (
  docker exec -it esphome-builder-usb-c3c6 bash
) else if "%MODE%"=="ota" (
  docker exec -it esphome-builder-ota bash
) else (
  echo Usage: open-container.bat ota ^| usb ^| usb-c3c6
  exit /b 1  
)
