#include <windows.h>

#include "include/cef_app.h"
#include "include/cef_sandbox_win.h"
#include "include/cef_version_info.h"

#include "src/app/maen_app.h"
#include "src/storage/local_profile.h"
#include "src/updater/update_manager.h"

namespace {

int RunMain(HINSTANCE instance, void* sandbox_info) {
  CefMainArgs main_args(instance);

  CefRefPtr<maenbrowser::MaenApp> app(new maenbrowser::MaenApp());

  const int subprocess_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (subprocess_code >= 0) {
    return subprocess_code;
  }

  CefSettings settings;
  settings.persist_session_cookies = true;
  settings.log_severity = LOGSEVERITY_WARNING;

  CefString(&settings.root_cache_path) =
      maenbrowser::storage::GetProfileRoot();

  // Chrome runtime uses its own "Default" profile below root_cache_path.
  CefString(&settings.cache_path) =
      maenbrowser::storage::GetProfileRoot();

  if (!sandbox_info) {
    settings.no_sandbox = true;
  }

  if (!CefInitialize(main_args, settings, app, sandbox_info)) {
    MessageBoxW(nullptr,
                L"MaenBrowser could not initialize Chromium.",
                L"MaenBrowser",
                MB_ICONERROR | MB_OK);
    return CefGetExitCode();
  }

  maenbrowser::updater::UpdateManager::Initialize();
  CefRunMessageLoop();
  maenbrowser::updater::UpdateManager::Shutdown();
  CefShutdown();
  return 0;
}

}  // namespace

#if defined(CEF_USE_BOOTSTRAP)

CEF_BOOTSTRAP_EXPORT int RunWinMain(HINSTANCE hInstance,
                                    LPWSTR,
                                    int,
                                    void* sandbox_info,
                                    cef_version_info_t*) {
  return RunMain(hInstance, sandbox_info);
}

#else

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE,
                      LPWSTR,
                      int) {
  void* sandbox_info = nullptr;

#if defined(CEF_USE_SANDBOX)
  CefScopedSandboxInfo scoped_sandbox;
  sandbox_info = scoped_sandbox.sandbox_info();
#endif

  return RunMain(hInstance, sandbox_info);
}

#endif
