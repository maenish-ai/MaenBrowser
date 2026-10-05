#pragma once

#include <string>

#include "include/cef_request_context.h"

namespace maenbrowser::extensions {

// Foundation for user-installed web extensions.
// CEF loads unpacked extensions from a directory containing manifest.json.
// CRX3 package verification/extraction is intentionally a separate security layer.
class ExtensionManager {
 public:
  explicit ExtensionManager(CefRefPtr<CefRequestContext> context);

  // Must be invoked on the CEF browser-process UI thread.
  bool LoadUnpacked(const std::wstring& absolute_directory);
  bool Unload(const std::string& extension_id);

 private:
  CefRefPtr<CefRequestContext> context_;
};

}  // namespace maenbrowser::extensions
