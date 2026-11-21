#!/bin/bash
# Commit the verified double copper chest sheets; remove the one-time fetch script.
set -e
REPO="$(cd "$(dirname "$0")" && pwd)"
cd "$REPO"
(cd Minecraft.Client/Common/res/1_2_2/item && md5sum -c - <<'EOF'
891bfbb84f9ff3307a44a5903170fa3f  copper_chest_large.png
ea9a5b3194b2b5a43e375e9861c3f6e6  copper_chest_large_exposed.png
e90a8d43c7b55112fc5694efa1c2ef12  copper_chest_large_weathered.png
d0263c4ca209a95d72949f9060b89133  copper_chest_large_oxidized.png
EOF
)
git add Minecraft.Client/Common/res/1_2_2/item/copper_chest_large.png Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_exposed.png Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_weathered.png Minecraft.Client/Common/res/1_2_2/item/copper_chest_large_oxidized.png
git rm -q fetch-chest-sheets.sh
git status --short
git diff --cached --stat
git commit -m "Add fixed double copper chest sheets (Nov 20); remove one-time fetch script

The 4 large-chest sheets were regenerated with the corrected build_large_chest2.py (lid/back + Right-face fixes, 0 holes in used UV regions) and verified on the VM. They were Deck-side-only until now, so the Nov 21 rebuild restored the old flipped sheets; committing them makes the fix stick. The fetch script was a one-time helper (its muse.ai URL expires 2026-11-23) and is removed in the same commit."
git log --oneline -1
git status --short
