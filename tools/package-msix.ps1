#requires -Version 7.2
param(
 [Parameter(Mandatory)][string]$PayloadDirectory,
 [Parameter(Mandatory)][string]$OutputDirectory,
 [string]$IdentityPath='config/store-identity.json',
 [string]$CertificateThumbprint='',
 [string]$EngineReport=''
)
$ErrorActionPreference='Stop'
if($env:OS -ne 'Windows_NT'){throw 'MSIX packaging requires the Windows SDK and a real Windows payload.'}
$root=Split-Path $PSScriptRoot -Parent
$identity=Get-Content -LiteralPath $IdentityPath -Raw -Encoding utf8 | ConvertFrom-Json
if(!$identity.configured -or !$identity.windowsIdentityName -or !$identity.windowsPublisher -or !$identity.windowsPublisherDisplayName){throw 'Publisher-owned Store identity must be configured. No fabricated identity is packaged.'}
if($CertificateThumbprint){
  if(!$EngineReport){throw 'Fresh engine report and completed production acceptance are required before signing.'}
  python "$root/tools/release-readiness.py" --engine-report $EngineReport --require
  if($LASTEXITCODE -ne 0){throw 'Production acceptance gate rejected signing.'}
}
$version=(Get-Content "$root/VERSION" -Raw).Trim()
$payload=[IO.Path]::GetFullPath($PayloadDirectory);$output=[IO.Path]::GetFullPath($OutputDirectory)
if(!(Test-Path "$payload/MaenBrowser.exe")){throw 'Built and validated Windows payload is required.'}
if($output.StartsWith($payload+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase) -or $output -eq $payload){throw 'Output must be outside the payload.'}
$makeappx=Get-Command MakeAppx.exe -ErrorAction Stop
$stage=Join-Path ([IO.Path]::GetTempPath()) ('maen-msix-'+[Guid]::NewGuid())
try{
 New-Item -ItemType Directory -Path $stage,$output -Force | Out-Null
 Copy-Item "$payload/*" $stage -Recurse -Force
 Add-Type -AssemblyName System.Drawing
 $assetDir=Join-Path $stage 'Assets';New-Item -ItemType Directory -Path $assetDir -Force | Out-Null
 $source=[Drawing.Image]::FromFile("$root/assets/companion/icon128.png")
 try{foreach($item in @(@('StoreLogo',50),@('Square150x150Logo',150),@('Square44x44Logo',44))){
  $bitmap=[Drawing.Bitmap]::new([int]$item[1],[int]$item[1]);$graphics=[Drawing.Graphics]::FromImage($bitmap)
  try{$graphics.InterpolationMode=[Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic;$graphics.DrawImage($source,0,0,[int]$item[1],[int]$item[1]);$bitmap.Save((Join-Path $assetDir ($item[0]+'.png')),[Drawing.Imaging.ImageFormat]::Png)}finally{$graphics.Dispose();$bitmap.Dispose()}
 }}finally{$source.Dispose()}
 $xml=Get-Content "$root/installer/msix/AppxManifest.xml.in" -Raw -Encoding utf8
 $values=@{IDENTITY=$identity.windowsIdentityName;PUBLISHER=$identity.windowsPublisher;PUBLISHER_NAME=$identity.windowsPublisherDisplayName;VERSION=$version}
 foreach($key in $values.Keys){$xml=$xml.Replace('@'+$key+'@',[Security.SecurityElement]::Escape([string]$values[$key]))}
 [void][xml]$xml
 Set-Content -LiteralPath "$stage/AppxManifest.xml" -Value $xml -Encoding utf8
 $package=Join-Path $output "MaenBrowser-$version-x64.msix"
 & $makeappx.Source pack /d $stage /p $package /o
 if($LASTEXITCODE -ne 0){throw 'MakeAppx validation/pack failed.'}
 if($CertificateThumbprint){
  if($CertificateThumbprint -notmatch '^[A-Fa-f0-9]{40}$'){throw 'Invalid installed publisher certificate thumbprint.'}
  $cert=Get-Item "Cert:/CurrentUser/My/$CertificateThumbprint" -ErrorAction Stop
  if(!$cert.HasPrivateKey -or $cert.Subject -ne $identity.windowsPublisher){throw 'Signing identity differs from the configured publisher.'}
  $signtool=Get-Command SignTool.exe -ErrorAction Stop
  & $signtool.Source sign /sha1 $CertificateThumbprint /fd SHA256 /tr https://timestamp.digicert.com /td SHA256 $package
  if($LASTEXITCODE -ne 0){throw 'Publisher signing failed.'}
  & $signtool.Source verify /pa /v $package
  if($LASTEXITCODE -ne 0){throw 'Package signature validation failed.'}
 }else{
  $unsigned=Join-Path $output "MaenBrowser-$version-x64-UNSIGNED.msix";Move-Item $package $unsigned -Force;$package=$unsigned
  Write-Warning 'UNSIGNED package: Store submission review artifact only; not a direct-install security update.'
 }
 "$((Get-FileHash $package -Algorithm SHA256).Hash.ToLowerInvariant())  $([IO.Path]::GetFileName($package))" | Set-Content "$package.sha256" -Encoding ascii
 if($CertificateThumbprint -and $identity.packageBaseUrl){
  $base=[Uri]$identity.packageBaseUrl
  if($base.Scheme -ne 'https' -or $base.UserInfo -or $base.Query -or $base.Fragment){throw 'HTTPS publisher package directory URL required.'}
  $url=$identity.packageBaseUrl.TrimEnd('/')+'/'
  $escape={param($s)[Security.SecurityElement]::Escape($s)}
  $appinstaller=@"
<?xml version="1.0" encoding="utf-8"?>
<AppInstaller xmlns="http://schemas.microsoft.com/appx/appinstaller/2018" Version="$version.0" Uri="$(& $escape ($url+'MaenBrowser.appinstaller'))">
<MainPackage Name="$(& $escape $identity.windowsIdentityName)" Publisher="$(& $escape $identity.windowsPublisher)" Version="$version.0" ProcessorArchitecture="x64" Uri="$(& $escape ($url+[IO.Path]::GetFileName($package)))"/>
<UpdateSettings><OnLaunch HoursBetweenUpdateChecks="1" ShowPrompt="true" UpdateBlocksActivation="false"/><AutomaticBackgroundTask/></UpdateSettings>
</AppInstaller>
"@
  Set-Content (Join-Path $output 'MaenBrowser.appinstaller') $appinstaller -Encoding utf8
 }
 Write-Host "Prepared $package"
}catch{if($package -and (Test-Path $package)){Remove-Item $package -Force};throw}finally{if(Test-Path $stage){Remove-Item $stage -Recurse -Force}}
