# Roadmap

## 1.1.0 baseline
- Windows x64 executable
- Chrome-style tabs/address/navigation
- local profile
- unpacked extension loading
- installer/uninstaller
- GitHub CI artifacts
- no private cloud

## Before calling 1.1.0 production-stable
- Add a signed application icon/resource
- Exercise installer/uninstaller in CI
- Configure Authenticode signing
- Configure signed automatic updates
- Add crash/telemetry policy (default off)
- Add explicit memory benchmarks on 4 GB / 8 GB machines
- Validate extension compatibility matrix
- Add Windows 10/11 smoke tests

## Legacy channel
Use a separately pinned CEF branch for older Windows versions, with explicit
security support limits.
