param(
  [string]$DownloadDir = "C:\maen-cef-build",
  [string]$Branch = "7977",
  [string]$OutputDir = "C:\maen-cef-output",
  [string]$Checkout = "708dc14"
)
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"
$ExpectedCef='152.0.6+g708dc14+chromium-152.0.7977.83'
$ExpectedBranch='7977'
$ExpectedCheckout='708dc14'
if($Branch -ne $ExpectedBranch){throw "Wrong CEF branch: $Branch"}
if($Checkout -ne $ExpectedCheckout){throw "Wrong CEF checkout: $Checkout"}

$env:GN_DEFINES='is_official_build=true is_component_build=false proprietary_codecs=true ffmpeg_branding=Chrome chrome_pgo_phase=0 symbol_level=0 blink_symbol_level=0'
$env:GN_ARGUMENTS='--ide=vs2022 --sln=cef --filters=//cef/*'
New-Item -ItemType Directory -Force -Path $DownloadDir,$OutputDir | Out-Null
$automateDir=Join-Path $DownloadDir 'automate'
$automate=Join-Path $automateDir 'automate-git.py'
if(!(Test-Path $automate)){
  New-Item -ItemType Directory -Force -Path $automateDir | Out-Null
  Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/chromiumembedded/cef/master/tools/automate/automate-git.py' -OutFile $automate
}
$args=@("--download-dir=$DownloadDir","--x64-build","--branch=$Branch","--checkout=$Checkout","--no-debug-build","--minimal-distrib","--client-distrib","--force-distrib")
python $automate @args
if($LASTEXITCODE -ne 0){throw "CEF media build failed: $LASTEXITCODE"}

$dist=Get-ChildItem $DownloadDir -Recurse -Directory -Filter 'cef_binary_*_windows64_minimal' |
  Where-Object {Test-Path (Join-Path $_.FullName 'cmake\FindCEF.cmake')} |
  Sort-Object LastWriteTime -Descending | Select-Object -First 1
if(-not $dist){throw 'CEF minimal Windows x64 distribution not found.'}
$vh=Join-Path $dist.FullName 'include\cef_version.h'
if(!(Test-Path $vh)){throw 'cef_version.h missing.'}
$vt=Get-Content $vh -Raw
if(-not $vt.Contains('CEF_VERSION "152.0.6+g708dc14+chromium-152.0.7977.83"')){throw "CEF version mismatch."}

$marker=Join-Path $dist.FullName 'MAEN_MEDIA_RUNTIME.txt'
@('MaenBrowser CEF media runtime',"cef_version=$ExpectedCef","branch=$ExpectedBranch","checkout=$ExpectedCheckout",'platform=windows64','is_official_build=true','is_component_build=false','proprietary_codecs=true','ffmpeg_branding=Chrome','chrome_pgo_phase=0') |
  Set-Content $marker -Encoding ASCII

$zip=Join-Path $OutputDir 'maen-cef-152-media-windows64.zip'
Remove-Item $zip -Force -ErrorAction SilentlyContinue
Compress-Archive -Path $dist.FullName -DestinationPath $zip -CompressionLevel Optimal
$sha=(Get-FileHash $zip -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content "$zip.sha256" $sha -NoNewline
@('MaenBrowser CEF media runtime',"cef_line=$ExpectedCef","branch=$ExpectedBranch","checkout=$ExpectedCheckout",'platform=windows64','is_official_build=true','is_component_build=false','proprietary_codecs=true','ffmpeg_branding=Chrome','chrome_pgo_phase=0',"sha256=$sha") |
  Set-Content (Join-Path $OutputDir 'maen-cef-152-media-build-manifest.txt') -Encoding ASCII
Write-Host "MEDIA_CEF_ARCHIVE=$zip"
Write-Host "MEDIA_CEF_SHA256=$sha"
