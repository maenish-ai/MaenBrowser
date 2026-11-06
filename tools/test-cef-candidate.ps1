#requires -Version 7.2
param(
 [Parameter(Mandatory)][string]$OutputDir,
 [Parameter(Mandatory)][string]$WebView2SdkRoot,
 [string]$ProfilePath='config/cef-upgrade-candidate.json',
 [string]$BuildDir='build/candidate'
)
$ErrorActionPreference='Stop'
if($env:OS -ne 'Windows_NT'){throw 'Actual Windows candidate build required.'}
$root=Split-Path $PSScriptRoot -Parent
& "$PSScriptRoot/verify-cef-media-build.ps1" -OutputDir $OutputDir -ProfilePath $ProfilePath
$profile=Get-Content $ProfilePath -Raw -Encoding utf8 | ConvertFrom-Json
$major=$profile.cef_version.Split('.')[0];$archive=Join-Path $OutputDir "maen-cef-$major-media-windows64.zip"
$sha=(Get-FileHash $archive -Algorithm SHA256).Hash.ToLowerInvariant()
$identity="https://local-build.invalid/maen-cef-$major-media-windows64.zip"
$lock=[ordered]@{schema=1;provider='local-candidate-not-distributable';release='unpublished';cef_version=$profile.cef_version;branch=$profile.branch;checkout=$profile.checkout;platform='windows64';archive_name=[IO.Path]::GetFileName($archive);url=$identity;sha256=$sha;size_bytes=(Get-Item $archive).Length;codec_build_flags=@('is_official_build=true','proprietary_codecs=true','ffmpeg_branding=Chrome')}
$lockPath=Join-Path ([IO.Path]::GetFullPath($OutputDir)) 'candidate-runtime.lock.json'
$lock | ConvertTo-Json -Depth 4 | Set-Content $lockPath -Encoding utf8NoBOM
# Scoped process environment; restoring the production selection is mandatory.
$priorUrl=$env:MAEN_CEF_ARCHIVE_URL;$priorSha=$env:MAEN_CEF_ARCHIVE_SHA256
try{
 $env:MAEN_CEF_ARCHIVE_URL=$identity;$env:MAEN_CEF_ARCHIVE_SHA256=$sha
 $cef=& python "$PSScriptRoot/media-runtime.py" --lock $lockPath --archive $archive --destination (Join-Path $OutputDir 'verified-runtime')
 if($LASTEXITCODE -ne 0){throw 'Candidate archive verification failed.'}
 cmake -S $root -B $BuildDir -G 'Visual Studio 17 2022' -A x64 "-DCEF_ROOT=$cef" "-DWEBVIEW2_SDK_ROOT=$WebView2SdkRoot" "-DMAEN_RUNTIME_LOCK_FILE=$lockPath" -DMAEN_ENABLE_UPDATER=OFF -DUSE_SANDBOX=ON
 if($LASTEXITCODE -ne 0){throw 'Candidate configure/API compatibility failed.'}
 cmake --build $BuildDir --config Release --parallel
 if($LASTEXITCODE -ne 0){throw 'Candidate C++ build failed.'}
 & "$PSScriptRoot/set-exe-icon.ps1" -ExePath "$BuildDir/Release/MaenBrowser.exe" -IconPath "$root/assets/maenbrowser.ico" -ResourceDll "$BuildDir/Release/MaenBrowser.dll"
 ctest --test-dir $BuildDir -C Release --output-on-failure
 if($LASTEXITCODE -ne 0){throw 'Candidate native policy tests failed.'}
 node "$root/tests/browser_smoke.mjs" "$BuildDir/Release/MaenBrowser.exe"
 if($LASTEXITCODE -ne 0){throw 'Candidate real codec/download/protection runtime tests failed.'}
 @{schema=1;cef_version=$profile.cef_version;archive_sha256=$sha;testedUtc=[DateTime]::UtcNow.ToString('o');nativeTestsPassed=$true;runtimeSmokePassed=$true;distributionApproved=$false} | ConvertTo-Json | Set-Content (Join-Path $OutputDir 'candidate-test-result.json') -Encoding utf8
}finally{$env:MAEN_CEF_ARCHIVE_URL=$priorUrl;$env:MAEN_CEF_ARCHIVE_SHA256=$priorSha}
