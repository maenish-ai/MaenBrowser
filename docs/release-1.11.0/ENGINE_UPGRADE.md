# Windows engine upgrade, not yet built here

The default working codec runtime remains154. The separate pinned upstream
candidate is CEF156.0.15 / Chromium156.0.8078.80, official commit
cf671c072925b27823a61181152f4f288e8e4306. Automation source is SHA256-pinned in
`config/cef-upgrade-candidate.json`. This improves the available build path;
it is not proof of a completed engine upgrade or latest Chrome fixes.

Use the **Build pinned newer CEF codec candidate** workflow on a dedicated
Windows x64 VS2022 Chromium builder (300GB+ free SSD, recommended32–64GB RAM).
The browser does not need that memory; compiling Chromium does. This workspace
has under8GB disk, so this core build cannot run here. The original154 builder
continues unchanged unless `-ProfilePath` is explicitly selected.

The workflow builds official/non-component/proprietary_codecs/Chrome FFmpeg
CEF and creates a version-checked archive, hash and build manifest. It neither
changes the default lock nor publishes an untested installer. After a successful
build, the maintainer must host the archive over HTTPS, update the runtime lock
with its new source/version/branch/checkout/hash/size, update any CEF API changes,
then run the Windows sandbox/native/live playback/image/shim tests. Verify codec
playback with the actual new binary, not just its build marker. Rerun
`python tools/audit-engine.py --output engine-audit.json --require-current` before
claiming current-stable Chromium security parity; CEF156 can still trail Chrome.
Never replace only libcef.dll or an FFmpeg DLL inside the old runtime.
