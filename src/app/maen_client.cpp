#include "src/app/maen_client.h"
#include "src/app/ui_language.h"

#include <windows.h>
#include <shellapi.h>

#include <filesystem>
#include <sstream>

#include "include/cef_app.h"
#include "include/cef_task.h"
#include "include/base/cef_bind.h"
#include "include/base/cef_callback.h"
#include "include/wrapper/cef_closure_task.h"
#include "src/app/internal_navigation.h"
#include "include/wrapper/cef_helpers.h"
#include "src/media/webview2_media_router.h"
#include "src/media/webview2_embedded.h"
#include "src/protection/protection.h"
#include "include/cef_id_mappers.h"
#include "include/cef_request_context.h"

namespace maenbrowser {
bool MaenClient::OnChromeCommand(CefRefPtr<CefBrowser> browser, int command_id, cef_window_open_disposition_t) {
  CEF_REQUIRE_UI_THREAD();
  const int about_id = cef_id_for_command_id_name("IDC_ABOUT");
  if (about_id > 0 && command_id == about_id && browser && browser->GetMainFrame()) {
    browser->GetMainFrame()->LoadURL(protection::ControlsUrl("about.html"));
    return true;
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
  return protection::ContextHandler()->GetResourceRequestHandler(browser, frame, request, navigation, download, initiator, disable);
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
      GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR));
  HICON small = static_cast<HICON>(LoadImageW(
      exe, MAKEINTRESOURCEW(1), IMAGE_ICON,
      GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
  if (big) SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(big));
  if (small) SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(small));
}
}  // namespace

void MaenClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  browser_count_.fetch_add(1, std::memory_order_relaxed);
  if (!browser) return;

  ApplyMaenWindowIcon(browser->GetHost()->GetWindowHandle());

  if (browser->IsPopup()) {
    std::lock_guard<std::mutex> lock(downloads_mutex_);
    popup_browser_ids_.insert(browser->GetIdentifier());
    popup_browsers_[browser->GetIdentifier()] = browser;
  }
}

bool MaenClient::OnBeforePopup(CefRefPtr<CefBrowser>,
                               CefRefPtr<CefFrame>,
                               int popup_id,
                               const CefString&,
                               const CefString&,
                               CefLifeSpanHandler::WindowOpenDisposition,
                               bool,
                               const CefPopupFeatures&,
                               CefWindowInfo&,
                               CefRefPtr<CefClient>& client,
                               CefBrowserSettings&,
                               CefRefPtr<CefDictionaryValue>& extra_info,
                               bool*) {
  CEF_REQUIRE_UI_THREAD();

  // Keep the same MaenClient on opener-created windows so download events can
  // be correlated. We deliberately do not cancel the popup here: OAuth,
  // payment and sign-in windows must remain functional.
  client = this;

  return false;
}

bool MaenClient::DoClose(CefRefPtr<CefBrowser>) {
  CEF_REQUIRE_UI_THREAD();
  return false;
}

void MaenClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
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
      [browser](const std::wstring& target) {
        if (browser->IsValid() && browser->GetMainFrame()) browser->GetMainFrame()->LoadURL(target);
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
  if (!callback || !download_item) return true;

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

  const uint32_t id = item->GetId();
  CefRefPtr<CefBrowser> popup;
  bool hide_popup = false;
  bool close_popup = false;

  {
    std::lock_guard<std::mutex> lock(downloads_mutex_);
    auto& state = downloads_[id];
    const auto full_path = Utf16(item->GetFullPath());
    if (!full_path.empty()) state.path = full_path;
    state.last_percent = item->GetPercentComplete();

    if (state.popup_browser_id != 0) {
      auto it = popup_browsers_.find(state.popup_browser_id);
      if (it != popup_browsers_.end()) popup = it->second;

      // Chrome/Edge-style transient download window behavior:
      // hide only AFTER Chromium confirms transfer activity. The browser stays
      // alive in the background, so redirect-backed downloads are not canceled.
      if (!state.popup_hidden && item->IsInProgress() &&
          (item->GetReceivedBytes() > 0 || item->GetPercentComplete() >= 0)) {
        state.popup_hidden = true;
        hide_popup = true;
      }
    }

    if (item->IsComplete() && !state.completed_notified) {
      state.completed_notified = true;
      close_popup = state.popup_browser_id != 0;
    } else if (item->IsCanceled() || item->IsInterrupted()) {
      // Never hide an error from the user forever; close only a transient popup
      // that was already hidden after confirmed transfer activity.
      close_popup = state.popup_browser_id != 0 && state.popup_hidden;
    }

    if (item->IsComplete() || item->IsCanceled() || item->IsInterrupted())
      downloads_.erase(id);
  }

  if (hide_popup && popup) {
    HWND hwnd = popup->GetHost()->GetWindowHandle();
    if (hwnd) ShowWindow(hwnd, SW_HIDE);
  }

  // Closing is deliberately terminal-only. Never close at download start.
  if (close_popup && popup) popup->GetHost()->CloseBrowser(false);

  // Chrome Runtime owns non-modal download UI. Do not block its UI thread
  // with a completion MessageBox or automatically open downloaded files.
}

}  // namespace maenbrowser
