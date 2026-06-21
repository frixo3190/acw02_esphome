#pragma once
#include <map>
#include <string>
#include <vector>
#include <algorithm>  // std::remove

namespace acw02_localization {

using OptDict   = std::vector<std::pair<std::string, std::string>>;
using CatDict   = std::map<std::string, OptDict>;
using LocaleMap = std::map<std::string, CatDict>;

// ---------- INTERNAL BUILDERS (no global static init) ----------

inline const std::map<std::string, std::map<std::string, std::string>>& _LOCALIZED_NAMES_() {
  static const std::map<std::string, std::map<std::string, std::string>> LN = {
    {"fr", {
      {"climate","Climatiseur"},
      {"mode","Mode"},
      {"fan","Ventilation"},
      {"swing","Oscillation vertical"},
      {"swingHorizontal","Oscillation horizontal"},
      {"eco","Eco"},
      {"night","Nuit"},
      {"clean","Nettoyage"},
      {"purifier","Purificateur"},
      {"display","Affichage"},
      {"unit","Unité"},
      {"temp","Temp cible"},
      {"ambient","Température"},
      {"lastCmdOrigin","Origine commande"},
      {"filterToClean","Filtre à nettoyer"},
      {"g1Mute","G1: Commande silencieuse"},
      {"g1OptionRecalculateClimate","G1: Auto calc climate (eco)"},
      {"restartModule","G1: Redémarrer"},
      {"rebuildMQTT","G1: Reconstruire les entitées MQTT"},
      {"getStatus","G1: Actualiser data"},
      {"resetEcoPurifier","G1: Mode éteint reset Eco/Purificateur"},
      {"warn","Warn"},
      {"error","Error"},
      {"warnText","Warn Info"},
      {"errorText","Error Info"},
      {"ZCMDMuteDelay","G1: Z-CMD Delai cmd silencieuse"},
      {"ZCMDMuteAfterOnDelay","G1: Z-CMD Delai cmd silencieuse après un allumage"},
      {"ZCMDPublishAfterOnDelay","G1: Z-CMD Delai de publication de status apres un allumage"},
      {"disableModeAuto","Z-Config: Exclu mode Auto"},
      {"disableModeHeat","Z-Config: Exclu mode Chauffage"},
      {"disableModeDry","Z-Config: Exclu mode Déshumidification"},
      {"disableModeFan","Z-Config: Exclu mode Ventilation"},
      {"disableSwingVertical","Z-Config: Exclu oscillation vertical"},
      {"disableSwingHorizontal","Z-Config: Exclu oscillation horizontal"},
      {"disableValidate","Z-Config: Valider"},
      {"preset","Z-Preset-1: Presets dans climate"},
      {"presetList","Presets"},
      {"presetListConfig","Z-Preset-2: liste"},
      {"presetName","Z-Preset-3: Nom"},
      {"presetSave","Z-Preset-4: Sauvegarder"},
      {"presetDelete","Z-Preset-5: Supprimer"},
      {"presetNone","Aucun"},
      {"cmdFailureCounter","Compteur d'échec de cmd"},
      {"MDNS","WiFi: mDNS"},
      {"WiFiIP","WiFi: IP"},
      {"WiFiSSID","WiFi: Point d'accès connecté"},
      {"WiFiBSSID","WiFi: BSSID"},
      {"WiFiMAC","WiFi: MAC"},
      {"ESPVERSION","ESP: Version"},
      {"WiFiSIGNAL","WiFi: Signal"},
      {"INTERNALTEMP","ESP: Température CPU"},
      {"ESPMEMORY","ESP: Mémoire libre"}
    }},
    {"en", {
      {"climate","AC"},
      {"mode","Mode"},
      {"fan","Fan"},
      {"swing","Vertical swing"},
      {"swingHorizontal","Horizontal swing"},
      {"eco","Eco"},
      {"night","Night"},
      {"clean","Clean"},
      {"purifier","Purifier"},
      {"display","Display"},
      {"unit","Unit"},
      {"temp","Target Temp"},
      {"ambient","Temperature"},
      {"lastCmdOrigin","Command origin"},
      {"filterToClean","Filter to clean"},
      {"g1Mute","G1: Mute command"},
      {"g1OptionRecalculateClimate","G1: Auto calc climate (eco)"},
      {"restartModule","G1: Restart"},
      {"rebuildMQTT","G1: Rebuild MQTT entities"},
      {"getStatus","G1: Refresh data"},
      {"resetEcoPurifier","G1: Mode Off reset Eco/Purifier"},
      {"warn","Warn"},
      {"error","Error"},
      {"warnText","Warn Info"},
      {"errorText","Error Info"},
      {"ZCMDMuteDelay","G1: Z-CMD Delay mute between cmd"},
      {"ZCMDMuteAfterOnDelay","G1: Z-CMD Delay mute next cmd after AC on"},
      {"ZCMDPublishAfterOnDelay","G1: Z-CMD Delai publish state after AC on"},
      {"disableModeAuto","Z-Config: Auto mode excluded"},
      {"disableModeHeat","Z-Config: Heat mode excluded"},
      {"disableModeDry","Z-Config: Dry mode excluded"},
      {"disableModeFan","Z-Config: Fan mode excluded"},
      {"disableSwingVertical","Z-Config: Swing vertical excluded"},
      {"disableSwingHorizontal","Z-Config: Swing horizontal excluded"},
      {"disableValidate","Z-Config: Validate"},
      {"preset","Z-Preset-1: Presets in climate"},
      {"presetList","Presets"},
      {"presetListConfig","Z-Preset-2: list"},
      {"presetName","Z-Preset-3: Name"},
      {"presetSave","Z-Preset-4: Save"},
      {"presetDelete","Z-Preset-5: Delete"},
      {"presetNone","Off"},
      {"cmdFailureCounter","Cmd failure counter"},
      {"MDNS","WiFi: mDNS"},
      {"WiFiIP","WiFi: IP"},
      {"WiFiSSID","WiFi: Connected Access Point"},
      {"WiFiBSSID","WiFi: BSSID"},
      {"WiFiMAC","WiFi: MAC"},
      {"ESPVERSION","ESP: Version"},
      {"WiFiSIGNAL","WiFi: Signal"},
      {"INTERNALTEMP","ESP: CPU Temperature"},
      {"ESPMEMORY","ESP: Free Memory"}
    }}
  };
  return LN;
}

inline const LocaleMap& _LOCALIZED_() {
  static const LocaleMap L = {
    { "fr", {
        { "mode", OptDict{
          {"AUTO","Auto"},{"HEAT","Chauffage"},{"COOL","Climatisation"},
          {"DRY","Déshumidification"},{"FAN","Ventilation uniquement"},{"OFF","Éteint"} } },
        { "fan", OptDict{
          {"AUTO","Auto"},{"P20","20%"},{"P40","40%"},{"P60","60%"},
          {"P80","80%"},{"P100","100%"},{"SILENT","Silencieux"},{"TURBO","Turbo"} } },
        { "swing", OptDict{
          {"STOP","Désactivé"},{"SWING","Activé"},
          {"P1","Position 1"},{"P2","Position 2"},{"P3","Position 3"},
          {"P4","Position 4"},{"P5","Position 5"} } },
        { "swingHorizontal", OptDict{
          {"STOP","Désactivé"},{"AUTO_LEFT","Auto gauche-droit"},
          {"P1","Position 1"},{"P2","Position 2"},{"P3","Position 3"},
          {"P4","Position 4"},{"P5","Position 5"},{"RANGE_P1_P5","P1-P5"},{"AUTO_MID_OUT","Auto centre-extérieur"} } }
    }},
    { "en", {
        { "mode", OptDict{
          {"AUTO","Auto"},{"HEAT","Heat"},{"COOL","Cool"},
          {"DRY","Dry"},{"FAN","Fan only"},{"OFF","Off"} } },
        { "fan", OptDict{
          {"AUTO","Auto"},{"P20","20%"},{"P40","40%"},{"P60","60%"},
          {"P80","80%"},{"P100","100%"},{"SILENT","Silent"},{"TURBO","Turbo"} } },
        { "swing", OptDict{
          {"STOP","Off"},{"SWING","On"},
          {"P1","Position 1"},{"P2","Position 2"},{"P3","Position 3"},
          {"P4","Position 4"},{"P5","Position 5"} } },
        { "swingHorizontal", OptDict{
          {"STOP","Off"},{"AUTO_LEFT","Auto left-right"},
          {"P1","Position 1"},{"P2","Position 2"},{"P3","Position 3"},
          {"P4","Position 4"},{"P5","Position 5"},{"RANGE_P1_P5","P1-P5"},{"AUTO_MID_OUT","Auto center-out"} } }
    }}
  };
  return L;
}

// Expose the same symbol name used by your code:
#define LOCALIZED       (::acw02_localization::_LOCALIZED_())
#define LOCALIZED_NAMES (::acw02_localization::_LOCALIZED_NAMES_())

// ---------- HELPERS (string versions to match your calls) ----------

inline std::string join(const std::vector<std::string>& vec,
                        const std::string& sep = ",") {
  std::string out;
  for (size_t i = 0; i < vec.size(); ++i) {
    out += vec[i];
    if (i + 1 < vec.size()) out += sep;
  }
  return out;
}

inline std::string key_to_txt(const std::string& lang, const std::string& cat, const std::string& key) {
  auto loc_it = LOCALIZED.find(lang);
  if (loc_it == LOCALIZED.end()) return key;
  const auto& cat_map = loc_it->second;
  auto c_it = cat_map.find(cat);
  if (c_it == cat_map.end()) return key;
  for (const auto& kv : c_it->second)
    if (kv.first == key) return kv.second;
  return key;
}

inline std::string txt_to_key(const std::string& lang, const std::string& cat, const std::string& txt) {
  auto loc_it = LOCALIZED.find(lang);
  if (loc_it == LOCALIZED.end()) return txt;
  const auto& cat_map = loc_it->second;
  auto c_it = cat_map.find(cat);
  if (c_it == cat_map.end()) return txt;
  for (const auto& kv : c_it->second)
    if (kv.second == txt) return kv.first;
  return txt;
}

inline std::vector<std::string> list_txt(const std::string& lang, const std::string& cat) {
  std::vector<std::string> out;
  auto loc_it = LOCALIZED.find(lang);
  if (loc_it == LOCALIZED.end()) return out;
  const auto& cat_map = loc_it->second;
  auto c_it = cat_map.find(cat);
  if (c_it == cat_map.end()) return out;
  out.reserve(c_it->second.size());
  for (const auto& kv : c_it->second) out.push_back(kv.second);
  return out;
}

inline std::string get_localized_name(const std::string& lang, const std::string& key) {
  auto lang_it = LOCALIZED_NAMES.find(lang);
  if (lang_it == LOCALIZED_NAMES.end()) return key;
  const auto& map = lang_it->second;
  auto it = map.find(key);
  return (it != map.end()) ? it->second : key;
}

inline std::string build_options_json(const std::string& lang, const std::string& cat, const std::string& exclude_key = "") {
  auto opts = list_txt(lang, cat);
  if (!exclude_key.empty()) {
    const auto excluded_txt = key_to_txt(lang, cat, exclude_key);
    opts.erase(std::remove(opts.begin(), opts.end(), excluded_txt), opts.end());
  }
  return "[\"" + join(opts, R"(",")") + "\"]";
}

} // namespace acw02_localization
