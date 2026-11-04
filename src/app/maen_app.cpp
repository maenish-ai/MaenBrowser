#include "src/app/maen_app.h"

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_request_context.h"
#include "include/cef_preference.h"
#include "include/wrapper/cef_helpers.h"

#include "include/maenbrowser/version.h"
#include "src/app/maen_client.h"
#include "src/media/media_window.h"
#include "src/app/ui_language.h"
#include "src/app/resource_mode.h"
#include "src/app/browser_preferences.h"
#include "src/app/start_page.h"
#include "src/storage/local_profile.h"
#include "src/protection/protection.h"
#include "src/protection/identity.h"
#include "src/app/popup_policy.h"
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

  if (storage::IsAgentMode()) {
    command_line->AppendSwitchWithValue("remote-debugging-address", "127.0.0.1");
    command_line->RemoveSwitch("remote-allow-origins");
    command_line->RemoveSwitch("remote-debugging-pipe");
  }
  // Resource policy is conservative: old PCs get Lite Mode without
  // weakening Chromium sandbox/site isolation.
  resource::ApplyResourcePolicy(command_line, resource::DetectResourceProfile());
  command_line->AppendSwitchWithValue("lang", ui::Locale());
  const auto companion = protection::InstallDirectory() / L"companion";
  if (std::filesystem::exists(companion / L"manifest.json")) {
    auto paths = command_line->GetSwitchValue("load-extension").ToWString();
    if (!paths.empty()) paths += L",";
    paths += companion.wstring();
    command_line->AppendSwitchWithValue("load-extension", paths);
  }
}

bool MaenApp::OnAlreadyRunningAppRelaunch(CefRefPtr<CefCommandLine> command_line, const CefString&) {
  CEF_REQUIRE_UI_THREAD();
  CefCommandLine::ArgumentList arguments;
  command_line->GetArguments(arguments);
  for(const auto& arg:arguments){
    const auto url=arg.ToString();const auto parsed=protection::ParseUrl(url);
    if(!parsed.valid||(parsed.scheme!="http"&&parsed.scheme!="https"))continue;
    if(protection::BlockNavigation(url))return true;
    if(AuthPopupHost(parsed.host)||AuthPopupUrl(url))return false;
    return client_->NavigateExistingTab(url);
  }
  return client_->NavigateExistingTab("");
}

void MaenApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  if (storage::IsAgentMode()) client_->PrepareAgentProtection();
  protection::Initialize();
  if (storage::IsAgentMode() && (protection::FamilyEnabled() || !client_->AgentPolicyUnchanged())) {
    MessageBoxW(nullptr, ui::Text(L"Agent control is unavailable while Family Protection is enabled or its settings are changing.",
        L"التحكم الآلي غير متاح أثناء تفعيل حماية الأسرة أو تغيير إعداداتها."), L"MaenBrowser", MB_OK | MB_ICONWARNING);
    CefQuitMessageLoop();
    return;
  }

  // Chrome extension fetches can use the global profile even when the visible
  // browser shares its storage through a separate request-context object.
  auto global = CefRequestContext::GetGlobalContext();
  auto context = CefRequestContext::CreateContext(global, protection::ContextHandler());
  if (!protection::RegisterLocalEndpoint(global) ||
      !protection::RegisterLocalEndpoint(context)) {
    MessageBoxW(nullptr, ui::Text(L"The browser's internal service could not start.",
        L"تعذر بدء الخدمة الداخلية للمتصفح."), L"MaenBrowser", MB_OK | MB_ICONERROR);
    CefQuitMessageLoop();
    return;
  }
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
  // Also used by CI to exercise real WebView2 playback. This flag is local
  // startup input only and follows the same URL/family checks as the menu.
  const auto media_url = CefCommandLine::GetGlobalCommandLine()->GetSwitchValue("maen-media-url");
  // Diagnostic media window is deliberately opt-in and disabled in normal use.
  if (CefCommandLine::GetGlobalCommandLine()->HasSwitch("maen-enable-media-diagnostics") &&
      !media_url.empty()) media::OpenMediaWindow(media_url.ToWString());
}

}  // namespace maenbrowser
