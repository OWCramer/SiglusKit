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
# 0003: a click brings a hidden message window back (only keys did), so closing it with the
#       window's own button doesn't strand a touch screen.
# 0004: right-click opens the system menu after a stage object's .omv (an animated background)
#       has played; only a full-screen movie that is playing holds it off.
# 0005: the game's own Quit ends the engine on iOS (the mobile host never reported the exit).
# 0009: no font is compiled in. Upstream embeds Microsoft's MS PGothic as its fallback, which
#       may not be redistributed; Shiori links BIZ UDGothic (OFL) into each game's font/.
# 0010: no RealLive. game_launcher drops its reallive dependency (and RealLive games report
#       unsupported); Shiori's Siglus games never use it, and the crate carries translated
#       decompilations of VisualArt's DT00.DLL and PT00.DLL, which can't ship.
for patch in Scripts/siglus/patches/[0-9]*.patch; do
  git -C "$DEST" apply "$PWD/$patch"
done
# The MS PGothic file itself goes too, so nothing can build it back in (0009).
rm -f "$DEST/crates/siglus_scene_vm/assets/fonts/default.ttf"
# The reallive crate goes too (0010), dt00/pt00 and all, so nothing can link it back in.
rm -rf "$DEST/crates/reallive"

echo "$VERSION" > "$STAMP"
echo "siglus_rs ready in $DEST"
