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

```
substitutions:
  dev_name: acw02-salon
  dev_friendly: ACW02 salon
  lang: "en"
  api_encrypted_key: "D2oldc0VP++fni6src89tCSC0UwBhNPgyc8vgYN8/mA="
  ota_password: "REPLACE_WITH_YOUR_PASSWORD"
  ap_password: "fallbackpassword"
  board: esp32dev
```

I recommend going to this page to obtain a unique api_encrypted_key:
https://esphome.io/components/api.html

#### WIFI settings
open file secrets.yaml and put on this file your WIFI settings
```
wifi_ssid: "testesp32"
wifi_password: "testesp32"
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


### Note: important
This component requires MQTT to function. I invite you to check how to install and configure it in Home Assistant.

> ⚠️ **WARNING**  
> Be careful with what you're doing, and make sure you have the necessary knowledge before attempting anything.  
> I will not be responsible if you damage your device (air conditioner, ESP, etc.).