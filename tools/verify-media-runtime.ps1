param(
  [Parameter(Mandatory=$true)][string]$CefRoot,
  [switch]$RequireMedia
)
$ErrorActionPreference="Stop"
$runtimeTool = Join-Path $PSScriptRoot "media-runtime.py"
python $runtimeTool --verify $CefRoot
if ($LASTEXITCODE -ne 0) { throw "CEF runtime provenance/layout verification failed." }
