#pragma once
#include <map>
#include <string>
#include <vector>

/* Outils de localisation – aucun log ni dépendance ESPHome ici.
 * Tout est dans le namespace acw02_localization.
 */
namespace acw02_localization {


//------------------------------------------------------------------
// 1. Alias de types
//------------------------------------------------------------------
using OptDict   = std::vector<std::pair<std::string, std::string>>; // Ordonné
using CatDict   = std::map<std::string, OptDict>;
using LocaleMap = std::map<std::string, CatDict>;

//------------------------------------------------------------------
// 2. Dictionnaires
//------------------------------------------------------------------
static const std::map<std::string, std::map<std::string, std::string>> LOCALIZED_NAMES = {
        {"fr", {
            {"climate", "AC"},
            {"mode", "Mode"},
            {"fan", "Ventilation"},
            {"swing", "Oscillation"},
            {"eco", "Eco"},
            {"night", "Nuit"},
            {"clean", "Nettoyage"},
            {"purifier", "Purificateur"},
            {"display", "Affichage"},
            {"unit", "Unité"},
            {"temp", "Temp cible"},
            {"ambient", "Température"}
        }},
        {"en", {
            {"climate", "AC"},
            {"mode", "Mode"},
            {"fan", "Fan"},
            {"swing", "Swing"},
            {"eco", "Eco"},
            {"night", "Night"},
            {"clean", "Clean"},
            {"purifier", "Purifier"},
            {"display", "Display"},
            {"unit", "Unit"},
            {"temp", "Target Temp"},
            {"ambient", "Temperature"}
        }}
    };

static const LocaleMap LOCALIZED{
  { "fr", {
      { "mode",  { {"AUTO","Auto"},{"HEAT","Chauffage"},{"COOL","Climatisation"},
                   {"DRY","Déshumidification"},{"FAN","Ventilation uniquement"},{"OFF","Éteint"} } },
      { "fan",   { {"AUTO","Auto"},{"P20","20%"},{"P40","40%"},{"P60","60%"},
                   {"P80","80%"},{"P100","100%"},{"SILENT","Silencieux"},{"TURBO","Turbo"} } },
      { "swing", { {"STOP","Désactivé"},{"SWING","Activé"},
                   {"P1","Position 1"},{"P2","Position 2"},{"P3","Position 3"},
                   {"P4","Position 4"},{"P5","Position 5"} } }
  }},
  { "en", {
      { "mode",  { {"AUTO","Auto"},{"HEAT","Heat"},{"COOL","Cool"},
                   {"DRY","Dry"},{"FAN","Fan only"}, {"OFF","Off"} } },
      { "fan",   { {"AUTO","Auto"},{"P20","20%"},{"P40","40%"},{"P60","60%"},
                   {"P80","80%"},{"P100","100%"},{"SILENT","Silent"},{"TURBO","Turbo"} } },
      { "swing", { {"STOP","Off"},{"SWING","On"},
                   {"P1","Position 1"},{"P2","Position 2"},{"P3","Position 3"},
                   {"P4","Position 4"},{"P5","Position 5"} } }
  }}
};

//------------------------------------------------------------------
// 3. Helpers
//------------------------------------------------------------------
inline std::string join(const std::vector<std::string>& vec,
                        const std::string& sep = ",") {
  std::string out;
  for (size_t i = 0; i < vec.size(); ++i) {
    out += vec[i];
    if (i + 1 < vec.size()) out += sep;
  }
  return out;
}

// clé ➜ texte
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

// texte ➜ clé
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

// liste localisée
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

// nom d’entité -----------------------------------------------------
inline std::string get_localized_name(const std::string& lang, const std::string& key) {
  auto lang_it = LOCALIZED_NAMES.find(lang);
  if (lang_it == LOCALIZED_NAMES.end()) return key;
  const auto& map = lang_it->second;
  auto it = map.find(key);
  return (it != map.end()) ? it->second : key;
}

// options JSON prêtes à injecter ----------------------------------
inline std::string build_options_json(const std::string& lang, const std::string& cat) {
  auto opts = list_txt(lang, cat);
  return "[\"" + join(opts, R"(",")") + "\"]";
}

}  // namespace acw02_localization
