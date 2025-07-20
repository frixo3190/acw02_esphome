@echo off
setlocal

set MODE=%1
if "%MODE%"=="" (
  echo Usage: open-container.bat ota ^| usb
  exit /b 1
)

if "%MODE%"=="usb" (
  docker exec -it esphome-builder-usb bash
) else (
  docker exec -it esphome-builder-ota bash
)
