!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"

!define PRODUCT_NAME "MaenBrowser"
!define PRODUCT_VERSION "1.5.27"
!define PRODUCT_PUBLISHER "MaenBrowser"
!define PRODUCT_EXE "MaenBrowser.exe"
!define APP_REG_KEY "Software\Clients\StartMenuInternet\MaenBrowser"
!define CAP_KEY "Software\MaenBrowser\Capabilities"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\MaenBrowser"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "MaenBrowser-1.5.27-Setup.exe"
InstallDir "$LOCALAPPDATA\Programs\MaenBrowser"
RequestExecutionLevel user
Unicode True
SetCompressor /SOLID lzma

Icon "..\assets\maenbrowser.ico"
UninstallIcon "..\assets\maenbrowser.ico"

VIProductVersion "1.5.27.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "FileDescription" "MaenBrowser Installer"
VIAddVersionKey "FileVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "LegalCopyright" "MaenBrowser contributors"

!define MUI_ABORTWARNING
!define MUI_ICON "..\assets\maenbrowser.ico"
!define MUI_UNICON "..\assets\maenbrowser.ico"
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${PRODUCT_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Launch MaenBrowser"
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

Function .onInit
  SetShellVarContext current
  nsExec::ExecToStack 'cmd /C tasklist /FI "IMAGENAME eq ${PRODUCT_EXE}" /NH | find /I "${PRODUCT_EXE}" >nul'
  Pop $0
  Pop $1
  ${If} $0 == 0
    MessageBox MB_ICONEXCLAMATION|MB_OK "MaenBrowser is currently running.$\r$\nClose all MaenBrowser windows, then run Setup again."
    Abort
  ${EndIf}
FunctionEnd

Function CleanOldProgramFiles
  ; Clean application/runtime only. Preserve $LOCALAPPDATA\MaenBrowser user profile.
  IfFileExists "$INSTDIR\${PRODUCT_EXE}" 0 no_old_install
  DetailPrint "Removing previous MaenBrowser program files while preserving user data..."
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
    MessageBox MB_ICONSTOP|MB_OK "The previous MaenBrowser installation could not be removed completely. Close MaenBrowser and try Setup again."
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

  ; Full uninstall removes MaenBrowser profile/cache/site data.
  RMDir /r "$LOCALAPPDATA\MaenBrowser"
  RMDir /r "$INSTDIR"
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'
SectionEnd
