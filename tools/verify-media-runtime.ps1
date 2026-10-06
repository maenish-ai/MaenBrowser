param(
  [Parameter(Mandatory=$true)][string]$CefRoot
)
$ErrorActionPreference = "Stop"
$root = [System.IO.Path]::GetFullPath($CefRoot)
$libcef = Join-Path $root "Release\libcef.dll"
if (!(Test-Path $libcef)) { throw "CEF media gate: libcef.dll not found: $libcef" }
$ffmpeg = Join-Path $root "Release\ffmpeg.dll"
Write-Host "CEF media runtime gate"
Write-Host "CEF root: $root"
Write-Host "libcef.dll: present"
if (Test-Path $ffmpeg) { Write-Host "ffmpeg.dll: present" } else { Write-Host "ffmpeg.dll: integrated/not separately shipped" }
Write-Host "Required browser capability targets: MP4/H.264, AAC, WebM/VP8/VP9, AV1, Opus, Vorbis, FLAC, MP3, MSE, WebRTC, WebCodecs."
Write-Host "NOTE: this gate verifies runtime integrity, not codec licensing or decoded playback. Playback is a release test."
