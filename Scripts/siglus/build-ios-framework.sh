#!/bin/sh
# Builds siglus_rs's iOS library (the game_launcher crate, C interface siglus.h) as
# Vendor/ShioriSiglus.xcframework: one dynamic framework per slice (device and simulator), so
# the Rust runtime and its dependencies stay inside their own image, apart from Ren'Py's and
# KiriKiri's. Run Scripts/siglus/fetch-siglus.sh first.
#
#     Scripts/siglus/build-ios-framework.sh [siglus_rs checkout]
set -eu
cd "$(dirname "$0")/../.."
ROOT=$PWD
SRC=${1:-$ROOT/Vendor/siglus_rs}
OUT=$ROOT/Vendor/ShioriSiglus.xcframework
WORK=$ROOT/build/siglus-framework
NAME=ShioriSiglus
export IPHONEOS_DEPLOYMENT_TARGET=18.0

rm -rf "$WORK" "$OUT"
mkdir -p "$WORK"
args=""
for target in aarch64-apple-ios aarch64-apple-ios-sim; do
  echo "==> cargo build ($target)"
  (cd "$SRC" && cargo build --release -p game_launcher --target "$target")
  case $target in
    *-sim) platform=iPhoneSimulator ;;
    *) platform=iPhoneOS ;;
  esac
  framework=$WORK/$target/$NAME.framework
  mkdir -p "$framework"
  cp "$SRC/target/$target/release/libgame_launcher.dylib" "$framework/$NAME"
  install_name_tool -id "@rpath/$NAME.framework/$NAME" "$framework/$NAME"
  cat > "$framework/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleExecutable</key><string>$NAME</string>
  <key>CFBundleIdentifier</key><string>com.owencramer.vnhub.siglus</string>
  <key>CFBundleName</key><string>$NAME</string>
  <key>CFBundlePackageType</key><string>FMWK</string>
  <key>CFBundleShortVersionString</key><string>1.0</string>
  <key>CFBundleVersion</key><string>1</string>
  <key>CFBundleSupportedPlatforms</key><array><string>$platform</string></array>
  <key>MinimumOSVersion</key><string>$IPHONEOS_DEPLOYMENT_TARGET</string>
</dict>
</plist>
PLIST
  args="$args -framework $framework"
done

# The C interface, for Swift (Packages/SiglusKit).
cp "$SRC/platform/ios/SiglusLauncher/Headers/siglus.h" "$ROOT/Packages/SiglusKit/Sources/CSiglus/include/siglus.h"

echo "==> $OUT"
# shellcheck disable=SC2086
xcodebuild -create-xcframework $args -output "$OUT"
echo "==> Vendor/$NAME.xcframework ready"
