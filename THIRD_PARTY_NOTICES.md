# Third-party notices

MaenBrowser is built with the Chromium Embedded Framework (CEF) and Chromium open-source components. Chromium's media stack includes third-party open-source components such as FFmpeg and libvpx. Those components remain subject to their respective licenses and notices distributed with the corresponding source/binary distribution.

MaenBrowser does not claim authorship of Chromium, CEF, FFmpeg, libvpx, or other third-party components. Product names and trademarks belong to their respective owners.

For custom media-enabled builds, MaenBrowser uses the same CEF/Chromium source line as the application and enables the build configuration recorded in `docs/CUSTOM_CEF_MEDIA_BUILD.md`. Patent or other redistribution rights for particular codecs are separate from open-source copyright licenses and must be handled by the distributor.


## Codec-enabled CEF runtime (1.9.9)

Normal builds use the pinned psmux/godwinmix-cef CEF 154.0.34 Windows x64 minimal
distribution recorded in config/media-runtime.lock.json. CEF/Chromium and their
third-party components retain their own licenses. The archive's LICENSE.txt and
CREDITS.html (when present), together with the download provenance receipt, are
copied to the installer payload notices directory. No upstream build scripts are
incorporated or executed by the downloader. Source/release links and patent-license
limits are documented in docs/MEDIA_RUNTIME_SOURCES.md.

## 1.10.0 separate filter datasets and compatibility scripts

HaGeZi Light data is pinned to commit eb7db8ed27237af2f3c0cb79c9c735e3e076dbad
of https://github.com/hagezi/dns-blocklists, under GPL-3.0; its license is bundled
in filters/sources/HaGeZi-LICENSE.txt (and copied to installer notices). Data is
normalized to sorted domain lines. Exact source/output hashes are in the filter
lock and manifest. The StevenBlack adult dataset was refreshed at pinned commit
6d56ab44f2b3d27de062b5194eb51bcd8115a773, retaining existing MIT source notices.

The two local JavaScript compatibility resources come from Raymond Hill's uBlock
Origin, https://github.com/gorhill/uBlock, commit
cc0da2303b66154acf293c807ba0c5026e4834ba, GPL-3.0-or-later. Their original copyright
headers, corresponding editable sources and full license are distributed in the
shims directory. config/ad-shims.lock.json records source links and packaged file
hashes. They are separate scripts, not the whole uBlock extension; no endorsement
or universal anti-adblock capability is claimed.

## Optional start-page weather (1.12.0)

Forecast data: MET Norway, https://www.met.no/ and https://api.met.no/ .
Data licensed under Creative Commons Attribution 4.0:
https://creativecommons.org/licenses/by/4.0/ . Values are rounded/reformatted
for display; no endorsement is implied. Service terms:
https://api.met.no/doc/TermsOfService . Desktop weather starts automatically in1.12.1 unless disabled. Browser location
permission is respected; a saved or timezone city can be used as a fallback.
Approximate coordinates and the public IP reach the provider; a15–60minute
local cache and visible-page-only refresh reduce traffic. No API key is
bundled. The service is not guaranteed to be continuously available.
