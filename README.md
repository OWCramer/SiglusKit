# SiglusKit

SiglusEngine games in [Shiori](https://github.com/OWCramer/shiori), on [siglus_rs](https://github.com/xmoezzz/siglus_rs) with Shiori's patches: input, the cursor, menus, the backlog, and movies handed to Shiori's FFmpeg instead of siglus_rs's FFmpeg-derived decoders.

It plays games you own; no game content is included.

- `Sources/CSiglus`: the engine's C interface for Swift.
- `Scripts/siglus`: fetches siglus_rs at a pinned commit, applies `patches/`, and builds `Vendor/ShioriSiglus.xcframework` (one dynamic framework per slice), which the app embeds.

Needs Rust with the iOS targets (`rustup target add aarch64-apple-ios aarch64-apple-ios-sim`).

```sh
Scripts/siglus/fetch-siglus.sh
Scripts/siglus/build-ios-framework.sh
```

## Licence

Under the [Mozilla Public License 2.0](LICENSE), as siglus_rs is; the patches in `Scripts/siglus/patches` are Shiori's changes to it. The Rust crates it builds keep their licences (MIT, Apache-2.0, MPL-2.0, BSD-3 and others; Shiori's Acknowledgements list each).
