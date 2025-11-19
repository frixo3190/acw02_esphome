@echo off
setlocal

set MODE=%1
if "%MODE%"=="" (
  echo Usage: build-fr.bat ota ^| usb ^| usb-c3c6
  exit /b 1
)

if "%MODE%"=="usb" (
  docker exec -it esphome-builder-usb /entrypoint.sh run /src/esphome-acw02-fr.yaml --device /dev/ttyUSB0
) else if "%MODE%"=="usb-c3c6" (
  docker exec -it esphome-builder-usb-c3c6 /entrypoint.sh run /src/esphome-acw02-fr.yaml --device /dev/ttyACM0
) else if "%MODE%"=="ota" (
  docker exec -it esphome-builder-ota /entrypoint.sh run /src/esphome-acw02-fr.yaml
) else (
  echo Usage: build-fr.bat ota ^| usb ^| usb-c3c6
  exit /b 1
)