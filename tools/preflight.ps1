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
if ($version -ne "1.4.0") { throw "Unexpected VERSION: $version" }
$cmake=Get-Content CMakeLists.txt -Raw
if ($cmake -notmatch 'project\(MaenBrowser VERSION 1\.3\.0') { throw "CMake version mismatch" }
$nsi=Get-Content installer/MaenBrowser.nsi -Raw
if ($nsi -notmatch 'PRODUCT_VERSION "1\.3\.0"') { throw "NSIS version mismatch" }
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
  'OutFile "MaenBrowser-1.4.0-Setup.exe"',
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
