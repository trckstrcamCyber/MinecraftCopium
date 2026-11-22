#!/bin/bash
# Fetch + place Illager batch 6 binaries (Deck side).
# The zip holds the 47 binary files at repo-mirroring paths (5 new mob
# PNGs, 40 new oggs, 2 pre-spliced items.png) plus md5s.txt, and the
# placement script at the zip root. Placement needs no Python, just
# cp + md5. Run from the repo root inside distrobox:
# bash fetch-illager-binaries-b6.sh
set -e
URL="https://muse.ai/files/1468297609701611/930381396826637/bo0h1n11q2x5tvin1yck16ri/illager-binaries-b6.zip"
MD5="0005003391b5859e6fd62b45bc10f479"
cd "$(git rev-parse --show-toplevel)"
curl -L -o /tmp/illager-binaries-b6.zip "$URL"
echo "$MD5  /tmp/illager-binaries-b6.zip" | md5sum -c -
rm -rf /tmp/illager-b6-bin
mkdir -p /tmp/illager-b6-bin
unzip -o -q /tmp/illager-binaries-b6.zip -d /tmp/illager-b6-bin
bash /tmp/illager-b6-bin/place-illager-binaries-b6.sh
