#!/bin/bash
# Place round-4 atlas + chest-sheet assets (downloaded by fetch-round4-assets.sh).
# Run from the repo root (/home/deck/CLionProjects/neoLegacy), inside distrobox.
set -e
cd "$(git rev-parse --show-toplevel)"
python3 -c "import zipfile; zipfile.ZipFile('/tmp/round4-atlas.zip').extractall('/tmp/atlas'); zipfile.ZipFile('/tmp/chest-sheets.zip').extractall('/tmp/chest')"
echo "--- atlas files ---"
ls -la /tmp/atlas/
echo "--- chest files ---"
ls -la /tmp/chest/
# Atlas -> source tree (TitleUpdate + Common)
cp /tmp/atlas/terrain.png Minecraft.Client/Common/res/TitleUpdate/res/terrain.png
cp /tmp/atlas/terrain-Common-res.png Minecraft.Client/Common/res/terrain.png
# Chest sheets -> source tree res/1_2_2/item/
mkdir -p Minecraft.Client/Common/res/1_2_2/item
cp /tmp/chest/copper_chest_large.png Minecraft.Client/Common/res/1_2_2/item/
cp /tmp/chest/copper_chest_large_exposed.png Minecraft.Client/Common/res/1_2_2/item/
cp /tmp/chest/copper_chest_large_weathered.png Minecraft.Client/Common/res/1_2_2/item/
cp /tmp/chest/copper_chest_large_oxidized.png Minecraft.Client/Common/res/1_2_2/item/
# Chest sheets -> install dir res/1_2_2/item/ (so the running game picks them up)
INSTALL="$HOME/.local/share/neoLegacy/client"
mkdir -p "$INSTALL/Common/res/1_2_2/item"
cp /tmp/chest/copper_chest_large.png "$INSTALL/Common/res/1_2_2/item/"
cp /tmp/chest/copper_chest_large_exposed.png "$INSTALL/Common/res/1_2_2/item/"
cp /tmp/chest/copper_chest_large_weathered.png "$INSTALL/Common/res/1_2_2/item/"
cp /tmp/chest/copper_chest_large_oxidized.png "$INSTALL/Common/res/1_2_2/item/"
echo "--- verify md5s (expected: TitleUpdate ec3850b5, Common 519aeffb; chest base 891bfbb8, exposed ea9a5b31, weathered e90a8d43, oxidized d0263c4c) ---"
md5sum Minecraft.Client/Common/res/TitleUpdate/res/terrain.png Minecraft.Client/Common/res/terrain.png
md5sum Minecraft.Client/Common/res/1_2_2/item/copper_chest_large*.png
md5sum "$INSTALL/Common/res/1_2_2/item/copper_chest_large*.png"
echo "--- done ---"
git status --porcelain -- Minecraft.Client/Common/res/TitleUpdate/res/terrain.png Minecraft.Client/Common/res/terrain.png Minecraft.Client/Common/res/1_2_2/item/
