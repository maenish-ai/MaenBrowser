param(
  [string]$DownloadDir = "C:\maen-cef-build",
  [string]$Branch = "7977",
  [string]$OutputDir = "C:\maen-cef-output"
)
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

# CEF 152 / Chromium 152.0.7977.x media build. These are compile-time flags;
# they cannot be added to the stock CEF runtime after it has been built.
$env:GN_DEFINES = 'is_official_build=true proprietary_codecs=true ffmpeg_branding=Chrome chrome_pgo_phase=0'
$env:GN_ARGUMENTS = '--ide=vs2022 --sln=cef --filters=//cef/*'

New-Item -ItemType Directory -Force -Path $DownloadDir,$OutputDir | Out-Null
$automateDir = Join-Path $DownloadDir 'automate'
$automate = Join-Path $automateDir 'automate-git.py'
if (!(Test-Path $automate)) {
  New-Item -ItemType Directory -Force -Path $automateDir | Out-Null
  $url='https://raw.githubusercontent.com/chromiumembedded/cef/master/tools/automate/automate-git.py'
  Write-Host "Downloading official CEF automated build script..."
  Invoke-WebRequest -Uri $url -OutFile $automate
}

Write-Host "Building CEF branch $Branch x64 with Chrome media codecs..."
Write-Host "GN_DEFINES=$env:GN_DEFINES"
python $automate "--download-dir=$DownloadDir" --x64-build "--branch=$Branch" --no-debug-build --minimal-distrib --client-distrib --force-distrib
if ($LASTEXITCODE -ne 0) { throw "CEF media build failed: $LASTEXITCODE" }

# Find the generated minimal binary distribution and archive it for MaenBrowser CI.
$dist = Get-ChildItem $DownloadDir -Recurse -Directory -Filter 'cef_binary_*_windows64_minimal' |
  Where-Object { Test-Path (Join-Path $_.FullName 'cmake\FindCEF.cmake') } |
  Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $dist) { throw 'CEF minimal media distribution was not found after build.' }

$zip = Join-Path $OutputDir 'maen-cef-152-media-windows64.zip'
Remove-Item $zip -Force -ErrorAction SilentlyContinue
Compress-Archive -Path $dist.FullName -DestinationPath $zip -CompressionLevel Optimal
$sha=(Get-FileHash $zip -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content -Path "$zip.sha256" -Value $sha -NoNewline
Write-Host "MEDIA_CEF_ARCHIVE=$zip"
Write-Host "MEDIA_CEF_SHA256=$sha"
Write-Host 'Build complete. Use this exact archive + SHA256 as MAEN_CEF_ARCHIVE_URL / MAEN_CEF_ARCHIVE_SHA256.'
