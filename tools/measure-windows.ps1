param([ValidateRange(1,3600)][int]$Seconds = 60, [string]$OutputPath = "maen-performance.csv")
$ErrorActionPreference = "Stop"
$rows = @()
for ($i=0; $i -lt $Seconds; $i++) {
  $now = Get-Date
  $inventory = @(Get-CimInstance Win32_Process)
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
  $processes = @(Get-Process -ErrorAction SilentlyContinue | Where-Object { $ids.Contains($_.Id) })
  $working = ($processes | Measure-Object WorkingSet64 -Sum).Sum
  $private = ($processes | Measure-Object PrivateMemorySize64 -Sum).Sum
  $rows += [pscustomobject]@{
    Time=$now.ToString("o"); Processes=$processes.Count
    WorkingSetMB=[Math]::Round($working/1MB,2); PrivateMB=[Math]::Round($private/1MB,2)
  }
  Start-Sleep -Seconds 1
}
$rows | Export-Csv -NoTypeInformation -Encoding UTF8 $OutputPath
Write-Host "Saved $OutputPath. Includes MaenBrowser process descendants. Working-set sums include shared pages; private bytes are not RAM residency. Record URLs, hardware, engine version and idle duration."
