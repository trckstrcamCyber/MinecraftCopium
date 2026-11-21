#!/bin/bash
# One-time helper: place the verified corrected 26.3 atlases into the repo,
# verify their md5s in place, stage them, and remove the two one-time helper
# scripts (check-atlases.sh, fetch-and-check.sh). Run: bash place-atlases.sh
# Then commit + push from Deck git (binaries can't go via push-files).
set -e
cd "$(git rev-parse --show-toplevel)"
cp /tmp/corrected-atlases-26.3/common-corrected.png Minecraft.Client/Common/res/terrain.png
cp /tmp/corrected-atlases-26.3/common-corrected.png Minecraft.Client/Common/res/1_2_2/terrain.png
cp /tmp/corrected-atlases-26.3/titleupdate-corrected.png Minecraft.Client/Common/res/TitleUpdate/res/terrain.png
echo "dest md5s (expect 269760d9656414a4b5edcfc8a7915b3a x2, e7b319267005f90e15705c4f40218f01):"
md5sum Minecraft.Client/Common/res/terrain.png Minecraft.Client/Common/res/1_2_2/terrain.png Minecraft.Client/Common/res/TitleUpdate/res/terrain.png
git add Minecraft.Client/Common/res/terrain.png Minecraft.Client/Common/res/1_2_2/terrain.png Minecraft.Client/Common/res/TitleUpdate/res/terrain.png
git rm -q check-atlases.sh fetch-and-check.sh
git status --short
