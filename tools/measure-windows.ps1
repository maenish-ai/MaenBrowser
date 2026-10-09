param(
  [ValidateRange(1,3600)][int]$Seconds = 60,
  [ValidateRange(1,30)][int]$IntervalSeconds = 5,
  [string]$OutputPath = "maen-performance.csv"
)
$ErrorActionPreference = "Stop"
# Read-only, opt-in capture. No URLs, keystrokes, registry edits or cache clearing.
$cpuCount = [Environment]::ProcessorCount
$previous = @{}
$rows = [System.Collections.Generic.List[object]]::new()
$clock = [Diagnostics.Stopwatch]::StartNew()
$lastTime = $null
$os = Get-CimInstance Win32_OperatingSystem
$hardware = [pscustomobject]@{
  OS=$os.Caption; LogicalProcessors=$cpuCount
  PhysicalMemoryMB=[math]::Round($os.TotalVisibleMemorySize/1024)
  SampleIntervalSeconds=$IntervalSeconds
  Note="Process IO includes file/network/device IO, not physical disk activity. Working sets include shared pages. Compare identical tabs and workload."
}
do {
  $inventory = @(Get-CimInstance Win32_Process -Property Name,ProcessId,ParentProcessId,CreationDate,KernelModeTime,UserModeTime,ReadTransferCount,WriteTransferCount,WorkingSetSize,PrivatePageCount)
  $ids = [System.Collections.Generic.HashSet[int]]::new()
  foreach ($entry in $inventory) {
    if ($entry.Name -eq "MaenBrowser.exe") { [void]$ids.Add([int]$entry.ProcessId) }
  }
  do {
    $added = $false
    foreach ($entry in $inventory) {
      if ($ids.Contains([int]$entry.ParentProcessId) -and $ids.Add([int]$entry.ProcessId)) { $added = $true }
    }
  } while ($added)
  $now=$clock.Elapsed.TotalSeconds
  $cpu=0.0; $read=0.0; $write=0.0; $working=0.0; $private=0.0
  $next=@{}
  $processes=@($inventory | Where-Object { $ids.Contains([int]$_.ProcessId) })
  foreach ($entry in $processes) {
    $key=[string]$entry.ProcessId + ':' + [string]$entry.CreationDate
    $value=@{Cpu=([double]$entry.KernelModeTime+[double]$entry.UserModeTime)/1e7; Read=[double]$entry.ReadTransferCount; Write=[double]$entry.WriteTransferCount}
    $next[$key]=$value
    if ($previous.ContainsKey($key)) {
      $cpu += [math]::Max(0,$value.Cpu-$previous[$key].Cpu)
      $read += [math]::Max(0,$value.Read-$previous[$key].Read)
      $write += [math]::Max(0,$value.Write-$previous[$key].Write)
    }
    $working += [double]$entry.WorkingSetSize
    $private += [double]$entry.PrivatePageCount
  }
  if ($null -ne $lastTime) {
    $elapsed=$now-$lastTime
    $rows.Add([pscustomobject]@{
      Time=(Get-Date).ToString('o'); Processes=$processes.Count
      CPUPercent=[math]::Round(100*$cpu/$elapsed/$cpuCount,2)
      WorkingSetMB=[math]::Round($working/1MB,2); PrivateMB=[math]::Round($private/1MB,2)
      ReadIOKiBPerSecond=[math]::Round($read/1024/$elapsed,2)
      WriteIOKiBPerSecond=[math]::Round($write/1024/$elapsed,2)
    })
  }
  $previous=$next; $lastTime=$now
  if ($clock.Elapsed.TotalSeconds -ge $Seconds) { break }
  Start-Sleep -Milliseconds ([int](1000*[math]::Min($IntervalSeconds,$Seconds-$clock.Elapsed.TotalSeconds)))
} while ($true)
$rows | Export-Csv -NoTypeInformation -Encoding UTF8 $OutputPath
$hardware | ConvertTo-Json | Set-Content -Encoding UTF8 "$OutputPath.hardware.json"
Write-Host "Saved $OutputPath and $OutputPath.hardware.json. CPU/IO deltas exclude processes born or exited between samples; no speed improvement is implied by this capture."
