#include "src/app/maen_client.h"

#include <windows.h>
#include <shellapi.h>

#include <filesystem>
#include <sstream>

#include "include/cef_app.h"
#include "include/wrapper/cef_helpers.h"

namespace maenbrowser {
namespace {
std::wstring Utf16(const CefString& value) { return value.ToWString(); }

std::wstring FileNameFromPath(const std::wstring& path) {
  if (path.empty()) return L"download";
  return std::filesystem::path(path).filename().wstring();
}

void OpenPath(const std::wstring& path) {
  if (!path.empty()) ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void ShowInFolder(const std::wstring& path) {
  if (path.empty()) return;
  std::wstring args = L"/select,\"" + path + L"\"";
  ShellExecuteW(nullptr, L"open", L"explorer.exe", args.c_str(), nullptr, SW_SHOWNORMAL);
}

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
                               WindowOpenDisposition,
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
    std::lock_guard<std::mutex> lock(downloads_mutex_);
    const int id = browser->GetIdentifier();
    popup_browser_ids_.erase(id);
    popup_browsers_.erase(id);
  }
  if (browser_count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
    CefQuitMessageLoop();
}

void MaenClient::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
  CEF_REQUIRE_UI_THREAD();
  HWND hwnd = browser ? browser->GetHost()->GetWindowHandle() : nullptr;
  if (hwnd) {
    std::wstring page_title = Utf16(title);
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
  callback->Continue(CefString(), true);
  return true;
}

void MaenClient::OnDownloadUpdated(CefRefPtr<CefBrowser>,
                                   CefRefPtr<CefDownloadItem> item,
                                   CefRefPtr<CefDownloadItemCallback>) {
  CEF_REQUIRE_UI_THREAD();
  if (!item) return;

  const uint32_t id = item->GetId();
  DownloadState snapshot;
  CefRefPtr<CefBrowser> popup;
  bool hide_popup = false;
  bool close_popup = false;
  bool notify = false;

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
      notify = true;
      close_popup = state.popup_browser_id != 0;
    } else if (item->IsCanceled() || item->IsInterrupted()) {
      // Never hide an error from the user forever; close only a transient popup
      // that was already hidden after confirmed transfer activity.
      close_popup = state.popup_browser_id != 0 && state.popup_hidden;
    }

    snapshot = state;
    if (item->IsComplete() || item->IsCanceled() || item->IsInterrupted())
      downloads_.erase(id);
  }

  if (hide_popup && popup) {
    HWND hwnd = popup->GetHost()->GetWindowHandle();
    if (hwnd) ShowWindow(hwnd, SW_HIDE);
  }

  // Closing is deliberately terminal-only. Never close at download start.
  if (close_popup && popup) popup->GetHost()->CloseBrowser(false);

  if (notify) ShowDownloadComplete(snapshot, FileNameFromPath(snapshot.path));
}

void MaenClient::ShowDownloadComplete(const DownloadState& state,
                                      const std::wstring& file_name) {
  std::wstring message = L"Download complete:\n" + file_name +
                         L"\n\nYes = Open file\nNo = Show in folder\nCancel = Close";
  const int result = MessageBoxW(nullptr, message.c_str(), L"MaenBrowser Downloads",
                                 MB_YESNOCANCEL | MB_ICONINFORMATION | MB_TOPMOST);
  if (result == IDYES) OpenPath(state.path);
  else if (result == IDNO) ShowInFolder(state.path);
}

}  // namespace maenbrowser
