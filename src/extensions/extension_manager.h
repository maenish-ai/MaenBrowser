#pragma once

#include <cstddef>
#include <string>

#include "include/cef_request_context.h"

namespace maenbrowser::extensions {

// Loads user-supplied unpacked Chromium extensions from local disk.
// Each immediate subdirectory must contain manifest.json.
// CRX verification/extraction is deliberately not implemented here because
// accepting arbitrary unsigned packages without verification would be unsafe.
class ExtensionManager {
 public:
  explicit ExtensionManager(CefRefPtr<CefRequestContext> context);

  bool LoadUnpacked(const std::wstring& absolute_directory);
  std::size_t LoadAllUnpacked(const std::wstring& extensions_root);
  bool Unload(const std::string& extension_id);

 private:
  CefRefPtr<CefRequestContext> context_;
};

}  // namespace maenbrowser::extensions
