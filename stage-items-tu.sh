#!/bin/bash
# Stage the Deck's TitleUpdate items.png onto a scratch branch so it can be
# fetched for the illager platinum splice. The Deck file is 71,192 B with
# Deck-only icons (head's is 64,605 B) - it must NOT be overwritten by the
# head-based spliced copy. Restores the worktree file afterwards.
set -e
cd "$(git rev-parse --show-toplevel)"
git checkout -b tmp-items-tu
git add Minecraft.Client/Common/res/TitleUpdate/res/items.png
git commit -m "Deck TitleUpdate items.png (Deck-only icons, for illager splice)"
git push -u github tmp-items-tu
git checkout port/copper-rosegold-nov19
git checkout tmp-items-tu -- Minecraft.Client/Common/res/TitleUpdate/res/items.png
git rev-parse --abbrev-ref HEAD
git status --porcelain -- Minecraft.Client/Common/res/TitleUpdate/res/items.png
echo "[ok] staged on tmp-items-tu, worktree restored"
