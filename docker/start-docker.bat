@echo off
setlocal

echo Starting esphome container...
docker-compose build

echo.
echo Waiting 3 seconds before UP...
timeout /t 3 /nobreak >nul

docker-compose up -d esphome