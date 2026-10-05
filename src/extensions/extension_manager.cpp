#include "src/extensions/extension_manager.h"

#include <filesystem>

#include "include/wrapper/cef_helpers.h"

namespace maenbrowser::extensions {

ExtensionManager::ExtensionManager(CefRefPtr<CefRequestContext> context)
    : context_(std::move(context)) {}

bool ExtensionManager::LoadUnpacked(
    const std::wstring& absolute_directory) {
  CEF_REQUIRE_UI_THREAD();

  if (!context_) {
    return false;
  }

  const std::filesystem::path root(absolute_directory);
  if (!std::filesystem::is_directory(root) ||
      !std::filesystem::exists(root / L"manifest.json")) {
    return false;
  }

  context_->LoadExtension(root.wstring(), nullptr, nullptr);
  return true;
}

std::size_t ExtensionManager::LoadAllUnpacked(
    const std::wstring& extensions_root) {
  CEF_REQUIRE_UI_THREAD();

  const std::filesystem::path root(extensions_root);
  std::error_code ec;
  std::filesystem::create_directories(root, ec);

  std::size_t loaded = 0;
  for (const auto& entry :
       std::filesystem::directory_iterator(root, ec)) {
    if (ec) {
      break;
    }
    if (!entry.is_directory()) {
      continue;
    }
    if (LoadUnpacked(entry.path().wstring())) {
      ++loaded;
    }
  }
  return loaded;
}

bool ExtensionManager::Unload(const std::string& extension_id) {
  CEF_REQUIRE_UI_THREAD();

  if (!context_ || !context_->HasExtension(extension_id)) {
    return false;
  }

  auto extension = context_->GetExtension(extension_id);
  if (!extension || !extension->IsLoaded()) {
    return false;
  }

  extension->Unload();
  return true;
}

}  // namespace maenbrowser::extensions
