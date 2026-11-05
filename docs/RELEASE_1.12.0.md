# MaenBrowser 1.12.0 — clearer start page and same-window links

- Replace the translucent start-page treatment with solid, readable surfaces,
  dark headings, explicit form colors and visible keyboard focus. English/Arabic,
  RTL, small screens and the low-resource style use the same readable palette.
  No background images, blur effects or continuous animation are added.
- Add device-local date, time and named timezone. The clock updates once per
  minute while visible and stops its timer when the page is hidden. It uses the
  device clock/timezone, not IP geolocation; changing the weather city does not
  change the system clock.
- Add optional MET Norway forecasts: temperature, condition and wind, city
  selection or separately requested approximate location, one-hour local cache,
  attribution and CC BY4.0 license links. Off by default; coordinates and public
  IP reach MET only after activation. Location is rounded to two decimal places.
  Turning weather off removes its saved preference/cache and aborts the request.
  Weather errors never disable search or the clock. No paid key or SDK is added.
- Fix installer source decoding with UTF8 BOM and explicit NSIS INPUTCHARSET.
  Unicode output alone did not tell Windows NSIS how to decode the Arabic source.
  The Arabic title was verified as UTF16 inside a disposable NSIS test build;
  this test payload is not distributed as a browser installer.
- Ordinary user-clicked links create a new Chrome Runtime tab in the originating
  window, preserving the source chat. The embedded WhatsApp WebView2 callback and
  same-profile OS URL relaunch use this path too. Non-user popups remain blocked
  or follow existing policy; in-page OAuth/payment popup semantics remain intact.
  Family checks run before dispatch and before destination loading. No external
  shell launch is added to web links. New-tab handoffs are window-bound, bounded
  and expire after20 seconds.
- Migrate deprecated Java/Node/Python/Android SDK/Gradle-wrapper and artifact
  actions to Node24-capable releases. Keep Android explicit SDK packages and
  Ubuntu24.04. Upgrade detection, codec tests and the sandbox remain mandatory.
- Version1.12.0; Android source versionCode11200. Android startup UI remains the
  mobile implementation; the dashboard in this release is the desktop start page.

## Validation and limits

Portable checks, PowerShell preflight, real Chromium page rendering at desktop
English/Arabic and mobile Arabic sizes, mocked weather success/off, live MET
endpoint/CORS and the NSIS Arabic encoding fixture were exercised locally.
Android debug/release build, unit tests and Lint passed locally. Screenshots are
provided for review. Windows cannot be compiled/run in this Linux workspace.
Windows CI now tests that an external launch and a user-gesture link each open a
new tab in the same window without replacing the source. Its result after upload
is required before distributing the new Setup. Live WhatsApp accounts and actual
Windows installer UI still need device acceptance.

The previous GitHub run42008977527 succeeded; its notices were action-deprecation
warnings, not a failed video build. A future runner/tool may emit new warnings;
this release does not suppress diagnostics to make a run look green.

The working Windows codec engine remains CEF154; the separate engine-upgrade
work and current-security-parity limitations documented in1.11.0 remain. No new
universal codec claim or protection bypass is introduced here.

Weather service terms and data rights:
https://api.met.no/doc/TermsOfService
https://api.met.no/
https://creativecommons.org/licenses/by/4.0/
