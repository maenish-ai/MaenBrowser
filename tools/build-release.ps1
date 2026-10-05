param(
  [string]$BuildDir = "build",
  [string]$DistDir = "dist"
)

$ErrorActionPreference = "Stop"

& "$PSScriptRoot\configure-win-x64.ps1" -BuildDir $BuildDir

cmake --build $BuildDir --config Release --parallel

$releaseDir = Join-Path $BuildDir "Release"
if (-not (Test-Path (Join-Path $releaseDir "MaenBrowser.exe"))) {
  throw "MaenBrowser.exe was not produced."
}

Remove-Item $DistDir -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null
Copy-Item "$releaseDir\*" $DistDir -Recurse -Force

Write-Host "Release staged in $DistDir"
