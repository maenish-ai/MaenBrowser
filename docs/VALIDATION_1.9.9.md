# 1.9.9 validation record

Source base: maenish-ai/MaenBrowser commit
24a5c31f3953f9d13997b5044dd21bfaf734753c (the user's uploaded 1.9.8).

Completed in the Linux preparation environment:

- Downloaded the Windows x64 CEF 154.0.34 upstream archive (173,759,232 bytes).
  SHA-256 matched the committed lock and GitHub release asset digest.
- Extracted 1,001 archive members; approximately 423 MiB unpacked.
  Verified exact version header, x64 PE architecture, sandbox bootstrap, DLLs,
  import library, resources, Arabic/English locales and notices. Critical file
  hashes are recorded in CEF154_ARCHIVE_PROVENANCE.json and verified by the tool.
- All portable JavaScript/Python/native C++ checks passed; see PORTABLE_1.9.9_LOG.txt.
  Synthetic runtime tests cover default selection, partial overrides, invalid
  URLs/digests, tampering, incompatible version, stock-cache replacement and
  archive traversal. These synthetic files do not establish real codec decoding.
- Official PowerShell 7.6.9 was downloaded and SHA-256 checked for validation.
  All PowerShell scripts parsed successfully; tools/preflight.ps1 passed, and the
  PowerShell runtime verifier passed against the actual extracted Windows archive.
- Both workflow YAML files parsed. Executing the actual configuration shell step
  with both secrets absent selected the locked codec-enabled archive and emitted
  media_ready=true. Browser integration is now unconditional in Setup builds.

Not completed here:

- Windows application compilation/linking and sandboxed launch with the new CEF.
  The Windows distribution cannot be compiled as a Linux CEF application; a
  Linux header syntax attempt lacked platform-specific Linux headers, as expected.
- Real playback (including the added local-file MP4 check), audible output,
  H.264 hardware decode, WhatsApp login/calls or Adobe application acceptance.
- Performance measurements on the user's hardware, public release or remote push.

GitHub Actions must perform the Windows build and real codec playback before
creating Setup. The source package and archive digest alone are not a claim that
those pending checks passed. This change removes the known stock-runtime selection
cause; damaged files, expired media URLs, DRM and unrelated site failures still
require separate diagnosis.
