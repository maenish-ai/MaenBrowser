param([string]$OutputDir = "C:\maen-cef-output", [string]$ProfilePath = "")
$ErrorActionPreference = "Stop"
$major='154';$branch='8037';$checkout='14c5a08';$cef='154.0.34+g14c5a08+chromium-154.0.8037.98'
if($ProfilePath){$profile=Get-Content -LiteralPath $ProfilePath -Raw -Encoding UTF8 | ConvertFrom-Json;$cef=$profile.cef_version;$major=$cef.Split('.')[0];$branch=$profile.branch;$checkout=$profile.checkout}
$zip = Join-Path $OutputDir "maen-cef-$major-media-windows64.zip"
$shaFile = "$zip.sha256"
$manifest = Join-Path $OutputDir "maen-cef-$major-media-build-manifest.txt"
foreach ($f in @($zip,$shaFile,$manifest)) { if (!(Test-Path $f)) { throw "Missing media build output: $f" } }
$expected=(Get-Content $shaFile -Raw).Trim().ToLowerInvariant()
$actual=(Get-FileHash $zip -Algorithm SHA256).Hash.ToLowerInvariant()
if ($expected -ne $actual) { throw 'CEF media archive SHA256 mismatch.' }
if ($expected -notmatch '^[0-9a-f]{64}$') { throw 'Invalid SHA256 file.' }
$m=Get-Content $manifest -Raw
if ($m -notmatch "(?m)^sha256=$actual\r?$") { throw 'Manifest SHA256 does not match archive.' }
foreach ($needle in @('proprietary_codecs=true','ffmpeg_branding=Chrome','is_official_build=true','chrome_pgo_phase=0',"branch=$branch","checkout=$checkout",'platform=windows64',"cef_line=$cef")) {
  if (-not $m.Contains($needle)) { throw "Media build manifest missing: $needle" }
}
Write-Host "Verified Maen CEF media archive: $actual" -ForegroundColor Green
