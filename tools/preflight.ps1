$ErrorActionPreference = "Stop"
$required = @(
  "VERSION", "CMakeLists.txt", "installer/MaenBrowser.nsi",
  ".github/workflows/maenbrowser-ci.yml", "tools/fetch-cef.ps1",
  "src/main_win.cpp", "src/app/maen_app.cpp", "src/app/maen_client.cpp",
  "src/app/resource_mode.cpp", "src/app/browser_preferences.cpp", "src/app/start_page.cpp",
  "src/win/maenbrowser.rc", "assets/maenbrowser.ico",
  "src/extensions/extension_manager.cpp", "src/storage/local_profile.cpp",
  "src/updater/update_manager.cpp", "docs/PERFORMANCE.md", "docs/MEDIA_COMPATIBILITY.md",
  "tools/verify-media-runtime.ps1", "tools/build-cef-media.ps1", "docs/CUSTOM_CEF_MEDIA_BUILD.md",
  "src/media/windows_media_foundation.cpp", "src/media/windows_media_foundation.h", "docs/WINDOWS_MEDIA_FOUNDATION.md"
)
foreach ($f in $required) { if (!(Test-Path $f)) { throw "Missing required file: $f" } }
$version=(Get-Content VERSION -Raw).Trim()
if ($version -notmatch '^\d+\.\d+\.\d+$') { throw "VERSION must be semantic x.y.z: $version" }
$cmake=Get-Content CMakeLists.txt -Raw
$escapedVersion = [regex]::Escape($version)
if ($cmake -notmatch "project\(MaenBrowser VERSION $escapedVersion") {
  throw "CMake version mismatch: expected $version"
}
$nsi=Get-Content installer/MaenBrowser.nsi -Raw
if ($nsi -notmatch ('PRODUCT_VERSION "' + $escapedVersion + '"')) {
  throw "NSIS version mismatch: expected $version"
}
$wf=Get-Content .github/workflows/maenbrowser-ci.yml -Raw
# Distribution policy: publish Setup only; do not spend CI/storage on a Portable artifact.
if ($wf -match '(?i)Package portable|Upload portable|Windows-x64-Portable|Compress-Archive') {
  throw "Distribution regression: GitHub Actions must publish Setup only (no Portable artifact)."
}
if ($wf -notmatch 'runs-on: windows-2022') { throw "CI must be pinned to windows-2022" }
if ($wf -notmatch 'Visual Studio 17') { throw "CI must use VS 2022 generator" }
if ($wf -notmatch 'USE_SANDBOX=ON') { throw "Production CI must enable CEF sandbox" }

# Clean-upgrade regression guards.
$installer = Get-Content "installer/MaenBrowser.nsi" -Raw
foreach ($needle in @("Function CleanOldProgramFiles", "Call CleanOldProgramFiles", 'RMDir /r "$INSTDIR"', 'tasklist /FI "IMAGENAME eq ${PRODUCT_EXE}"')) {
  if (-not $installer.Contains($needle)) { throw "Clean-upgrade guard missing: $needle" }
}
$upgradeStart = $installer.IndexOf("Function CleanOldProgramFiles")
$upgradeEnd = $installer.IndexOf("FunctionEnd", $upgradeStart)
$upgradeBody = $installer.Substring($upgradeStart, $upgradeEnd - $upgradeStart)
if ($upgradeBody.Contains('RMDir /r "$LOCALAPPDATA\MaenBrowser"')) { throw "Upgrade must preserve user profile." }


# Unified taskbar identity + safe background-download lifecycle guards.
$clientCpp = Get-Content "src/app/maen_client.cpp" -Raw
$mainWin = Get-Content "src/main_win.cpp" -Raw
foreach ($needle in @("ApplyMaenWindowIcon", "WM_SETICON", "MAKEINTRESOURCEW(1)", "SetCurrentProcessExplicitAppUserModelID")) {
  if (-not ($clientCpp.Contains($needle) -or $mainWin.Contains($needle))) {
    throw "Unified Windows identity guard missing: $needle"
  }
}
$beforeStart = $clientCpp.IndexOf("bool MaenClient::OnBeforeDownload")
$updatedStart = $clientCpp.IndexOf("void MaenClient::OnDownloadUpdated")
$beforeBody = $clientCpp.Substring($beforeStart, $updatedStart - $beforeStart)
if ($beforeBody.Contains("CloseBrowser") -or $beforeBody.Contains("SW_HIDE")) {
  throw "Download regression: popup must not be hidden/closed in OnBeforeDownload."
}
$updatedEnd = $clientCpp.IndexOf("void MaenClient::ShowDownloadComplete", $updatedStart)
$updatedBody = $clientCpp.Substring($updatedStart, $updatedEnd - $updatedStart)
foreach ($needle in @("IsInProgress()", "GetReceivedBytes()", "SW_HIDE", "CloseBrowser(false)")) {
  if (-not $updatedBody.Contains($needle)) { throw "Safe background-download lifecycle guard missing: $needle" }
}




# Windows taskbar identity compile-portability guard.
$mainWinSource = Get-Content "src/main_win.cpp" -Raw
foreach ($needle in @("GetModuleHandleW", "GetProcAddress", '"SetCurrentProcessExplicitAppUserModelID"', '"MaenBrowser.Desktop"')) {
  if (-not $mainWinSource.Contains($needle)) {
    throw "Dynamic AppUserModelID resolution guard missing: $needle"
  }
}
if ($mainWinSource -match '(?m)^\s*SetCurrentProcessExplicitAppUserModelID\s*\(') {
  throw "Do not directly call SetCurrentProcessExplicitAppUserModelID; resolve it dynamically for SDK portability."
}


# Media compatibility regression gate: do not disable Chromium media/GPU paths.
$resourceSource = Get-Content "src/app/resource_mode.cpp" -Raw
$allCpp = (Get-ChildItem "src" -Recurse -Include *.cpp,*.h | ForEach-Object { Get-Content $_.FullName -Raw }) -join "`n"
foreach ($forbidden in @("disable-gpu", "disable-accelerated-video-decode", "disable-webrtc", "disable-media-source")) {
  if ($allCpp.Contains($forbidden)) { throw "Media regression: forbidden switch present: $forbidden" }
}
$startPageSource = Get-Content "src/app/start_page.cpp" -Raw
foreach ($needle in @("MediaDiagnosticsUrl", "H.264 / MP4", "AAC / MP4", "VP9 / WebM", "AV1", "MediaSource", "WebRTC", "WebCodecs")) {
  if (-not $startPageSource.Contains($needle)) { throw "Media diagnostics guard missing: $needle" }
}



# Adaptive performance/media regression guards.
$resourceAdaptive = Get-Content "src/app/resource_mode.cpp" -Raw
foreach ($needle in @("logical_processors", "67108864", "33554432", "100663296", "50331648")) {
  if (-not $resourceAdaptive.Contains($needle)) { throw "Adaptive low-memory policy missing: $needle" }
}
foreach ($forbidden in @("ignore-gpu-blocklist", "disable-gpu", "disable-accelerated-video-decode", "disable-webgl")) {
  if ($allCpp.Contains($forbidden)) { throw "Unsafe performance regression: $forbidden" }
}
foreach ($needle in @("HEVC/H.265", "FLAC", "Media &amp; 3D capability check")) {
  if (-not $startPageSource.Contains($needle)) { throw "Adaptive media diagnostics missing: $needle" }
}

# Media diagnostics compile guard.
$startPageCompileSource = Get-Content "src/app/start_page.cpp" -Raw
if ($startPageCompileSource.Contains("PercentEncode(")) {
  throw "Undefined PercentEncode helper must not be used; use CefURIEncode."
}
foreach ($needle in @('#include "include/cef_parser.h"', "CefURIEncode(kMediaHtml, false).ToString()")) {
  if (-not $startPageCompileSource.Contains($needle)) { throw "Media diagnostics compile guard missing: $needle" }
}

# CEF 152 API regression guards.
$obsoletePatterns = @(
  "settings.chrome_runtime",
  "settings.persist_user_preferences",
  "->LoadExtension(",
  "->HasExtension(",
  "->GetExtension("
)
$sourceFiles = Get-ChildItem "src" -Recurse -Include *.cpp,*.h
foreach ($pattern in $obsoletePatterns) {
  $hit = $sourceFiles | Select-String -SimpleMatch $pattern | Select-Object -First 1
  if ($hit) { throw "Obsolete CEF 152 API detected: $pattern in $($hit.Path):$($hit.LineNumber)" }
}

# scoped_refptr<T> members need complete T where the owning class destructor is instantiated.
$maenAppHeader = Get-Content "src/app/maen_app.h" -Raw
if ($maenAppHeader -match "class\s+MaenClient\s*;" -and
    $maenAppHeader -match "CefRefPtr<MaenClient>") {
  throw "MaenApp stores CefRefPtr<MaenClient> but only forward-declares MaenClient."
}
if ($maenAppHeader -notmatch '#include\s+"src/app/maen_client.h"') {
  throw "maen_app.h must include maen_client.h for CefRefPtr<MaenClient>."
}

# Branding/start-page regression guards.
$cmake=Get-Content CMakeLists.txt -Raw
if ($cmake -notmatch 'src/app/start_page.cpp') { throw "Start page missing from CMake" }
if ($cmake -notmatch 'src/win/maenbrowser.rc') { throw "Windows icon resource missing from CMake" }
$nsi=Get-Content installer/MaenBrowser.nsi -Raw
if ($nsi -notmatch 'maenbrowser.ico') { throw "Installer icon branding missing" }

# Windows installation/registration guards.
$nsi = Get-Content "installer/MaenBrowser.nsi" -Raw
$installerRequired = @(
  ('OutFile "MaenBrowser-' + $version + '-Setup.exe"'),
  'WriteUninstaller "$INSTDIR\Uninstall.exe"',
  'Software\RegisteredApplications',
  'URLAssociations',
  'MaenBrowserURL',
  'App Paths\MaenBrowser.exe',
  'CreateShortcut "$DESKTOP\MaenBrowser.lnk"'
)
foreach ($needle in $installerRequired) {
  if (-not $nsi.Contains($needle)) { throw "Installer integration missing: $needle" }
}

# Unified lightweight policy regression guards.
$resource = Get-Content "src/app/resource_mode.cpp" -Raw
foreach ($needle in @("Mode::Lite","Mode::Balanced","Mode::Performance","Prerender2")) {
  if (-not $resource.Contains($needle)) { throw "Resource policy missing: $needle" }
}
foreach ($forbidden in @("disable-site-isolation","no-sandbox","ignore-certificate-errors")) {
  if ($resource.Contains($forbidden)) { throw "Forbidden security weakening in resource policy: $forbidden" }
}

# Download reliability regression guards.
# Chrome Runtime owns downloads. Never close the initiating browser during
# OnBeforeDownload: doing so can race redirect/popup download handoff and
# produce an immediate "Canceled" result (for example GitHub artifacts).
$clientH = Get-Content "src/app/maen_client.h" -Raw
$clientCpp = Get-Content "src/app/maen_client.cpp" -Raw
if (-not ($clientH.Contains("OnBeforePopup") -or $clientCpp.Contains("OnBeforePopup"))) {
  throw "Popup handler missing; OAuth/payment/login popup compatibility must be preserved."
}
if ($clientCpp -match 'OnBeforePopup[\s\S]{0,1800}return true;') {
  throw "Popup policy must not blanket-cancel normal OAuth/payment/login popups."
}
$beforeStart = $clientCpp.IndexOf("bool MaenClient::OnBeforeDownload")
$updatedStart = $clientCpp.IndexOf("void MaenClient::OnDownloadUpdated")
if ($beforeStart -lt 0 -or $updatedStart -le $beforeStart) { throw "Download handler boundaries not found." }
$beforeDownloadBody = $clientCpp.Substring($beforeStart, $updatedStart - $beforeStart)
if ($beforeDownloadBody.Contains("CloseBrowser")) {
  throw "Download regression: never close the initiating browser from OnBeforeDownload."
}
if (-not $beforeDownloadBody.Contains("callback->Continue(CefString(), true)")) {
  throw "Explicit CEF download continuation missing."
}
if (-not $beforeDownloadBody.Contains("return true;")) {
  throw "OnBeforeDownload must return true when MaenBrowser explicitly continues the transfer."
}
# Popup tracking itself is safe and required for transient-window cleanup.
# The dangerous regression is acting on the popup during OnBeforeDownload.
if ($beforeDownloadBody.Contains("CloseBrowser") -or $beforeDownloadBody.Contains("SW_HIDE")) {
  throw "Download regression: popup tracking may not close/hide the initiating browser in OnBeforeDownload."
}

# WhatsApp/blob download reliability guard.
# Do not use a naive search for every `return false;` in this function: guard
# clauses may legitimately return before a valid callback exists. Validate the
# actual ownership path instead: explicit Continue + terminal return true.
if ($beforeDownloadBody -notmatch 'callback->Continue\(CefString\(\), true\);\s*return true;') {
  throw "Download regression: explicit Continue must be followed by return true for blob/service-worker downloads."
}

# Search-choice start page regression guards.
$start = Get-Content "src/app/start_page.cpp" -Raw
foreach ($needle in @(
  "www.google.com/search?q=",
  "www.bing.com/search?q=",
  "duckduckgo.com/?q=",
  "search.brave.com/search?q=",
  "www.youtube.com/",
  "mail.google.com/",
  "web.whatsapp.com/",
  "www.wikipedia.org/",
  "localStorage"
)) {
  if (-not $start.Contains($needle)) { throw "Start-page provider missing: $needle" }
}
if ($start -match '<iframe') { throw "Start page must not preload third-party services in iframes." }


# Start-page visual identity guard: embedded landscape only, no remote background request.
foreach ($needle in @('class="shell"', 'backdrop-filter:blur', 'image/svg+xml', 'linear-gradient')) {
  if (-not $start.Contains($needle)) { throw "Start-page visual identity missing: $needle" }
}

# Unified icon regression guards: sandbox output is copied bootstrap.exe, so
# both final EXE stamping and direct shortcut/shell icon references are required.
$workflow = Get-Content ".github/workflows/maenbrowser-ci.yml" -Raw
$nsi = Get-Content "installer/MaenBrowser.nsi" -Raw
if (-not (Test-Path "tools/set-exe-icon.ps1")) { throw "EXE icon stamper missing." }
if (-not $workflow.Contains("set-exe-icon.ps1")) { throw "CI does not stamp final sandbox EXE icon." }
if (-not $nsi.Contains('File /oname=maenbrowser.ico "..\assets\maenbrowser.ico"')) { throw "Installer does not deploy unified icon." }
if (-not $nsi.Contains('"$INSTDIR\maenbrowser.ico" 0')) { throw "Desktop/Start shortcut icon is not unified." }
if (-not $nsi.Contains('"DisplayIcon" "$INSTDIR\maenbrowser.ico"')) { throw "Installed Apps icon is not unified." }
if (-not $nsi.Contains('\DefaultIcon" "" "$INSTDIR\maenbrowser.ico"')) { throw "Browser/protocol shell icon is not unified." }

# Security hardening release gates.
$allSource = (Get-ChildItem "src" -Recurse -Include *.cpp,*.h | Get-Content -Raw) -join "`n"
$workflow = Get-Content ".github/workflows/maenbrowser-ci.yml" -Raw
$securityDoc = Get-Content "docs/SECURITY.md" -Raw
foreach ($forbidden in @(
  "ignore-certificate-errors",
  "allow-insecure-localhost",
  "disable-web-security",
  "disable-site-isolation-trials",
  "disable-features=SitePerProcess",
  "disable-features=IsolateOrigins",
  "allow-running-insecure-content"
)) {
  if ($allSource.Contains($forbidden)) { throw "Release-blocking insecure browser switch detected: $forbidden" }
}
if (-not $workflow.Contains("-DUSE_SANDBOX=ON")) { throw "Release build must keep CEF sandbox enabled." }
foreach ($needle in @("ApplyWindowsProcessHardening","ProcessDEPPolicy","ProcessASLRPolicy","ProcessExtensionPointDisablePolicy")) {
  if (-not $allSource.Contains($needle)) { throw "Windows hardening layer missing: $needle" }
}
if ($allSource -match 'Continue\([^\)]*,\s*false\s*\).*Open|ShellExecute') {
  Write-Warning "Review download flow: downloads must never auto-execute."
}



# 1.6.0 hybrid media release gate.
# Main browsing remains CEF 152. WhatsApp Web is deliberately routed to a
# Microsoft WebView2 host so H.264/AAC playback does not depend on a custom CEF.
$wf = Get-Content ".github/workflows/maenbrowser-ci.yml" -Raw
foreach ($needle in @(
  "WEBVIEW2_SDK_VERSION",
  "Microsoft.Web.WebView2",
  "WebView2LoaderStatic.lib",
  "MicrosoftEdgeWebview2Setup.exe",
  "MaenMediaHost.exe",
  "fetch-cef.ps1",
  "-DUSE_SANDBOX=ON",
  "Upload Setup only"
)) {
  if (-not $wf.Contains($needle)) { throw "Hybrid media CI guard missing: $needle" }
}
if ($wf -match '(?i)Package portable|Upload portable|Windows-x64-Portable') {
  throw "Distribution regression: publish Setup only."
}
$router = Get-Content "src/media/webview2_media_router.cpp" -Raw
$mediaHostSource = Get-Content "src/media/webview2_media_host.cpp" -Raw
$client = Get-Content "src/app/maen_client.cpp" -Raw
foreach ($needle in @("web.whatsapp.com", "MaenMediaHost.exe")) {
  if (-not $router.Contains($needle)) { throw "WhatsApp media router missing: $needle" }
}
foreach ($needle in @("CoInitializeEx", "CreateCoreWebView2EnvironmentWithOptions", "ICoreWebView2Controller", "Navigate")) {
  if (-not $mediaHostSource.Contains($needle)) { throw "WebView2 media host missing: $needle" }
}
if (-not $client.Contains("OpenInMediaHost")) { throw "CEF-to-WebView2 WhatsApp routing is missing." }
$cmake = Get-Content "CMakeLists.txt" -Raw
if (-not $cmake.Contains("advapi32")) { throw "WebView2 static loader dependency advapi32 is missing." }
$nsi = Get-Content "installer/MaenBrowser.nsi" -Raw
foreach ($needle in @("MicrosoftEdgeWebview2Setup.exe", "/silent /install")) {
  if (-not $nsi.Contains($needle)) { throw "WebView2 installer integration missing: $needle" }
}

# PowerShell automatic variables are case-insensitive. Never assign to $Host.
if ($MyInvocation.MyCommand.Path) {
  $selfText = Get-Content $MyInvocation.MyCommand.Path -Raw
  if ($selfText -match '(?im)^\s*\$host\s*=') {
    throw "Preflight regression: `$Host is a read-only PowerShell automatic variable."
  }
}

Write-Host "MaenBrowser preflight passed." -ForegroundColor Green
