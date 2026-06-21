@echo off
setlocal

echo Stopping esphome container...
docker compose stop esphome-usb
docker compose stop esphome-usb-c3c6
docker compose stop esphome-ota

echo.
echo Waiting 3 seconds before Detach...
timeout /t 3 /nobreak >nul

echo Detach USB...
detach-usb.bat
