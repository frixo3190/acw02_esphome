#pragma once
#include <map>
#include <string>
#include <vector>


namespace acw02_localization {


using OptDict   = std::vector<std::pair<std::string, std::string>>;
using CatDict   = std::map<std::string, OptDict>;
using LocaleMap = std::map<std::string, CatDict>;

static const std::map<std::string, std::map<std::string, std::string>> LOCALIZED_NAMES = {
        {"fr", {
            {"climate", "AC"},
            {"mode", "Mode"},
            {"fan", "Ventilation"},
            {"swing", "Oscillation vertical"},
            {"swingHorizontal", "Oscillation horizontal"},
            {"eco", "Eco"},
            {"night", "Nuit"},
            {"clean", "Nettoyage"},
            {"purifier", "Purificateur"},
            {"display", "Affichage"},
            {"unit", "Unité"},
            {"temp", "Temp cible"},
            {"ambient", "Température"},
            {"lastCmdOrigin", "Origine commande"},
            {"filterToClean", "Filtre à nettoyer"},
            {"g1Mute", "G1: Commande silencieuse"},
            {"g1OptionRecalculateClimate", "G1: Auto calc climate (eco)"},
            {"restartModule", "G1: Redémarrer"},
            {"rebuildMQTT", "G1: Reconstruire les entitées MQTT"},
            {"getStatus", "G1: Actualiser data"},
            {"resetEcoPurifier", "G1: Mode éteint reset Eco/Purificateur"}
        }},
        {"en", {
            {"climate", "AC"},
            {"mode", "Mode"},
            {"fan", "Fan"},
            {"swing", "Vertical swing"},
            {"swingHorizontal", "Horizontal swing"},
            {"eco", "Eco"},
            {"night", "Night"},
            {"clean", "Clean"},
            {"purifier", "Purifier"},
            {"display", "Display"},
            {"unit", "Unit"},
            {"temp", "Target Temp"},
            {"ambient", "Temperature"},
            {"lastCmdOrigin", "Command origin"},
            {"filterToClean", "Filter to clean"},
            {"g1Mute", "G1: Mute command"},
            {"g1OptionRecalculateClimate", "G1: Auto calc climate (eco)"},
            {"restartModule", "G1: Restart"},
            {"rebuildMQTT", "G1: Rebuild MQTT entities"},
            {"getStatus", "G1: Refresh data"},
            {"resetEcoPurifier", "G1: Mode Off reset Eco/Purifier"}
        }}
    };

static const LocaleMap LOCALIZED{
  { "fr", {
      { "mode",         { {"AUTO","Auto"},{"HEAT","Chauffage"},{"COOL","Climatisation"},
                          {"DRY","Déshumidification"},{"FAN","Ventilation uniquement"},{"OFF","Éteint"} } },
      { "fan",          { {"AUTO","Auto"},{"P20","20%"},{"P40","40%"},{"P60","60%"},
                          {"P80","80%"},{"P100","100%"},{"SILENT","Silencieux"},{"TURBO","Turbo"} } },
      { "swing",        { {"STOP","Désactivé"},{"SWING","Activé"},
                          {"P1","Position 1"},{"P2","Position 2"},{"P3","Position 3"},
                          {"P4","Position 4"},{"P5","Position 5"} } },
      { "swingHorizontal", { {"STOP","Désactivé"},{"AUTO_LEFT","Auto gauche-droit"},
                        {"P1","Position 1"},{"P2","Position 2"},{"P3","Position 3"},
                        {"P4","Position 4"},{"P5","Position 5"},{"RANGE_P1_P5","P1-P5"},{"AUTO_MID_OUT","Auto centre-extérieur"} } }
  }},
  { "en", {
      { "mode",         { {"AUTO","Auto"},{"HEAT","Heat"},{"COOL","Cool"},
                          {"DRY","Dry"},{"FAN","Fan only"}, {"OFF","Off"} } },
      { "fan",          { {"AUTO","Auto"},{"P20","20%"},{"P40","40%"},{"P60","60%"},
                          {"P80","80%"},{"P100","100%"},{"SILENT","Silent"},{"TURBO","Turbo"} } },
      { "swing",        { {"STOP","Off"},{"SWING","On"},
                          {"P1","Position 1"},{"P2","Position 2"},{"P3","Position 3"},
                          {"P4","Position 4"},{"P5","Position 5"} } },
      { "swingHorizontal", { {"STOP","Off"},{"AUTO_LEFT","Auto left-right"},
                        {"P1","Position 1"},{"P2","Position 2"},{"P3","Position 3"},
                        {"P4","Position 4"},{"P5","Position 5"},{"RANGE_P1_P5","P1-P5"},{"AUTO_MID_OUT","Auto center-out"} } }
  }}
};

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

inline std::string build_options_json(const std::string& lang, const std::string& cat) {
  auto opts = list_txt(lang, cat);
  return "[\"" + join(opts, R"(",")") + "\"]";
}

}  // namespace acw02_localization
