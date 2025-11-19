#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include <vector>
#include <cstdint>
#include "esphome/core/preferences.h"
#include <deque>
#include "esphome/components/mqtt/mqtt_client.h"
// #include <esp_wifi.h>
#include <esp_mac.h>
#include "esphome/core/application.h"
#include <algorithm>
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text/text.h"
#include "localization.h"

namespace esphome {
namespace acw02 {

using namespace acw02_localization;

  // Enum for mapping action to byte
  enum class Mode : uint8_t { COOL = 0x01, DRY = 0x02, FAN = 0x03, HEAT = 0x04, AUTO = 0x00 };
  enum class Fan  : uint8_t { AUTO = 0x0, P20 = 0x1, P40 = 0x2, P60 = 0x3,
                              P80 = 0x4, P100 = 0x5, SILENT = 0x6, TURBO = 0xD };
  enum class Swing  : uint8_t { P1 = 0x02, P2 = 0x03, P3 = 0x04, P4 = 0x05,
                          P5 = 0x06, SWING = 0x01, STOP = 0x00 };
  enum class SwingHorizontal : uint8_t { STOP = 0x0, AUTO_LEFT = 0x1, P1 = 0x2, P2 = 0x3,
                                         P3 = 0x4, P4 = 0x5, P5 = 0x6, RANGE_P1_P5 = 0xC, AUTO_MID_OUT = 0xD};

  // globals Statics
  static const char *const TAG = "acw02";

  struct Frame_with_Fingerprint {
    uint32_t fingerprint;
    std::string description;
    std::vector<uint8_t> frame;
    uint32_t timestamp_ms;
    int tryCnt;
  };

  static constexpr int maxRetry = 3;


  // globals Statics base frame (keep alive?)
  static const std::vector<uint8_t> keepalive_frame_ = {
    0x7A, 0x7A, 0x21, 0xD5, 0x0C, 0x00, 0x00, 0xAB,
    0x0A, 0x0A, 0xFC, 0xF9
  };

  // globals Statics base frame (ask status AC)
  static const std::vector<uint8_t> get_status_frame_ = {
    0x7A, 0x7A, 0x21, 0xD5, 0x0C, 0x00, 0x00, 0xA2,
    0x0A, 0x0A, 0xFE, 0x29
  };

  // globals Statics byte for °F from 61 to 88
  static const uint8_t fahrenheit_encoding_table_[28] = { 
    0x20, 0x21, 0x31, 0x22, 0x32, 0x23, 0x33, 0x24,0x25,
    0x35, 0x26, 0x36, 0x27, 0x37, 0x28, 0x38, 0x29,0x2A, 
    0x3A, 0x2B, 0x3B, 0x2C, 0x3C, 0x2D, 0x3D, 0x2E, 0x2F, 
    0x3F };

  struct PresetSlot {
    int index;
    std::string name;
    Frame_with_Fingerprint frame_with_fp;
  };

  const std::string PRESETS_LIST_ELEMENT_CONFIG_DEFAULT = "_______________";

class ACW02 : public Component, public uart::UARTDevice {
 public:
  

  //override
  void setup() override;
  void loop() override;
  
  // Setters publics AC
  void set_mode_climate(const std::string &mode);      
  void set_mode(const std::string &mode);
  void set_swing(const std::string &pos);   
  void set_swing_horizontal(const std::string &pos);       
  void set_purifier(bool on);
  void set_mute(bool on);
  void set_unit(const std::string &unit); 
  void set_clean(bool on);

  // Setters publics AC with bool for detect if send cmd is necessary
  bool set_temperature_c(float temp); 
  bool set_temperature_f(float temp);              
  bool set_fan(const std::string &speed);         
  bool set_display(bool on);
  bool set_eco(bool on);
  bool set_eco_internal(bool on, bool force = false);
  bool set_night(bool on);

  // Setters reset option when AC off
  void set_auto_off_options_when_ac_off(bool on);

  // Setters set mute after ac power on with delay
  void set_mute_next_cmd_delay_string(const std::string &value);
  void set_mute_next_cmd_after_on_delay_string(const std::string &value);
  void set_publish_stats_after_power_on_delay_string(const std::string &value);

  
  // Setters G1 : MQTT
  void set_g1_mqtt_options(bool on);

  // Setters publics disable option mode
  void set_disable_mode_auto(bool on, bool published = true);
  void set_disable_mode_heat(bool on, bool published = true);
  void set_disable_mode_dry(bool on, bool published = true);
  void set_disable_mode_fan(bool on, bool published = true);
  void set_disable_swing_vertical(bool on, bool published = true);
  void set_disable_swing_horizontal(bool on, bool published = true);

  // Setters publics, sync element esphome native in yaml to cpp
  void entity_mdns_sync(const std::string &payload);
  void entity_wifi_ip_sync(const std::string &payload);
  void entity_wifi_ssid_sync(const std::string &payload);
  void entity_wifi_bssid_sync(const std::string &payload);
  void entity_wifi_mac_sync(const std::string &payload);
  void entity_esp_version_sync(const std::string &payload);
  void entity_wifi_signal_sync(const std::string &payload);
  void entity_internal_temperature_sync(const std::string &payload);
  void entity_esp_memory_sync(const std::string &payload);

  // Setters publics presets
  void set_preset(bool on, bool published = true);

  // Setters publics option for rebuild climate if mode auto or option eco enable
  void set_option_recalculate_climate(bool on);

  // Setters publics MQTT
  void set_mqtt_broker(const std::string &value);
  void set_mqtt_username(const std::string &value);
  void set_mqtt_password(const std::string &value);
  void set_mqtt_port_from_string(const std::string &value);
  void set_mqtt_connected_sensor(esphome::binary_sensor::BinarySensor *sensor);

  // Setters sensor fault/warn
  void set_filter_dirty_sensor(binary_sensor::BinarySensor *sensor);
  void set_warn_sensor(binary_sensor::BinarySensor *sensor);
  void set_error_sensor(binary_sensor::BinarySensor *sensor);
  void set_warn_text_sensor(esphome::text_sensor::TextSensor *sensor);
  void set_error_text_sensor(esphome::text_sensor::TextSensor *sensor);
  void set_cmd_ignore_tx_sensor(esphome::text_sensor::TextSensor *sensor);
  void set_cmd_ignore_rx_sensor(esphome::text_sensor::TextSensor *sensor);

  // Setters sensor for mute with delay and condition
  void set_mute_next_cmd_delay_text(esphome::text::Text *text);
  void set_mute_next_cmd_after_on_delay_text(esphome::text::Text *text);
  void set_publish_stats_after_power_on_delay_text(esphome::text::Text *text);


  // Send command UART
  void reload_ac_info();

  // Getters publics globals
  std::string get_mac_address();
  std::string get_address();

  // Getters publics AC
  std::string get_mode_string_climate() const;
  std::string get_mode_string() const;
  std::string get_fan_string() const;
  std::string get_swing_string() const;
  std::string get_unit() const;

  float get_target_temperature_c() const;
  float get_target_temperature_f() const;
 
  bool is_power_on() const;
  bool is_eco_on() const;
  bool is_night_on() const;
  bool is_purifier_on() const;
  bool is_display_on() const;
  bool is_mute_on() const;
  bool is_clean_on() const;
  bool is_using_fahrenheit() const;

  // reset option when AC off
  bool is_auto_off_options_when_ac_off() const;

  // mute with delay and condition
  int get_mute_next_cmd_delay() const;
  int get_mute_next_cmd_after_on_delay() const;
  int get_publish_stats_after_power_on_delay() const;
  
  // G1: MQTT
   bool is_g1_mqtt_options() const;

  // Getters publics disable option mode
  bool is_disable_mode_auto() const;
  bool is_disable_mode_heat() const;
  bool is_disable_mode_dry() const;
  bool is_disable_mode_fan() const;
  bool is_disable_swing_vertical() const;
  bool is_disable_swing_horizontal() const;

  bool is_option_recalculate_climate() const;

  //  Getters publics MQTT
  std::string get_mqtt_broker() const;
  std::string get_mqtt_username() const;
  std::string get_mqtt_password() const;
  int get_mqtt_port() const;

  // MQTT publics function
  void mqtt_connexion();
  void mqtt_initializer();
  void mqtt_callback(const std::string &topic, const std::string &payload);
  void publish_state();
  void publish_availability();
  std::string build_common_config_suffix() const;
  void publish_discovery_climate_deref();
  void publish_discovery_climate(bool recreate = false);
  void publish_discovery_mode_select(bool recreate = false);
  void publish_discovery_fan_select(bool recreate = false);
  void publish_discovery_unit_select(bool recreate = false);
  void publish_discovery_swing_select(bool recreate = false);
  void publish_discovery_swing_horizontal_select(bool recreate = false);
  void publish_discovery_preset_select(bool recreate = false);
  void publish_discovery_preset_config_select(bool recreate = false);
  void publish_discovery_clean_switch(bool recreate = false);
  void publish_discovery_eco_switch(bool recreate = false);
  void publish_discovery_display_switch(bool recreate = false);
  void publish_discovery_night_switch(bool recreate = false);
  void publish_discovery_purifier_switch(bool recreate = false);
  void publish_discovery_g1_mute_switch(bool recreate = false);
  void publish_discovery_g1_reset_eco_purifier_ac_off_switch(bool recreate = false);
  void publish_discovery_g1_option_recalculate_climate_switch(bool recreate = false);
  void publish_discovery_disable_mode_auto_switch(bool recreate = false);
  void publish_discovery_disable_mode_heat_switch(bool recreate = false);
  void publish_discovery_disable_mode_dry_switch(bool recreate = false);
  void publish_discovery_disable_mode_fan_switch(bool recreate = false);
  void publish_discovery_disable_swing_vertical_switch(bool recreate = false);
  void publish_discovery_disable_swing_horizontal_switch(bool recreate = false);
  void publish_discovery_preset_switch(bool recreate = false);
  void publish_discovery_g1_mute_next_cmd_delay_text(bool recreate = false);
  void publish_discovery_g1_mute_next_cmd_after_on_delay_text(bool recreate = false);
  void publish_discovery_g1_publish_stats_after_power_on_delay_text(bool recreate = false);
  void publish_discovery_preset_name_config_text(bool recreate = false);
  void publish_discovery_temperature_number(bool recreate = false);
  void publish_discovery_g1_reload_button(bool recreate = false);
  void publish_discovery_g1_rebuild_mqtt_entities_button(bool recreate = false);
  void publish_discovery_g1_get_status_button(bool recreate = false);
  void publish_discovery_z_config_validate_button(bool recreate = false);
  void publish_discovery_preset_save_button(bool recreate = false);
  void publish_discovery_preset_delete_button(bool recreate = false);
  void publish_discovery_temperature_sensor(bool recreate = false);
  void publish_discovery_last_cmd_origin_sensor(bool recreate = false);
  void publish_discovery_filter_dirty_sensor(bool recreate = false);
  void publish_discovery_warn_sensor(bool recreate = false);
  void publish_discovery_error_sensor(bool recreate = false);
  void publish_discovery_cmd_failure_counter_sensor(bool recreate = false);
  void publish_discovery_warn_text_sensor(bool recreate = false);
  void publish_discovery_error_text_sensor(bool recreate = false);
  void publish_discovery_mdns_text_sensor(bool recreate = false);
  void publish_discovery_wifi_ip_text_sensor(bool recreate = false);
  void publish_discovery_wifi_ssid_text_sensor(bool recreate = false);
  void publish_discovery_wifi_bssid_text_sensor(bool recreate = false);
  void publish_discovery_wifi_mac_text_sensor(bool recreate = false);
  void publish_discovery_esp_version_text_sensor(bool recreate = false);
  void publish_discovery_wifi_signal_text_sensor(bool recreate = false);
  void publish_discovery_internal_temperature_text_sensor(bool recreate = false);
  void publish_discovery_esp_memory_text_sensor(bool recreate = false);

  // MQTT publics function for rebuild all mqtt entities
  void rebuild_mqtt_entity();
  // MQTT publics function for apply disable option mode
  void apply_disable_settings();
  
  // Functions publics
  std::string sanitize_name(const std::string &input) const;
  std::string int_to_string(int value);

 protected:
  // variables
  // variables globales
  std::string app_name_ {""};
  std::string app_friendly_name_ {""};
  std::string app_sanitize_name_ {""};
  std::string app_mac_ {""};
  std::string app_lang_ {"en"};
  std::string app_board_ {""};

  std::string app_mdns_ {""};
  std::string app_wifi_ip_ {""};
  std::string app_wifi_ssid_ {""};
  std::string app_wifi_bssid_ {""};
  std::string app_wifi_mac_ {""};
  std::string app_esp_version_ {""};
  std::string app_wifi_signal_ {""};
  std::string app_internal_temperature_ {""};
  std::string app_esp_memory_ {""};

  // variables command queue
  std::vector<uint8_t> rx_buffer_;
  uint32_t last_rx_byte_time_{0};
  std::deque<Frame_with_Fingerprint> tx_queue_;
  uint32_t last_tx_{0};
  static constexpr uint32_t INTERVAL_MQTT_BETWEEN_CMD = 60;
  static constexpr uint32_t DELAY_MQTT_G1_GENERATE = 1500;
  static constexpr uint32_t INTERVAL_INTERNAL_MQTT_KEEPALIVE = 15000;
  static constexpr uint32_t INTERVAL_INTERNAL_AC_KEEPALIVE = 60000;
  static constexpr uint32_t SILENCE_RX_MS = 120;
  static constexpr uint32_t ACK_WINDOW_MS = 120;
  static constexpr uint32_t TX_INTERVAL_MS = 180;
  static constexpr uint32_t ACK_EVAL_MIN_MS  = 50;  // wait ≥50 ms after TX before comparing RX/TX
  uint32_t rx_max_depth_ = 0; // for check size rx buffer
  bool ack_wait_ = false;
  uint32_t ack_block_until_ = 0;
  // Timing constants (already have these near your others)
  static constexpr uint32_t ACK_RETRY_TIMEOUT_MS = 220;  // 200–300ms is fine
  static constexpr uint32_t ACK_ABORT_MS = 1200;
  bool timeout_retry_pending_{false};
  
  // variables AC
  Mode mode_ {Mode::COOL};
  bool power_on_ {false};
  Fan  fan_ {Fan::AUTO};
  Fan  previous_fan_ {Fan::AUTO};
  uint8_t target_temp_c_ {26};
  uint8_t target_temp_f_ {78};
  float ambient_temp_c_ {0.0f};
  float ambient_temp_f_ {0.0f};

  // variables ac previous target temp (mode auto)
  uint8_t previous_target_temp_c_ {26};
  uint8_t previous_target_temp_f_ {78};
  
  bool eco_ {false};
  bool night_ {false};
  bool purifier_ {false};
  bool display_ {false};
  bool mute_ {false};
  bool mute_tmp_mqtt_ {false};
  bool mute_tmp_other_ {false};
  int mute_next_cmd_delay_ = 0;
  int mute_next_cmd_after_on_delay_ = 0;
  int publish_stats_after_power_on_delay_ = 0;
  uint32_t time_publish_stats_after_power_on_ {0};
  bool clean_ {false};
  bool force_clean_ {false};
  bool use_fahrenheit_ {false};
  bool option_recalculate_climate_ {false};
  bool option_g1_mqtt_ {false};
  bool from_remote_ {false};
  bool filter_dirty_ {false};
  bool warn_ {false};
  bool error_ {false};
  std::string warn_text_ {"No Warn"};
  std::string error_text_ {"No Error"};
  Swing  swing_position_{Swing::P1};
  SwingHorizontal  swing_horizontal_{SwingHorizontal::STOP};

  // variables AC fault/warn
  binary_sensor::BinarySensor *filter_dirty_sensor_{nullptr};
  binary_sensor::BinarySensor *warn_sensor_{nullptr};
  binary_sensor::BinarySensor *error_sensor_{nullptr};
  esphome::text_sensor::TextSensor *warn_text_sensor_{nullptr};
  esphome::text_sensor::TextSensor *error_text_sensor_{nullptr};

  // sensor for mute with delay and condition
  esphome::text::Text *mute_next_cmd_delay_text_{nullptr};
  esphome::text::Text *mute_next_cmd_after_on_delay_text_{nullptr};
  esphome::text::Text *publish_stats_after_power_on_delay_text_{nullptr};

  // variables persisted AC settings
  ESPPreferenceObject mute_pref_;
  ESPPreferenceObject disable_mode_auto_pref_; 
  ESPPreferenceObject disable_mode_heat_pref_; 
  ESPPreferenceObject disable_mode_dry_pref_; 
  ESPPreferenceObject disable_mode_fan_pref_; 
  ESPPreferenceObject disable_swing_vertical_pref_; 
  ESPPreferenceObject disable_swing_horizontal_pref_; 
  ESPPreferenceObject option_recalculate_climate_pref_;
  ESPPreferenceObject option_G1_MQTT_pref_;
  ESPPreferenceObject auto_off_options_when_ac_off_pref_;

  // variables persisted MQTT
  ESPPreferenceObject mqtt_broker_address_pref_;
  ESPPreferenceObject mqtt_username_pref_;
  ESPPreferenceObject mqtt_password_pref_;
  ESPPreferenceObject mqtt_port_pref_;

  // variables persisted previous target temp (mode auto)
  ESPPreferenceObject previous_temp_c_pref_;
  ESPPreferenceObject previous_temp_f_pref_;

  // variables for mute with delay and condition
   ESPPreferenceObject mute_next_cmd_after_on_delay_pref_;
   ESPPreferenceObject mute_next_cmd_delay_pref_;
   ESPPreferenceObject publish_stats_after_power_on_delay_pref_;

   // variables persisted presets
   ESPPreferenceObject preset_pref_;


  // variables MQTT
  mqtt::MQTTClientComponent *mqtt_ = nullptr;
  std::string mqtt_broker_address_;
  std::string mqtt_username_;
  std::string mqtt_password_;
  int mqtt_port_ = 1883;
  int mqtt_delay_rebuild_short_ = 300;
  int mqtt_delay_rebuild_ = 800;
  binary_sensor::BinarySensor *mqtt_connected_sensor_{nullptr};


  // Protected variables for disable mode
  bool disable_mode_auto_ {false};
  bool disable_mode_heat_ {false};
  bool disable_mode_dry_ {false};
  bool disable_mode_fan_ {false};
  bool disable_swing_vertical_ {false};
  bool disable_swing_horizontal_ {false};


  // reset option when AC off
  bool auto_off_options_when_ac_off_ {false};

  // Protected variables optimization
  struct MqttPublishEntry {
    std::string topic;
    std::string payload;
    int qos;
    bool retain;
  };
  std::deque<MqttPublishEntry> mqtt_publish_queue_;

  // Protected functions for command queue
  void process_tx_queue();

  // jitter retry
  uint32_t compute_retry_jitter(int tryCnt) const;

  // Check and retry if cmd don't have ACK AC
  void check_timeout_retry();

  // Protected functions for command UART
  Frame_with_Fingerprint build_frame(bool bypassMute = false) const;
  std::vector<uint8_t> make_muted_with_fixed_crc(const std::vector<uint8_t>& frame) const;
   void send_static_command_basic(const std::vector<uint8_t> &data);
  void send_command_basic(const Frame_with_Fingerprint &data);
  void send_command(bool skipResetClean = false);
  static uint16_t crc16(const uint8_t *data, size_t len);
  void decode_state(const std::vector<uint8_t> &frame);

  // force mode if select disable mode
  bool force_cool_mode_if_disabled();

  // Protected functions optimization
  void publish_async(const std::string &topic, const std::string &payload, int qos, bool retain);

  // Protected functions for AC convert
  static Fan str_to_fan(const std::string& lang, const std::string &speed);
  static Mode str_to_mode_climate(const std::string &mode);
  static Mode str_to_mode(const std::string& lang, const std::string &mode);
  static std::string mode_to_string_climate(Mode mode);
  static std::string mode_to_string(const std::string& lang, Mode mode);
  static std::string fan_to_string(const std::string& lang, Fan fan);
  static Swing str_to_swing(const std::string& lang, const std::string &s);
  static SwingHorizontal str_to_swing_horizontal(const std::string& lang, const std::string &s);
  static std::string swing_to_string(const std::string& lang, Swing swing);
  static std::string swing_horizontal_to_string(const std::string& lang, SwingHorizontal swing);
  float celsius_to_fahrenheit(float c, bool floor = true) const;
  float fahrenheit_to_celsius(float f) const;
  uint8_t encode_temperature_byte() const;
  std::string build_modes_json_climate() const;
  std::string build_modes_json() const;
  std::string build_fan_speed_json() const;

  //reset option on off
  void reset_options_when_off();

  // temperature auto
  uint8_t auto_temp_defined_heat_cool_calculator();

  // Protected functions for rebuild climate if mode auto or option eco enable
  void recalculate_climate_depending_by_option();

  // fingerprint
  int cmd_failure_counter_ = 0;
  mutable Frame_with_Fingerprint cmd_send_fingerprint_ = {0, "", {}, 0, 0};
  Frame_with_Fingerprint fingerprint() const;
  uint32_t ac_to_fingerprint() const;
  std::string fingerprint_to_string() const;
  void log_fingerprint(std::string from, Frame_with_Fingerprint fp, Frame_with_Fingerprint tfp = {0, "", {}, 0, 0}) const;
  bool compare_fingerprints(uint32_t a, uint32_t b);

  // presets
  bool preset_ {false};
  std::string presets_list_element_ = {""};
  std::string presets_list_element_config_ = {PRESETS_LIST_ELEMENT_CONFIG_DEFAULT};
  std::string preset_name_config_ = {""};

  std::array<PresetSlot, 8> presets_list = {{
    {1, "Preset 1 (empty)", {0, "", {}, 0, 0}},
    {2, "Preset 2 (empty)", {0, "", {}, 0, 0}},
    {3, "Preset 3 (empty)", {0, "", {}, 0, 0}},
    {4, "Preset 4 (empty)", {0, "", {}, 0, 0}},
    {5, "Preset 5 (empty)", {0, "", {}, 0, 0}},
    {6, "Preset 6 (empty)", {0, "", {}, 0, 0}},
    {7, "Preset 7 (empty)", {0, "", {}, 0, 0}},
    {8, "Preset 8 (empty)", {0, "", {}, 0, 0}}
  }};
  std::string get_preset_list(bool only_non_empty, bool forClimate = false);
  std::string encode_frame_base64(const std::vector<uint8_t> &data);
  std::vector<uint8_t> decode_frame_base64(const std::string &base64_str);

  void update_selected_preset(const std::string &new_name, const Frame_with_Fingerprint &new_frame);
  void load_presets_from_flash();
  void save_presets_to_flash();
  void save_single_preset_to_flash(const PresetSlot &preset);
  void delete_preset_by_name();
  PresetSlot get_preset_by_name(const std::string &name);
};

}  // namespace acw02
}  // namespace esphome
