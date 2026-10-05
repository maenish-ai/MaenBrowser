#include "src/app/maen_app.h"

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_request_context.h"
#include "include/cef_preference.h"
#include "include/wrapper/cef_helpers.h"

#include "include/maenbrowser/version.h"
#include "src/app/maen_client.h"
#include "src/app/resource_mode.h"
#include "src/app/browser_preferences.h"
#include "src/app/start_page.h"
#include "src/storage/local_profile.h"

namespace maenbrowser {

MaenApp::MaenApp() : client_(new MaenClient()) {}

CefRefPtr<CefClient> MaenApp::GetDefaultClient() {
  // Required by Chrome-style UI for additional tabs/windows.
  return client_;
}

void MaenApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
  if (!process_type.empty()) {
    return;
  }

  // Resource policy is conservative: old PCs get Lite Mode without
  // weakening Chromium sandbox/site isolation.
  resource::ApplyResourcePolicy(command_line, resource::DetectResourceProfile());
}

void MaenApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  auto context = CefRequestContext::GetGlobalContext();
  preferences::ApplyLocalBrowserPreferences(
      CefPreferenceManager::GetGlobalPreferenceManager());

  extension_manager_ =
      std::make_unique<extensions::ExtensionManager>();

  CefWindowInfo window_info;
  window_info.SetAsPopup(nullptr, L"MaenBrowser");
  window_info.runtime_style = CEF_RUNTIME_STYLE_CHROME;

  CefBrowserSettings browser_settings;

  CefBrowserHost::CreateBrowser(
      window_info,
      client_,
      ui::GetStartPageDataUrl(),
      browser_settings,
      nullptr,
      context);
}

}  // namespace maenbrowser
