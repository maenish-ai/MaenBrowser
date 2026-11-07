# Apply previously uploaded fixes — 2026-11-07

Base: `a5f1e0165e` on GitHub main. Commits `cfd93ba` and `a5f1e01` uploaded files under an extra top-level `files/` directory. Those copies were not used by the application build. This change copies the supplied, byte-identical files to their real paths under `assets/`, `config/`, `tools/`, `tests/`, `android/` and `docs/`. Existing `files/` copies are retained because they are already part of the owner's repository.

The applied changes add 900 entries to the additional ad list (some overlap the main list), reduce redundant tab eligibility work and parse Android request hosts once. These do not demonstrate real-world performance gains. Origin sources, pinned hashes and reproduction instructions are in the filter lock and manifest. Existing security, sandbox and family checks are retained.

This does not claim universal codec, site, extension or ad-block detection compatibility. CEF154 remains the active Windows runtime, older than the stable CEF reported in `research/2026-11-07/engine.json`; upgrading it safely requires building a compatible codec-enabled runtime and Windows acceptance tests. Real chat, call, extension and low-memory device tests remain necessary.
