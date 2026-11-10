#requires -Version 7.2
param(
  [string]$ExecutablePath = "$env:LOCALAPPDATA\Programs\MaenBrowser\MaenBrowser.exe",
  [string]$ProfilePath = "$env:LOCALAPPDATA\MaenBrowser\Agent User Data",
  [ValidatePattern('^[A-Za-z0-9][A-Za-z0-9_-]{0,47}$')][string]$SessionName = 'default',
  [ValidateRange(0,65535)][int]$Port = 0
)
$ErrorActionPreference='Stop'
# Keep the legacy default endpoint stable; named tool sessions receive their own port.
if ($Port -eq 0 -and $SessionName -eq 'default') { $Port=9223 }
# Each named session gets a durable, isolated cookie/storage profile.
if ($SessionName -ne 'default') { $ProfilePath=Join-Path $ProfilePath $SessionName }
$ExecutablePath=[IO.Path]::GetFullPath($ExecutablePath)
$ProfilePath=[IO.Path]::GetFullPath($ProfilePath)
if (!(Test-Path -LiteralPath $ExecutablePath -PathType Leaf)) { throw 'Install MaenBrowser 1.10.0 or specify its executable.' }
$markerPath=Join-Path (Split-Path $ExecutablePath) 'maen-agent-capabilities.json'
$capabilities=Get-Content -LiteralPath $markerPath -Raw -Encoding UTF8 | ConvertFrom-Json
if ($capabilities.product -ne 'MaenBrowser' -or [version]$capabilities.version -lt [version]'1.10.0' -or
    $capabilities.schema -ne 1 -or $capabilities.adapter -ne 'loopback-cdp-v1' -or
    -not $capabilities.isolatedProfile -or -not $capabilities.familyGuard -or -not $capabilities.readOnlySettings) {
  throw 'This installation does not declare the required isolated agent support.'
}
if ($Port -eq 0) {
  # Let the OS choose a currently free loopback port for concurrently running tools.
  $listener=[Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback,0)
  try { $listener.Start(); $Port=[int]$listener.LocalEndpoint.Port } finally { $listener.Stop() }
} else {
  $listener=[Net.Sockets.TcpListener]::new([Net.IPAddress]::Loopback,$Port)
  try { $listener.Start() } finally { $listener.Stop() }
}
$null=New-Item -ItemType Directory -Force -Path $ProfilePath
$start=[Diagnostics.ProcessStartInfo]::new()
$start.FileName=$ExecutablePath
$start.WorkingDirectory=Split-Path $ExecutablePath
$start.UseShellExecute=$false
foreach ($argument in @('--maen-agent-mode',"--user-data-dir=$ProfilePath",'--remote-debugging-address=127.0.0.1',"--remote-debugging-port=$Port",'--no-first-run','--no-default-browser-check')) {
  [void]$start.ArgumentList.Add($argument)
}
$process=[Diagnostics.Process]::Start($start)
$deadline=[DateTime]::UtcNow.AddSeconds(30)
$endpoint="http://127.0.0.1:$Port"
while ([DateTime]::UtcNow -lt $deadline) {
  if ($process.HasExited) { throw 'MaenBrowser exited. Close another manual window using this agent profile and check Family Protection.' }
  try {
    $targets=@(Invoke-RestMethod -Uri "$endpoint/json/list" -TimeoutSec 1 -MaximumRedirection 0)
  } catch { $targets=@() }
  if (@($targets | Where-Object { $_.type -eq 'page' }).Count -gt 0) {
    try {
      $owned=Get-CimInstance -ClassName Win32_Process -Filter "ProcessId = $($process.Id)"
      $ownedCommand=[string]$owned.CommandLine
      $isOwned=$ownedCommand -and
        $ownedCommand.IndexOf("--remote-debugging-port=$Port",[StringComparison]::OrdinalIgnoreCase) -ge 0 -and
        $ownedCommand.IndexOf("--user-data-dir=$ProfilePath",[StringComparison]::OrdinalIgnoreCase) -ge 0
    } catch {
      if (!$process.HasExited) { $process.Kill() }
      throw
    }
    if (!$isOwned) {
      if (!$process.HasExited) { $process.Kill() }
      throw 'The selected CDP port did not resolve to the requested MaenBrowser process and isolated profile.'
    }
    [pscustomobject]@{cdp=$endpoint; processId=$process.Id; sessionName=$SessionName; profilePath=$ProfilePath; isolatedProfile=$true} | ConvertTo-Json
    return
  }
  Start-Sleep -Milliseconds 150
}
if (!$process.HasExited) { $process.Kill() }
throw 'The owned agent browser did not become ready. Family Protection must be off and the profile must be free.'
