param(
  [string]$Destination = "third_party/cef",
  [string]$CefVersion = "152.0.6+g708dc14+chromium-152.0.7977.83",
  [switch]$RequireMedia
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

# Optional trusted custom CEF archive. This is the supported path for a
# version-matched media-enabled CEF build after codec licensing/provenance review.
# Never accept an unverified runtime archive.
$customUrl = $env:MAEN_CEF_ARCHIVE_URL
$customSha256 = $env:MAEN_CEF_ARCHIVE_SHA256
if ($RequireMedia -and [string]::IsNullOrWhiteSpace($customUrl)) {
  throw "Media release requires MAEN_CEF_ARCHIVE_URL; stock CEF fallback is forbidden."
}
if ($customUrl) {
  if (-not $customSha256 -or $customSha256 -notmatch '^[0-9A-Fa-f]{64}$') {
    throw "MAEN_CEF_ARCHIVE_URL requires a 64-hex MAEN_CEF_ARCHIVE_SHA256."
  }
  $tempRoot = if ($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { $env:TEMP }
  $customArchive = Join-Path $tempRoot "maen-custom-cef.zip"
  $customExtract = Join-Path $tempRoot "maen-custom-cef-extract"
  Remove-Item $customArchive -Force -ErrorAction SilentlyContinue
  Remove-Item $customExtract -Recurse -Force -ErrorAction SilentlyContinue
  New-Item -ItemType Directory -Force -Path $customExtract | Out-Null
  Write-Host "Downloading trusted custom CEF media runtime..."
  Invoke-WebRequest -Uri $customUrl -OutFile $customArchive
  $actual=(Get-FileHash $customArchive -Algorithm SHA256).Hash.ToLowerInvariant()
  if ($actual -ne $customSha256.ToLowerInvariant()) { throw "Custom CEF SHA256 mismatch." }
  Expand-Archive -Path $customArchive -DestinationPath $customExtract -Force
  $found = Get-ChildItem $customExtract -Directory -Recurse | Where-Object { Test-Path (Join-Path $_.FullName "cmake\FindCEF.cmake") } | Select-Object -First 1
  if (-not $found) { throw "Custom CEF archive is invalid: FindCEF.cmake missing." }
  if ($RequireMedia) {
    $marker = Join-Path $found.FullName "MAEN_MEDIA_RUNTIME.txt"
    if (!(Test-Path $marker)) { throw "Custom CEF is not a verified Maen media runtime: MAEN_MEDIA_RUNTIME.txt missing." }
    $markerText = Get-Content $marker -Raw
    foreach ($needle in @(
      "cef_version=152.0.6+g708dc14+chromium-152.0.7977.83",
      "proprietary_codecs=true",
      "ffmpeg_branding=Chrome",
      "branch=7977"
    )) {
      if (-not $markerText.Contains($needle)) { throw "Custom CEF media marker missing: $needle" }
    }
    $versionHeader = Join-Path $found.FullName "include\cef_version.h"
    if (!(Test-Path $versionHeader)) { throw "Custom CEF media runtime is missing cef_version.h." }
    $versionText = Get-Content $versionHeader -Raw
    if (-not $versionText.Contains('CEF_VERSION "152.0.6+g708dc14+chromium-152.0.7977.83"')) {
      throw "Custom CEF media runtime version mismatch."
    }
  }
  Write-Output $found.FullName
  exit 0
}

$platform = "windows64"
$archiveName = "cef_binary_${CefVersion}_${platform}_minimal.tar.bz2"
$baseUrl = "https://cef-builds.spotifycdn.com"
$indexUrl = "$baseUrl/index.json"

$destinationPath = [System.IO.Path]::GetFullPath($Destination)
New-Item -ItemType Directory -Force -Path $destinationPath | Out-Null
$cefRoot = Join-Path $destinationPath "cef_binary_${CefVersion}_${platform}_minimal"

if (Test-Path (Join-Path $cefRoot "cmake\FindCEF.cmake")) {
  Write-Output $cefRoot
  exit 0
}

Write-Host "Reading official CEF index..."
$index = Invoke-RestMethod -Uri $indexUrl
$versionEntry = $index.$platform.versions |
  Where-Object { $_.cef_version -eq $CefVersion } |
  Select-Object -First 1
if (-not $versionEntry) { throw "CEF $CefVersion not found for $platform." }

$fileEntry = $versionEntry.files |
  Where-Object { $_.name -eq $archiveName } |
  Select-Object -First 1
if (-not $fileEntry) { throw "Exact minimal CEF archive metadata not found: $archiveName" }

$tempRoot = if ($env:RUNNER_TEMP) { $env:RUNNER_TEMP } else { $env:TEMP }
$tempArchive = Join-Path $tempRoot $archiveName
$tempExtract = Join-Path $tempRoot "maen-cef-extract"
Remove-Item $tempExtract -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $tempExtract | Out-Null

Write-Host "Downloading CEF $CefVersion..."
Invoke-WebRequest -Uri "$baseUrl/$archiveName" -OutFile $tempArchive

if ($fileEntry.sha1) {
  $actual = (Get-FileHash $tempArchive -Algorithm SHA1).Hash.ToLowerInvariant()
  $expected = ([string]$fileEntry.sha1).ToLowerInvariant()
  if ($actual -ne $expected) {
    throw "CEF SHA1 mismatch. Expected $expected, got $actual."
  }
}

$sevenZip = (Get-Command 7z.exe -ErrorAction SilentlyContinue).Source
if (-not $sevenZip) {
  $candidate = "C:\Program Files\7-Zip\7z.exe"
  if (Test-Path $candidate) { $sevenZip = $candidate }
}
if (-not $sevenZip) { throw "7-Zip is required to extract CEF." }

Write-Host "Extracting bzip2 layer with 7-Zip..."
& $sevenZip x $tempArchive "-o$tempExtract" -y | Write-Host
if ($LASTEXITCODE -ne 0) { throw "7-Zip failed to extract the bzip2 layer." }

$tarFile = Get-ChildItem $tempExtract -Filter "*.tar" | Select-Object -First 1
if (-not $tarFile) { throw "CEF tar payload was not produced." }

Write-Host "Extracting CEF tar payload with 7-Zip..."
& $sevenZip x $tarFile.FullName "-o$destinationPath" -y | Write-Host
if ($LASTEXITCODE -ne 0) { throw "7-Zip failed to extract the CEF tar payload." }

Remove-Item $tempArchive -Force -ErrorAction SilentlyContinue
Remove-Item $tempExtract -Recurse -Force -ErrorAction SilentlyContinue

if (-not (Test-Path (Join-Path $cefRoot "cmake\FindCEF.cmake"))) {
  $found = Get-ChildItem $destinationPath -Directory |
    Where-Object { Test-Path (Join-Path $_.FullName "cmake\FindCEF.cmake") } |
    Select-Object -First 1
  if ($found) { $cefRoot = $found.FullName }
}

if (-not (Test-Path (Join-Path $cefRoot "cmake\FindCEF.cmake"))) {
  throw "CEF extraction failed: FindCEF.cmake missing."
}

Write-Output $cefRoot
