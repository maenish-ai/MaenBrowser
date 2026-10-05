param(
  [string]$Destination = "third_party/cef",
  [string]$CefVersion = "152.0.6+g708dc14+chromium-152.0.7977.83"
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$platform = "windows64"
$archiveName = "cef_binary_${CefVersion}_${platform}_minimal.tar.bz2"
$baseUrl = "https://cef-builds.spotifycdn.com"
$indexUrl = "$baseUrl/index.json"

$destinationPath = [System.IO.Path]::GetFullPath($Destination)
New-Item -ItemType Directory -Force -Path $destinationPath | Out-Null

$cefRoot = Join-Path $destinationPath "cef_binary_${CefVersion}_${platform}"

if (Test-Path (Join-Path $cefRoot "cmake\FindCEF.cmake")) {
  Write-Output $cefRoot
  exit 0
}

Write-Host "Reading official CEF index..."
$index = Invoke-RestMethod -Uri $indexUrl

$versionEntry = $index.$platform.versions |
  Where-Object { $_.cef_version -eq $CefVersion } |
  Select-Object -First 1

if (-not $versionEntry) {
  throw "CEF $CefVersion not found for $platform."
}

$fileEntry = $versionEntry.files |
  Where-Object { $_.name -eq $archiveName -or $_.type -eq "minimal" } |
  Select-Object -First 1

if (-not $fileEntry) {
  throw "Minimal CEF archive metadata not found."
}

$tempRoot = if ($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { $env:TEMP }
$tempArchive = Join-Path $tempRoot $archiveName

Write-Host "Downloading CEF $CefVersion..."
Invoke-WebRequest -Uri "$baseUrl/$archiveName" -OutFile $tempArchive

if ($fileEntry.sha1) {
  $actual = (Get-FileHash $tempArchive -Algorithm SHA1).Hash.ToLowerInvariant()
  $expected = ([string]$fileEntry.sha1).ToLowerInvariant()
  if ($actual -ne $expected) {
    throw "CEF SHA1 mismatch. Expected $expected, got $actual."
  }
}

Write-Host "Extracting CEF..."
tar -xjf $tempArchive -C $destinationPath
Remove-Item $tempArchive -Force -ErrorAction SilentlyContinue

if (-not (Test-Path (Join-Path $cefRoot "cmake\FindCEF.cmake"))) {
  throw "CEF extraction failed: FindCEF.cmake missing."
}

Write-Output $cefRoot
