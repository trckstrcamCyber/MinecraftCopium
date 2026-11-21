#!/bin/bash
# One-time helper: check which minecraft-lce-client exes exist (stale-exe test A)
# and fetch + verify the corrected 26.3 atlases zip. Run: bash fetch-and-check.sh
echo "=== exe copies (mtime + md5) ==="
find /home/deck -name "minecraft-lce-client" -not -path "*/proc/*" 2>/dev/null | while read -r f; do ls -l --time-style=full-iso "$f"; md5sum "$f"; done
echo "=== build dirs ==="
ls -d /home/deck/CLionProjects/neoLegacy/build/* 2>/dev/null || echo "(no build/ entries)"
echo "=== fetch corrected atlases ==="
cd /tmp || exit 1
curl -L -o corrected-atlases-26.3.zip "https://muse.ai/files/1468297609701611/1166956166021718/j1ps475g0s9th089puzrvef1/corrected-atlases-26.3.zip"
echo "zip md5 (expect 43df1edccaf618a2e6e405d41611d6e7):"
md5sum corrected-atlases-26.3.zip
rm -rf corrected-atlases-26.3
if command -v unzip >/dev/null 2>&1; then unzip -o corrected-atlases-26.3.zip -d corrected-atlases-26.3; else python3 -m zipfile -e corrected-atlases-26.3.zip corrected-atlases-26.3; fi
echo "atlas md5s (expect common 269760d9656414a4b5edcfc8a7915b3a, titleupdate e7b319267005f90e15705c4f40218f01):"
md5sum corrected-atlases-26.3/*.png
ls -l corrected-atlases-26.3/
