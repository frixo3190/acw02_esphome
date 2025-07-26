#!/bin/bash

BUILD_DIR="/tmp/esphome_build"
SRC_DIR="/src"

CMD="$1"
YAML_SRC_PATH="$2"
YAML_FILENAME=$(basename "$YAML_SRC_PATH")
YAML_PATH="$BUILD_DIR/$YAML_FILENAME"

# Crée le dossier de build s'il n'existe pas
mkdir -p "$BUILD_DIR"

# Supprime tout sauf .esphome (cache interne si présent)
find "$BUILD_DIR" -mindepth 1 -maxdepth 1 ! -name ".esphome" -exec rm -rf {} +

# Copie toutes les sources (yaml + external_component) dans le build
cp -r "$SRC_DIR"/* "$BUILD_DIR/"

# Se place dans le répertoire de build
cd "$BUILD_DIR" || exit 1

# Lance la commande esphome
esphome "$CMD" "$YAML_PATH" "${@:3}"

# Récupère dev_name dans les substitutions du fichier YAML
DEV_NAME=$(grep -A5 '^substitutions:' "$YAML_PATH" | grep 'dev_name:' | awk -F': ' '{print $2}' | tr -d '\r')

# Fallback si pas trouvé
if [ -z "$DEV_NAME" ]; then
  DEV_NAME="default"
fi

# Crée le répertoire de sortie avec dev_name
OUTPUT_DIR="/output/${YAML_FILENAME%.*}/$DEV_NAME"
mkdir -p "$OUTPUT_DIR"

# Copie tous les fichiers .bin compilés vers le bon dossier
find "$BUILD_DIR/.esphome/build" -type f -name "*.bin" -exec cp {} "$OUTPUT_DIR/" \;