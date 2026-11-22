#!/bin/bash
# Fetch + place Illager batch 5 binaries v2 (Deck side).
# v2: the items.png files in the zip are pre-spliced (row 17 platinum icons
# only); the placement script needs no Python, just cp + md5.
# Run from the repo root inside distrobox: bash fetch-illager-binaries-v2.sh
set -e
URL="https://muse.ai/files/1468297609701611/979309868340104/wp9xj98dp8z9m8wc3g3bhi8a/illager-binaries-v2.zip"
MD5="faa0b7ee4119f5870b5d854672a60a93"
cd "$(git rev-parse --show-toplevel)"
curl -L -o /tmp/illager-binaries-v2.zip "$URL"
echo "$MD5  /tmp/illager-binaries-v2.zip" | md5sum -c -
rm -rf /tmp/illager-b5-bin-v2
mkdir -p /tmp/illager-b5-bin-v2
unzip -o -q /tmp/illager-binaries-v2.zip -d /tmp/illager-b5-bin-v2
bash /tmp/illager-b5-bin-v2/place-illager-binaries-v2.sh
