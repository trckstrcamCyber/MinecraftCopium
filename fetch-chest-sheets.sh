#!/bin/bash
# One-time fetch of the verified double copper chest sheets (Nov 20).
# The muse.ai URL expires 2026-11-23; delete this script after use.
set -e
REPO="$(cd "$(dirname "$0")" && pwd)"
curl -L -o /tmp/chest-sheets-nov20.zip "https://muse.ai/files/1468297609701611/1476432444438513/4o9vfwhd8op3a78e0btrvenw/chest-sheets-nov20.zip"
echo "bd4229add94877cce30caf5ac7a0e25e  /tmp/chest-sheets-nov20.zip" | md5sum -c -
rm -rf /tmp/chest-sheets
python3 -m zipfile -e /tmp/chest-sheets-nov20.zip /tmp/chest-sheets
(cd /tmp/chest-sheets && md5sum -c - <<'EOF'
891bfbb84f9ff3307a44a5903170fa3f  copper_chest_large.png
ea9a5b3194b2b5a43e375e9861c3f6e6  copper_chest_large_exposed.png
e90a8d43c7b55112fc5694efa1c2ef12  copper_chest_large_weathered.png
d0263c4ca209a95d72949f9060b89133  copper_chest_large_oxidized.png
EOF
)
cp /tmp/chest-sheets/*.png "$REPO/Minecraft.Client/Common/res/1_2_2/item/"
mkdir -p "$HOME/.local/share/neoLegacy/client/Common/res/1_2_2/item/"
cp /tmp/chest-sheets/*.png "$HOME/.local/share/neoLegacy/client/Common/res/1_2_2/item/"
echo "Placed 4 chest sheets in repo + install dir."
