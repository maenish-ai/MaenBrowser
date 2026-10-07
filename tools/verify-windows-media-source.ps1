$ErrorActionPreference = "Stop"
$cmake = Get-Content CMakeLists.txt -Raw
$cpp = Get-Content src/media/windows_media_foundation.cpp -Raw
foreach ($n in @('windows_media_foundation.cpp','mfplat','mfuuid')) { if (-not $cmake.Contains($n)) { throw "Missing Windows media build integration: $n" } }
foreach ($n in @('MFStartup','MFTEnumEx','MFVideoFormat_H264','MFAudioFormat_AAC','MFShutdown')) { if (-not $cpp.Contains($n)) { throw "Missing Windows Media Foundation probe: $n" } }
Write-Host "Windows Media Foundation source integration verified." -ForegroundColor Green
