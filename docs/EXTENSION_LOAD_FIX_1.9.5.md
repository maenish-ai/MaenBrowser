# Extension load fix — 1.9.5

The bundled `MaenBrowser Controls` extension failed to load because `manifest.json`
referenced `background-1.9.5.js`, but that file was missing from the package.

This fix adds the versioned service-worker entry point:

```js
import './background.js';
```

The package verifier now also checks that the service worker declared in the manifest
exists and remains configured as an ES module. This prevents a green package build from
shipping another broken extension entry point.

The ad/tracker ON/OFF control is part of this bundled extension; it is not a separate
second extension. Once the extension loads successfully, its popup exposes the native
ad/tracker protection toggle and the extension can be inspected from `chrome://extensions/`.


## Visible ad-block switch
The new-tab/start page now includes a persistent **Ad blocking ON/OFF** control. This is intentionally redundant with the extension action popup so users can always see and change protection even when the extension action is not pinned in the toolbar.
