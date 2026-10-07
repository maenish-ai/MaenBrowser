#include "src/media/webview2_media_router.h"
#include <windows.h>
#include <shellapi.h>
#include <filesystem>
#include <cwctype>

namespace maenbrowser::media {
namespace {
std::wstring Lower(std::wstring value) {
  for (auto& ch : value) ch = static_cast<wchar_t>(std::towlower(ch));
  return value;
}
}

bool IsWhatsAppWebUrl(const std::wstring& url) {
  const auto lower = Lower(url);
  return lower.rfind(L"https://web.whatsapp.com", 0) == 0 ||
         lower.rfind(L"https://whatsapp.com", 0) == 0 ||
         lower.rfind(L"http://web.whatsapp.com", 0) == 0;
}

bool OpenInMediaHost(const std::wstring& url) {
  wchar_t exe_path[MAX_PATH]{};
  if (!GetModuleFileNameW(nullptr, exe_path, MAX_PATH)) return false;
  std::filesystem::path host = std::filesystem::path(exe_path).parent_path() / L"MaenMediaHost.exe";
  if (!std::filesystem::exists(host)) return false;
  std::wstring args = L"\"" + url + L"\"";
  auto result = reinterpret_cast<INT_PTR>(ShellExecuteW(nullptr, L"open", host.c_str(), args.c_str(), nullptr, SW_SHOWNORMAL));
  return result > 32;
}
}
