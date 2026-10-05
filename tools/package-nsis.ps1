$ErrorActionPreference = "Stop"

$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))

& "$PSScriptRoot\build-release.ps1"

$makensis = Get-Command makensis -ErrorAction SilentlyContinue
if (-not $makensis) {
  throw "NSIS not found. Install with: winget install NSIS.NSIS"
}

Push-Location (Join-Path $repoRoot "installer")
try {
  & $makensis.Source "MaenBrowser.nsi"
} finally {
  Pop-Location
}

$setup = Join-Path $repoRoot "installer\MaenBrowser-1.5.6-Setup.exe"
if (-not (Test-Path $setup)) {
  throw "Installer was not produced."
}

Write-Host "Installer: $setup"
