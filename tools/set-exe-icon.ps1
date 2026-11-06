param(
  [Parameter(Mandatory=$true)][string]$ExePath,
  [Parameter(Mandatory=$true)][string]$IconPath,
  [string]$ResourceDll = ""
)
$ErrorActionPreference="Stop"
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public static class MaenIconNative {
 [DllImport("version.dll", SetLastError=true, CharSet=CharSet.Unicode)]
 public static extern uint GetFileVersionInfoSize(string file, out uint handle);
 [DllImport("version.dll", SetLastError=true, CharSet=CharSet.Unicode)]
 public static extern bool GetFileVersionInfo(string file, uint handle, uint length, byte[] data);
 [DllImport("kernel32.dll", SetLastError=true, CharSet=CharSet.Unicode)]
 public static extern IntPtr BeginUpdateResource(string pFileName, bool bDeleteExistingResources);
 [DllImport("kernel32.dll", SetLastError=true)]
 public static extern bool UpdateResource(IntPtr hUpdate, IntPtr lpType, IntPtr lpName, ushort wLanguage, byte[] lpData, uint cbData);
 [DllImport("kernel32.dll", SetLastError=true)]
 public static extern bool EndUpdateResource(IntPtr hUpdate, bool fDiscard);
}
'@
$exe=(Resolve-Path $ExePath).Path
$ico=(Resolve-Path $IconPath).Path
$bytes=[IO.File]::ReadAllBytes($ico)
if ([BitConverter]::ToUInt16($bytes,0)-ne 0 -or [BitConverter]::ToUInt16($bytes,2)-ne 1) { throw "Invalid ICO header." }
$count=[BitConverter]::ToUInt16($bytes,4)
if ($count -lt 1) { throw "ICO has no images." }
$entries=New-Object System.Collections.Generic.List[byte[]]
$group=New-Object System.Collections.Generic.List[byte]
$group.AddRange([BitConverter]::GetBytes([UInt16]0))
$group.AddRange([BitConverter]::GetBytes([UInt16]1))
$group.AddRange([BitConverter]::GetBytes([UInt16]$count))
for($i=0;$i -lt $count;$i++){
 $o=6+16*$i
 $w=$bytes[$o];$h=$bytes[$o+1];$colors=$bytes[$o+2];$reserved=$bytes[$o+3]
 $planes=[BitConverter]::ToUInt16($bytes,$o+4);$bpp=[BitConverter]::ToUInt16($bytes,$o+6)
 $size=[BitConverter]::ToUInt32($bytes,$o+8);$offset=[BitConverter]::ToUInt32($bytes,$o+12)
 if($offset+$size -gt $bytes.Length){throw "ICO image range invalid."}
 $img=New-Object byte[] $size
 [Array]::Copy($bytes,[int]$offset,$img,0,[int]$size)
 $entries.Add($img)
 $group.Add($w);$group.Add($h);$group.Add($colors);$group.Add($reserved)
 $group.AddRange([BitConverter]::GetBytes($planes));$group.AddRange([BitConverter]::GetBytes($bpp))
 $group.AddRange([BitConverter]::GetBytes([UInt32]$size));$group.AddRange([BitConverter]::GetBytes([UInt16]($i+1)))
}
$versionBytes=$null
if($ResourceDll){
 $dll=(Resolve-Path $ResourceDll).Path
 [uint32]$ignored=0;$length=[MaenIconNative]::GetFileVersionInfoSize($dll,[ref]$ignored)
 if(!$length -or $length -gt 65536){throw "Version resource missing or exceeds bounded size."}
 $versionBytes=New-Object byte[] $length
 if(-not [MaenIconNative]::GetFileVersionInfo($dll,0,$length,$versionBytes)){throw "Could not read application version resource."}
}
$hUpdate=[MaenIconNative]::BeginUpdateResource($exe,$false)
if($hUpdate -eq [IntPtr]::Zero){throw "BeginUpdateResource failed: $([Runtime.InteropServices.Marshal]::GetLastWin32Error())"}
$ok=$true
try{
 for($i=0;$i -lt $entries.Count;$i++){
  if(-not [MaenIconNative]::UpdateResource($hUpdate,[IntPtr]3,[IntPtr]($i+1),0,$entries[$i],[uint32]$entries[$i].Length)){ $ok=$false; break }
 }
 if($ok){
  $g=$group.ToArray()
  $ok=[MaenIconNative]::UpdateResource($hUpdate,[IntPtr]14,[IntPtr]1,0,$g,[uint32]$g.Length)
 }
 if($ok -and $versionBytes){
  foreach($language in @(0,1033)){
   if(-not [MaenIconNative]::UpdateResource($hUpdate,[IntPtr]16,[IntPtr]1,[uint16]$language,$versionBytes,[uint32]$versionBytes.Length)){$ok=$false;break}
  }
 }
} finally {
 if(-not [MaenIconNative]::EndUpdateResource($hUpdate,-not $ok)){throw "EndUpdateResource failed."}
}
if(-not $ok){throw "Failed to stamp MaenBrowser icon resources."}
Write-Host "Stamped MaenBrowser identity resources into $exe" -ForegroundColor Green

if($ResourceDll){
 $sourceVersion=[Diagnostics.FileVersionInfo]::GetVersionInfo((Resolve-Path $ResourceDll).Path).FileVersion
 $exeVersion=[Diagnostics.FileVersionInfo]::GetVersionInfo($exe).FileVersion
 if($sourceVersion -ne $exeVersion){throw "Bootstrap version stamp differs from the built application module."}
}
