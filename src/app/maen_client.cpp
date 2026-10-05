#include "src/app/maen_client.h"

#include "include/cef_app.h"

namespace maenbrowser {

void MaenClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
  browser_count_.fetch_add(1, std::memory_order_relaxed);
}

bool MaenClient::DoClose(CefRefPtr<CefBrowser> browser) {
  return false;
}

void MaenClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
  if (browser_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
    CefQuitMessageLoop();
  }
}

}  // namespace maenbrowser
