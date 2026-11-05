param(
  [string]$DownloadDir = "C:\maen-cef-build",
  [string]$Branch = "8037",
  [string]$OutputDir = "C:\maen-cef-output",
  [string]$Checkout = "14c5a08",
  [string]$ProfilePath = ""
)
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"
& "$PSScriptRoot\check-media-builder.ps1" -BuildRoot $DownloadDir
$ExpectedCef='154.0.34+g14c5a08+chromium-154.0.8037.98'
$ExpectedBranch='8037'
$ExpectedCheckout='14c5a08'
$AutomateUrl='https://raw.githubusercontent.com/chromiumembedded/cef/14c5a08/tools/automate/automate-git.py'
$AutomateSha=''
if ($ProfilePath) {
  $profile=Get-Content -LiteralPath $ProfilePath -Raw -Encoding UTF8 | ConvertFrom-Json
  if($profile.schema -ne 1 -or $profile.automate_sha256 -notmatch '^[a-f0-9]{64}$' -or $profile.checkout -notmatch '^[a-f0-9]{40}$') { throw 'Invalid pinned build profile.' }
  if($profile.branch -notmatch '^\d+$' -or $profile.cef_version -notmatch '^\d+\.\d+\.\d+\+g[a-f0-9]+\+chromium-\d+\.\d+\.\d+\.\d+$') { throw 'Invalid profile version/branch.' }
  $ExpectedCef=$profile.cef_version
  $ExpectedBranch=$profile.branch
  $ExpectedCheckout=$profile.checkout
  $Branch=$ExpectedBranch; $Checkout=$ExpectedCheckout
  $AutomateUrl="https://raw.githubusercontent.com/chromiumembedded/cef/$Checkout/tools/automate/automate-git.py"
  if($profile.automate_url -ne $AutomateUrl) { throw 'Profile automation URL mismatch.' }
  $AutomateSha=$profile.automate_sha256
}
$Major=$ExpectedCef.Split('.')[0]

if($Branch -ne $ExpectedBranch){throw "Wrong CEF branch: $Branch"}
if($Checkout -ne $ExpectedCheckout){throw "Wrong CEF checkout: $Checkout"}

$env:GN_DEFINES='is_official_build=true is_component_build=false proprietary_codecs=true ffmpeg_branding=Chrome chrome_pgo_phase=0 symbol_level=0 blink_symbol_level=0'
$env:GN_ARGUMENTS='--ide=vs2022 --sln=cef --filters=//cef/*'
New-Item -ItemType Directory -Force -Path $DownloadDir,$OutputDir | Out-Null
$automateDir=Join-Path $DownloadDir 'automate'
$automate=Join-Path $automateDir 'automate-git.py'
if(!(Test-Path $automate)){
  New-Item -ItemType Directory -Force -Path $automateDir | Out-Null
  Invoke-WebRequest -Uri $AutomateUrl -OutFile $automate
}
if($AutomateSha -and (Get-FileHash $automate -Algorithm SHA256).Hash.ToLowerInvariant() -ne $AutomateSha){throw 'Pinned automation script hash mismatch; use a separate clean candidate build directory.'}
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
if(-not $vt.Contains("CEF_VERSION `"$ExpectedCef`"")){throw "CEF version mismatch."}

$marker=Join-Path $dist.FullName 'MAEN_MEDIA_RUNTIME.txt'
@('MaenBrowser CEF media runtime',"cef_version=$ExpectedCef","branch=$ExpectedBranch","checkout=$ExpectedCheckout",'platform=windows64','is_official_build=true','is_component_build=false','proprietary_codecs=true','ffmpeg_branding=Chrome','chrome_pgo_phase=0') |
  Set-Content $marker -Encoding ASCII

$zip=Join-Path $OutputDir "maen-cef-$Major-media-windows64.zip"
Remove-Item $zip -Force -ErrorAction SilentlyContinue
Compress-Archive -Path $dist.FullName -DestinationPath $zip -CompressionLevel Optimal
$sha=(Get-FileHash $zip -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content "$zip.sha256" $sha -NoNewline
@('MaenBrowser CEF media runtime',"cef_line=$ExpectedCef","branch=$ExpectedBranch","checkout=$ExpectedCheckout",'platform=windows64','is_official_build=true','is_component_build=false','proprietary_codecs=true','ffmpeg_branding=Chrome','chrome_pgo_phase=0',"sha256=$sha") |
  Set-Content (Join-Path $OutputDir "maen-cef-$Major-media-build-manifest.txt") -Encoding ASCII
Write-Host "MEDIA_CEF_ARCHIVE=$zip"
Write-Host "MEDIA_CEF_SHA256=$sha"
