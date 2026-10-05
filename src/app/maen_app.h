#pragma once

#include "include/cef_app.h"
#include "include/cef_browser_process_handler.h"

namespace maenbrowser {

class MaenApp final : public CefApp,
                      public CefBrowserProcessHandler {
 public:
  MaenApp() = default;

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }

  void OnBeforeCommandLineProcessing(const CefString& process_type,
                                     CefRefPtr<CefCommandLine> command_line) override;
  void OnContextInitialized() override;

 private:
  IMPLEMENT_REFCOUNTING(MaenApp);
  DISALLOW_COPY_AND_ASSIGN(MaenApp);
};

}  // namespace maenbrowser
