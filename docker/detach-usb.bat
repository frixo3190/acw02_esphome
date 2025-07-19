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
set /p BUSID=Enter the BusID of the device to detach/unbind (e.g. 3-3): 

echo.
echo Detaching device with BusID %BUSID% from WSL...
usbipd detach --busid %BUSID%

echo.
echo Waiting 3 seconds before unbind...
timeout /t 3 /nobreak >nul

echo Unbinding device with BusID %BUSID%...
usbipd unbind --busid %BUSID%

echo.
echo Updated USB device list:
usbipd list

pause