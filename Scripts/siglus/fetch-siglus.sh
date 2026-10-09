#!/bin/sh
# Fetches siglus_rs, the SiglusEngine reimplementation Shiori embeds (MPL-2.0), into
# Vendor/siglus_rs. See spec.md, "Siglus engine". Vendor/ is gitignored. Safe to re-run: an
# existing checkout at the pinned commit is left alone.
set -eu
cd "$(dirname "$0")/../.."

SIGLUS_COMMIT=5f358ef1360b0439e92aed2b749e8f25d8721954
DEST=Vendor/siglus_rs
STAMP="$DEST/.shiori-version"
VERSION="$SIGLUS_COMMIT $(cat Scripts/siglus/patches/*.patch | shasum -a 256 | cut -c1-16)"

if [ -f "$STAMP" ] && [ "$(cat "$STAMP")" = "$VERSION" ]; then
  echo "siglus_rs already in $DEST"
  exit 0
fi

rm -rf "$DEST"
git clone https://github.com/xmoezzz/siglus_rs.git "$DEST"
git -C "$DEST" checkout -q "$SIGLUS_COMMIT"

# 0001: iOS host exports for a mouse button (a two-finger tap is a right-click), the wheel
#       and keys, beside siglus_ios_touch's left button.
# 0002: the iOS host keeps the game's mouse-cursor sprite hidden (there's no pointer to show).
for patch in Scripts/siglus/patches/[0-9]*.patch; do
  git -C "$DEST" apply "$PWD/$patch"
done

echo "$VERSION" > "$STAMP"
echo "siglus_rs ready in $DEST"
