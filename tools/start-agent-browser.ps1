#requires -Version 7.2
param(
  [string]$ExecutablePath = "$env:LOCALAPPDATA\Programs\MaenBrowser\MaenBrowser.exe",
  [string]$ProfilePath = "$env:LOCALAPPDATA\MaenBrowser\Agent User Data",
  [ValidatePattern('^[A-Za-z0-9][A-Za-z0-9_-]{0,47}$')][string]$SessionName = 'default',
  [ValidateRange(1024,65535)][int]$Port = 9223
)
$ErrorActionPreference='Stop'
# A separate durable cookie profile per named agent session.
if ($SessionName -ne 'default') { $ProfilePath=Join-Path $ProfilePath $SessionName }
$ExecutablePath=[IO.Path]::GetFullPath($ExecutablePath)
if (!(Test-Path -LiteralPath $ExecutablePath -PathType Leaf)) { throw 'Install MaenBrowser 1.10.0 or specify its executable.' }
$markerPath=Join-Path (Split-Path $ExecutablePath) 'maen-agent-capabilities.json'
$capabilities=Get-Content -LiteralPath $markerPath -Raw -Encoding UTF8 | ConvertFrom-Json
if ($capabilities.product -ne 'MaenBrowser' -or [version]$capabilities.version -lt [version]'1.10.0' -or
    $capabilities.schema -ne 1 -or $capabilities.adapter -ne 'loopback-cdp-v1' -or
    -not $capabilities.isolatedProfile -or -not $capabilities.familyGuard -or -not $capabilities.readOnlySettings) {
  throw 'This installation does not declare the required isolated agent support.'
}
$listener=[Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback,$Port)
try { $listener.Start() } finally { $listener.Stop() }
$start=[Diagnostics.ProcessStartInfo]::new()
$start.FileName=$ExecutablePath
$start.WorkingDirectory=Split-Path $ExecutablePath
$start.UseShellExecute=$false
foreach ($argument in @('--maen-agent-mode',"--user-data-dir=$([IO.Path]::GetFullPath($ProfilePath))",'--remote-debugging-address=127.0.0.1',"--remote-debugging-port=$Port",'--no-first-run','--no-default-browser-check')) {
  [void]$start.ArgumentList.Add($argument)
}
$process=[Diagnostics.Process]::Start($start)
$deadline=[DateTime]::UtcNow.AddSeconds(30)
$endpoint="http://127.0.0.1:$Port"
while ([DateTime]::UtcNow -lt $deadline) {
  if ($process.HasExited) { throw 'MaenBrowser exited. Close another manual window using this agent profile and check Family Protection.' }
  try {
    $targets=@(Invoke-RestMethod -Uri "$endpoint/json/list" -TimeoutSec 1 -MaximumRedirection 0)
    if (@($targets | Where-Object { $_.type -eq 'page' }).Count -gt 0) {
      [pscustomobject]@{cdp=$endpoint; processId=$process.Id; isolatedProfile=$true} | ConvertTo-Json
      return
    }
  } catch { }
  Start-Sleep -Milliseconds 150
}
if (!$process.HasExited) { $process.Kill() }
throw 'The owned agent browser did not become ready. Family Protection must be off and the profile must be free.'
