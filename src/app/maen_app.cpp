#include "src/app/maen_app.h"

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"
#include "include/maenbrowser/version.h"
#include "src/app/maen_client.h"

namespace maenbrowser {

void MaenApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
  // Intentionally avoid security-reducing Chromium flags. Memory tuning belongs
  // in MaenBrowser's tab lifecycle manager, not in disabling site isolation.
  (void)process_type;
  (void)command_line;
}

void MaenApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  CefWindowInfo window_info;
  window_info.SetAsPopup(nullptr, kProductName);

  CefBrowserSettings browser_settings;
  auto client = CefRefPtr<MaenClient>(new MaenClient());

  CefBrowserHost::CreateBrowser(window_info,
                                client,
                                kHomepage,
                                browser_settings,
                                nullptr,
                                nullptr);
}

}  // namespace maenbrowser
