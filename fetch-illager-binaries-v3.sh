#!/bin/bash
# Fetch + place Illager batch 5 binaries v3 (Deck side).
# v3: the TitleUpdate items.png is spliced into the DECK's 71,192-byte file
# (copper icons in rows 18-20 preserved); the gui copy is spliced from head.
# Placement needs no Python, just cp + md5.
# Run from the repo root inside distrobox: bash fetch-illager-binaries-v3.sh
set -e
URL="https://muse.ai/files/1468297609701611/1453642313610476/81a7wrqq88xmeu2ltf5zsft8/illager-binaries-v3.zip"
MD5="69db62c69ede93301b271f168133af3d"
cd "$(git rev-parse --show-toplevel)"
curl -L -o /tmp/illager-binaries-v3.zip "$URL"
echo "$MD5  /tmp/illager-binaries-v3.zip" | md5sum -c -
rm -rf /tmp/illager-b5-bin-v3
mkdir -p /tmp/illager-b5-bin-v3
unzip -o -q /tmp/illager-binaries-v3.zip -d /tmp/illager-b5-bin-v3
bash /tmp/illager-b5-bin-v3/place-illager-binaries-v3.sh
