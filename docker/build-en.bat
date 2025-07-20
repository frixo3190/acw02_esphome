@echo off
setlocal

set MODE=%1
if "%MODE%"=="" (
  echo Usage: build-en.bat ota ^| usb
  exit /b 1
)

if "%MODE%"=="usb" (
  docker exec -it esphome-builder-usb /entrypoint.sh run /src/esphome-acw02-en.yaml --device /dev/ttyUSB0
) else (
  docker exec -it esphome-builder-ota /entrypoint.sh run /src/esphome-acw02-en.yaml
)

