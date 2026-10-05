$ErrorActionPreference = "Stop"
$required = @(
  "VERSION", "CMakeLists.txt", "installer/MaenBrowser.nsi",
  ".github/workflows/maenbrowser-ci.yml", "tools/fetch-cef.ps1",
  "src/main_win.cpp", "src/app/maen_app.cpp", "src/app/maen_client.cpp",
  "src/app/resource_mode.cpp", "src/app/browser_preferences.cpp", "src/app/start_page.cpp",
  "src/win/maenbrowser.rc", "assets/maenbrowser.ico",
  "src/extensions/extension_manager.cpp", "src/storage/local_profile.cpp",
  "src/updater/update_manager.cpp"
)
foreach ($f in $required) { if (!(Test-Path $f)) { throw "Missing required file: $f" } }
$version=(Get-Content VERSION -Raw).Trim()
if ($version -ne "1.5.1") { throw "Unexpected VERSION: $version" }
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
if ($wf -notmatch 'runs-on: windows-2022') { throw "CI must be pinned to windows-2022" }
if ($wf -notmatch 'Visual Studio 17') { throw "CI must use VS 2022 generator" }
if ($wf -notmatch 'USE_SANDBOX=ON') { throw "Production CI must enable CEF sandbox" }
Write-Host "MaenBrowser preflight passed." -ForegroundColor Green

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
  'OutFile "MaenBrowser-1.5.1-Setup.exe"',
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

# Smart-download popup regression guards.
$clientH = Get-Content "src/app/maen_client.h" -Raw
$clientCpp = Get-Content "src/app/maen_client.cpp" -Raw
foreach ($needle in @("OnBeforePopup", "popup_browser_ids_", "browser->IsPopup()", "CloseBrowser(false)")) {
  if (-not ($clientH.Contains($needle) -or $clientCpp.Contains($needle))) {
    throw "Smart download handling missing: $needle"
  }
}
if ($clientCpp -match 'OnBeforePopup[\s\S]{0,1800}return true;') {
  throw "Popup policy must not blanket-cancel normal OAuth/payment/login popups."
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

# Download reliability regression gate.
$clientSource = Get-Content "src/app/maen_client.cpp" -Raw
$beforeStart = $clientSource.IndexOf("bool MaenClient::OnBeforeDownload")
$updatedStart = $clientSource.IndexOf("void MaenClient::OnDownloadUpdated")
if ($beforeStart -lt 0 -or $updatedStart -le $beforeStart) { throw "Download handler boundaries not found." }
$beforeDownloadBody = $clientSource.Substring($beforeStart, $updatedStart - $beforeStart)
if ($beforeDownloadBody.Contains("CloseBrowser")) {
  throw "Download regression: never close the initiating browser from OnBeforeDownload."
}
if (-not $beforeDownloadBody.Contains("return false;")) {
  throw "Chrome Runtime download delegation missing."
}
