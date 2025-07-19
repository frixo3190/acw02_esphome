@echo off
setlocal

:: Check if running as admin
net session >nul 2>&1
if %errorlevel% neq 0 (
    powershell -Command "Start-Process '%~f0' -Verb runAs"
    exit /b
)

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

pause