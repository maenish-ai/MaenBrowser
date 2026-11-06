# Engine upgrade plan

Capture: 2026-11-05; [machine-readable freshness snapshot](research/2026-11-05/engine-freshness.json). The audit tool now reads active release cohorts rather than assuming the first item of the version-history list is the deployed stable version; ended/future/zero-fraction releases are excluded and absent evidence fails explicitly.

| Source | Captured result | Meaning |
|---|---|---|
| `config/media-runtime.lock.json` | CEF154.0.34+g14c5a08+chromium-154.0.8037.98 | Current bundled codec runtime, not current upstream |
| Official CEF build index | CEF156.0.15+gcf671c0+chromium-156.0.8078.80 | Current available CEF stable at capture |
| Existing upgrade candidate | Same CEF156, branch8078, commit cf671c072925b27823a61181152f4f288e8e4306 | Candidate verified against index, still unbuilt here |
| Chrome active Windows stable releases | 156.0.8078.80/.81/.82,157.0.8103.38/.39,158.0.8127.18 across cohorts | Highest active158 is not the only user-deployed version; CEF availability is separate |

The currently pinned runtime cannot be described as up-to-date. Release gating requires comparing Chromium security advisories and the exact CEF branch/backport log. Example advisory context: [CVE-2026-118357](https://www.cve.org/CVERecord?id=2026-118357) describes FileSystem use-after-free before156.0.8078.73. This does not establish exploit reachability in MaenBrowser or prove all later versions fixed every issue. Restricted Chromium issues were not accessed. No fabricated CVE coverage list.

## Build and adoption sequence

1. Refresh [official CEF index](https://cef-builds.spotifycdn.com/index.json), [CEF branches/building](https://github.com/chromiumembedded/cef/blob/master/docs/branches_and_building.md), [active Chrome cohorts](https://versionhistory.googleapis.com/v1/chrome/platforms/win/channels/stable/versions/all/releases?filter=endtime=none&page_size=100) and [official security release notes](https://chromereleases.googleblog.com/). Prefer the newest supported stable CEF carrying required fixes, not an untested development snapshot or a higher unrelated Chrome DLL.
2. Provision isolated Windows builder using current branch toolchain requirements (VS2022/SDK per upstream, at least300GB SSD), locked depot_tools/automate and source commits. Existing candidate automate SHA:94d3ddd1e0d35482132f45dc101a593c9371b6eb2d8ff4af6cfe2734d2e48c6a. This Linux workspace has less than8GB filesystem capacity and cannot perform that build.
3. Use existing `tools/build-cef-media.ps1`, `build-cef-upgrade.yml`; record proprietary_codecs=true and ffmpeg_branding=Chrome only with distribution rights, sandbox enabled, targetx64, complete GN/toolchain provenance and SBOM. Source-build output must match its own headers/import libraries/resources/locales/subprocess/sandbox; never mix versions.
4. Run candidate app build and actual codec decode, worker/popups, download-context, camera/mic/WebRTC, origin/Family, native resource branding, TLS, profiles, Arabic and installer tests. New Chromium IDs/API changes may require audited adaptations. Test profile upgrade and compatibility before switching lock.
5. Compare advisory/backport list, sign the complete runtime/application, calculate SHA256, update runtime lock/version/provenance only after gates pass. Keep previous signed source/build evidence; rollback only above security floor. Android updateable WebView engine is managed separately by Android/vendor; app release cannot guarantee its installed version.

This batch does not download or replace the active engine, enable insecure flags, claim a successful CEF156 build, or publish unsigned packages. A larger newer CEF build infrastructure and native acceptance remain P0 release blockers.

## 1.13.0 follow-up,2026-11-06

Fresh capture remains CEF154pinned versus stable CEF156 and active Chrome cohorts up to158. Added explicit candidate lock support, `tools/test-cef-candidate.ps1`, candidate compilation/CTest/runtime smoke after source build, sandbox and provenance unchanged. No candidate binary was built in this Linux workspace. Production tag gate rejects old/unknown/stale engine evidence. See current IMPLEMENTATION_STATUS.md.
