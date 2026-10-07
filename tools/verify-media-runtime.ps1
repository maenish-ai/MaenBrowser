param(
  [Parameter(Mandatory=$true)][string]$CefRoot,
  [switch]$RequireMedia
)
$ErrorActionPreference="Stop"
$root=[System.IO.Path]::GetFullPath($CefRoot)
foreach($relative in @("Release\libcef.dll","include\cef_version.h","cmake\FindCEF.cmake")){
  $p=Join-Path $root $relative
  if(!(Test-Path $p)){throw "CEF runtime integrity failure: missing $relative"}
}
$version=Get-Content (Join-Path $root "include\cef_version.h") -Raw
if(-not $version.Contains('CEF_VERSION "152.0.6+g708dc14+chromium-152.0.7977.83"')){
  throw "CEF runtime version mismatch."
}
if($RequireMedia){
  $marker=Join-Path $root "MAEN_MEDIA_RUNTIME.txt"
  if(!(Test-Path $marker)){throw "Verified media marker missing. Stock CEF is forbidden for Media Setup."}
  $m=Get-Content $marker -Raw
  foreach($needle in @(
    "cef_version=152.0.6+g708dc14+chromium-152.0.7977.83",
    "branch=7977","checkout=708dc14","platform=windows64",
    "proprietary_codecs=true","ffmpeg_branding=Chrome","is_official_build=true"
  )){
    if(-not $m.Contains($needle)){throw "Media runtime marker missing: $needle"}
  }
}
Write-Host "CEF 152 runtime integrity gate passed." -ForegroundColor Green
if($RequireMedia){Write-Host "Verified Maen media-build marker passed. Real WhatsApp playback remains the final runtime acceptance test." -ForegroundColor Green}
