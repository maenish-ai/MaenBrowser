@echo off
setlocal
cd /d "%~dp0"
where git >nul 2>nul
if errorlevel 1 (
  echo Git is required. Install Git for Windows and retry.
  pause
  exit /b 1
)
where py >nul 2>nul
if not errorlevel 1 (
  py -3 integrate_maenbrowser.py --clone --output-dir "%~dp0EXPORT"
) else (
  python integrate_maenbrowser.py --clone --output-dir "%~dp0EXPORT"
)
if errorlevel 1 (
  echo Preparation stopped safely. Review the error above.
  pause
  exit /b 1
)
echo.
echo READY: The full Git checkout and source upload files are in EXPORT.
echo Review changes and run tests before committing and pushing.
pause
