# MaenBrowser adaptive performance policy

MaenBrowser 1.5.12 keeps Chromium/CEF GPU and media acceleration available and applies conservative resource policy around it.

## Low-memory systems

- <=4 GiB RAM: Lite mode, Prerender2 and BFCache disabled, 64 MiB disk cache, 32 MiB media cache.
- >4 to 6 GiB RAM: Lite mode, 96 MiB disk cache, 48 MiB media cache.
- Balanced: Prerender2 disabled, 256 MiB disk cache, 128 MiB media cache.
- >=16 GiB and >=8 logical processors: Performance mode stays close to Chromium defaults.

The browser does **not** disable GPU acceleration, WebGL, accelerated video decode, WebRTC, MediaSource, sandboxing, site isolation, or TLS validation to save RAM. Chromium's GPU blocklist and safe fallback behavior remain authoritative.

## Media policy

The browser does not ship random codec packs or download executable codecs. It exposes local capability checks for H.264, HEVC/H.265, VP9, AV1, AAC, Opus, FLAC, MP3 and WebGL2. A positive `canPlayType` result is a capability claim from the current engine, not a performance guarantee. Proprietary codec availability depends on the CEF build, OS facilities, licensing, and hardware.

## 3D/games

WebGL/WebGL2 remain enabled. MaenBrowser does not force GPU features past Chromium's compatibility blocklist because doing so can make old GPUs unstable. The 1.5.12 policy reduces browser-side speculative/cache overhead on 4–6 GiB systems so more resources remain available to the active workload. A future tab-aware Game Boost must be benchmarked before release and must not weaken process isolation.

## Acceptance target

Performance claims require measurements on at least 4 GiB low-end, 8 GiB mainstream, and modern >=16 GiB systems, including idle browsing, 1080p video, 4K where hardware supports it, WebGL2, multi-tab memory pressure, and recovery after closing heavy tabs.
