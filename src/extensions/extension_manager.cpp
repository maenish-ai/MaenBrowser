#include "src/extensions/extension_manager.h"

#include <filesystem>

namespace maenbrowser::extensions {

ExtensionManager::ExtensionManager(CefRefPtr<CefRequestContext> context)
    : context_(std::move(context)) {}

bool ExtensionManager::LoadUnpacked(const std::wstring& absolute_directory) {
  if (!context_) return false;
  const std::filesystem::path root(absolute_directory);
  if (!std::filesystem::is_directory(root) ||
      !std::filesystem::exists(root / L"manifest.json")) {
    return false;
  }

  context_->LoadExtension(root.wstring(), nullptr, nullptr);
  return true;
}

bool ExtensionManager::Unload(const std::string& extension_id) {
  if (!context_ || !context_->HasExtension(extension_id)) return false;
  auto extension = context_->GetExtension(extension_id);
  if (!extension || !extension->IsLoaded()) return false;
  extension->Unload();
  return true;
}

}  // namespace maenbrowser::extensions
