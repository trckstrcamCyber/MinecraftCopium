#!/bin/bash
# Fetch + place Illager batch 5 binaries (Deck side).
# Run from the repo root inside distrobox: bash fetch-illager-binaries.sh
# Downloads illager-binaries.zip (mob PNGs, sounds, items.png splice source),
# verifies its md5, unzips, and runs place-illager-binaries.sh (which places
# the files and verifies them; it does NOT commit).
set -e
URL="https://muse.ai/files/1468297609701611/1855361792236103/sa4tciiimtfgnwj4qm5ahwfs/illager-binaries.zip"
MD5="d67d083ee84d1b2fdda04e96bf7f01ca"
cd "$(git rev-parse --show-toplevel)"
curl -L -o /tmp/illager-binaries.zip "$URL"
echo "$MD5  /tmp/illager-binaries.zip" | md5sum -c -
rm -rf /tmp/illager-b5-bin
mkdir -p /tmp/illager-b5-bin
unzip -o -q /tmp/illager-binaries.zip -d /tmp/illager-b5-bin
bash /tmp/illager-b5-bin/place-illager-binaries.sh
