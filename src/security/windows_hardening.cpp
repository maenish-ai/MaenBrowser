#include "src/security/windows_hardening.h"
#include <windows.h>
namespace maenbrowser::security {
bool ApplyWindowsProcessHardening() {
  bool ok = true;
  // DEP is permanent once enabled. This is an OS mitigation, not a scanner.
  PROCESS_MITIGATION_DEP_POLICY dep{};
  dep.Enable = 1;
  dep.Permanent = 1;
  if (!SetProcessMitigationPolicy(ProcessDEPPolicy, &dep, sizeof(dep))) ok = false;

  // Prefer ASLR for images that support relocation. Do not prohibit dynamic
  // code here: Chromium's JIT legitimately requires it.
  PROCESS_MITIGATION_ASLR_POLICY aslr{};
  aslr.EnableForceRelocateImages = 1;
  aslr.DisallowStrippedImages = 0;
  if (!SetProcessMitigationPolicy(ProcessASLRPolicy, &aslr, sizeof(aslr))) ok = false;

  // Disable legacy Win32 extension points in this process where supported.
  PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY ext{};
  ext.DisableExtensionPoints = 1;
  if (!SetProcessMitigationPolicy(ProcessExtensionPointDisablePolicy, &ext, sizeof(ext))) ok = false;
  // Restrict subsequent DLL searches to the executable directory, System32
  // and directories explicitly registered by the app. No current-directory
  // search; keep app-local CEF/codecs available. Does not cover bootstrap DLLs
  // already loaded before this function runs.
  if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS)) ok = false;
  if (!SetDllDirectoryW(L"")) ok = false;
  PROCESS_MITIGATION_IMAGE_LOAD_POLICY images{};
  images.NoRemoteImages = 1;
  images.PreferSystem32Images = 1;
  if (!SetProcessMitigationPolicy(ProcessImageLoadPolicy, &images, sizeof(images))) ok = false;
  return ok;
}
}