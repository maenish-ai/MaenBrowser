param(
  [string]$BuildDir = "build"
)

$ErrorActionPreference = "Stop"

$cefRoot = & "$PSScriptRoot\fetch-cef.ps1"

cmake -S "$PSScriptRoot\.." `
      -B $BuildDir `
      -G "Visual Studio 17 2022" `
      -A x64 `
      "-DCEF_ROOT=$cefRoot" `
      -DMAEN_ENABLE_UPDATER=OFF
