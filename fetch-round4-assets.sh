#!/bin/bash
# Fetch round-4 atlas + chest-sheet zips from Muse file storage.
# URLs are exact (copied from the upload tool, not retyped).
set -e
curl -L -o /tmp/round4-atlas.zip "https://muse.ai/files/1468297609701611/3024230444596776/45i9uq908ibmuix4s0tn5y6a/round4-thin-tile-atlas.zip"
curl -L -o /tmp/chest-sheets.zip "https://muse.ai/files/1468297609701611/1410747011048864/bw2gqwe75m5um80nzi3y4c73/chest-sheets-nov20.zip"
ls -la /tmp/round4-atlas.zip /tmp/chest-sheets.zip
echo "expected: round4-atlas.zip 377867 bytes; chest-sheets.zip 13791 bytes"
