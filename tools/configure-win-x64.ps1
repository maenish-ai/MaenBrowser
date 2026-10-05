param(
  [Parameter(Mandatory=$true)][string]$CefRoot,
  [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"
cmake -S . -B $BuildDir -G "Visual Studio 17 2022" -A x64 -DCEF_ROOT="$CefRoot"
Write-Host "Configured MaenBrowser. Build with: cmake --build $BuildDir --config Release"
