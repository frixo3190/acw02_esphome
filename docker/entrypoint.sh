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

# Export les bin si compilation (ou run) réussie
OUTPUT_DIR="/output/${YAML_FILENAME%.*}"
mkdir -p "$OUTPUT_DIR"
find "$BUILD_DIR/.esphome/build" -type f -name "*.bin" -exec cp {} "$OUTPUT_DIR/" \;