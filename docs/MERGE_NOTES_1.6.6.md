# MaenBrowser 1.6.8 merge notes

- Main browser: WebView2, retained from working 1.6.5.
- Restored original branded CEF-era start page as static HTML asset.
- Added Home button and default new-tab start page.
- The original CEF-specific download manager, extension integration, and adaptive policies are **not yet ported**.
- Runtime video compatibility depends on Microsoft Edge WebView2 and site DRM support; universal codec playback is not guaranteed.
- This ZIP is source for GitHub Actions; no Windows compilation or video tests have been performed in this environment.
