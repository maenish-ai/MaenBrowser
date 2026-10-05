#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <string>

#include "include/cef_client.h"
#include "include/cef_display_handler.h"
#include "include/cef_download_handler.h"
#include "include/cef_life_span_handler.h"

namespace maenbrowser {

class MaenClient final : public CefClient,
                         public CefLifeSpanHandler,
                         public CefDisplayHandler,
                         public CefDownloadHandler {
 public:
  MaenClient() = default;

  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
  CefRefPtr<CefDownloadHandler> GetDownloadHandler() override { return this; }

  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;

  bool OnBeforeDownload(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefDownloadItem> download_item,
                        const CefString& suggested_name,
                        CefRefPtr<CefBeforeDownloadCallback> callback) override;
  void OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefDownloadItem> download_item,
                         CefRefPtr<CefDownloadItemCallback> callback) override;

 private:
  struct DownloadState {
    std::wstring path;
    int last_percent = -1;
    bool completed_notified = false;
  };

  void ShowDownloadComplete(const DownloadState& state, const std::wstring& file_name);

  std::atomic<int> browser_count_{0};
  std::mutex downloads_mutex_;
  std::map<uint32_t, DownloadState> downloads_;

  IMPLEMENT_REFCOUNTING(MaenClient);
  DISALLOW_COPY_AND_ASSIGN(MaenClient);
};

}  // namespace maenbrowser
