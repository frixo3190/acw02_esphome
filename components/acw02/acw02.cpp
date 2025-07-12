#include "acw02.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
  namespace acw02 {

    void ACW02::setup() {
      #if defined(DLOCALE_LANG)
        app_lang_ = DLOCALE_LANG;
      #else
        app_lang_ = "en";
      #endif
      #if defined(DBOARD)
        app_board_ = DBOARD;
      #endif
      mqtt_connexion();
      app_name_ = App.get_name();
      app_friendly_name_ = App.get_friendly_name();
      app_sanitize_name_ = sanitize_name(app_name_);
      app_mac_ = get_mac_address();

      send_command_basic(keepalive_frame_);
      send_command_basic(get_status_frame_);

      if (mqtt_) {
        mqtt_->set_on_connect([this](bool first) {
          ESP_LOGI(TAG, "MQTT connected → publishing discovery and state");
          set_timeout("mqtt_discovery_delay", 100, [this]() {
            publish_discovery_climate();
            publish_discovery_mode_select();
            publish_discovery_fan_select();
            publish_discovery_swing_select();
            publish_discovery_swing_horizontal_select();
            publish_discovery_unit_select();
            publish_discovery_clean_switch();
            publish_discovery_eco_switch();
            publish_discovery_display_switch();
            publish_discovery_night_switch();
            publish_discovery_purifier_switch();
            publish_discovery_temperature_number();
            publish_discovery_temperature_sensor();
            publish_discovery_last_cmd_origin_sensor();
          });
          mqtt_->subscribe(app_name_ + "/cmd/#", [this](const std::string &topic, const std::string &payload) {
            mqtt_callback(topic, payload);
          });
          if (!first) {
            send_command_basic(get_status_frame_);
          }
        });
        mqtt_->set_on_disconnect([this](mqtt::MQTTClientDisconnectReason reason) {
          ESP_LOGI(TAG, "MQTT disconnected (reason=%d)", static_cast<int>(reason));
        });
      }
      mute_pref_ = global_preferences->make_preference<bool>(1U, "ac_mute");
      mute_pref_.load(&mute_);
      disable_mode_auto_pref_ = global_preferences->make_preference<bool>(2U, "ac_disable_mode_auto");
      disable_mode_auto_pref_.load(&disable_mode_auto_);
      disable_mode_heat_pref_ = global_preferences->make_preference<bool>(3U, "ac_disable_mode_heat");
      disable_mode_heat_pref_.load(&disable_mode_heat_);
      disable_mode_dry_pref_ = global_preferences->make_preference<bool>(4U, "ac_disable_mode_dry");
      disable_mode_dry_pref_.load(&disable_mode_dry_);
      disable_mode_fan_pref_ = global_preferences->make_preference<bool>(5U, "ac_disable_mode_fan");
      disable_mode_fan_pref_.load(&disable_mode_fan_);
      disable_swing_vertical_pref_ = global_preferences->make_preference<bool>(6U, "ac_disable_swing_vertical");
      disable_swing_vertical_pref_.load(&disable_swing_vertical_);
      disable_swing_horizontal_pref_ = global_preferences->make_preference<bool>(7U, "ac_disable_swing_horizontal");
      disable_swing_horizontal_pref_.load(&disable_swing_horizontal_);
      option_recalculate_climate_pref_ = global_preferences->make_preference<bool>(8U, "ac_option_recalculate_climate");
      option_recalculate_climate_pref_.load(&option_recalculate_climate_);

      set_timeout("test", 10000, [this]() {
        ESP_LOGI(TAG, "Setup %s ", "INIT");
        ESP_LOGI(TAG, "Board %s ", app_board_.c_str());
        ESP_LOGI(TAG, "MAC %s ", app_mac_.c_str());
        ESP_LOGI(TAG, "name %s ", app_name_.c_str());
        ESP_LOGI(TAG, "friendly name %s ", app_friendly_name_.c_str());
        ESP_LOGI(TAG, "sanitze name %s ", app_sanitize_name_.c_str());
        ESP_LOGI(TAG, "MQTT broker %s ", mqtt_broker_address_.c_str());
        ESP_LOGI(TAG, "MQTT username %s ", mqtt_username_.c_str());
        ESP_LOGI(TAG, "MQTT port %d ", mqtt_port_);
        ESP_LOGI(TAG, "Lang %s ", app_lang_.c_str());
        ESP_LOGI(TAG, "name %s ", get_localized_name(app_lang_, "climate").c_str());
        ESP_LOGI(TAG, "mode json %s ", build_options_json(app_lang_, "mode").c_str());
        ESP_LOGI(TAG, "fan json %s ", build_options_json(app_lang_, "fan").c_str());
        ESP_LOGI(TAG, "swing json %s ", build_options_json(app_lang_, "swing").c_str());
        send_command_basic(get_status_frame_);
      });
    }

    void ACW02::loop() {
      process_tx_queue();

      while (available()) {
        uint8_t byte = read();
        rx_buffer_.push_back(byte);
        last_rx_byte_time_ = millis();
      }

      if (!rx_buffer_.empty() && millis() - last_rx_byte_time_ > 50) {
        ESP_LOGI(TAG, "RX: [%s]", format_hex_pretty(rx_buffer_).c_str());
        decode_state(rx_buffer_);
        rx_buffer_.clear();
      }

      static uint32_t last_keepalive = 0;
      if (millis() - last_keepalive > 60000 && tx_queue_.empty()) {
        if (millis() - last_rx_byte_time_ > 200) {
          last_keepalive = millis();
          send_command_basic(keepalive_frame_);
        }
      }
    }

    void ACW02::set_mode_climate(const std::string &mode) {
      if (mode != "off") {
        const Mode previous_mode = mode_;
        power_on_ = true;
        mode_ = str_to_mode_climate(mode);
        if (mode_ != Mode::COOL) set_eco_internal(false, false);
        if (mode_ == Mode::FAN) night_ = false;
        if (mode_ == Mode::AUTO) recalculate_climate_depending_by_option();
        if (previous_mode == Mode::AUTO) recalculate_climate_depending_by_option();
      } else {
        power_on_ = false;
      }
    }

    void ACW02::set_mode(const std::string &mode) {
      if (mode != key_to_txt(app_lang_, "mode", "OFF")) {
        const Mode previous_mode = mode_;
        power_on_ = true;
        mode_ = str_to_mode(app_lang_, mode);
        if (mode_ != Mode::COOL) set_eco_internal(false, false);
        if (mode_ == Mode::FAN) night_ = false;
        if (mode_ == Mode::AUTO) recalculate_climate_depending_by_option();
        if (previous_mode == Mode::AUTO) recalculate_climate_depending_by_option();
      } else {
        power_on_ = false;
      }
    }

    void ACW02::set_temperature_c(float temp) {
      if (!use_fahrenheit_) {
        const uint8_t oldC = target_temp_c_;
        const uint8_t oldF = target_temp_f_;

        if (temp < 16) temp = 16;
        if (temp > 31) temp = 31;
        target_temp_f_ = static_cast<uint8_t>(celsius_to_fahrenheit(temp));
        target_temp_c_ = static_cast<uint8_t>(temp);
        if (!eco_ && mode_ != Mode::AUTO) {
          send_command();
        } else {
          set_timeout("publishStateDelay", 100, [this, oldC, oldF]() {
            target_temp_f_ = oldF;
            target_temp_c_ = oldC;
            publish_state();
          });
        }
      }
    }

    void ACW02::set_temperature_f(float temp) {
      if (use_fahrenheit_) {
        const uint8_t oldC = target_temp_c_;
        const uint8_t oldF = target_temp_f_;

        if (temp < 61) temp = 61;
        if (temp > 88) temp = 88;
        target_temp_f_ = static_cast<uint8_t>(temp);
        target_temp_c_ = static_cast<uint8_t>(fahrenheit_to_celsius(temp));
        if (!eco_ && mode_ != Mode::AUTO) {
          send_command();
        } else {
          set_timeout("publishStateDelay", 100, [this, oldC, oldF]() {
            target_temp_f_ = oldF;
            target_temp_c_ = oldC;
            publish_state();
          });
        }
      }
    }

    void ACW02::set_fan(const std::string &speed) {
      if (!eco_) {
        fan_ = str_to_fan(app_lang_, speed);
        send_command();
      } else {
        ESP_LOGI(TAG, "Ignore command : mode ECO enabled");
        fan_ = str_to_fan(app_lang_, speed);
        set_timeout("publishStateDelay", 100, [this]() {
          fan_ = Fan::AUTO;
          publish_state();
        });
      }
    }

    void ACW02::set_swing(const std::string &pos) {
      swing_position_ = str_to_swing(app_lang_, pos);
    }

    void ACW02::set_swing_horizontal(const std::string &pos) {
      swing_horizontal_ = str_to_swing_horizontal(app_lang_, pos);
    }

    void ACW02::set_display(bool on) {
      ESP_LOGI(TAG, "set display current : %s target : %s", display_ ? "on" : "off", on ? "on" : "off");
      if (display_ != on) {
        display_ = on;
        send_command();
      }
    }

    void ACW02::set_eco(bool on) {
      if (eco_ != on) {
        if (power_on_) {
          if (mode_ == Mode::COOL) {
            set_eco_internal(on, true);
          } else {
            eco_ = false;
          }
        } else {
          eco_ = false;
        }
      }
    }

    void ACW02::set_eco_internal(bool on, bool sendCmd) {
      if (eco_ != on) {
        eco_ = on;
        if (on) {
          night_ = false;
          previous_fan_ = fan_;
          fan_ = Fan::AUTO;
          recalculate_climate_depending_by_option();
          ESP_LOGI(TAG, "set_eco %s", "store previous fan et enable auto");
        } else {
          fan_ = previous_fan_;
          recalculate_climate_depending_by_option();
          ESP_LOGI(TAG, "set_eco %s", "restore previous");
        }
        if (sendCmd) {
          send_command();
        }
      }
    }

    void ACW02::set_night(bool on) {
      if (power_on_) {
        if (mode_ == Mode::COOL || mode_ == Mode::DRY || mode_ == Mode::HEAT) {
          if (night_ != on) {
            night_ = on;
            if (night_) {
              if (eco_)
              {
                set_eco_internal(false, false);
              }
            }
            send_command();
          }
        } else {
          night_ = false;
        }
      } else {
        night_ = false;
      }
    }

    void ACW02::set_purifier(bool on) {
      if (power_on_) {
        purifier_ = on;
      } else {
        purifier_ = false;
      }
    }

    void ACW02::set_mute(bool on) {
      mute_ = on;
      mute_pref_.save(&mute_);
    }

    void ACW02::set_unit(const std::string &unit) {
      bool oldValue = use_fahrenheit_;
      bool newValue = (unit == "°F" || unit == "°f");

      if (oldValue != newValue) {
        use_fahrenheit_ = newValue;
        publish_discovery_climate(true);
        publish_discovery_temperature_number(true);
        publish_discovery_temperature_sensor(true);
      }
    }

    void ACW02::set_clean(bool on) {
      if (on != clean_) {
        if (on && (power_on_ || mode_ != Mode::COOL && mode_ != Mode::DRY))
        {
          power_on_ = false;
          if (mode_ != Mode::COOL && mode_ != Mode::DRY) {
            mode_ = Mode::COOL;
          }
          send_command_basic(build_frame());
          force_clean_ = true;
          clean_ = on;
          set_timeout("clean_delay", 3000, [this, on]() {
            send_command_basic(build_frame(true));
            force_clean_ = false;
          });
        } else {
          clean_ = on;
          force_clean_ = false;
          send_command_basic(build_frame(true));
        }
      }
    }

    void ACW02::set_disable_mode_auto(bool on) {
      if (disable_mode_auto_ != on) {
        disable_mode_auto_ = on;
        disable_mode_auto_pref_.save(&disable_mode_auto_);
      }
    }

    void ACW02::set_disable_mode_heat(bool on) {
      if (disable_mode_heat_ != on) {
        disable_mode_heat_ = on;
        disable_mode_heat_pref_.save(&disable_mode_heat_);
      }
    }

    void ACW02::set_disable_mode_dry(bool on) {
      if (disable_mode_dry_ != on) {
        disable_mode_dry_ = on;
        disable_mode_dry_pref_.save(&disable_mode_dry_);
      }
    }

    void ACW02::set_disable_mode_fan(bool on) {
      if (disable_mode_fan_ != on) {
        disable_mode_fan_ = on;
        disable_mode_fan_pref_.save(&disable_mode_fan_);
      }
    }

    void ACW02::set_disable_swing_vertical(bool on) {
      if (disable_swing_vertical_ != on) {
        disable_swing_vertical_ = on;
        disable_swing_vertical_pref_.save(&disable_swing_vertical_);
      }
    }

    void ACW02::set_disable_swing_horizontal(bool on) {
      if (disable_swing_horizontal_ != on) {
        disable_swing_horizontal_ = on;
        disable_swing_horizontal_pref_.save(&disable_swing_horizontal_);
      }
    }

    void ACW02::set_option_recalculate_climate(bool on) {
      if (option_recalculate_climate_ != on) {
        option_recalculate_climate_ = on;
        option_recalculate_climate_pref_.save(&option_recalculate_climate_);
        publish_discovery_climate(true);
      }
    }

    void ACW02::set_mqtt_broker(const std::string &value) {
      mqtt_broker_address_ = value;
      char buf[64];
      std::strncpy(buf, value.c_str(), sizeof(buf));
      buf[sizeof(buf) - 1] = '\0';
      mqtt_broker_address_pref_.save((const char (*)[64]) &buf);
    }

    void ACW02::set_mqtt_username(const std::string &value) {
      mqtt_username_ = value;
      char buf[64];
      std::strncpy(buf, value.c_str(), sizeof(buf));
      buf[sizeof(buf) - 1] = '\0';
      mqtt_username_pref_.save((const char (*)[64]) &buf);
    }

    void ACW02::set_mqtt_password(const std::string &value) {
      mqtt_password_ = value;
      char buf[64];
      std::strncpy(buf, value.c_str(), sizeof(buf));
      buf[sizeof(buf) - 1] = '\0';
      mqtt_password_pref_.save((const char (*)[64]) &buf);
    }

    void ACW02::set_mqtt_port_from_string(const std::string &value) {
      if (value.empty() || !std::all_of(value.begin(), value.end(), ::isdigit)) {
        ESP_LOGI(TAG, "Invalid MQTT port (non-numeric) : '%s'", value.c_str());
        return;
      }

      int port = std::stoi(value);
      if (port < 1 || port > 65535) {
        ESP_LOGI(TAG, "Invalid MQTT port (out of range) : %d", port);
        port = 1883;
      }

      mqtt_port_ = port;
      mqtt_port_pref_.save(&mqtt_port_);

      ESP_LOGI(TAG, "MQTT port saved : %d", port);
    }

    void ACW02::reload_ac_info() {
      send_command_basic(get_status_frame_);
    }

    std::string ACW02::get_mac_address() {
      uint8_t mac[6];
      esp_read_mac(mac, ESP_MAC_WIFI_STA);
      char buf[13];
      snprintf(buf, sizeof(buf), "%02x%02x%02x%02x%02x%02x",
      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      return std::string(buf);
    }

    std::string ACW02::get_address() {
      return wifi::global_wifi_component->get_use_address();
    }

    std::string ACW02::get_ip_address() {
      IPAddress ip = WiFi.localIP();
     return std::string(ip.toString().c_str());
    }

    std::string ACW02::get_mode_string_climate() const {
      if (is_power_on()) {
        return mode_to_string_climate(mode_);
      } else {
        return "off";
      }
    }

    std::string ACW02::get_mode_string() const {
      if (is_power_on()) {
        return mode_to_string(app_lang_, mode_);
      } else {
        return key_to_txt(app_lang_, "mode", "OFF");
      }
    }

    std::string ACW02::get_fan_string() const {
      return fan_to_string(app_lang_, fan_);
    }

    std::string ACW02::get_swing_string() const {
      return swing_to_string(app_lang_, swing_position_);
    }

    std::string ACW02::get_unit() const {
      return use_fahrenheit_ ? "°F" : "°C";
    }

    float ACW02::get_target_temperature_c() const {
      return target_temp_c_;
    }

    float ACW02::get_target_temperature_f() const {
      return target_temp_f_;
    }

    bool ACW02::is_power_on() const {
      return power_on_;
    }

    bool ACW02::is_eco_on() const {
      return eco_;
    }

    bool ACW02::is_night_on() const {
      return night_;
    }

    bool ACW02::is_purifier_on() const {
      return purifier_;
    }

    bool ACW02::is_display_on() const {
      return display_;
    }

    bool ACW02::is_mute_on() const {
      return mute_;
    }

    bool ACW02::is_clean_on() const {
      return clean_;
    }

    bool ACW02::is_using_fahrenheit() const {
      return use_fahrenheit_;
    }

    bool ACW02::is_disable_mode_auto() const {
      return disable_mode_auto_;
    }

    bool ACW02::is_disable_mode_heat() const {
      return disable_mode_heat_;
    }

    bool ACW02::is_disable_mode_dry() const {
      return disable_mode_dry_;
    }

    bool ACW02::is_disable_mode_fan() const {
      return disable_mode_fan_;
    }

    bool ACW02::is_disable_swing_vertical() const {
      return disable_swing_vertical_;
    }

    bool ACW02::is_disable_swing_horizontal() const {
      return disable_swing_horizontal_;
    }

    bool ACW02::is_option_recalculate_climate() const {
      return option_recalculate_climate_;
    }

    std::string ACW02::get_mqtt_broker() const {
      return mqtt_broker_address_;
    }

    std::string ACW02::get_mqtt_username() const {
      return mqtt_username_;
    }

    std::string ACW02::get_mqtt_password() const {
      return mqtt_password_;
    }

    int ACW02::get_mqtt_port() const {
      return mqtt_port_;
    }

    void ACW02::mqtt_connexion() {
      mqtt_ = mqtt::global_mqtt_client;
      mqtt_broker_address_pref_ = global_preferences->make_preference<char[64]>(9991U);
      char bufBroker[64] = {0};
      if (mqtt_broker_address_pref_.load((char (*)[64]) &bufBroker)) {
        mqtt_broker_address_ = bufBroker;
      } else {
        mqtt_broker_address_ = "";
      }

      mqtt_username_pref_ = global_preferences->make_preference<char[64]>(9992U);
      char bufUsername[64] = {0};
      if (mqtt_username_pref_.load((char (*)[64]) &bufUsername)) {
        mqtt_username_ = bufUsername;
      } else {
        mqtt_username_ = "";
      }

      mqtt_password_pref_ = global_preferences->make_preference<char[64]>(9993U);
      char bufPassword[64] = {0};
      if (mqtt_password_pref_.load((char (*)[64]) &bufPassword)) {
        mqtt_password_ = bufPassword;
      } else {
        mqtt_password_ = "";
      }

      mqtt_port_pref_ = global_preferences->make_preference<int>(9994U);
      if (!mqtt_port_pref_.load(&mqtt_port_)) {
        mqtt_port_ = 1883;
      }

      mqtt_->disable();
      mqtt_->set_enable_on_boot(false);
      mqtt_->set_broker_address(mqtt_broker_address_);
      mqtt_->set_broker_port(mqtt_port_);
      if (!mqtt_username_.empty())
      mqtt_->set_username(mqtt_username_);
      if (!mqtt_password_.empty())
      mqtt_->set_password(mqtt_password_);

      set_timeout("mqtt_connect", 2000, [this]() {
        if (wifi::global_wifi_component->is_connected()) {
          ESP_LOGI(TAG, "Wi-Fi OK; attempting MQTT connection…");
          mqtt_->enable();
          set_timeout("mqtt_retry", 5000, [this]() {
            if (!mqtt_->is_connected()) {
              mqtt_->disable();
            }
          });
        } else {
          ESP_LOGI(TAG, "Wi-Fi not connected; retrying in 5 s");
          set_timeout("mqtt_retry", 2000, [this]() { mqtt_connexion(); });
        }
      });
    }

    void ACW02::mqtt_callback(const std::string &topic, const std::string &payload) {
      std::string cmd = topic.substr(topic.find_last_of('/') + 1);
      ESP_LOGI(TAG, "mqtt_callback_ payload %s %s %s", cmd.c_str(), topic.c_str(), payload.c_str());
      if (cmd == "power_climate") {
        set_mode_climate(payload == "off" ? "off" : get_mode_string_climate());
        send_command();
      } else if (cmd == "power") {
        set_mode(payload == key_to_txt(app_lang_, "mode", "OFF") ? key_to_txt(app_lang_, "mode", "OFF") : get_mode_string());
        send_command();
      } else if (cmd == "mode_climate") {
        set_mode_climate(payload);
        send_command();
      } else if (cmd == "mode") {
        set_mode(payload);
        send_command();
      } else if (cmd == "fan") {
        set_fan(payload);
      } else if (cmd == "temp_c") {
        set_temperature_c(std::stof(payload));
      } else if (cmd == "temp_f") {
        set_temperature_f(std::stof(payload));
      } else if (cmd == "eco") {
        set_eco(payload == "on");
      } else if (cmd == "night") {
        set_night(payload == "on");
      } else if (cmd == "purifier") {
        set_purifier(payload == "on");
        send_command();
      } else if (cmd == "display") {
        set_display(payload == "on");
      } else if (cmd == "swing") {
        set_swing(payload);
        send_command();
      }  else if (cmd == "swing_horizontal") {
        set_swing_horizontal(payload);
        send_command();
      } else if (cmd == "clean") {
        set_clean(payload == "on");
      } else if (cmd == "unit") {
        set_unit(payload);
        send_command();
      }
      publish_state();
    }

    void ACW02::publish_state() {
      if (!mqtt_)
        return;

      std::string payload = "{";
      payload += "\"power_climate\":\"" + std::string(power_on_ ? "on" : "off") + "\",";
      payload += "\"power\":\"" + std::string(power_on_ ? "on" : "off") + "\",";
      payload += "\"mode_climate\":\"" + get_mode_string_climate() + "\",";
      payload += "\"mode\":\"" + get_mode_string() + "\",";
      payload += "\"fan\":\"" + fan_to_string(app_lang_, fan_) + "\",";
      payload += "\"temp_c\":" + std::to_string(target_temp_c_) + ",";
      payload += "\"temp_f\":" + std::to_string(target_temp_f_) + ",";
      payload += "\"ambient_c\":" + std::to_string(ambient_temp_c_) + ",";
      payload += "\"ambient_f\":" + std::to_string(ambient_temp_f_) + ",";
      payload += "\"eco\":\"" + std::string(eco_ ? "on" : "off") + "\",";
      payload += "\"night\":\"" + std::string(night_ ? "on" : "off") + "\",";
      payload += "\"clean\":\"" + std::string(clean_ ? "on" : "off") + "\",";
      payload += "\"purifier\":\"" + std::string(purifier_ ? "on" : "off") + "\",";
      payload += "\"display\":\"" + std::string(display_ ? "on" : "off") + "\",";
      payload += "\"swing\":\"" + swing_to_string(app_lang_, swing_position_) + "\",";
      payload += "\"swing_horizontal\":\"" + swing_horizontal_to_string(app_lang_, swing_horizontal_) + "\",";
      payload += "\"unit\":\"" + std::string(use_fahrenheit_ ? "°F" : "°C") + "\",";
      payload += "\"last_cmd_origin\":\"" + std::string(from_remote_ ? "Remote" : "ESP") + "\"";
      payload += "}";

      mqtt_->publish(app_name_ + "/state", payload, 0, true);
    }

    void ACW02::publish_availability() {
      if (!mqtt_) return;

      const std::string topic_eco = app_name_ + "/eco_availability";
      std::string payload_eco = (mode_ == Mode::COOL) ? "available" : "unavailable";
      mqtt_->publish(topic_eco, payload_eco, 1, true);

      const std::string topic_night = app_name_ + "/night_availability";
      std::string payload_night = (power_on_ && (mode_ == Mode::COOL || mode_ == Mode::DRY || mode_ == Mode::HEAT)) ? "available" : "unavailable";
      mqtt_->publish(topic_night, payload_night, 1, true);

      const std::string topic_fan_speed = app_name_ + "/fan_speed_availability";
      std::string payload_fan_speed = (eco_ == false) ? "available" : "unavailable";
      mqtt_->publish(topic_fan_speed, payload_fan_speed, 1, true);

      const std::string topic_target_temp = app_name_ + "/target_temp_availability";
      std::string payload_target_temp = (mode_ != Mode::AUTO && eco_ == false) ? "available" : "unavailable";
      mqtt_->publish(topic_target_temp, payload_target_temp, 1, true);

      const std::string topic_purifier = app_name_ + "/purifier_availability";
      std::string payload_purifier = (power_on_) ? "available" : "unavailable";
      mqtt_->publish(topic_purifier, payload_purifier, 1, true);

      const std::string topic_clean = app_name_ + "/clean_availability";
      std::string payload_clean = (!power_on_) ? "available" : "unavailable";
      mqtt_->publish(topic_clean, payload_clean, 1, true);
    }

    std::string ACW02::build_common_config_suffix() const {
      return R"(
      ,"dev": {
        "ids": [")" + app_sanitize_name_ + R"("],
        "name": ")" + app_friendly_name_ + R"(",
        "mdl": ")" + app_board_ + R"(",
        "mf": "Espressif",
        "cns": [["mac",")" + app_mac_ + R"("]]
      })";
    }

    void ACW02::publish_discovery_climate(bool recreate) {
      if (!mqtt_) return;

      const std::string topic_base = app_name_;
      const std::string unique_id = app_sanitize_name_ + "_mqtt_climate";

      const std::string temp_suffix = use_fahrenheit_ ? "_f" : "_c";

      bool disable_set_temp = false;
      if (option_recalculate_climate_) {
        if (eco_ || mode_ == Mode::AUTO) {
          disable_set_temp = true;
        }
      }
      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "climate") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "stat_t": ")" + topic_base + R"(/state",
        "cmd_t": ")" + topic_base + R"(/cmd/mode_climate",)";
        if (!disable_swing_vertical_ && !disable_swing_horizontal_) {
          payload += R"(
          "preset_mode_command_topic": ")" + topic_base + R"(/cmd/swing_horizontal",
          "preset_mode_state_topic": ")" + topic_base + R"(/state",
          "preset_mode_value_template": "{{ value_json.swing_horizontal }}",
          "preset_modes": )" + build_options_json(app_lang_, "swingHorizontal") + R"(,)";
        }
        payload += R"(
        "mode_stat_t": ")" + topic_base + R"(/state",
        "mode_stat_tpl": "{{ value_json.mode_climate }}",
        "mode_cmd_t": ")" + topic_base + R"(/cmd/mode_climate",
        "modes": )" + build_modes_json_climate() + R"(,)";

        if (disable_set_temp == false) {
          payload += R"(
          "temp_cmd_t": ")" + topic_base + "/cmd/temp" + temp_suffix + R"(",
          "temp_stat_t": ")" + topic_base + R"(/state",
          "temp_stat_tpl": "{{ value_json.temp)" + temp_suffix + R"( }}",)";
        }

        payload += R"(
        "curr_temp_t": ")" + topic_base + R"(/state",
        "curr_temp_tpl": "{{ value_json.ambient)" + temp_suffix + R"( }}",
        "fan_mode_cmd_t": ")" + topic_base + R"(/cmd/fan",
        "fan_mode_stat_t": ")" + topic_base + R"(/state",
        "fan_mode_stat_tpl": "{{ value_json.fan }}",
        "fan_modes": )" + build_fan_speed_json() + R"(,)";
        
        if (!disable_swing_vertical_ && !disable_swing_horizontal_) {
          payload += R"(
          "swing_mode_cmd_t": ")" + topic_base + R"(/cmd/swing",
          "swing_mode_stat_t": ")" + topic_base + R"(/state",
          "swing_mode_stat_tpl": "{{ value_json.swing }}",
          "swing_modes": )" + build_options_json(app_lang_, "swing") + R"(,)";
        } else {
          if (disable_swing_vertical_ && !disable_swing_horizontal_) {
             payload += R"(
            "swing_mode_cmd_t": ")" + topic_base + R"(/cmd/swing_horizontal",
            "swing_mode_stat_t": ")" + topic_base + R"(/state",
            "swing_mode_stat_tpl": "{{ value_json.swing_horizontal }}",
            "swing_modes": )" + build_options_json(app_lang_, "swingHorizontal") + R"(,)";
          } else if (disable_swing_horizontal_ && !disable_swing_vertical_) {
             payload += R"(
            "swing_mode_cmd_t": ")" + topic_base + R"(/cmd/swing",
            "swing_mode_stat_t": ")" + topic_base + R"(/state",
            "swing_mode_stat_tpl": "{{ value_json.swing }}",
            "swing_modes": )" + build_options_json(app_lang_, "swing") + R"(,)";
          }
        }
        payload += R"(
        "pow_cmd_t": ")" + topic_base + R"(/cmd/power_climate",
        "pow_stat_t": ")" + topic_base + R"(/state",
        "pow_stat_tpl": "{{ value_json.power_climate }}",
        "min_temp": )" + (use_fahrenheit_ ? "61" : "16") + R"(,
        "max_temp": )" + (use_fahrenheit_ ? "88" : "31") + R"(,
        "temp_step": 1,
        "avty_t":")" + topic_base + R"(/status")" +
        build_common_config_suffix() + R"(
      })";

      std::string config_topic = "homeassistant/climate/" + topic_base + "/config";
      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_climate_publish", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::publish_discovery_mode_select(bool recreate) {
      if (!mqtt_)
      return;

      const std::string topic_base = app_name_;
      const std::string unique_id = app_sanitize_name_ + "_mqtt_mode";

      std::string config_topic = "homeassistant/select/" + topic_base + "-mode/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "mode") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "icon": "mdi:thermostat",
        "cmd_t": ")" + topic_base + R"(/cmd/mode",
        "stat_t": ")" + topic_base + R"(/state",
        "val_tpl": "{{ value_json.mode }}",
        "options": )" + build_modes_json() + R"(,
        "avty_t": ")" + topic_base + R"(/status",
        "pl_avail": "online",
        "pl_not_avail": "offline")" +
        build_common_config_suffix() + R"(
      })";

      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_mode_select_publish", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::publish_discovery_fan_select(bool recreate) {
      if (!mqtt_)
      return;

      const std::string topic_base = app_name_;
      const std::string unique_id = app_sanitize_name_ + "_mqtt_fan";

      std::string config_topic = "homeassistant/select/" + topic_base + "-fan/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "fan") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "icon": "mdi:fan",
        "cmd_t": ")" + topic_base + R"(/cmd/fan",
        "stat_t": ")" + topic_base + R"(/state",
        "val_tpl": "{{ value_json.fan }}",
        "options": )" + build_options_json(app_lang_, "fan") + R"(,
        "availability_mode": "all",
        "availability": [
        {
          "topic": ")" + topic_base + R"(/status",
          "payload_available": "online",
          "payload_not_available": "offline"
        },
        {
          "topic": ")" + topic_base + R"(/fan_speed_availability",
          "payload_available": "available",
          "payload_not_available": "unavailable"
        }
        ])" +
        build_common_config_suffix() + R"(
      })";

      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_fan_select_publish", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::publish_discovery_unit_select(bool recreate) {
      if (!mqtt_)
      return;

      const std::string topic_base = app_name_;
      const std::string unique_id = app_sanitize_name_ + "_mqtt_unit";

      std::string config_topic = "homeassistant/select/" + topic_base + "-unit/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "unit") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "cmd_t": ")" + topic_base + R"(/cmd/unit",
        "stat_t": ")" + topic_base + R"(/state",
        "val_tpl": "{{ value_json.unit }}",
        "options": ["°C", "°F"],
        "avty_t": ")" + topic_base + R"(/status",
        "pl_avail": "online",
        "pl_not_avail": "offline")" +
        build_common_config_suffix() + R"(
      })";

      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_unit_select_publish", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::publish_discovery_swing_select(bool recreate) {
      if (!mqtt_)
      return;

      const std::string topic_base = app_name_;
      const std::string unique_id = app_sanitize_name_ + "_mqtt_swing";

      std::string config_topic = "homeassistant/select/" + topic_base + "-swing/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "swing") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "icon": "mdi:swap-vertical",
        "cmd_t": ")" + topic_base + R"(/cmd/swing",
        "stat_t": ")" + topic_base + R"(/state",
        "val_tpl": "{{ value_json.swing }}",
        "options": )" + build_options_json(app_lang_, "swing") + R"(,
        "avty_t": ")" + topic_base + R"(/status",
        "pl_avail": "online",
        "pl_not_avail": "offline")" +
        build_common_config_suffix() + R"(
      })";

      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_swing_select_publish", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::publish_discovery_swing_horizontal_select(bool recreate) {
      if (!mqtt_)
      return;

      const std::string topic_base = app_name_;
      const std::string unique_id = app_sanitize_name_ + "_mqtt_swing_horizontal";

      std::string config_topic = "homeassistant/select/" + topic_base + "-swing-horizontal/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "swingHorizontal") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "icon": "mdi:swap-vertical",
        "cmd_t": ")" + topic_base + R"(/cmd/swing_horizontal",
        "stat_t": ")" + topic_base + R"(/state",
        "val_tpl": "{{ value_json.swing_horizontal }}",
        "options": )" + build_options_json(app_lang_, "swingHorizontal") + R"(,
        "avty_t": ")" + topic_base + R"(/status",
        "pl_avail": "online",
        "pl_not_avail": "offline")" +
        build_common_config_suffix() + R"(
      })";

      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_swing_horizontal_select_publish", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::publish_discovery_clean_switch(bool recreate) {
      if (!mqtt_) return;

      const std::string topic_base = app_name_;
      const std::string unique_id = app_sanitize_name_ + "_mqtt_clean";

      std::string config_topic = "homeassistant/switch/" + topic_base + "-clean/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "clean") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "icon": "mdi:spray-bottle",
        "cmd_t": ")" + topic_base + R"(/cmd/clean",
        "stat_t": ")" + topic_base + R"(/state",
        "val_tpl": "{{ value_json.clean }}",
        "pl_on": "on",
        "pl_off": "off",
        "availability_mode": "all",
        "availability": [
        {
          "topic": ")" + topic_base + R"(/status",
          "payload_available": "online",
          "payload_not_available": "offline"
        },
        {
          "topic": ")" + topic_base + R"(/clean_availability",
          "payload_available": "available",
          "payload_not_available": "unavailable"
        }
        ])" +
        build_common_config_suffix() + R"(
      })";


      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_clean_switch_publish", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::publish_discovery_eco_switch(bool recreate) {
      if (!mqtt_) return;

      const std::string topic_base = app_name_;
      const std::string unique_id = app_sanitize_name_ + "_mqtt_eco";

      std::string config_topic = "homeassistant/switch/" + topic_base + "-eco/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "eco") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "icon": "mdi:currency-usd-off",
        "cmd_t": ")" + topic_base + R"(/cmd/eco",
        "stat_t": ")" + topic_base + R"(/state",
        "availability_mode": "all",
        "availability_template": "{{ value_json.mode == 'cool' }}",
        "val_tpl": "{{ value_json.eco }}",
        "pl_on": "on",
        "pl_off": "off",
        "availability_mode": "all",
        "availability": [
        {
          "topic": ")" + topic_base + R"(/status",
          "payload_available": "online",
          "payload_not_available": "offline"
        },
        {
          "topic": ")" + topic_base + R"(/eco_availability",
          "payload_available": "available",
          "payload_not_available": "unavailable"
        }
        ])" +
        build_common_config_suffix() + R"(
      })";

      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_eco_switch_publish", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::publish_discovery_display_switch(bool recreate) {
      if (!mqtt_)
      return;

      const std::string topic_base = app_name_;
      const std::string unique_id = app_sanitize_name_ + "_mqtt_display";

      std::string config_topic = "homeassistant/switch/" + topic_base + "-display/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "display") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "icon": "mdi:lightbulb",
        "cmd_t": ")" + topic_base + R"(/cmd/display",
        "stat_t": ")" + topic_base + R"(/state",
        "val_tpl": "{{ value_json.display }}",
        "pl_on": "on",
        "pl_off": "off",
        "avty_t": ")" + topic_base + R"(/status",
        "pl_avail": "online",
        "pl_not_avail": "offline")" +
        build_common_config_suffix() + R"(
      })";

      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_display_switch_publish", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::publish_discovery_night_switch(bool recreate) {
      if (!mqtt_)
      return;

      const std::string topic_base = app_name_;
      const std::string unique_id = app_sanitize_name_ + "_mqtt_night";

      std::string config_topic = "homeassistant/switch/" + topic_base + "-night/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "night") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "cmd_t": ")" + topic_base + R"(/cmd/night",
        "stat_t": ")" + topic_base + R"(/state",
        "val_tpl": "{{ value_json.night }}",
        "pl_on": "on",
        "pl_off": "off",
        "icon": "mdi:power-sleep",
        "availability_mode": "all",
        "availability": [
        {
          "topic": ")" + topic_base + R"(/status",
          "payload_available": "online",
          "payload_not_available": "offline"
        },
        {
          "topic": ")" + topic_base + R"(/night_availability",
          "payload_available": "available",
          "payload_not_available": "unavailable"
        }
        ])" +
        build_common_config_suffix() + R"(
      })";

      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_night_switch_publish", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::publish_discovery_purifier_switch(bool recreate) {
      if (!mqtt_)
      return;

      const std::string topic_base = app_name_;
      const std::string unique_id = app_sanitize_name_ + "_mqtt_purifier";

      std::string config_topic = "homeassistant/switch/" + topic_base + "-purifier/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "purifier") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "cmd_t": ")" + topic_base + R"(/cmd/purifier",
        "stat_t": ")" + topic_base + R"(/state",
        "val_tpl": "{{ value_json.purifier }}",
        "pl_on": "on",
        "pl_off": "off",
        "icon": "mdi:leaf-circle",
        "availability_mode": "all",
        "availability": [
        {
          "topic": ")" + topic_base + R"(/status",
          "payload_available": "online",
          "payload_not_available": "offline"
        },
        {
          "topic": ")" + topic_base + R"(/purifier_availability",
          "payload_available": "available",
          "payload_not_available": "unavailable"
        }
        ])" +
        build_common_config_suffix() + R"(
      })";

      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_purifier_switch_publish", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::publish_discovery_temperature_number(bool recreate) {
      if (!mqtt_)
      return;

      const std::string topic_base = app_name_;
      const std::string unit = use_fahrenheit_ ? "°F" : "°C";
      const std::string temp_suffix = use_fahrenheit_ ? "_f" : "_c";
      const std::string unique_id = app_sanitize_name_ + "_mqtt_temp";
      const std::string icon = use_fahrenheit_ ? "mdi:temperature-fahrenheit" : "mdi:temperature-celsius";

      float min = use_fahrenheit_ ? 61.0f : 16.0f;
      float max = use_fahrenheit_ ? 88.0f : 31.0f;

      std::string config_topic = "homeassistant/number/" + topic_base + "-temp/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "temp") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "cmd_t": ")" + topic_base + "/cmd/temp" + temp_suffix + R"(",
        "stat_t": ")" + topic_base + R"(/state",
        "val_tpl": "{{ value_json.temp)" + temp_suffix + R"( }}",
        "min": )" + to_string(min) + R"(,
        "max": )" + to_string(max) + R"(,
        "step": 1,
        "mode": "box",
        "icon": ")" + icon + R"(",
        "unit_of_meas": ")" + unit + R"(",
        "availability_mode": "all",
        "availability": [
        {
          "topic": ")" + topic_base + R"(/status",
          "payload_available": "online",
          "payload_not_available": "offline"
        },
        {
          "topic": ")" + topic_base + R"(/target_temp_availability",
          "payload_available": "available",
          "payload_not_available": "unavailable"
        }
        ])" +
        build_common_config_suffix() + R"(
      })";

      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_temperature_number_publish", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::publish_discovery_temperature_sensor(bool recreate) {
      if (!mqtt_) return;

      const std::string topic_base = app_name_;
      const std::string unit = use_fahrenheit_ ? "°F" : "°C";
      const std::string temp_suffix = use_fahrenheit_ ? "_f" : "_c";
      const std::string unique_id = app_sanitize_name_ + "_mqtt_sensor_temp";
      const std::string icon = use_fahrenheit_ ? "mdi:temperature-fahrenheit" : "mdi:temperature-celsius";

      std::string config_topic = "homeassistant/sensor/" + topic_base + "-temp/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "ambient") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "stat_t": ")" + topic_base + R"(/state",
        "unit_of_meas": ")" + unit + R"(",
        "val_tpl": "{{ value_json.ambient)" + temp_suffix + R"( }}",
        "state_class": "measurement",
        "icon": ")" + icon + R"(",
        "avty_t": ")" + topic_base + R"(/status",
        "pl_avail": "online",
        "pl_not_avail": "offline")" +
        build_common_config_suffix() + R"(
      })";

      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_temp_sensor", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

        void ACW02::publish_discovery_last_cmd_origin_sensor(bool recreate) {
      if (!mqtt_) return;

      const std::string topic_base = app_name_;
      const std::string unique_id = app_sanitize_name_ + "_mqtt_sensor_last_cmd_origin";

      std::string config_topic = "homeassistant/sensor/" + topic_base + "-last-cmd-origin/config";

      std::string payload = R"({
        "name": ")" + get_localized_name(app_lang_, "lastCmdOrigin") + R"(",
        "object_id": ")" + unique_id + R"(",
        "unique_id": ")" + unique_id + R"(",
        "stat_t": ")" + topic_base + R"(/state",
        "icon" : "mdi:origin",
        "val_tpl": "{{ value_json.last_cmd_origin }}",
        "avty_t": ")" + topic_base + R"(/status",
        "pl_avail": "online",
        "pl_not_avail": "offline")" +
        build_common_config_suffix() + R"(
      })";

      if (recreate) {
        mqtt_->publish(config_topic, std::string(""), 1, true);
        set_timeout("mqtt_publish_discovery_last_cmd_origin_sensor", mqtt_delay_rebuild_, [this, config_topic, payload]() {
          mqtt_->publish(config_topic, payload, 1, true);
        });
      } else {
        mqtt_->publish(config_topic, payload, 1, true);
      }
    }

    void ACW02::rebuild_mqtt_entity() {
      publish_discovery_climate(true);
      publish_discovery_mode_select(true);
      publish_discovery_fan_select(true);
      publish_discovery_swing_select(true);
      publish_discovery_swing_horizontal_select(true);
      publish_discovery_unit_select(true);
      publish_discovery_clean_switch(true);
      publish_discovery_eco_switch(true);
      publish_discovery_display_switch(true);
      publish_discovery_night_switch(true);
      publish_discovery_purifier_switch(true);
      publish_discovery_temperature_number(true);
      publish_discovery_temperature_sensor(true);
      publish_discovery_last_cmd_origin_sensor(true);
    }

    void ACW02::apply_disable_settings() {
      if (mqtt_) {
        publish_discovery_climate(true);
        publish_discovery_mode_select(true);
        publish_discovery_fan_select(true);
        publish_discovery_swing_select(true);
        publish_discovery_swing_horizontal_select(true);
      }
    }

    std::string ACW02::sanitize_name(const std::string &input) const  {
      std::string output = input;
      std::replace(output.begin(), output.end(), '-', '_');
      std::replace(output.begin(), output.end(), ' ', '_');
      std::transform(output.begin(), output.end(), output.begin(), ::tolower);
      return output;
    }

    std::string ACW02::int_to_string(int value) {
      char buf[12];
      snprintf(buf, sizeof(buf), "%d", value);
      return std::string(buf);
    }

    void ACW02::process_tx_queue() {
      if (tx_queue_.empty())
      return;

      // security for concurr RX/TX
      if (!rx_buffer_.empty() || (millis() - last_rx_byte_time_ < 100)) {
        return;
      }

      if (millis() - last_tx_ < TX_INTERVAL_MS)
      return;

      const auto &pkt = tx_queue_.front();
      ESP_LOGI(TAG, "TX: [%s]", format_hex_pretty(pkt).c_str());
      write_array(pkt);
      last_tx_ = millis();
      tx_queue_.pop_front();
    }

    std::vector<uint8_t> ACW02::build_frame(bool bypassMute) const {
      std::vector<uint8_t> frame(24, 0x00);
      frame[0] = frame[1] = 0x7A;
      frame[2] = 0x21;
      frame[3] = 0xD5;
      frame[4] = 0x18;
      frame[5] = frame[6] = 0x00;
      frame[7] = 0xA1;

      uint8_t fan_n = (static_cast<uint8_t>(fan_) & 0x0F) << 4;
      uint8_t pwr   = (power_on_ ? 1 : 0) << 3;
      uint8_t mode  = static_cast<uint8_t>(mode_) & 0x07;
      frame[12] = fan_n | pwr | mode;
      if (clean_ == false) {
        uint8_t base = encode_temperature_byte();
        if (fan_ == Fan::SILENT) {
          frame[13] = base  + 0x40;
        }
        else {
          frame[13] = base;
        }
      } else {
        frame[13] = 0x0B;
      }
      frame[14] = (static_cast<uint8_t>(swing_horizontal_) << 4) | (static_cast<uint8_t>(swing_position_) & 0x0F);

      uint8_t b15 = 0;
      if (eco_)      b15 |= 0x01;
      if (night_)    b15 |= 0x02;
      if (clean_)    b15 |= 0x10;
      if (purifier_) b15 |= 0x40;
      if (display_)  b15 |= 0x80;

      frame[15] = b15;
      if (is_mute_on()) {
        if (bypassMute == false) {
          frame[16] = 0x01;
        }
      }
      uint16_t crc = crc16(frame.data(), 22);
      frame[22] = (crc >> 8) & 0xFF;
      frame[23] = crc & 0xFF;
      return frame;
    }

    void ACW02::send_command_basic(const std::vector<uint8_t> &data) {
      tx_queue_.push_back(data);
    }

    void ACW02::send_command() {
      clean_ = false;
      force_clean_ = false;
      ESP_LOGI(TAG, "send command");
      send_command_basic(build_frame());
    }

    uint16_t ACW02::crc16(const uint8_t *data, size_t len) {
      uint16_t crc = 0xFFFF;
      for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j)
        crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : (crc >> 1);
      }
      return crc;
    }

    void ACW02::decode_state(const std::vector<uint8_t> &f) {

      if (f.size() == 28 && f[0] == 0x7A && f[1] == 0x7A && f[2] == 0xD5 && f[3] == 0x21) {
        uint8_t fault = f[12];
        if (fault != 0x00) {
          std::string fault_msg;
          switch (fault) {
            case 0x04: fault_msg = "unsupported_mode"; break;
            case 0x01: fault_msg = "sensor_error"; break;
            default: fault_msg = "unknown_fault"; break;
          }

          ESP_LOGE(TAG, "AC error : fault_code=0x%02X (%s)", fault, fault_msg.c_str());
        }
        return;
      }


      if (f.size() < 34 || f[0] != 0x7A || f[1] != 0x7A)
      return;

      bool previous_fahrenheit = use_fahrenheit_;
      bool previous_eco = eco_;
      Mode previous_mode = mode_;

      const uint8_t b13       = f[13];
      power_on_               = b13 & 0x08;

      mode_ = static_cast<Mode>(b13 & 0x07);
      fan_  = static_cast<Fan>((b13 >> 4) & 0x0F);


      uint8_t temp_byte_rx = f[14];
      const bool silent_bit = (temp_byte_rx & 0x40);

      temp_byte_rx &= 0x3F;

      float temp_c{}, temp_f{};
      bool is_fahrenheit = false;

      for (int i = 0; i < 28; i++) {
        if (temp_byte_rx == fahrenheit_encoding_table_[i]) {
          is_fahrenheit   = true;
          use_fahrenheit_ = true;
          target_temp_f_  = static_cast<uint8_t>(i + 61);
          target_temp_c_  = static_cast<uint8_t>(fahrenheit_to_celsius(target_temp_f_));
          break;
        }
      }
      if (!is_fahrenheit) {
        use_fahrenheit_ = false;
        target_temp_c_  = 16 + temp_byte_rx;
        target_temp_f_  = static_cast<uint8_t>(celsius_to_fahrenheit(target_temp_c_));
      }

      if (silent_bit) {
        fan_ = Fan::SILENT;
      }

      // swing_position_ = static_cast<Swing>(f[15]);
      const uint8_t swing_raw = f[15];
      swing_position_ = static_cast<Swing>(swing_raw & 0x0F);
      swing_horizontal_ = static_cast<SwingHorizontal>((swing_raw >> 4) & 0x0F);

      const uint8_t flags = f[16];
      eco_      = flags & 0x01;
      night_    = flags & 0x02;
      if (!force_clean_) {
        // Only consider the clean bit if mode cool of dry
        if (mode_ == Mode::COOL || mode_ == Mode::DRY) {
          clean_ = flags & 0x10;
        } else {
          clean_ = false;
        }
      }
      from_remote_ = flags & 0x04;
      purifier_ = flags & 0x40;
      display_  = flags & 0x80;

      ESP_LOGI(TAG,
      "RX decode: PWR=%s | Mode=%s | Mode climate=%s | Fan=%s | Temp=%.1f°C / %.1f°F [%s] | "
      "Eco=%d | Night=%d | Clean=%d | Purifier=%d | Display=%d | Swing=%s | SwingH=%s | Silent=%s | Origin=%s",
      power_on_ ? "ON" : "OFF",
      mode_to_string(app_lang_, mode_).c_str(),
      mode_to_string_climate(mode_).c_str(),
      fan_to_string(app_lang_, fan_).c_str(),
      static_cast<float>(target_temp_c_), static_cast<float>(target_temp_f_), is_fahrenheit ? "°F" : "°C",
      eco_, night_, clean_, purifier_, display_,
      swing_to_string(app_lang_, swing_position_).c_str(),
      swing_horizontal_to_string(app_lang_, swing_horizontal_).c_str(),
      silent_bit ? "YES" : "NO",
      from_remote_ ? "Remote" : "ESP");

      if (f.size() >= 12) {
        const uint8_t temp_int = f[10];
        const uint8_t temp_dec = f[11];
        ambient_temp_c_  = temp_int + temp_dec / 10.0f;
        ambient_temp_f_  = celsius_to_fahrenheit(ambient_temp_c_, false);

        ESP_LOGI(TAG, "RX ambient temp: %.1f°C / %.1f°F (raw=0x%02X 0x%02X)",
        ambient_temp_c_, ambient_temp_f_, temp_int, temp_dec);
      }

      if (mqtt_) {
        publish_availability();
        publish_state();
      }

      if (previous_eco != eco_) {
        recalculate_climate_depending_by_option();
      } else if (previous_mode != mode_ && (previous_mode == Mode::AUTO || mode_ == Mode::AUTO)) {
        recalculate_climate_depending_by_option();
      } else if (previous_fahrenheit != use_fahrenheit_) {
        publish_discovery_climate(true);
      }
      force_cool_mode_if_disabled();
    }

    void ACW02::force_cool_mode_if_disabled() {
      Mode tmp = mode_;
      if (mode_ == Mode::AUTO && is_disable_mode_auto()) tmp = Mode::COOL;
      if (mode_ == Mode::HEAT && is_disable_mode_heat()) tmp = Mode::COOL;
      if (mode_ == Mode::FAN && is_disable_mode_fan())   tmp = Mode::COOL;
      if (mode_ == Mode::DRY && is_disable_mode_dry())   tmp = Mode::COOL;
      if (tmp != mode_) {
        mode_ = tmp;
        send_command();
      }
    }

    Fan ACW02::str_to_fan(const std::string& lang, const std::string &s) {
      if (s == key_to_txt(lang, "fan", "AUTO"))    return Fan::AUTO;
      if (s == key_to_txt(lang, "fan", "P20"))     return Fan::P20;
      if (s == key_to_txt(lang, "fan", "P40"))     return Fan::P40;
      if (s == key_to_txt(lang, "fan", "P60"))     return Fan::P60;
      if (s == key_to_txt(lang, "fan", "P80"))     return Fan::P80;
      if (s == key_to_txt(lang, "fan", "P100"))    return Fan::P100;
      if (s == key_to_txt(lang, "fan", "SILENT"))  return Fan::SILENT;
      if (s == key_to_txt(lang, "fan", "TURBO"))   return Fan::TURBO;
      return Fan::AUTO;
    }

    Mode ACW02::str_to_mode_climate(const std::string &m) {
      if (m == "dry") return Mode::DRY;
      if (m == "fan_only") return Mode::FAN;
      if (m == "auto") return Mode::AUTO;
      if (m == "heat") return Mode::HEAT;
      return Mode::COOL;
    }

    Mode ACW02::str_to_mode(const std::string& lang, const std::string &m) {
      if (m == key_to_txt(lang, "mode", "DRY")) return Mode::DRY;
      if (m == key_to_txt(lang, "mode", "FAN")) return Mode::FAN;
      if (m == key_to_txt(lang, "mode", "AUTO")) return Mode::AUTO;
      if (m == key_to_txt(lang, "mode", "HEAT")) return Mode::HEAT;
      return Mode::COOL;
    }

    std::string ACW02::mode_to_string_climate(Mode mode) {
      switch (mode) {
        case Mode::COOL: return "cool";
        case Mode::DRY:  return "dry";
        case Mode::FAN:  return "fan_only";
        case Mode::HEAT: return "heat";
        case Mode::AUTO: return "auto";
        default:         return "?";
      }
    }

    std::string ACW02::mode_to_string(const std::string& lang, Mode mode) {
      switch (mode) {
        case Mode::COOL: return key_to_txt(lang, "mode", "COOL");
        case Mode::DRY:  return key_to_txt(lang, "mode", "DRY");
        case Mode::FAN:  return key_to_txt(lang, "mode", "FAN");
        case Mode::HEAT: return key_to_txt(lang, "mode", "HEAT");
        case Mode::AUTO: return key_to_txt(lang, "mode", "AUTO");
        default:         return "?";
      }
    }

    std::string ACW02::fan_to_string(const std::string& lang, Fan fan) {
      switch (fan) {
        case Fan::AUTO:   return key_to_txt(lang, "fan", "AUTO");
        case Fan::P20:    return key_to_txt(lang, "fan", "P20");
        case Fan::P40:    return key_to_txt(lang, "fan", "P40");
        case Fan::P60:    return key_to_txt(lang, "fan", "P60");
        case Fan::P80:    return key_to_txt(lang, "fan", "P80");
        case Fan::P100:   return key_to_txt(lang, "fan", "P100");
        case Fan::SILENT: return key_to_txt(lang, "fan", "SILENT");
        case Fan::TURBO:  return key_to_txt(lang, "fan", "TURBO");
        default:          return "?";
      }
    }

    Swing ACW02::str_to_swing(const std::string& lang, const std::string &s) {
      if (s == key_to_txt(lang, "swing", "STOP")) return Swing::STOP;
      if (s == key_to_txt(lang, "swing", "SWING")) return Swing::SWING;
      if (s == key_to_txt(lang, "swing", "P1"))   return Swing::P1;
      if (s == key_to_txt(lang, "swing", "P2"))   return Swing::P2;
      if (s == key_to_txt(lang, "swing", "P3"))   return Swing::P3;
      if (s == key_to_txt(lang, "swing", "P4"))   return Swing::P4;
      if (s == key_to_txt(lang, "swing", "P5"))   return Swing::P5;
      return Swing::P1;
    }

    std::string ACW02::swing_to_string(const std::string& lang, Swing swing) {
      switch (swing) {
        case Swing::STOP:  return key_to_txt(lang, "swing", "STOP");
        case Swing::SWING: return key_to_txt(lang, "swing", "SWING");
        case Swing::P1:    return key_to_txt(lang, "swing", "P1");
        case Swing::P2:    return key_to_txt(lang, "swing", "P2");
        case Swing::P3:    return key_to_txt(lang, "swing", "P3");
        case Swing::P4:    return key_to_txt(lang, "swing", "P4");
        case Swing::P5:    return key_to_txt(lang, "swing", "P5");
        default:           return "?";
      }
    }

    SwingHorizontal ACW02::str_to_swing_horizontal(const std::string& lang, const std::string &s) {
      if (s == key_to_txt(lang, "swingHorizontal", "STOP"))           return SwingHorizontal::STOP;
      if (s == key_to_txt(lang, "swingHorizontal", "AUTO_LEFT"))      return SwingHorizontal::AUTO_LEFT;
      if (s == key_to_txt(lang, "swingHorizontal", "P1"))             return SwingHorizontal::P1;
      if (s == key_to_txt(lang, "swingHorizontal", "P2"))             return SwingHorizontal::P2;
      if (s == key_to_txt(lang, "swingHorizontal", "P3"))             return SwingHorizontal::P3;
      if (s == key_to_txt(lang, "swingHorizontal", "P4"))             return SwingHorizontal::P4;
      if (s == key_to_txt(lang, "swingHorizontal", "P5"))             return SwingHorizontal::P5;
      if (s == key_to_txt(lang, "swingHorizontal", "RANGE_P1_P5"))    return SwingHorizontal::RANGE_P1_P5;
      if (s == key_to_txt(lang, "swingHorizontal", "AUTO_MID_OUT"))   return SwingHorizontal::AUTO_MID_OUT;
      return SwingHorizontal::STOP;
    }

    std::string ACW02::swing_horizontal_to_string(const std::string& lang, SwingHorizontal swing) {
       switch (swing) {
        case SwingHorizontal::STOP:           return key_to_txt(lang, "swingHorizontal", "STOP");
        case SwingHorizontal::AUTO_LEFT:     return key_to_txt(lang, "swingHorizontal", "AUTO_LEFT");
        case SwingHorizontal::P1:            return key_to_txt(lang, "swingHorizontal", "P1");
        case SwingHorizontal::P2:            return key_to_txt(lang, "swingHorizontal", "P2");
        case SwingHorizontal::P3:            return key_to_txt(lang, "swingHorizontal", "P3");
        case SwingHorizontal::P4:            return key_to_txt(lang, "swingHorizontal", "P4");
        case SwingHorizontal::P5:            return key_to_txt(lang, "swingHorizontal", "P5");
        case SwingHorizontal::RANGE_P1_P5:   return key_to_txt(lang, "swingHorizontal", "RANGE_P1_P5");
        case SwingHorizontal::AUTO_MID_OUT:  return key_to_txt(lang, "swingHorizontal", "AUTO_MID_OUT");
        default:                             return "?";
      }
    }

    float ACW02::celsius_to_fahrenheit(float c, bool floor) const {
      if (floor) {
        return floorf(c * 9.0f / 5.0f + 32.0f);
      } else {
        return (c * 9.0f / 5.0f + 32.0f);
      }
    }

    float ACW02::fahrenheit_to_celsius(float f) const {
      return roundf((f - 32.0f) * 5.0f / 9.0f);
    }

    uint8_t ACW02::encode_temperature_byte() const {
      if (use_fahrenheit_) {
        uint8_t t = target_temp_f_;
        if (t < 61) t = 61;
        if (t > 88) t = 88;
        uint8_t idx = t - 61;
        if (idx >= sizeof(fahrenheit_encoding_table_)) return 0x20;
        return fahrenheit_encoding_table_[idx];
      } else {
        uint8_t t = target_temp_c_;
        if (t < 16) t = 16;
        if (t > 31) t = 31;
        return t - 16;
      }
    }

    std::string ACW02::build_modes_json_climate() const {
      std::string out = "[\"off\"";
      auto push = [&out](const char *m) {
        out += ",\""; out += m; out += "\"";
      };

      if (!is_disable_mode_auto()) push("auto");
      push("cool");
      if (!is_disable_mode_dry() ) push("dry");
      if (!is_disable_mode_heat()) push("heat");
      if (!is_disable_mode_fan() ) push("fan_only");

      out += "]";
      return out;
    }

    std::string ACW02::build_modes_json() const {
      const auto &mode_dict = LOCALIZED.at(app_lang_).at("mode");
      std::vector<std::string> out;
      for (auto it = mode_dict.begin(); it != mode_dict.end(); ++it) {
        const std::string &key = it->first;
        const std::string &label = it->second;
        if (key == "OFF") {
          out.push_back(label);
        } else if (key == "AUTO" && !is_disable_mode_auto()) {
          out.push_back(label);
        } else if (key == "COOL") {
          out.push_back(label);
        } else if (key == "DRY" && !is_disable_mode_dry()) {
          out.push_back(label);
        } else if (key == "HEAT" && !is_disable_mode_heat()) {
          out.push_back(label);
        } else if (key == "FAN" && !is_disable_mode_fan()) {
          out.push_back(label);
        }
      }
      return "[\"" + join(out, R"(",")") + "\"]";
    }

    std::string ACW02::build_fan_speed_json() const {
      const auto &fan_map = LOCALIZED.at(app_lang_).at("fan");
      std::vector<std::string> options;
      for (const auto &kv : fan_map) {
        const std::string &key = kv.first;
        if (eco_ && option_recalculate_climate_ && key != "AUTO")
        continue;
        options.push_back(kv.second);
      }
      return "[\"" + join(options, R"(",")") + "\"]";
    }

    void ACW02::recalculate_climate_depending_by_option() {
      if (option_recalculate_climate_) {
        publish_discovery_climate(true);
      }
    }

  }
}
