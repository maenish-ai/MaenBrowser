param(
  [string]$DownloadDir = "C:\maen-cef-build",
  [string]$Branch = "7977",
  [switch]$NoUpdate,
  [switch]$NoBuild
)
$ErrorActionPreference = "Stop"
$env:GN_DEFINES = 'is_official_build=true proprietary_codecs=true ffmpeg_branding=Chrome chrome_pgo_phase=0'
$env:GN_ARGUMENTS = '--ide=vs2022 --sln=cef --filters=//cef/*'
Write-Host "MaenBrowser CEF media build"
Write-Host "GN_DEFINES=$env:GN_DEFINES"
Write-Warning "H.264/AAC/MP4 redistribution may require patent/licensing agreements. Do not publish this build until cleared."
$automate = Join-Path $DownloadDir 'automate\automate-git.py'
if (!(Test-Path $automate)) {
  throw "CEF automate-git.py not found at $automate. Follow the official CEF AutomatedBuildSetup first, then rerun this script."
}
$args = @($automate, "--download-dir=$DownloadDir", '--x64-build', "--branch=$Branch", '--minimal-distrib', '--client-distrib', '--force-distrib')
if ($NoUpdate) { $args += '--no-update' }
if ($NoBuild) { $args += '--no-build' }
python @args
if ($LASTEXITCODE -ne 0) { throw "CEF media build failed with exit code $LASTEXITCODE" }
Write-Host "CEF media build completed. Test H.264/AAC playback with cefclient before publishing the archive."
