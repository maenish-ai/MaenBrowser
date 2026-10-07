# CI fix 1.6.3

This revision fixes the native WebView2 host integration by linking `advapi32`,
which is required by Microsoft's static WebView2 loader, and explicitly
initializing a single-threaded COM apartment before creating the WebView2
environment. The Setup-only workflow and stock CEF 152 + WebView2 hybrid
architecture are otherwise unchanged.
