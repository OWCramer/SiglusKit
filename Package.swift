// swift-tools-version: 6.0
import PackageDescription

// Siglus games run on siglus_rs (MPL-2.0), fetched and patched by Scripts/siglus/fetch-siglus.sh
// and built as Vendor/ShioriSiglus.xcframework. This package holds its C interface.
let package = Package(
    name: "SiglusKit",
    platforms: [.iOS(.v18), .macOS(.v15)],
    products: [
        .library(name: "CSiglus", targets: ["CSiglus"]),
    ],
    targets: [
        .target(name: "CSiglus"),
    ]
)
