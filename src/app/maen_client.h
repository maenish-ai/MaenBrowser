#pragma once

#include <atomic>
#include <map>
#include <set>
#include <mutex>
#include <string>

#include "include/cef_client.h"
#include "include/cef_display_handler.h"
#include "include/cef_download_handler.h"
#include "include/cef_life_span_handler.h"
#include "include/cef_request_handler.h"
#include "include/cef_command_handler.h"
#include "include/cef_load_handler.h"

namespace maenbrowser {

class MaenClient final : public CefClient,
                         public CefLifeSpanHandler,
                         public CefDisplayHandler,
                         public CefDownloadHandler,
                         public CefRequestHandler,
                         public CefCommandHandler,
                         public CefLoadHandler {
 public:
  MaenClient() = default;

  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
  CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
  CefRefPtr<CefDownloadHandler> GetDownloadHandler() override { return this; }
  CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }
  CefRefPtr<CefCommandHandler> GetCommandHandler() override { return this; }
  CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
  void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                   TransitionType transition_type) override;
  void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int status) override;
  bool OnChromeCommand(CefRefPtr<CefBrowser> browser, int command_id,
                       cef_window_open_disposition_t disposition) override;
  CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(
      CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
      CefRefPtr<CefRequest> request, bool is_navigation, bool is_download,
      const CefString& request_initiator, bool& disable_default_handling) override;

  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     int popup_id,
                     const CefString& target_url,
                     const CefString& target_frame_name,
                     CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                     bool user_gesture,
                     const CefPopupFeatures& popupFeatures,
                     CefWindowInfo& windowInfo,
                     CefRefPtr<CefClient>& client,
                     CefBrowserSettings& settings,
                     CefRefPtr<CefDictionaryValue>& extra_info,
                     bool* no_javascript_access) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
  bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                        cef_log_severity_t level,
                        const CefString& message,
                        const CefString& source,
                        int line) override;

  bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                      CefRefPtr<CefFrame> frame,
                      CefRefPtr<CefRequest> request,
                      bool user_gesture,
                      bool is_redirect) override;

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
    int popup_browser_id = 0;
    bool popup_hidden = false;
  };


  // UI-thread-only: only GET navigations intercepted by the media router.
  std::map<int, std::wstring> media_navigations_;
  // Main-frame URLs whose CEF page reported an unsupported HTML5 media
  // source.  They are retried once in the on-demand WebView2 surface.
  std::map<int, std::wstring> media_fallbacks_;
  std::atomic<int> browser_count_{0};
  std::mutex downloads_mutex_;
  std::map<uint32_t, DownloadState> downloads_;
  std::set<int> popup_browser_ids_;
  std::map<int, CefRefPtr<CefBrowser>> popup_browsers_;

  IMPLEMENT_REFCOUNTING(MaenClient);
  DISALLOW_COPY_AND_ASSIGN(MaenClient);
};

}  // namespace maenbrowser
