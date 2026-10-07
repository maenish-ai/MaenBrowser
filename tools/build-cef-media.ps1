param(
  [string]$DownloadDir = "C:\maen-cef-build",
  [string]$Branch = "7977",
  [string]$OutputDir = "C:\maen-cef-output",
  [string]$Checkout = "708dc14"
)
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$ExpectedCef = '152.0.6+g708dc14+chromium-152.0.7977.83'
$ExpectedBranch = '7977'
$ExpectedCheckout = '708dc14'

if ($Branch -ne $ExpectedBranch) {
  throw "Refusing wrong CEF branch '$Branch'. MaenBrowser 1.5.32 requires branch $ExpectedBranch."
}
if ($Checkout -ne $ExpectedCheckout) {
  throw "Refusing wrong CEF checkout '$Checkout'. Required CEF commit is $ExpectedCheckout."
}

# Compile-time media switches. These must be present in libcef.dll itself.
# Keep Chromium security features intact; this does not disable sandbox/site isolation/TLS checks.
$env:GN_DEFINES = 'is_official_build=true is_component_build=false proprietary_codecs=true ffmpeg_branding=Chrome chrome_pgo_phase=0 symbol_level=0 blink_symbol_level=0'
$env:GN_ARGUMENTS = '--ide=vs2022 --sln=cef --filters=//cef/*'

New-Item -ItemType Directory -Force -Path $DownloadDir,$OutputDir | Out-Null
$automateDir = Join-Path $DownloadDir 'automate'
$automate = Join-Path $automateDir 'automate-git.py'
if (!(Test-Path $automate)) {
  New-Item -ItemType Directory -Force -Path $automateDir | Out-Null
  Invoke-WebRequest `
    -Uri 'https://raw.githubusercontent.com/chromiumembedded/cef/master/tools/automate/automate-git.py' `
    -OutFile $automate
}

Write-Host "CEF media build"
Write-Host "  branch   = $Branch"
Write-Host "  checkout = $Checkout"
Write-Host "  target   = Windows x64"
Write-Host "  expected = $ExpectedCef"
Write-Host "  GN       = $env:GN_DEFINES"

# Do not use --force-clean here: preserving the checkout allows an interrupted large build
# to resume instead of redownloading Chromium.
$args = @(
  "--download-dir=$DownloadDir",
  "--x64-build",
  "--branch=$Branch",
  "--checkout=$Checkout",
  "--no-debug-build",
  "--minimal-distrib",
  "--client-distrib",
  "--force-distrib"
)
python $automate @args
if ($LASTEXITCODE -ne 0) { throw "CEF media build failed: $LASTEXITCODE" }

$dist = Get-ChildItem $DownloadDir -Recurse -Directory -Filter 'cef_binary_*_windows64_minimal' |
  Where-Object { Test-Path (Join-Path $_.FullName 'cmake\FindCEF.cmake') } |
  Sort-Object LastWriteTime -Descending |
  Select-Object -First 1
if (-not $dist) { throw 'CEF minimal Windows x64 distribution was not found after build.' }

$versionHeader = Join-Path $dist.FullName 'include\cef_version.h'
if (!(Test-Path $versionHeader)) {
  throw 'Generated CEF distribution is missing include\cef_version.h.'
}
$versionText = Get-Content $versionHeader -Raw
if (-not $versionText.Contains('CEF_VERSION "152.0.6+g708dc14+chromium-152.0.7977.83"')) {
  throw "Generated CEF does not match $ExpectedCef."
}

# A marker used by MaenBrowser's release pipeline. This proves which recipe produced
# the archive; runtime WhatsApp playback still remains the final acceptance test.
$marker = Join-Path $dist.FullName 'MAEN_MEDIA_RUNTIME.txt'
@(
  'MaenBrowser CEF media runtime',
  "cef_version=$ExpectedCef",
  "branch=$ExpectedBranch",
  "checkout=$ExpectedCheckout",
  'platform=windows64',
  'is_official_build=true',
  'is_component_build=false',
  'proprietary_codecs=true',
  'ffmpeg_branding=Chrome',
  'chrome_pgo_phase=0'
) | Set-Content -Path $marker -Encoding ASCII

$zip = Join-Path $OutputDir 'maen-cef-152-media-windows64.zip'
Remove-Item $zip -Force -ErrorAction SilentlyContinue
Compress-Archive -Path $dist.FullName -DestinationPath $zip -CompressionLevel Optimal

$sha = (Get-FileHash $zip -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content -Path "$zip.sha256" -Value $sha -NoNewline

$manifest = Join-Path $OutputDir 'maen-cef-152-media-build-manifest.txt'
@(
  'MaenBrowser CEF media runtime',
  "cef_line=$ExpectedCef",
  "branch=$ExpectedBranch",
  "checkout=$ExpectedCheckout",
  'platform=windows64',
  'is_official_build=true',
  'is_component_build=false',
  'proprietary_codecs=true',
  'ffmpeg_branding=Chrome',
  'chrome_pgo_phase=0',
  "sha256=$sha"
) | Set-Content -Path $manifest -Encoding ASCII

Write-Host "MEDIA_CEF_ARCHIVE=$zip"
Write-Host "MEDIA_CEF_SHA256=$sha"
Write-Host "Build complete. Use this Windows64 archive in MaenBrowser's -RequireMedia pipeline."
