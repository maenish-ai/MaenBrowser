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
}  // namespace

void MaenClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  CEF_REQUIRE_UI_THREAD();
  browser_count_.fetch_add(1, std::memory_order_relaxed);
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

  // Chrome Runtime already implements Chromium's native download UI.
  // Returning false delegates the download to that UI (download bubble/shelf).
  // The download.prompt_for_download preference controls whether Save As is shown.
  // OnDownloadUpdated still receives progress/completion notifications.
  return false;
}

void MaenClient::OnDownloadUpdated(CefRefPtr<CefBrowser>,
                                   CefRefPtr<CefDownloadItem> item,
                                   CefRefPtr<CefDownloadItemCallback>) {
  CEF_REQUIRE_UI_THREAD();
  if (!item) return;

  const uint32_t id = item->GetId();
  DownloadState snapshot;
  bool notify = false;
  {
    std::lock_guard<std::mutex> lock(downloads_mutex_);
    auto& state = downloads_[id];
    const auto full_path = Utf16(item->GetFullPath());
    if (!full_path.empty()) state.path = full_path;
    state.last_percent = item->GetPercentComplete();
    if (item->IsComplete() && !state.completed_notified) {
      state.completed_notified = true;
      notify = true;
    }
    snapshot = state;
    if (item->IsCanceled() || item->IsInterrupted()) downloads_.erase(id);
  }

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
