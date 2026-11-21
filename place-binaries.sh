#!/bin/bash
# One-time helper: fetch the v3 chest sheets + model atlases, place them,
# verify md5s, commit each set via Deck git, push. Delete after use.
set -e
cd /home/deck/CLionProjects/neoLegacy
git pull github "$(git branch --show-current)"
curl -sL -o /tmp/chest-sheets-v3.zip 'https://muse.ai/files/1468297609701611/1428736219046960/vvxljmq9lxg2aupl0b717mqm/chest-sheets-v3.zip'
curl -sL -o /tmp/model-atlases.zip 'https://muse.ai/files/1468297609701611/1119224937659273/oqdjoceyrmmp3lxd52uscse8/model-atlases.zip'
echo '4789f015aa1511a79441e0d365f070f3  /tmp/chest-sheets-v3.zip' | md5sum -c -
echo '625223bce4688e445520346390beaceb  /tmp/model-atlases.zip' | md5sum -c -
mkdir -p /tmp/chest-v3 /tmp/atlas-models
unzip -o -q /tmp/chest-sheets-v3.zip -d /tmp/chest-v3
unzip -o -q /tmp/model-atlases.zip -d /tmp/atlas-models
cp /tmp/chest-v3/copper_chest_large*.png Minecraft.Client/Common/res/1_2_2/item/
cp /tmp/chest-v3/copper_chest_large*.png ~/.local/share/neoLegacy/client/Common/res/1_2_2/item/
cp /tmp/atlas-models/common-corrected.png Minecraft.Client/Common/res/terrain.png
cp /tmp/atlas-models/titleupdate-corrected.png Minecraft.Client/Common/res/TitleUpdate/res/terrain.png
cp /tmp/atlas-models/common-corrected.png Minecraft.Client/Common/res/1_2_2/terrain.png
echo '680b4c9d6c3e7e109342e007c80ae3cd  Minecraft.Client/Common/res/1_2_2/item/copper_chest_large.png' | md5sum -c -
echo '5b870f3a1d4078ed4317ec153d3a0b31  Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_exposed.png' | md5sum -c -
echo '6eefb1baea20255145a00f762091810d  Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_weathered.png' | md5sum -c -
echo '4af522628b96595ba46972b85619dd9a  Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_oxidized.png' | md5sum -c -
echo '3a17f8181cdd5554d2a15945d5964c32  Minecraft.Client/Common/res/terrain.png' | md5sum -c -
echo '9fa4e1763389883b4c5c003c53de3c7a  Minecraft.Client/Common/res/TitleUpdate/res/terrain.png' | md5sum -c -
echo '3a17f8181cdd5554d2a15945d5964c32  Minecraft.Client/Common/res/1_2_2/terrain.png' | md5sum -c -
git add Minecraft.Client/Common/res/1_2_2/item/copper_chest_large.png Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_exposed.png Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_weathered.png Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_oxidized.png
git commit -m "Add swapped v3 double copper chest sheets (fix top seam)"
git add Minecraft.Client/Common/res/terrain.png Minecraft.Client/Common/res/TitleUpdate/res/terrain.png Minecraft.Client/Common/res/1_2_2/terrain.png
git commit -m "copper models: verbatim lantern/rod cells + shelf back cells for multi-cuboid models"
git push github "$(git branch --show-current)"
echo "PLACE-BINARIES DONE"
