# Current project status: 1.9.9 source candidate

Normal Windows CI now downloads the pinned codec-enabled CEF 154 archive without
runtime secrets. The stock-preview fallback is removed. See UPLOAD_AND_RELEASE_AR.md,
docs/RELEASE_1.9.9.md and docs/MEDIA_RUNTIME_SOURCES.md.

The upstream Windows archive was downloaded and SHA-256 verified in Linux.
Windows compilation and real playback acceptance remain pending; Setup is gated
on those tests. No Setup was produced or release published in this environment.
Archived patch exporters target the older 1.9.6 baseline; do not apply them to
this release. Use the full current source package or the changed-file upload ZIP.
