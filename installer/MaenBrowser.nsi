!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"

!define PRODUCT_NAME "MaenBrowser"
!define PRODUCT_VERSION "1.9.6"
!define PRODUCT_PUBLISHER "MaenBrowser"
!define PRODUCT_EXE "MaenBrowser.exe"
!define APP_REG_KEY "Software\Clients\StartMenuInternet\MaenBrowser"
!define CAP_KEY "Software\MaenBrowser\Capabilities"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\MaenBrowser"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "MaenBrowser-1.9.6-Setup.exe"
InstallDir "$LOCALAPPDATA\Programs\MaenBrowser"
RequestExecutionLevel user
Unicode True
SetCompressor /SOLID lzma

Icon "..\assets\maenbrowser.ico"
UninstallIcon "..\assets\maenbrowser.ico"

VIProductVersion "1.9.6.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "FileDescription" "MaenBrowser Installer"
VIAddVersionKey "FileVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "LegalCopyright" "MaenBrowser contributors"

!define MUI_ABORTWARNING
!define MUI_ICON "..\assets\maenbrowser.ico"
!define MUI_UNICON "..\assets\maenbrowser.ico"

; Always ask which setup/browser language to use before the wizard starts.
!define MUI_LANGDLL_WINDOWTITLE "MaenBrowser Setup Language / لغة تثبيت MaenBrowser"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${PRODUCT_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "$(LaunchMaenBrowser)"
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Arabic"

LangString LaunchMaenBrowser ${LANG_ENGLISH} "Launch MaenBrowser"
LangString LaunchMaenBrowser ${LANG_ARABIC} "تشغيل MaenBrowser"
LangString ClosingBrowser ${LANG_ENGLISH} "MaenBrowser is running. Setup will close it safely and continue the update."
LangString ClosingBrowser ${LANG_ARABIC} "MaenBrowser قيد التشغيل. سيغلقه برنامج التثبيت بأمان ثم يتابع التحديث."
LangString CouldNotClose ${LANG_ENGLISH} "Setup could not close MaenBrowser automatically. Please close all MaenBrowser windows, then click Retry."
LangString CouldNotClose ${LANG_ARABIC} "تعذر على برنامج التثبيت إغلاق MaenBrowser تلقائياً. أغلق جميع نوافذ MaenBrowser ثم اضغط إعادة المحاولة."
LangString PreservingData ${LANG_ENGLISH} "Updating MaenBrowser while preserving your profile and data..."
LangString PreservingData ${LANG_ARABIC} "جارٍ تحديث MaenBrowser مع الاحتفاظ بملفك الشخصي وبياناتك..."

; Ask the running browser to close through its normal top-level windows. This
; avoids force-killing CEF and gives the profile/session a chance to flush.
Function CloseRunningBrowser
  close_retry:
  nsExec::ExecToStack 'cmd /C tasklist /FI "IMAGENAME eq ${PRODUCT_EXE}" /NH | find /I "${PRODUCT_EXE}" >nul'
  Pop $0
  Pop $1
  ${If} $0 != 0
    Return
  ${EndIf}

  DetailPrint "$(ClosingBrowser)"
  ; CloseMainWindow sends the normal WM_CLOSE path to every top-level browser
  ; process. Child CEF processes normally exit after the browser process exits.
  nsExec::ExecToStack 'powershell.exe -NoLogo -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command "Get-Process -Name MaenBrowser -ErrorAction SilentlyContinue | Where-Object { $$_.MainWindowHandle -ne 0 } | ForEach-Object { [void]$$_.CloseMainWindow() }; for ($$i=0; $$i -lt 40; $$i++) { if (-not (Get-Process -Name MaenBrowser -ErrorAction SilentlyContinue)) { exit 0 }; Start-Sleep -Milliseconds 250 }; exit 1"'
  Pop $0
  Pop $1

  ; Re-check independently. We never use taskkill /F because an update must not
  ; risk corrupting the user's Chromium/CEF profile or unsaved session state.
  nsExec::ExecToStack 'cmd /C tasklist /FI "IMAGENAME eq ${PRODUCT_EXE}" /NH | find /I "${PRODUCT_EXE}" >nul'
  Pop $0
  Pop $1
  ${If} $0 == 0
    MessageBox MB_ICONEXCLAMATION|MB_RETRYCANCEL "$(CouldNotClose)" IDRETRY close_retry IDCANCEL close_cancel
  ${EndIf}
  Return

  close_cancel:
  Abort
FunctionEnd

Function SaveSelectedBrowserLanguage
  ; MaenBrowser already reads this file before CEF initialization, so the
  ; installer language becomes the browser language on first launch.
  CreateDirectory "$LOCALAPPDATA\MaenBrowser"
  FileOpen $0 "$LOCALAPPDATA\MaenBrowser\ui-language.txt" w
  ${If} $LANGUAGE == ${LANG_ARABIC}
    FileWrite $0 "ar"
  ${Else}
    FileWrite $0 "en"
  ${EndIf}
  FileClose $0
FunctionEnd

Function .onInit
  SetShellVarContext current
  ; Show the English/Arabic choice every time Setup is launched. The selected
  ; language controls both this installer UI and MaenBrowser's first launch.
  !insertmacro MUI_LANGDLL_DISPLAY
  Call CloseRunningBrowser
FunctionEnd

Function CleanOldProgramFiles
  ; Clean application/runtime only. Preserve $LOCALAPPDATA\MaenBrowser user profile.
  IfFileExists "$INSTDIR\${PRODUCT_EXE}" 0 no_old_install
  DetailPrint "$(PreservingData)"
  Delete "$DESKTOP\MaenBrowser.lnk"
  RMDir /r "$SMPROGRAMS\MaenBrowser"
  DeleteRegKey HKCU "${UNINSTALL_KEY}"
  DeleteRegKey HKCU "${APP_REG_KEY}"
  DeleteRegKey HKCU "${CAP_KEY}"
  DeleteRegValue HKCU "Software\RegisteredApplications" "${PRODUCT_NAME}"
  DeleteRegKey HKCU "Software\Classes\MaenBrowserURL"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\App Paths\MaenBrowser.exe"
  RMDir /r "$INSTDIR"
  IfFileExists "$INSTDIR\${PRODUCT_EXE}" 0 +3
    MessageBox MB_ICONSTOP|MB_OK "$(CouldNotClose)"
    Abort
  no_old_install:
FunctionEnd

Section "MaenBrowser" SEC_MAIN
  SetShellVarContext current
  Call CleanOldProgramFiles
  SetOutPath "$INSTDIR"
  File /r "..\dist\*.*"
  File /oname=maenbrowser.ico "..\assets\maenbrowser.ico"
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Save the installer's selected language after files are in place and before
  ; the Finish page can launch the browser.
  Call SaveSelectedBrowserLanguage

  ; Standard browsing and media use CEF. The optional compatibility window
  ; uses an existing WebView2 Runtime, with no background bootstrapper install.

  ; Windows shell integration.
  CreateDirectory "$SMPROGRAMS\MaenBrowser"
  CreateShortcut "$SMPROGRAMS\MaenBrowser\MaenBrowser.lnk" "$INSTDIR\${PRODUCT_EXE}" "" "$INSTDIR\maenbrowser.ico" 0
  CreateShortcut "$SMPROGRAMS\MaenBrowser\Uninstall MaenBrowser.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
  CreateShortcut "$DESKTOP\MaenBrowser.lnk" "$INSTDIR\${PRODUCT_EXE}" "" "$INSTDIR\maenbrowser.ico" 0
  ; Stable shell identity used by the running process/taskbar grouping.

  ; Installed Apps / Apps & features.
  WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayIcon" "$INSTDIR\maenbrowser.ico"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKCU "${UNINSTALL_KEY}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
  ; Windows Installed Apps reads EstimatedSize in KiB (REG_DWORD).
  ; Calculate it from the installed files, including the CEF runtime, rather
  ; than reporting the compressed installer download size or a fixed estimate.
  ; FileFunc GetSize returns a size in KiB with /S=0K.
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  ${If} $0 != "error"
    WriteRegDWORD HKCU "${UNINSTALL_KEY}" "EstimatedSize" $0
  ${EndIf}
  WriteRegDWORD HKCU "${UNINSTALL_KEY}" "NoModify" 1
  WriteRegDWORD HKCU "${UNINSTALL_KEY}" "NoRepair" 1

  ; Register as a browser candidate. Windows still requires the user to choose
  ; the default app; the installer never hijacks HTTP/HTTPS associations.
  WriteRegStr HKCU "${APP_REG_KEY}" "" "${PRODUCT_NAME}"
  WriteRegStr HKCU "${APP_REG_KEY}\DefaultIcon" "" "$INSTDIR\maenbrowser.ico"
  WriteRegStr HKCU "${APP_REG_KEY}\shell\open\command" "" '"$INSTDIR\${PRODUCT_EXE}" "%1"'

  WriteRegStr HKCU "${CAP_KEY}" "ApplicationName" "${PRODUCT_NAME}"
  WriteRegStr HKCU "${CAP_KEY}" "ApplicationDescription" "Fast, lightweight Chromium-based web browser"
  WriteRegStr HKCU "${CAP_KEY}" "ApplicationIcon" "$INSTDIR\maenbrowser.ico"
  WriteRegStr HKCU "${CAP_KEY}\URLAssociations" "http" "MaenBrowserURL"
  WriteRegStr HKCU "${CAP_KEY}\URLAssociations" "https" "MaenBrowserURL"
  WriteRegStr HKCU "Software\RegisteredApplications" "${PRODUCT_NAME}" "${CAP_KEY}"

  WriteRegStr HKCU "Software\Classes\MaenBrowserURL" "" "MaenBrowser URL"
  WriteRegStr HKCU "Software\Classes\MaenBrowserURL" "URL Protocol" ""
  WriteRegStr HKCU "Software\Classes\MaenBrowserURL\DefaultIcon" "" "$INSTDIR\maenbrowser.ico"
  WriteRegStr HKCU "Software\Classes\MaenBrowserURL\shell\open\command" "" '"$INSTDIR\${PRODUCT_EXE}" "%1"'

  ; App Paths allows Windows and other software to resolve MaenBrowser.exe.
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\App Paths\MaenBrowser.exe" "" "$INSTDIR\${PRODUCT_EXE}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\App Paths\MaenBrowser.exe" "Path" "$INSTDIR"

  ; Notify Explorer that associations/application registration changed.
  ; Shortcuts point directly at maenbrowser.ico, and this refresh makes the
  ; unified icon visible without relying on the copied CEF bootstrap icon.
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
SectionEnd

Section "Uninstall"
  SetShellVarContext current
  Delete "$DESKTOP\MaenBrowser.lnk"
  RMDir /r "$SMPROGRAMS\MaenBrowser"

  DeleteRegKey HKCU "${UNINSTALL_KEY}"
  DeleteRegKey HKCU "${APP_REG_KEY}"
  DeleteRegKey HKCU "${CAP_KEY}"
  DeleteRegValue HKCU "Software\RegisteredApplications" "${PRODUCT_NAME}"
  DeleteRegKey HKCU "Software\Classes\MaenBrowserURL"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\App Paths\MaenBrowser.exe"

  ; Preserve profile on uninstall, especially when users reinstall to repair.
  ; Profile deletion is an explicit separate user action.
  RMDir /r "$INSTDIR"
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
SectionEnd
