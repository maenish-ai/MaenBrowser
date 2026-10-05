#pragma once

#include <cstddef>
#include <string>

namespace maenbrowser::extensions {

// CEF 152 Chrome-style runtime no longer exposes the legacy
// CefRequestContext LoadExtension/HasExtension/GetExtension API used by
// older CEF releases. Keep this class as a compatibility boundary so the
// browser builds cleanly; extension installation/management is delegated
// to the Chrome-style runtime UI instead of calling removed APIs.
class ExtensionManager {
 public:
  ExtensionManager() = default;

  bool LoadUnpacked(const std::wstring& absolute_directory);
  std::size_t LoadAllUnpacked(const std::wstring& extensions_root);
  bool Unload(const std::string& extension_id);
};

}  // namespace maenbrowser::extensions
