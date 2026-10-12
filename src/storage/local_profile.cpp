#include "src/storage/local_profile.h"

#include <windows.h>
#include <shlobj.h>

#include <filesystem>

namespace maenbrowser::storage {
namespace {

std::wstring LocalAppData() {
  PWSTR raw = nullptr;
  if (FAILED(SHGetKnownFolderPath(
          FOLDERID_LocalAppData,
          KF_FLAG_CREATE,
          nullptr,
          &raw))) {
    return L".";
  }

  std::wstring result(raw);
  CoTaskMemFree(raw);
  return result;
}

std::wstring Ensure(const std::filesystem::path& path) {
  std::error_code ec;
  std::filesystem::create_directories(path, ec);
  return path.wstring();
}

}  // namespace

std::wstring GetAppDataRoot() {
  // Explicit process environment override for isolated CI/local integration tests.
  // Normal launches have no override and keep the existing user's profile path.
  wchar_t isolated[32768]{};
  const DWORD count = GetEnvironmentVariableW(L"MAEN_BROWSER_TEST_PROFILE", isolated, 32768);
  if (count > 0 && count < 32768 && std::filesystem::path(isolated).is_absolute())
    return Ensure(std::filesystem::path(isolated));
  return Ensure(
      std::filesystem::path(LocalAppData()) / L"MaenBrowser");
}

std::wstring GetProfileRoot() {
  return Ensure(
      std::filesystem::path(GetAppDataRoot()) / L"User Data");
}

std::wstring GetCachePath() {
  return Ensure(
      std::filesystem::path(GetProfileRoot()) / L"Default");
}

std::wstring GetExtensionsPath() {
  return Ensure(
      std::filesystem::path(GetProfileRoot()) / L"Extensions");
}

}  // namespace maenbrowser::storage
