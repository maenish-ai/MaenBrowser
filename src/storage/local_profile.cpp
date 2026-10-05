#include "src/storage/local_profile.h"

#include <windows.h>
#include <shlobj.h>

#include <filesystem>

namespace maenbrowser::storage {
namespace {
std::wstring LocalAppData() {
  PWSTR raw = nullptr;
  if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &raw))) {
    return L".";
  }
  std::wstring result(raw);
  CoTaskMemFree(raw);
  return result;
}

std::wstring Ensure(const std::filesystem::path& p) {
  std::error_code ec;
  std::filesystem::create_directories(p, ec);
  return p.wstring();
}
}  // namespace

std::wstring GetProfileRoot() {
  return Ensure(std::filesystem::path(LocalAppData()) / L"MaenBrowser" / L"User Data");
}

std::wstring GetCachePath() {
  return Ensure(std::filesystem::path(GetProfileRoot()) / L"Default");
}

std::wstring GetExtensionsPath() {
  return Ensure(std::filesystem::path(GetProfileRoot()) / L"Extensions");
}

}  // namespace maenbrowser::storage
