#include <windows.h>

#include "include/cef_app.h"
#include "include/maenbrowser/version.h"
#include "src/app/maen_app.h"
#include "src/storage/local_profile.h"
#include "src/updater/update_manager.h"

int APIENTRY wWinMain(HINSTANCE instance,
                      HINSTANCE,
                      wchar_t*,
                      int) {
  CefMainArgs main_args(instance);

  void* sandbox_info = nullptr;

  CefRefPtr<maenbrowser::MaenApp> app(new maenbrowser::MaenApp());

  const int subprocess_code = CefExecuteProcess(main_args, app, sandbox_info);
  if (subprocess_code >= 0) {
    return subprocess_code;
  }

  CefSettings settings;
  settings.chrome_runtime = true;
  settings.persist_session_cookies = true;
  settings.log_severity = LOGSEVERITY_WARNING;

  CefString(&settings.root_cache_path) = maenbrowser::storage::GetProfileRoot();
  CefString(&settings.cache_path) = maenbrowser::storage::GetCachePath();

  // Foundation build mode. Public release must migrate to CEF bootstrap sandbox.
  settings.no_sandbox = true;

  if (!CefInitialize(main_args, settings, app, sandbox_info)) {
    MessageBoxW(nullptr,
                L"MaenBrowser could not initialize the Chromium engine.",
                L"MaenBrowser",
                MB_ICONERROR | MB_OK);
    return 1;
  }

  maenbrowser::updater::UpdateManager::Initialize();
  CefRunMessageLoop();
  maenbrowser::updater::UpdateManager::Shutdown();
  CefShutdown();
  return 0;
}
