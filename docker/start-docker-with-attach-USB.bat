@echo off
setlocal

:: Enregistre le chemin absolu du script
set "SCRIPT_DIR=%~dp0"

:: Vérifie les droits admin
net session >nul 2>&1
if %errorlevel% neq 0 (
    powershell -Command "Start-Process '%~f0' -Verb runAs -WorkingDirectory '%SCRIPT_DIR%'"
    exit /b
)

cd /d "%SCRIPT_DIR%"

echo Listing current USB devices...
usbipd list

echo.
set /p BUSID=Enter the BusID of the device to bind/attach (e.g. 3-3): 

echo.
echo Binding device with BusID %BUSID%...
usbipd bind --busid %BUSID%

echo.
echo Attaching device with BusID %BUSID% to WSL...
usbipd attach --wsl --busid %BUSID%

echo.
echo Updated USB device list:
usbipd list

echo.
echo Waiting 3 seconds before UP...
timeout /t 3 /nobreak >nul
echo.
echo depending on whether you use a d1 mini or c3/c6 one of the containers usb or usb-c3c6 will be in error this is normal
echo.

docker compose up -d esphome-usb
docker compose up -d esphome-usb-c3c6
docker compose up -d esphome-ota

pause



