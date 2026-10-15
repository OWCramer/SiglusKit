#!/bin/sh
# Fetches siglus_rs, the SiglusEngine reimplementation Shiori embeds (MPL-2.0), into
# Vendor/siglus_rs. See spec.md, "Siglus engine". Vendor/ is gitignored. Safe to re-run: an
# existing checkout at the pinned commit is left alone.
set -eu
cd "$(dirname "$0")/../.."

SIGLUS_COMMIT=5f358ef1360b0439e92aed2b749e8f25d8721954
DEST=Vendor/siglus_rs
STAMP="$DEST/.shiori-version"
VERSION="$SIGLUS_COMMIT $(cat Scripts/siglus/patches/*.patch Scripts/siglus/kira-*.patch | shasum -a 256 | cut -c1-16)"
# kira, the audio library, from crates.io (the version and checksum siglus_rs's Cargo.lock
# pins), patched for 0012.
KIRA_VERSION=0.9.6
KIRA_SHA256=fb5a9f9dff5e262540b628b00d5e1a772270a962d6869f8695bb24832ff3b394

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
# 0011: siglus_ios_save_place: the host saves the game's place (the save point the game's own
#       SAVE writes) into a normal slot as the app leaves the screen or the player leaves;
#       save files are written beside the slot and renamed over it.
# 0012: siglus_ios_restart_audio: rebuilds the audio output stream, keeping what plays. iOS
#       stops the output unit on an interruption or a media services reset and never starts
#       it again; the host calls this when an interruption ends, the route changes or media
#       services come back. Builds kira from vendor/kira-<version> ([patch.crates-io]), with
#       kira-<version>-restart.patch: a restart request the stream manager's thread acts on,
#       and a failed (re)start keeps the renderer to try again.
for patch in Scripts/siglus/patches/[0-9]*.patch; do
  git -C "$DEST" apply "$PWD/$patch"
done
KIRA_CRATE=$(mktemp -t kira).crate
curl -fsSL "https://static.crates.io/crates/kira/kira-$KIRA_VERSION.crate" -o "$KIRA_CRATE"
echo "$KIRA_SHA256  $KIRA_CRATE" | shasum -a 256 -c -
mkdir -p "$DEST/vendor"
tar -xzf "$KIRA_CRATE" -C "$DEST/vendor"
rm -f "$KIRA_CRATE"
patch -s -p1 -d "$DEST/vendor/kira-$KIRA_VERSION" < "Scripts/siglus/kira-$KIRA_VERSION-restart.patch"
# The MS PGothic file itself goes too, so nothing can build it back in (0009).
rm -f "$DEST/crates/siglus_scene_vm/assets/fonts/default.ttf"
# The reallive crate goes too (0010), dt00/pt00 and all, so nothing can link it back in.
rm -rf "$DEST/crates/reallive"

echo "$VERSION" > "$STAMP"
echo "siglus_rs ready in $DEST"
