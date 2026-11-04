param([string]$OutputDir = "C:\maen-cef-output")
$ErrorActionPreference = "Stop"
$zip = Join-Path $OutputDir 'maen-cef-154-media-windows64.zip'
$shaFile = "$zip.sha256"
$manifest = Join-Path $OutputDir 'maen-cef-154-media-build-manifest.txt'
foreach ($f in @($zip,$shaFile,$manifest)) { if (!(Test-Path $f)) { throw "Missing media build output: $f" } }
$expected=(Get-Content $shaFile -Raw).Trim().ToLowerInvariant()
$actual=(Get-FileHash $zip -Algorithm SHA256).Hash.ToLowerInvariant()
if ($expected -ne $actual) { throw 'CEF media archive SHA256 mismatch.' }
if ($expected -notmatch '^[0-9a-f]{64}$') { throw 'Invalid SHA256 file.' }
$m=Get-Content $manifest -Raw
if ($m -notmatch "(?m)^sha256=$actual\r?$") { throw 'Manifest SHA256 does not match archive.' }
foreach ($needle in @('proprietary_codecs=true','ffmpeg_branding=Chrome','is_official_build=true','chrome_pgo_phase=0','branch=8037','checkout=14c5a08','platform=windows64','cef_line=154.0.34+g14c5a08+chromium-154.0.8037.98')) {
  if (-not $m.Contains($needle)) { throw "Media build manifest missing: $needle" }
}
Write-Host "Verified Maen CEF media archive: $actual" -ForegroundColor Green
