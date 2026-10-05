$ErrorActionPreference = "Stop"
$required = @(
  "VERSION", "CMakeLists.txt", "installer/MaenBrowser.nsi",
  ".github/workflows/maenbrowser-ci.yml", "tools/fetch-cef.ps1",
  "src/main_win.cpp", "src/app/maen_app.cpp", "src/app/maen_client.cpp",
  "src/app/resource_mode.cpp", "src/app/browser_preferences.cpp",
  "src/extensions/extension_manager.cpp", "src/storage/local_profile.cpp",
  "src/updater/update_manager.cpp"
)
foreach ($f in $required) { if (!(Test-Path $f)) { throw "Missing required file: $f" } }
$version=(Get-Content VERSION -Raw).Trim()
if ($version -ne "1.1.0") { throw "Unexpected VERSION: $version" }
$cmake=Get-Content CMakeLists.txt -Raw
if ($cmake -notmatch 'project\(MaenBrowser VERSION 1\.1\.0') { throw "CMake version mismatch" }
$nsi=Get-Content installer/MaenBrowser.nsi -Raw
if ($nsi -notmatch 'PRODUCT_VERSION "1\.1\.0"') { throw "NSIS version mismatch" }
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
