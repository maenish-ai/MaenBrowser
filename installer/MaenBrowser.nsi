!include "MUI2.nsh"

!define PRODUCT_NAME "MaenBrowser"
!define PRODUCT_VERSION "1.1.0"
!define PRODUCT_PUBLISHER "MaenBrowser"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\MaenBrowser"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "MaenBrowser-1.1.0-Setup.exe"
InstallDir "$LOCALAPPDATA\Programs\MaenBrowser"
RequestExecutionLevel user
Unicode True

VIProductVersion "1.1.0.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "ProductVersion" "${PRODUCT_VERSION}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "FileDescription" "MaenBrowser Installer"
VIAddVersionKey "FileVersion" "${PRODUCT_VERSION}"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "Install"
  SetShellVarContext current
  SetOutPath "$INSTDIR"

  File /r "..\dist\*.*"

  WriteUninstaller "$INSTDIR\Uninstall.exe"

  CreateDirectory "$SMPROGRAMS\MaenBrowser"
  CreateShortcut "$SMPROGRAMS\MaenBrowser\MaenBrowser.lnk" "$INSTDIR\MaenBrowser.exe"
  CreateShortcut "$DESKTOP\MaenBrowser.lnk" "$INSTDIR\MaenBrowser.exe"

  WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayIcon" "$INSTDIR\MaenBrowser.exe"
  WriteRegStr HKCU "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKCU "${UNINSTALL_KEY}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
  WriteRegDWORD HKCU "${UNINSTALL_KEY}" "NoModify" 1
  WriteRegDWORD HKCU "${UNINSTALL_KEY}" "NoRepair" 1
SectionEnd

Section "Uninstall"
  SetShellVarContext current

  Delete "$DESKTOP\MaenBrowser.lnk"
  RMDir /r "$SMPROGRAMS\MaenBrowser"

  DeleteRegKey HKCU "${UNINSTALL_KEY}"

  ; Full uninstall: remove local browser profile, cache, saved site data,
  ; local preferences, and user-installed extensions belonging to MaenBrowser.
  RMDir /r "$LOCALAPPDATA\MaenBrowser"

  RMDir /r "$INSTDIR"
SectionEnd
