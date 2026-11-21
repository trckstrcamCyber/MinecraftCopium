#!/bin/bash
# Fetch + place the copper horse armor texture (round-5 horse armor fix, 5fcea6f3).
# URL is exact — do NOT retype it; this script is the source of truth.
set -e
URL="https://muse.ai/files/1468297609701611/2920821964948521/wbp3y9j3w8o2lrq4qdc9xurx/horsearmor-texture.zip"
DEST="$HOME/CLionProjects/neoLegacy/Minecraft.Client/Common/res/TitleUpdate/res/mob/horse/armor/horse_armor_copper.png"
mkdir -p "$(dirname "$DEST")"
curl -fsSL -o /tmp/horsearmor-texture.zip "$URL"
unzip -o /tmp/horsearmor-texture.zip -d /tmp/horsearmor-texture
cp /tmp/horsearmor-texture/horse_armor_copper.png "$DEST"
md5sum "$DEST"
echo "placed: $DEST (expect md5 70d12d48019d15b63c0550e93ae6b3ee)"
