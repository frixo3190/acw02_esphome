# acw02_esphome (for teknopoint AC only for the moment)

## installation : 
### python:
install python 3.11 from microsoft store

### update pip:
```
 pip install --upgrade pip
```

### esphome:
```
pip install esphome
```

### Settings
#### Base settings
open esphome-acw02-en.yaml or esphome-acw02-fr.yaml (depending on your preferred language EN or FR)

edite substitutions, you can modify 
 - dev_name
 - dev_friendly
 - api_encrypted_key
 - ota_password
 - ap_password
 - web_server_username
 - web_server_password

```
substitutions:
  dev_name: acw02-salon
  dev_friendly: ACW02 salon
  lang: "en"
  api_encrypted_key: "D2oldc0VP++fni6src89tCSC0UwBhNPgyc8vgYN8/mA="
  ota_password: "REPLACE_WITH_YOUR_PASSWORD"
  ap_password: "fallbackpassword"
  web_server_username: admin
  web_server_password: admin
  board: esp32dev
```

I recommend going to this page to obtain a unique api_encrypted_key:
https://esphome.io/components/api.html

#### WIFI & WEB server settings
open file secrets.yaml and put on this file your WIFI settings and your desired username and password for the web server
```
wifi_ssid: "testesp32"
wifi_password: "testesp32"
wifi_ssid2: "testesp32"
wifi_password2: "testesp32"
wifi_ssid3: "testesp32"
wifi_password3: "testesp32"
```

### build 
```
.\build-fr.bat
```

or 

```
.\build-en.bat
```

### PCB comming soon

### 3d file comming soon


### Option : QRCODE info
#### You can generate a QR code to store module information.

- To do this, open the create_QRcode_info.html file in your browser.

- Import the esphome-acw02-en.yaml or esphome-acw02-fr.yaml file.

- Verify the information and click on the QR code to download it.

- Print it and stick it on the module.

### Note: important
This component requires MQTT to function. I invite you to check how to install and configure it in Home Assistant.

> ⚠️ **WARNING**  
> Be careful with what you're doing, and make sure you have the necessary knowledge before attempting anything.  
> I will not be responsible if you damage your device (air conditioner, ESP, etc.).
