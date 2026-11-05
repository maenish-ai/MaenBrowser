#include "src/app/maen_client.h"
#include "src/app/ui_language.h"

#include <windows.h>
#include <shellapi.h>

#include <filesystem>
#include <algorithm>
#include <sstream>

#include "include/cef_app.h"
#include "include/cef_task.h"
#include "include/base/cef_bind.h"
#include "include/base/cef_callback.h"
#include "include/wrapper/cef_closure_task.h"
#include "src/app/internal_navigation.h"
#include "src/app/external_protocol.h"
#include "src/app/popup_policy.h"
#include "src/storage/local_profile.h"
#include "include/wrapper/cef_helpers.h"
#include "src/media/webview2_media_router.h"
#include "src/media/webview2_embedded.h"
#include "src/media/media_window.h"
#include "src/protection/protection.h"
#include "include/cef_id_mappers.h"
#include "include/cef_request_context.h"

namespace maenbrowser {
namespace {
constexpr int kOpenMediaWindow = MENU_ID_USER_FIRST + 95;
bool CanOpenMedia(CefRefPtr<CefBrowser> browser) {
  if (!browser || !browser->GetMainFrame() ||
      browser->GetHost()->GetRequestContext()->GetCachePath().empty()) return false;
  const auto url = browser->GetMainFrame()->GetURL().ToString();
  const auto parsed = protection::ParseUrl(url);
  return parsed.valid && !parsed.host.empty() &&
      (parsed.scheme == "https" || parsed.scheme == "http") && !protection::BlockNavigation(url);
}
void OpenMedia(CefRefPtr<CefBrowser> browser) {
  if (!CanOpenMedia(browser)) return;
  if (!media::OpenMediaWindow(browser->GetMainFrame()->GetURL().ToWString()))
    MessageBoxW(browser->GetHost()->GetWindowHandle(),
        ui::Text(L"Could not start the media compatibility window.", L"تعذر بدء نافذة توافق الفيديو."),
        L"MaenBrowser", MB_OK | MB_ICONERROR);
}
}
void MaenClient::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame>,
    CefRefPtr<CefContextMenuParams>, CefRefPtr<CefMenuModel> model) {
  CEF_REQUIRE_UI_THREAD();
  if (!model || !CanOpenMedia(browser)) return;
  model->AddSeparator();
  model->AddItem(kOpenMediaWindow, ui::Text(
      L"Open page in media window (Ctrl+Shift+M)", L"فتح الصفحة في نافذة الفيديو (Ctrl+Shift+M)"));
}
bool MaenClient::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame>,
    CefRefPtr<CefContextMenuParams>, int command, CefContextMenuHandler::EventFlags) {
  CEF_REQUIRE_UI_THREAD();
  if (command != kOpenMediaWindow) return false;
  // Defer until the native menu callback returns.
  CefPostTask(TID_UI, CefCreateClosureTask(base::BindOnce([](CefRefPtr<CefBrowser> target) {
    if (target && target->IsValid()) OpenMedia(target);
  }, browser)));
  return true;
}
bool MaenClient::OnPreKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event,
    CefEventHandle, bool*) {
  CEF_REQUIRE_UI_THREAD();
  if (event.type == KEYEVENT_RAWKEYDOWN && event.windows_key_code == 'M' &&
      (event.modifiers & EVENTFLAG_CONTROL_DOWN) && (event.modifiers & EVENTFLAG_SHIFT_DOWN) &&
      !(event.modifiers & EVENTFLAG_ALT_DOWN)) {
    OpenMedia(browser); return true;
  }
  return false;
}

bool MaenClient::OnChromeCommand(CefRefPtr<CefBrowser> browser, int command_id, cef_window_open_disposition_t) {
  CEF_REQUIRE_UI_THREAD();
  struct Page { const char* command; const char* url; const char* controls; };
  const Page pages[]={
    {"IDC_OPTIONS",nullptr,"options.html"},{"IDC_ABOUT",nullptr,"about.html"},
    {"IDC_SHOW_HISTORY","chrome://history/",nullptr},
    {"IDC_SHOW_DOWNLOADS","chrome://downloads/",nullptr},
    {"IDC_SHOW_BOOKMARK_MANAGER","chrome://bookmarks/",nullptr}
  };
  for(const auto& page:pages) {
    const int mapped=cef_id_for_command_id_name(page.command);
    if(mapped>0&&command_id==mapped&&browser) {
      OpenUrlInNewTab(page.controls?protection::ControlsUrl(page.controls):page.url,browser,true,false);
      return true;
    }
  }
  if (!protection::FamilyEnabled()) return false;
  for (const char* name : {"IDC_NEW_INCOGNITO_WINDOW", "IDC_DEV_TOOLS", "IDC_DEV_TOOLS_CONSOLE", "IDC_DEV_TOOLS_INSPECT", "IDC_MANAGE_EXTENSIONS"}) {
    if (command_id == cef_id_for_command_id_name(name)) return true;
  }
  return false;
}

CefRefPtr<CefResourceRequestHandler> MaenClient::GetResourceRequestHandler(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request,
    bool navigation, bool download, const CefString& initiator, bool& disable) {
  return resource_handler_->GetResourceRequestHandler(browser, frame, request, navigation, download, initiator, disable);
}

namespace {
std::wstring Utf16(const CefString& value) { return value.ToWString(); }

void ApplyMaenWindowIcon(HWND hwnd) {
  if (!hwnd) return;
  // The final CEF bootstrap EXE is stamped by CI with the canonical Maen icon
  // as group icon resource #1. Load from the running EXE so taskbar/window
  // chrome and the desktop shortcut resolve to the same artwork.
  HINSTANCE exe = GetModuleHandleW(nullptr);
  HICON big = static_cast<HICON>(LoadImageW(
      exe, MAKEINTRESOURCEW(1), IMAGE_ICON,
      GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR | LR_SHARED));
  HICON small = static_cast<HICON>(LoadImageW(
      exe, MAKEINTRESOURCEW(1), IMAGE_ICON,
      GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR | LR_SHARED));
  if (big) SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(big));
  if (small) SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(small));
}
}  // namespace

void MaenClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  browser_count_.fetch_add(1, std::memory_order_relaxed);
  if (!browser) return;

  ApplyMaenWindowIcon(browser->GetHost()->GetWindowHandle());
  browser_instances_[browser->GetIdentifier()] = browser;
  std::erase_if(pending_tabs_,[](const PendingTab& tab){return GetTickCount64()>tab.deadline;});
  if(!browser->IsPopup()) {
    const HWND root=GetAncestor(browser->GetHost()->GetWindowHandle(),GA_ROOT);
    for(auto it=pending_tabs_.begin();it!=pending_tabs_.end();++it) {
      if(it->root!=root)continue;
      const std::string url=it->url;pending_tabs_.erase(it);
      CefPostTask(TID_UI,CefCreateClosureTask(base::BindOnce(
        [](CefRefPtr<CefBrowser> tab,std::string next) {
          if(tab->IsValid()&&tab->GetMainFrame()&&!protection::BlockNavigation(next))
            tab->GetMainFrame()->LoadURL(protection::Rewrite(next));
        },browser,url)));
      break;
    }
  }

  if (storage::IsAgentMode() && !agent_watch_running_) {
    agent_watch_running_ = true;
    CefPostDelayedTask(TID_UI, CefCreateClosureTask(base::BindOnce(
        [](CefRefPtr<MaenClient> client) { client->CheckAgentProtection(); }, CefRefPtr<MaenClient>(this))), 1000);
  }

  if (browser->IsPopup()) {
    std::lock_guard<std::mutex> lock(downloads_mutex_);
    popup_browser_ids_.insert(browser->GetIdentifier());
    popup_browsers_[browser->GetIdentifier()] = browser;
  }
}

void MaenClient::OnGotFocus(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  if (browser) last_active_browser_ = browser->GetIdentifier();
}
bool MaenClient::NavigateExistingTab(const std::string& url) {
  CEF_REQUIRE_UI_THREAD();
  if(!url.empty()&&protection::BlockNavigation(url))return true;
  CefRefPtr<CefBrowser> target;
  auto found=browser_instances_.find(last_active_browser_);
  if(found!=browser_instances_.end()&&found->second->IsValid())target=found->second;
  if(!target)for(const auto& [id,browser]:browser_instances_)
    if(browser->IsValid()&&!browser->IsPopup()){target=browser;break;}
  if(!target||!target->GetMainFrame())return false;
  if(!url.empty())target->GetMainFrame()->LoadURL(protection::Rewrite(url));
  const auto hwnd=target->GetHost()->GetWindowHandle();
  if(hwnd){ShowWindow(hwnd,SW_RESTORE);SetForegroundWindow(hwnd);}
  return true;
}

bool MaenClient::OpenUrlInNewTab(const std::string& url,CefRefPtr<CefBrowser> opener,bool internal,bool foreground) {
  CEF_REQUIRE_UI_THREAD();
  const auto parsed=protection::ParseUrl(url);
  const bool permitted_internal=internal && IsTrustedInternalTab(url);
  if(!permitted_internal&&(!parsed.valid||(parsed.scheme!="https"&&parsed.scheme!="http")))return false;
  if(protection::BlockNavigation(url))return true;
  if(!opener) {
    const auto found=browser_instances_.find(last_active_browser_);
    if(found!=browser_instances_.end())opener=found->second;
    if(!opener||opener->IsPopup())for(const auto& [id,candidate]:browser_instances_)
      if(candidate->IsValid()&&!candidate->IsPopup()){opener=candidate;break;}
  }
  std::erase_if(pending_tabs_,[](const PendingTab& tab){return GetTickCount64()>tab.deadline;});
  if(!opener||!opener->IsValid()||pending_tabs_.size()>=32)return false;
  const int command=cef_id_for_command_id_name("IDC_NEW_TAB");
  if(command<=0||!opener->GetHost()->CanExecuteChromeCommand(command))return false;
  const HWND root=GetAncestor(opener->GetHost()->GetWindowHandle(),GA_ROOT);
  if(!root)return false;
  pending_tabs_.push_back({root,url,GetTickCount64()+20000});
  if(foreground)opener->GetHost()->ExecuteChromeCommand(command,CEF_WOD_NEW_FOREGROUND_TAB);
  else opener->GetHost()->ExecuteChromeCommand(command,CEF_WOD_NEW_BACKGROUND_TAB);
  if(foreground){ShowWindow(root,SW_RESTORE);SetForegroundWindow(root);}
  return true;
}

void MaenClient::PrepareAgentProtection() {
  agent_settings_stamp_ = storage::ReadSettingsStamp(std::filesystem::path(storage::GetAppDataRoot()) / L"controls.dat");
}
bool MaenClient::AgentPolicyUnchanged() const {
  const auto now = storage::ReadSettingsStamp(std::filesystem::path(storage::GetAppDataRoot()) / L"controls.dat");
  return !storage::SettingsChanged(agent_settings_stamp_, now);
}
void MaenClient::CheckAgentProtection() {
  CEF_REQUIRE_UI_THREAD();
  if (browser_instances_.empty()) { agent_watch_running_ = false; return; }
  const auto now = storage::ReadSettingsStamp(std::filesystem::path(storage::GetAppDataRoot()) / L"controls.dat");
  if (storage::SettingsChanged(agent_settings_stamp_, now)) {
    // Parent settings changed in another normal browser process. Stop agent
    // browsing instead of continuing under a stale family/ad policy snapshot.
    agent_watch_running_ = false;
    const auto browsers = browser_instances_;
    for (const auto& [id, browser] : browsers)
      if (browser->IsValid()) browser->GetHost()->CloseBrowser(true);
    return;
  }
  CefPostDelayedTask(TID_UI, CefCreateClosureTask(base::BindOnce(
      [](CefRefPtr<MaenClient> client) { client->CheckAgentProtection(); }, CefRefPtr<MaenClient>(this))), 1000);
}

bool MaenClient::OnBeforePopup(CefRefPtr<CefBrowser> browser,
                               CefRefPtr<CefFrame>,
                               int,
                               const CefString& target_url,
                               const CefString&,
                               CefLifeSpanHandler::WindowOpenDisposition,
                               bool,
                               const CefPopupFeatures&,
                               CefWindowInfo&,
                               CefRefPtr<CefClient>& client,
                               CefBrowserSettings&,
                               CefRefPtr<CefDictionaryValue>&,
                               bool*) {
  CEF_REQUIRE_UI_THREAD();
  client = this;
  if (!browser || !browser->GetMainFrame()) return false;

  // Preserve the original WebContents, request method, upload body, referrer,
  // opener and cookie context. Cancel-and-replay as LoadURL loses POST/blob/
  // redirect state and may cancel a download that already started. The bundled
  // extension reparents ordinary pages only after a real document commits.
  if (protection::BlockNavigation(target_url.ToString())) return true;
  return false;
}

bool MaenClient::DoClose(CefRefPtr<CefBrowser>) {
  CEF_REQUIRE_UI_THREAD();
  return false;
}

void MaenClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  if (browser) browser_instances_.erase(browser->GetIdentifier());
  if (browser) {
    const int id = browser->GetIdentifier();
    media::CloseEmbeddedWebView2(id);
    media_navigations_.erase(id);
    std::lock_guard<std::mutex> lock(downloads_mutex_);
    popup_browser_ids_.erase(id);
    popup_browsers_.erase(id);
  }
  if (browser_count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
    CefQuitMessageLoop();
}

bool MaenClient::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefRequest> request,
                                bool user_gesture,
                                bool) {
  CEF_REQUIRE_UI_THREAD();
  if (!browser || !frame || !frame->IsMain() || !request) return false;
  const std::wstring url = request->GetURL().ToWString();

  if (IsSettingsAlias(request->GetURL().ToString())) {
    CefPostTask(TID_UI, CefCreateClosureTask(base::BindOnce(
        [](CefRefPtr<CefBrowser> target) {
          if (target->IsValid() && target->GetMainFrame())
            target->GetMainFrame()->LoadURL(protection::ControlsUrl("options.html"));
        }, browser)));
    return true;
  }
  if (IsAboutAlias(request->GetURL().ToString())) {
    // Defer replacement until the canceled navigation callback has returned.
    CefPostTask(TID_UI, CefCreateClosureTask(base::BindOnce(
        [](CefRefPtr<CefBrowser> target_browser) {
          if (!target_browser->IsValid()) return;
          CefRefPtr<CefFrame> target_frame = target_browser->GetMainFrame();
          if (target_frame)
            target_frame->LoadURL(protection::ControlsUrl("about.html"));
        }, browser)));
    return true;
  }

  if (protection::BlockNavigation(request->GetURL().ToString())) {
    MessageBoxW(browser->GetHost()->GetWindowHandle(), ui::Text(L"This address is blocked by MaenBrowser Family Protection. Open the shield to ask a parent to review it.", L"هذا العنوان محجوب بحماية الأسرة. افتح الحماية ليتمكن الوالدان من مراجعته."), L"MaenBrowser", MB_OK | MB_ICONINFORMATION);
    return true;
  }

  if (AllowedExternalUrl(request->GetURL().ToString())) {
    if (user_gesture && !protection::FamilyEnabled()) {
      const auto prompt = std::wstring(ui::Text(L"Open this link in its Windows application?\n", L"فتح هذا الرابط في تطبيق ويندوز؟\n")) + url;
      if (MessageBoxW(browser->GetHost()->GetWindowHandle(), prompt.c_str(), L"MaenBrowser", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
        ShellExecuteW(browser->GetHost()->GetWindowHandle(), L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }
    return true;
  }
  media::CloseEmbeddedWebView2(browser->GetIdentifier());
  const int browser_id = browser->GetIdentifier();
  media_navigations_.erase(browser_id);
  if (request->GetMethod() != "GET" || !media::UsesEmbeddedMedia(url)) {
    // If this tab was using the on-demand WebView2 media surface, returning to
    // an ordinary URL tears it down immediately to release RAM and processes.
    return false;
  }

  // Never attach the persistent WebView2 profile to a private CEF window.
  // Private media stays in the private CEF context (codec availability may differ).
  if (browser->GetHost()->GetRequestContext()->GetCachePath().empty()) return false;

  media_navigations_[browser_id] = url;
  // Let the native resource handler commit a local placeholder at this URL.
  // OnLoadEnd then attaches WebView2 without losing Chrome's address/history.
  return false;
}

void MaenClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int status) {
  CEF_REQUIRE_UI_THREAD();
  if (!browser || !frame || !frame->IsMain() || status != 200 ||
      browser->GetHost()->GetRequestContext()->GetCachePath().empty()) return;
  const auto url = frame->GetURL().ToWString();

  const auto pending = media_navigations_.find(browser->GetIdentifier());
  if (pending == media_navigations_.end() || pending->second != url ||
      protection::BlockNavigation(frame->GetURL().ToString())) return;
  // Consume once: duplicate load notifications must not navigate an existing surface.
  media_navigations_.erase(pending);
  const bool opened = media::OpenEmbeddedWebView2(browser->GetIdentifier(), browser->GetHost()->GetWindowHandle(), url,
      [self=CefRefPtr<MaenClient>(this),browser](const std::wstring& target) {
        CefPostTask(TID_UI,CefCreateClosureTask(base::BindOnce(
          [](CefRefPtr<MaenClient> client,CefRefPtr<CefBrowser> opener,std::string url){client->OpenUrlInNewTab(url,opener);},
          self,browser,CefString(target).ToString())));
      });
  if (!opened) frame->ExecuteJavaScript(
      ui::Arabic() ? "document.body.textContent='تعذر فتح الصفحة. أعد التحميل للمحاولة.';"
                   : "document.body.textContent='Could not open this page. Reload to retry.';",
      frame->GetURL(), 0);
}

void MaenClient::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
  CEF_REQUIRE_UI_THREAD();
  HWND hwnd = browser ? browser->GetHost()->GetWindowHandle() : nullptr;
  if (hwnd) {
    std::wstring page_title = Utf16(title);
    auto main_frame = browser ? browser->GetMainFrame() : nullptr;
    if (main_frame && media::IsWhatsAppWebUrl(main_frame->GetURL().ToWString()))
      page_title = L"WhatsApp Web";
    if (page_title.empty() || page_title == L"MaenBrowser") SetWindowTextW(hwnd, L"MaenBrowser");
    else SetWindowTextW(hwnd, (page_title + L" — MaenBrowser").c_str());
  }
}

bool MaenClient::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                                  CefRefPtr<CefDownloadItem> download_item,
                                  const CefString& suggested_name,
                                  CefRefPtr<CefBeforeDownloadCallback> callback) {
  CEF_REQUIRE_UI_THREAD();
  if (!callback || !download_item) return false;

  // Download reliability rule:
  // Do NOT close the initiating browser/popup from OnBeforeDownload.
  // In Chrome Runtime the download manager may still depend on that browser
  // while ownership is being transferred. Closing it here can race with the
  // transfer and surface as an immediate "Canceled" download on sites that
  // use redirects or short-lived download pages (including GitHub artifacts).
  //
  // We intentionally prefer a harmless transient popup over a canceled file.
  // Popup cleanup, if reintroduced later, must occur only after a separately
  // proven lifecycle signal and must never be coupled to download start.

  // If a real popup initiated this download, remember the association only.
  // We do NOT hide or close anything here. Chromium must first report the
  // download as in progress; this avoids the old GitHub immediate-Canceled race.
  if (browser) {
    std::lock_guard<std::mutex> lock(downloads_mutex_);
    const int browser_id = browser->GetIdentifier();
    if (popup_browser_ids_.count(browser_id) != 0) {
      downloads_[download_item->GetId()].popup_browser_id = browser_id;
    }
  }

  // Explicitly continue the download. CEF's download callback contract requires
  // Continue() when the application takes ownership of OnBeforeDownload. Using
  // an empty path preserves Chromium/CEF's suggested filename and default
  // download location; show_dialog=true keeps the user in control with Save As.
  // This is important for blob/service-worker generated downloads such as
  // WhatsApp Web media, where relying only on implicit Chrome Runtime handling
  // can surface as an immediate canceled transfer in embedded builds.
  const auto download_path = protection::DownloadPath(suggested_name.ToWString());
  const bool ask = protection::Current()->ask_download || download_path.empty();
  callback->Continue(download_path, ask);
  return true;
}

void MaenClient::OnDownloadUpdated(CefRefPtr<CefBrowser>,
                                   CefRefPtr<CefDownloadItem> item,
                                   CefRefPtr<CefDownloadItemCallback>) {
  CEF_REQUIRE_UI_THREAD();
  if (!item) return;
  // The download manager owns transfer lifetime. Never hide/close a window or
  // tab here: it can contain another transfer, or have been moved into the
  // main window while retaining its original popup identity.
  std::lock_guard<std::mutex> lock(downloads_mutex_);
  const uint32_t id=item->GetId();
  auto& state=downloads_[id];
  const auto path=Utf16(item->GetFullPath());
  if(!path.empty())state.path=path;
  state.last_percent=item->GetPercentComplete();
  if(item->IsComplete()||item->IsCanceled()||item->IsInterrupted())downloads_.erase(id);
}

}  // namespace maenbrowser
