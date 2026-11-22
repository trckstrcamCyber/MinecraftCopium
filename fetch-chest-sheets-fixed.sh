#!/bin/bash
# Fetch + place the corrected copper chest sheets (inside/top swap).
# The game reads singles from TitleUpdate/res/item/ and doubles from
# 1_2_2/item/ (Textures::readImage: singles are TU images, larges are not).
# Singles are NOT in the repo: placed as working-tree files only.
# Larges ARE tracked: commit + push them separately after this runs.
# Zip: chest-sheets-fixed.zip, md5 15d8311c0bedf2af738f9735e4f1e113
set -e
cd "$(git rev-parse --show-toplevel)"
curl -L -o /tmp/chest-sheets-fixed.zip "https://muse.ai/files/1468297609701611/1078684711899527/nwzgc2rukfuixmmez5781l6s/chest-sheets-fixed.zip"
echo "15d8311c0bedf2af738f9735e4f1e113  /tmp/chest-sheets-fixed.zip" | md5sum -c -
rm -rf /tmp/chest-sheets
mkdir -p /tmp/chest-sheets
unzip -o -q /tmp/chest-sheets-fixed.zip -d /tmp/chest-sheets
# Doubles -> tracked 1_2_2 paths (these are the ones the game reads)
cp /tmp/chest-sheets/Minecraft.Client/Common/res/1_2_2/item/copper_chest_large.png Minecraft.Client/Common/res/1_2_2/item/copper_chest_large.png
cp /tmp/chest-sheets/Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_exposed.png Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_exposed.png
cp /tmp/chest-sheets/Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_weathered.png Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_weathered.png
cp /tmp/chest-sheets/Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_oxidized.png Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_oxidized.png
# Singles -> TitleUpdate paths (these are the ones the game reads)
cp /tmp/chest-sheets/Minecraft.Client/Common/res/1_2_2/item/copper_chest.png Minecraft.Client/Common/res/TitleUpdate/res/item/copper_chest.png
cp /tmp/chest-sheets/Minecraft.Client/Common/res/1_2_2/item/copper_chest_exposed.png Minecraft.Client/Common/res/TitleUpdate/res/item/copper_chest_exposed.png
cp /tmp/chest-sheets/Minecraft.Client/Common/res/1_2_2/item/copper_chest_weathered.png Minecraft.Client/Common/res/TitleUpdate/res/item/copper_chest_weathered.png
cp /tmp/chest-sheets/Minecraft.Client/Common/res/1_2_2/item/copper_chest_oxidized.png Minecraft.Client/Common/res/TitleUpdate/res/item/copper_chest_oxidized.png
# Verify every placed file against the staged md5s (fails loudly on mismatch)
cat > /tmp/chest-md5.txt <<'EOF'
77ff1a292a4756938f9b9a24034a4a0c  Minecraft.Client/Common/res/1_2_2/item/copper_chest_large.png
f2faf3eb578f150dc2c93fb98972f2f3  Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_exposed.png
f7a1c8f49c969215c5f430bec2864aa0  Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_weathered.png
659c5049f7a9feff8b058c32e63a41ed  Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_oxidized.png
a96905753a6997f49fe8b2a17c12e69a  Minecraft.Client/Common/res/TitleUpdate/res/item/copper_chest.png
1fe96c590d32e014aff4a6990e7dede1  Minecraft.Client/Common/res/TitleUpdate/res/item/copper_chest_exposed.png
ed255aa3d44a7c3170c58fd4f9d6eb3d  Minecraft.Client/Common/res/TitleUpdate/res/item/copper_chest_weathered.png
73c191f52db73ba01295053e2eec5f0f  Minecraft.Client/Common/res/TitleUpdate/res/item/copper_chest_oxidized.png
EOF
md5sum -c /tmp/chest-md5.txt
echo "CHEST SHEETS PLACED + VERIFIED (8/8). Next: git add + commit + push the 4 large sheets."
