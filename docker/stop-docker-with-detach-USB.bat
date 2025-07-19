@echo off
setlocal

echo Stopping esphome container...
docker compose stop esphome

echo.
echo Waiting 3 seconds before Detach...
timeout /t 3 /nobreak >nul

echo Detach USB...
detach-usb.bat
