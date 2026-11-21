#!/bin/bash
# One-time: md5 every terrain.png in the repo tree + install dir, so the
# lantern/rod atlas question is settled by hashes, not by eye.
# Run on the Deck with: bash check-atlases.sh
set -u
REPO="$(cd "$(dirname "$0")" && pwd)"
INSTALL="$HOME/.local/share/neoLegacy/client"
echo "=== repo atlases (under Minecraft.Client/Common/res) ==="
find "$REPO/Minecraft.Client/Common/res" -name 'terrain.png' -exec md5sum {} +
echo "=== install atlases (under ~/.local/share/neoLegacy/client) ==="
find "$INSTALL" -name 'terrain.png' -exec md5sum {} +
echo "=== expected lantern-fixed values ==="
echo "Common      d3788097643338b22eda5b8c0244e7e5"
echo "TitleUpdate 445c385650a47f94358886e10f2ba246"
echo "=== old vanilla (pre-fix) Common atlas was 07be970a... (1_2_2 copy) ==="
echo "=== done ==="
