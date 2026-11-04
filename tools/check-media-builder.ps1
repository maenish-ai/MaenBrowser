param([string]$BuildRoot = 'C:\maen-cef-build')
$ErrorActionPreference='Stop'
if (-not [Environment]::Is64BitOperatingSystem -or $env:OS -ne 'Windows_NT') { throw 'Windows x64 is required.' }
foreach ($tool in @('git','python','cmake')) {
  if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) { throw "Install required tool: $tool" }
}
$vswhere="${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (!(Test-Path $vswhere)) { throw 'Install Visual Studio 2022 with Desktop development with C++, ATL/MFC and a compatible Windows SDK.' }
$vs=& $vswhere -latest -version '[17.0,18.0)' -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vs) { throw 'Visual Studio 2022 C++ x64 toolchain missing.' }
$memory=(Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory/1GB
if ($memory -lt 32) { Write-Warning '32-64 GB RAM recommended for Chromium builds; this is not a browser runtime requirement.' }
New-Item -ItemType Directory -Force -Path $BuildRoot | Out-Null
$drive=Get-PSDrive -Name ([IO.Path]::GetPathRoot((Resolve-Path $BuildRoot).Path).Substring(0,1))
if ($drive.Free -lt 300GB) { throw 'Provide at least 300 GB free SSD space for the CEF build; 500 GB recommended.' }
Write-Host "Builder prerequisite check passed. VS: $vs. Complete Chromium prerequisites still apply."
