param(
  [string]$DownloadDir = "C:\maen-cef-build",
  [string]$Branch = "7977",
  [string]$OutputDir = "C:\maen-cef-output"
)
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"
$ExpectedCef = '152.0.6+g708dc14+chromium-152.0.7977.83'

# These are compile-time media switches. A stock CEF binary cannot be upgraded
# to H.264/AAC support after compilation.
$env:GN_DEFINES = 'is_official_build=true proprietary_codecs=true ffmpeg_branding=Chrome chrome_pgo_phase=0'
$env:GN_ARGUMENTS = '--ide=vs2022 --sln=cef --filters=//cef/*'

New-Item -ItemType Directory -Force -Path $DownloadDir,$OutputDir | Out-Null
$automateDir = Join-Path $DownloadDir 'automate'
$automate = Join-Path $automateDir 'automate-git.py'
if (!(Test-Path $automate)) {
  New-Item -ItemType Directory -Force -Path $automateDir | Out-Null
  Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/chromiumembedded/cef/master/tools/automate/automate-git.py' -OutFile $automate
}

Write-Host "Building CEF branch $Branch x64 with H.264/AAC media support..."
Write-Host "GN_DEFINES=$env:GN_DEFINES"
python $automate "--download-dir=$DownloadDir" --x64-build "--branch=$Branch" --no-debug-build --minimal-distrib --client-distrib --force-distrib
if ($LASTEXITCODE -ne 0) { throw "CEF media build failed: $LASTEXITCODE" }

$dist = Get-ChildItem $DownloadDir -Recurse -Directory -Filter 'cef_binary_*_windows64_minimal' |
  Where-Object { Test-Path (Join-Path $_.FullName 'cmake\FindCEF.cmake') } |
  Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $dist) { throw 'CEF minimal media distribution was not found after build.' }

# Prove that the generated distribution is the exact CEF/Chromium line used by MaenBrowser.
$versionHeader = Join-Path $dist.FullName 'include\cef_version.h'
if (!(Test-Path $versionHeader)) { throw 'Generated CEF distribution is missing include\cef_version.h.' }
$versionText = Get-Content $versionHeader -Raw
if (-not $versionText.Contains('CEF_VERSION "152.0.6+g708dc14+chromium-152.0.7977.83"')) {
  throw "Generated CEF version does not match required runtime $ExpectedCef."
}

# Embed a marker inside the archive. The application Setup pipeline refuses a
# -RequireMedia runtime unless this marker and exact version are present.
$marker = Join-Path $dist.FullName 'MAEN_MEDIA_RUNTIME.txt'
@(
  'MaenBrowser verified media CEF runtime',
  "cef_version=$ExpectedCef",
  'branch=7977',
  'is_official_build=true',
  'proprietary_codecs=true',
  'ffmpeg_branding=Chrome',
  'chrome_pgo_phase=0'
) | Set-Content -Path $marker -Encoding ASCII

$zip = Join-Path $OutputDir 'maen-cef-152-media-windows64.zip'
Remove-Item $zip -Force -ErrorAction SilentlyContinue
Compress-Archive -Path $dist.FullName -DestinationPath $zip -CompressionLevel Optimal
$sha=(Get-FileHash $zip -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content -Path "$zip.sha256" -Value $sha -NoNewline
$manifest = Join-Path $OutputDir 'maen-cef-152-media-build-manifest.txt'
@(
  'MaenBrowser CEF media runtime',
  'branch=7977',
  "cef_line=$ExpectedCef",
  'is_official_build=true',
  'proprietary_codecs=true',
  'ffmpeg_branding=Chrome',
  'chrome_pgo_phase=0',
  "sha256=$sha"
) | Set-Content -Path $manifest -Encoding ASCII
Write-Host "MEDIA_CEF_ARCHIVE=$zip"
Write-Host "MEDIA_CEF_SHA256=$sha"
Write-Host 'Build complete. Publish this exact archive and configure MAEN_CEF_ARCHIVE_URL / MAEN_CEF_ARCHIVE_SHA256.'
