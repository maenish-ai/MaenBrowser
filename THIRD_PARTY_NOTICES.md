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
