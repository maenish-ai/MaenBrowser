# MaenBrowser1.12.1

- Compact18px clock, unchanged small date text, adjacent compact weather, no
  individual widget borders/backgrounds. Status strip is above the brand/search;
  search-engine and shortcut content is retained. City/location/off controls,
  forecast timestamps and detailed licensing are inside collapsed Weather settings.
  MET Norway credit remains visible beside the weather heading.
- Desktop weather starts automatically unless explicitly disabled. Browser/OS
  location permission is respected; approximate coordinates are rounded. If
  location is denied/unavailable, a known device-timezone city can supply a
  clearly named fallback. Saved city preferences are retained. No IP-location
  lookup service or new API key is introduced. Unknown locations can use the city
  picker. Nearby recognized cities receive a city label; other locations remain
  labelled approximate. The selected/approximate coordinates and public IP reach
  MET Norway. Weather can be turned off persistently.
- Refresh every15 minutes while visible, on returning to a visible tab, and on
  reopening. Shared local cache respects provider Expires (15–60minutes). Hidden
  tabs stop their refresh timer. This is automatic forecast refresh, not streaming
  sensor observations; model data may change less frequently than15 minutes.
- Native CEF resource labels for Product/Your/About/Customize/profile menus are
  overridden with MaenBrowser using version-safe resource-ID mapping. No DLL/PAK
  patching. Engine/version/security/license/copyright attribution stays accurate.
  Verify native-menu appearance on the Windows build; a Linux HTML screenshot
  cannot establish native Chrome-menu rendering.
- About, Settings, History, Downloads and Bookmark Manager commands request a
  distinct tab using background disposition rather than replacing the source.
  Existing ordinary link routing and auth/payment popup policy remain. Internal
  targets are a fixed exact allowlist, not arbitrary chrome/file/extension URLs.
  Family navigation restrictions still apply. Windows acceptance must verify
  background selection behavior in the shipped CEF runtime.
- Upload-artifact v5 still used Node20 in run42016872104 despite the earlier
  update. All artifact uploads now use v7 and downloads use v8; their published
  action metadata was checked for Node24. Other successful Android/Windows jobs
  and tests remain required. Do not hide warnings or skip device tests.

Run42016872104 was successful in every Windows/Android job, not a failed build.
This update addresses the recurring artifact-action warning. New remote execution
is required after upload to verify the new workflow and native UI.

Validation: portable checks including compiled internal-URL allowlist tests;
PowerShell preflight; real Chromium desktop EN/AR and mobile-width AR rendering;
automatic mocked geolocation+weather; compact styles; responsive layout;
persistent off state. Screenshots use synthetic weather data, not live conditions.
No Windows native execution or new Android build was run for this maintenance
change in this Linux workspace. CI builds version1.12.1 / Android versionCode11201.
