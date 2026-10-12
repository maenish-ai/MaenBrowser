param(
  [string]$BuildDir = "build",
  [string]$DistDir = "dist"
)

$ErrorActionPreference = "Stop"

python "$PSScriptRoot\run-checks.py"
if ($LASTEXITCODE -ne 0) { throw "Portable checks failed" }
& "$PSScriptRoot\preflight.ps1"
& "$PSScriptRoot\configure-win-x64.ps1" -BuildDir $BuildDir

cmake --build $BuildDir --config Release --parallel
if ($LASTEXITCODE -ne 0) { throw "Build failed" }
ctest --test-dir $BuildDir -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "Native tests failed" }
node tests/browser_smoke.mjs "$BuildDir/Release/MaenBrowser.exe"
if ($LASTEXITCODE -ne 0) { throw "Browser/media integration failed" }

$releaseDir = Join-Path $BuildDir "Release"
if (-not (Test-Path (Join-Path $releaseDir "MaenBrowser.exe"))) {
  throw "MaenBrowser.exe was not produced."
}

Remove-Item $DistDir -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null
Copy-Item "$releaseDir\*" $DistDir -Recurse -Force

Get-ChildItem $DistDir -Recurse -File -Include *.pdb,*.lib,*.exp,*.ilk | Remove-Item -Force
Get-ChildItem $DistDir -File -Filter "Maen*Tests.exe" | Remove-Item -Force
Write-Host "Release staged in $DistDir"
