param(
  [string]$Destination = "third_party/cef",
  [switch]$RequireMedia
)
$ErrorActionPreference = "Stop"
# Media is now mandatory, including when no repository secrets exist.
# Python prints only the selected CEF root on stdout; progress uses stderr.
$runtimeTool = Join-Path $PSScriptRoot "media-runtime.py"
python $runtimeTool --destination $Destination
if ($LASTEXITCODE -ne 0) { throw "Codec-enabled CEF fetch failed; stock fallback is forbidden." }
