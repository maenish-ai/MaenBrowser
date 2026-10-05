#pragma once

#include <atomic>

#include "include/cef_client.h"
#include "include/cef_life_span_handler.h"

namespace maenbrowser {

class MaenClient final : public CefClient,
                         public CefLifeSpanHandler {
 public:
  MaenClient() = default;

  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

 private:
  std::atomic<int> browser_count_{0};

  IMPLEMENT_REFCOUNTING(MaenClient);
  DISALLOW_COPY_AND_ASSIGN(MaenClient);
};

}  // namespace maenbrowser
