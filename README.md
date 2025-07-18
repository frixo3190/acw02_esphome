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
pip install esphome==2025.7.1
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

### PCB
#### Gerber file present in PCB directory
[GERBER File](https://github.com/devildant/acw02_esphome/raw/main/PCB/Gerber_climEspHome_esp32_d1_mini_PCB_climEspHome_esp32_d1_mini_2025-07-03.zip)

![front](PCB/images/pcb/front.PNG)

![back](PCB/images/pcb/back.PNG)

#### Components: 
- [12v => 5v (x1): D24V10F5](https://shop.mchobby.be/fr/regulateurs/554--regul-5v-1a-step-down-d24v10f5-3232100005549-pololu.html) [manufacturer](https://www.pololu.com/product/2831)

  ![D24V10F5](PCB/images/components/D24V10F5.PNG)

- [4 Channels Logic Level Converter Bi-Directional Shifter (x1): CYT1076](https://amzn.eu/d/2MhG08s)

  ![CYT1076](PCB/images/components/4%20Channels%20Logic%20Level%20Converter%20Bi-Directional%20Shifter.PNG)

- [ESP32-WROOM-32 D1 Mini NodeMCU (x1)](https://amzn.eu/d/3mS1B7W)

  ![D24V10F5](PCB/images/components/ESP32-WROOM-32%20D1%20Mini%20NodeMCU.PNG)

- [5 pins male 2.54mm (x1) : MaleL7.5-1X5P](https://fr.aliexpress.com/item/1005007128029220.html?spm=a2g0o.order_detail.order_detail_item.3.23f47d56yxv1SG&gatewayAdapt=glo2fra)

  ![MaleL7.5-1X5P](PCB/images/components/5%20pin%20male%202.54.PNG)

- [6 pins male 2.54mm (x2) : MaleL7.5-1X6p](https://fr.aliexpress.com/item/1005007128029220.html?spm=a2g0o.order_detail.order_detail_item.3.23f47d56yxv1SG&gatewayAdapt=glo2fra)

  ![MaleL7.5-1X5P](PCB/images/components/6%20pin%20male%202.54.PNG)

- [10 pins male 2.54mm (x2) : MaleL7.5-1X10P](https://fr.aliexpress.com/item/1005007128029220.html?spm=a2g0o.order_detail.order_detail_item.3.23f47d56yxv1SG&gatewayAdapt=glo2fra)

  ![MaleL7.5-1X5P](PCB/images/components/10%20pin%20male%202.54.PNG)

- [connector : JST XA 2.5 male 4 pins (Reverse direction) (x1)](https://www.aliexpress.com/item/1005008857984831.html?spm=a2g0o.cart.0.0.1dbf38daf0fL06&mp=1&pdp_npi=5%40dis%21EUR%21EUR%206.99%21EUR%206.99%21%21EUR%206.99%21%21%21%40210388c917527810571957734e8cea%2112000048990124847%21ct%21FR%211681384252%21%211%210)

  ![JST XA 2.5 male 4 pins (Reverse direction)](PCB/images/components/connector.PNG)

  Important: take reverse connexion for have this cable order : 

  ![connector](PCB/images/components/connector%202.PNG)

##### PCB solder and cable:

![pcb front](PCB/images/components/pcb%20front.PNG)

![pcb back](PCB/images/components/pcb%20back.PNG)

- 12v : RED
- RX  : BLACK
- TX  : YELLOW
- GND : GREEN

Note: solder pins between the esp32 and the PCB (red square)

![back](PCB/images/components/esp32%20solder%20pins.png)

[IMPORTANT : see solder board helper section](#solder-board-helper)

### 3d files
#### 3d file present in 3D files directory

- [acw02 case bot.stl](https://github.com/devildant/acw02_esphome/raw/main/3Dfiles/acw02%20case%20bot.stl)
- [acw02 case top.stl](https://github.com/devildant/acw02_esphome/raw/main/3Dfiles/acw02%20case%20top.stl)
- [Alternative : acw02 case top cable with thermo.stl](https://github.com/devildant/acw02_esphome/raw/main/3Dfiles/acw02%20case%20top%20cable%20with%20thermo.stl)
- [button.stl](https://github.com/devildant/acw02_esphome/raw/main/3Dfiles/button.stl)
- [solder board.stl](3Dfiles/solder%20board%2016%20hold.stl)

##### Components:
- [insert M2(OD3.2mm) Length 2.5mm 50pcs (x4)](https://www.aliexpress.com/item/1005003582355741.html?spm=a2g0o.order_list.order_list_main.10.1eba18024FtenS)

![insert M2](3Dfiles/images/bottom%20case%20insert.png)

- [insert M3(OD4.2mm) Length 3mm 50pcs (x2)](https://www.aliexpress.com/item/1005003582355741.html?spm=a2g0o.order_list.order_list_main.10.1eba18024FtenS)

![insert M3](3Dfiles/images/bottom%20case%20insert.png)

- [Optional : thermo white](https://amzn.eu/d/8Y1PVUU)

![thermo](3Dfiles/images/thermo.PNG)

![thermo in place](3Dfiles/images/alternative%20cable%20with%20thermo.jpg)

- countersunk head screw M2x6mm (x4)
- screw M3x4mm (x2)

##### Assembly:

![1](3Dfiles/images/button.PNG)

![2](3Dfiles/images/bottom%20case%20+%20pcb%20screw.png)

![5](3Dfiles/images/top%20case%20screw.png)

![3](3Dfiles/images/case%20with%20pcb.jpg)

![4](3Dfiles/images/case%20with%20pcb%20+%20button.jpg)

![8](3Dfiles/images/case%20close.jpg)

##### solder board helper

[solder board.stl](3Dfiles/solder%20board%2016%20hold.stl)

![solder board 1](3Dfiles/images/solder%20board1.jpg)

![solder board 2](3Dfiles/images/solder%20board2.jpg)

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
