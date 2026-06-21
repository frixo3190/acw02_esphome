# ⚡ Activation on Jeedom

## ✅ Prerequisites
- 🔌 **MQTT Broker plugin** must be installed  
- 🧭 **MQTT Discovery plugin** *(optional but highly recommended)*  

---

## 1️⃣ Installing MQTT Discovery
1. 📥 Install the plugin  
2. ▶️ Activate it  
3. ⚙️ Configure (automatic if you already use **MQTT Manager**)  
4. 🔎 In the plugin, enable **Automatic Discovery**  
   - Or manually add your ESP topic → `dev_name` (e.g. `acw02-salon`)  

---

## 2️⃣ From ESP Web Interface
- 🌐 Configure your **MQTT broker**  
  👉 See the [Specific Jeedom Documentation](https://github.com/devildant/acw02_esphome/blob/main/docs/interface_details.md#-optional-mqtt-only-without-esphome-ha-integration-ex-not-connect-to-ha-with-esphome-or-jeedom) for details.  

---

## 3️⃣ From Jeedom
- ✅ Check that the module was **automatically added**  
  ![jeedom discovery](images/jeedom/jeedom-discovery.png)  

- 👀 View the available **commands**  
  ![jeedom commands](images/jeedom/jeedom-commands.png)  

- 👀 **Module**  
![jeedom module](images/jeedom/jeedom-module.png)  