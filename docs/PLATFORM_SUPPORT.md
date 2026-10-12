# Supported target — 1.9.6

Windows x64 with an OS/CPU/GPU supported by the pinned CEF runtime. Resource modes
target 2 GB and 4 GB machines but do not guarantee performance for all sites.
Windows 7/8/8.1 and 32-bit Windows are not claimed as supported. No Android build.

Setup registers the browser as an HTTP/HTTPS candidate; Windows/user chooses the
default. Profiles survive upgrade and uninstall. The optional manual WebView2
window requires its Runtime already installed; ordinary browsing uses CEF.
Current release and hardware qualification remain subject to Windows acceptance.
