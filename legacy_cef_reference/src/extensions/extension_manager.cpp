#include "src/extensions/extension_manager.h"

namespace maenbrowser::extensions {

bool ExtensionManager::LoadUnpacked(const std::wstring&) {
  // Legacy programmatic loading was removed from the CEF 152 public API.
  // Returning false is intentional and safe; do not pretend an extension
  // was loaded when the runtime did not load it.
  return false;
}

std::size_t ExtensionManager::LoadAllUnpacked(const std::wstring&) {
  return 0;
}

bool ExtensionManager::Unload(const std::string&) {
  return false;
}

}  // namespace maenbrowser::extensions
