# build from docker (windows)

## install Docker Desktop on windows

you can install it from Microsoft store ou directly on : [link](https://docs.docker.com/desktop/setup/install/windows-install/)

## install WSL 

run powershell as admin, and run this command: 
 ```wsl --install```

## install usbipd

[Windows docs](https://learn.microsoft.com/fr-fr/windows/wsl/connect-usb)

[repo github for download](https://github.com/dorssel/usbipd-win)


## shared usb to container 

1) connect in usb your esp
2) run powershell as admin
3) run command for list usb

```usbipd list```

![usbipdlist](../docker/images/list%20usbipd.PNG)

4) find you esp, for esp32 : Silicon Labs CP210x USB to UART Bridge (COMX)
5) take the BUSID associate and run command bellow with this BUSID (replace 3-3 by yours BUSID)

```usbipd bind --busid 3-3```

6) run ugain the  usbipd list

![usbipdlist](../docker/images/list%20usbipd2.PNG)

7) Attach the usb (replace 3-3 by yours BUSID)

```usbipd attach --wsl --busid 3-3```

![usbipdlist](../docker/images/list%20usbipd3.PNG)

### reverse command : 
```
usbipd detach --busid 3-3
usbipd unbind --busid 3-3
```

## Build component
### install
1) open power shell on docker directory (present in project acw02_esphome)
2) run command one by one
```
docker-compose build
docker-compose up -d esphome
 ```
3) build and push fw to usb 

```build-fr.bat```

or (depending the langue want)

```build-en.bat```

 #### optionnal cmd
  - build-force-device-fr.bat : (FR language) for build and select directly the device (--device /dev/ttyUSB0)
  - build-force-device-en.bat : (EN language) for build and select directly the device (--device /dev/ttyUSB0)
  - build-compile-only-fr.bat : (FR language) for build only without publish on usb or ota
  - build-compile-only-en.bat : (EN language) for build only without publish on usb or ota
  - open-container.bat : enter on container

### bin files
you can bin files in directory docker/bin_generated