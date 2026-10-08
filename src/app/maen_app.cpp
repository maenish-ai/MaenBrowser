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
#include "src/protection/protection.h"
#include "src/protection/identity.h"
#include <filesystem>

namespace maenbrowser {

MaenApp::MaenApp() : client_(new MaenClient()) {}

CefRefPtr<CefClient> MaenApp::GetDefaultClient() {
  // Required by Chrome-style UI for additional tabs/windows.
  return client_;
}

CefRefPtr<CefRequestContextHandler> MaenApp::GetDefaultRequestContextHandler() {
  return protection::ContextHandler();
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
  const auto companion = protection::InstallDirectory() / L"companion";
  if (std::filesystem::exists(companion / L"manifest.json")) {
    auto paths = command_line->GetSwitchValue("load-extension").ToWString();
    if (!paths.empty()) paths += L",";
    paths += companion.wstring();
    command_line->AppendSwitchWithValue("load-extension", paths);
  }
}

void MaenApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  protection::Initialize();

  auto context = CefRequestContext::CreateContext(
      CefRequestContext::GetGlobalContext(), protection::ContextHandler());
  preferences::ApplyLocalBrowserPreferences(
      context);

  // Preserve user-pinned extensions and make the bundled control visible.
  if (context->CanSetPreference("extensions.pinned_extensions")) {
    auto pinned = context->GetPreference("extensions.pinned_extensions");
    auto list = pinned && pinned->GetType() == VTYPE_LIST ? pinned->GetList()->Copy() : CefListValue::Create();
    bool found = false;
    for (size_t i = 0; i < list->GetSize(); ++i) if (list->GetString(i) == protection::kExtensionId) found = true;
    if (!found) list->SetString(list->GetSize(), protection::kExtensionId);
    auto value = CefValue::Create(); value->SetList(list); CefString error;
    context->SetPreference("extensions.pinned_extensions", value, error);
  }

  extension_manager_ =
      std::make_unique<extensions::ExtensionManager>();

  CefWindowInfo window_info;
  window_info.SetAsPopup(nullptr, L"MaenBrowser");
  window_info.runtime_style = CEF_RUNTIME_STYLE_CHROME;

  CefBrowserSettings browser_settings;

  std::string initial = protection::ControlsUrl("start.html");
  if (!std::filesystem::exists(protection::InstallDirectory() / L"companion" / L"manifest.json")) initial = ui::GetStartPageDataUrl();
  CefCommandLine::ArgumentList arguments;
  CefCommandLine::GetGlobalCommandLine()->GetArguments(arguments);
  for (const auto& arg : arguments) {
    const auto url = protection::ParseUrl(arg.ToString());
    if ((url.scheme == "https" || url.scheme == "http") && !protection::BlockNavigation(arg.ToString())) {
      initial = protection::Rewrite(arg.ToString()); break;
    }
  }

  CefBrowserHost::CreateBrowser(
      window_info,
      client_,
      initial,
      browser_settings,
      nullptr,
      context);
}

}  // namespace maenbrowser
