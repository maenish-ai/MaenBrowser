# MaenBrowser media runtime: ready-built default and optional custom build

As of 1.9.9, normal Windows builds use the ready-built codec-enabled CEF 154
archive pinned in config/media-runtime.lock.json. It is fetched and verified by
tools/media-runtime.py, called by fetch-cef.ps1 and verify-media-runtime.ps1.
No secrets or full Chromium build are needed for the default path.

The application is rebuilt against the headers/import library from the same
archive as libcef.dll, bootstrap.exe and resources. Never drop an unrelated
FFmpeg/CEF DLL into a previous installation. CMake verifies download provenance
and extracted critical-file hashes. Windows browser tests establish actual decode.

## Optional build from source

Use a suitably provisioned Windows x64 builder and tools/build-cef-media.ps1,
or the separate Build CEF 154 Maen Media Runtime workflow. This is a full Chromium
build and may require hundreds of GB of disk and hours. It is not needed for
normal application CI using the prebuilt archive.

The pinned source line is CEF 154.0.34+g14c5a08+chromium-154.0.8037.98, branch 8037,
checkout 14c5a08. Required GN arguments include:

```
is_official_build=true proprietary_codecs=true ffmpeg_branding=Chrome
```

The script emits a ZIP with MAEN_MEDIA_RUNTIME.txt, SHA-256 and build manifest.
Host it on trusted HTTPS storage, then set BOTH MAEN_CEF_ARCHIVE_URL and
MAEN_CEF_ARCHIVE_SHA256 for an explicit custom override. Its header version and
media marker must match. An old CEF 152 override will fail deliberately.

The upstream prebuilt archive has upstream provenance, not a Maen custom-build
marker. The downloader accepts it only through its specific committed SHA-256
lock. It does not invent a custom-build marker or treat a marker as playback proof.
See docs/MEDIA_RUNTIME_SOURCES.md for dependency research and licensing boundaries.
