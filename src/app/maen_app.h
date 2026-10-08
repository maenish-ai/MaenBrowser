#pragma once

#include <memory>

#include "include/cef_app.h"
#include "include/cef_browser_process_handler.h"

#include "src/app/maen_client.h"
#include "src/extensions/extension_manager.h"

namespace maenbrowser {

class MaenApp final : public CefApp,
                      public CefBrowserProcessHandler {
 public:
  MaenApp();

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  CefRefPtr<CefClient> GetDefaultClient() override;
  CefRefPtr<CefRequestContextHandler> GetDefaultRequestContextHandler() override;

  void OnBeforeCommandLineProcessing(
      const CefString& process_type,
      CefRefPtr<CefCommandLine> command_line) override;

  void OnContextInitialized() override;

 private:
  CefRefPtr<MaenClient> client_;
  std::unique_ptr<extensions::ExtensionManager> extension_manager_;

  IMPLEMENT_REFCOUNTING(MaenApp);
  DISALLOW_COPY_AND_ASSIGN(MaenApp);
};

}  // namespace maenbrowser
