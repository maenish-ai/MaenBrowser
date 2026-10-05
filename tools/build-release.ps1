param([string]$BuildDir = "build")
$ErrorActionPreference = "Stop"
cmake --build $BuildDir --config Release --parallel
Write-Host "Build finished. Copy the release runtime into .\dist before packaging."
