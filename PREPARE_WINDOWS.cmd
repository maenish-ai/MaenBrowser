@echo off
setlocal
cd /d "%~dp0"
where py >nul 2>nul
if errorlevel 1 (
  echo Python launcher "py" is required.
  exit /b 1
)
py -3 apply_release.py --clone --output "%~dp0MaenBrowser-1.9.7-EXPORT"
if errorlevel 1 (
  echo Patch export failed. Read the error above.
  exit /b 1
)
echo Done. Open MaenBrowser-1.9.7-EXPORT.
pause
