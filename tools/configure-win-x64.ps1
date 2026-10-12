param([string]$BuildDir = "build")
$ErrorActionPreference = "Stop"
$cefRoot = & "$PSScriptRoot\fetch-cef.ps1" -RequireMedia
if (-not $cefRoot) { throw "CEF root not returned" }
if (-not $env:WEBVIEW2_SDK_ROOT) { throw "Set WEBVIEW2_SDK_ROOT to the extracted Microsoft.Web.WebView2 1.0.4258.31 NuGet SDK" }
cmake -S "$PSScriptRoot\.." -B $BuildDir -G "Visual Studio 17 2022" -A x64 "-DCEF_ROOT=$cefRoot" "-DWEBVIEW2_SDK_ROOT=$env:WEBVIEW2_SDK_ROOT" -DMAEN_ENABLE_UPDATER=OFF -DUSE_SANDBOX=ON
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }
