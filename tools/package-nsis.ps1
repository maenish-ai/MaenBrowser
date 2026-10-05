param(
  [string]$Makensis = "makensis.exe"
)
$ErrorActionPreference = "Stop"
& $Makensis "installer\MaenBrowser.nsi"
