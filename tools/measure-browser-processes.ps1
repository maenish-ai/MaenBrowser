#requires -Version 7.2
param(
 [Parameter(Mandatory)][string]$ExecutablePath,
 [Parameter(Mandatory)][string]$OutputPath,
 [ValidateRange(0,2147483647)][int]$RootProcessId=0,
 [ValidateSet(1,5,10,20)][int]$DeclaredTabs=1,
 [ValidateSet('idle','chat','video','switching','startup')][string]$Scenario='idle',
 [ValidateRange(10,3600)][int]$DurationSeconds=60,
 [ValidateRange(1,30)][int]$SampleSeconds=2
)
$ErrorActionPreference='Stop'
if($env:OS -ne 'Windows_NT'){throw 'Run on an actual Windows device; no measurements are fabricated on other systems.'}
$exe=[IO.Path]::GetFullPath($ExecutablePath)
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Browser executable missing.'}
$system=Get-CimInstance Win32_ComputerSystem
$os=Get-CimInstance Win32_OperatingSystem
$cpu=Get-CimInstance Win32_Processor | Select-Object -First 1
$logical=[Math]::Max(1,[int]$system.NumberOfLogicalProcessors)
$history=@{};$samples=[Collections.Generic.List[object]]::new()
$timer=[Diagnostics.Stopwatch]::StartNew();$previousSeconds=0.0
while($timer.Elapsed.TotalSeconds -lt $DurationSeconds){
 # Never export command lines, URLs, profile names, cookies or credentials.
 $snapshot=@(Get-CimInstance Win32_Process -Property ProcessId,ParentProcessId,ExecutablePath,CreationDate)
 $included=[Collections.Generic.HashSet[int]]::new()
 foreach($p in $snapshot){if(($RootProcessId -eq 0 -or $p.ProcessId -eq $RootProcessId) -and $p.ExecutablePath -and $p.ExecutablePath.Equals($exe,[StringComparison]::OrdinalIgnoreCase)){[void]$included.Add([int]$p.ProcessId)}}
 if(!$included.Count){throw 'No process from the specified installation is running.'}
 do{$changed=$false;foreach($p in $snapshot){if($included.Contains([int]$p.ParentProcessId) -and $included.Add([int]$p.ProcessId)){$changed=$true}}}while($changed)
 $privateBytes=0L;$workingSet=0L;$cpuDelta=0.0;$observed=0;$lost=0;$next=@{}
 foreach($p in $snapshot){
  if(!$included.Contains([int]$p.ProcessId)){continue}
  try{
   $process=Get-Process -Id $p.ProcessId -ErrorAction Stop
   $identity="$($p.ProcessId)/$($p.CreationDate.ToUniversalTime().Ticks)"
   $seconds=$process.TotalProcessorTime.TotalSeconds
   if($history.ContainsKey($identity)){$cpuDelta+=[Math]::Max(0,$seconds-$history[$identity])}
   $next[$identity]=$seconds;$privateBytes+=$process.PrivateMemorySize64;$workingSet+=$process.WorkingSet64;$observed++
  }catch{$lost++}
 }
 $now=$timer.Elapsed.TotalSeconds;$elapsed=$now-$previousSeconds
 $usage=if($samples.Count -and $elapsed -gt 0){100*$cpuDelta/$elapsed/$logical}else{$null}
 $samples.Add([pscustomobject]@{elapsedSeconds=[Math]::Round($now,3);processes=$observed;lostDuringSample=$lost;privateCommitMiB=[Math]::Round($privateBytes/1MB,2);summedWorkingSetMiB=[Math]::Round($workingSet/1MB,2);cpuPercentAllLogicalProcessors=$usage})
 $history=$next;$previousSeconds=$now
 Start-Sleep -Seconds $SampleSeconds
}
$result=[ordered]@{schema=1;capturedUtc=[DateTime]::UtcNow.ToString('o');os=$os.Caption;osBuild=$os.BuildNumber;physicalMemoryMiB=[Math]::Round($system.TotalPhysicalMemory/1MB);cpu=$cpu.Name;logicalProcessors=$logical;browserVersion=(Get-Item -LiteralPath $exe).VersionInfo.FileVersion;rootProcessId=$RootProcessId;declaredTabs=$DeclaredTabs;tabCountVerified=$false;scenario=$Scenario;scope='Matching installation roots (or selected RootProcessId) and observed descendants, including renderers/GPU/utility/WebView2';limitations=@('Summed working sets double-count shared pages; private commit is not physical resident RAM.','First sample has no CPU interval; terminated/unobserved/new process CPU may be missed.','Declared tab count is supplied by the operator, not read from page contents.','Compare identical devices/OS/browser versions/workloads and report failures as well as averages.');samples=$samples}
$dest=[IO.Path]::GetFullPath($OutputPath)
[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($dest)) | Out-Null
$result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $dest -Encoding utf8
"Measurement written: $dest"
