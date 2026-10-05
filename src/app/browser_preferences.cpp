#include "src/app/browser_preferences.h"
#include <windows.h>
#include <filesystem>
#include "include/cef_values.h"

namespace maenbrowser::preferences {
namespace {
std::wstring DownloadsDirectory() {
  wchar_t path[MAX_PATH]{};
  DWORD n = GetEnvironmentVariableW(L"USERPROFILE", path, MAX_PATH);
  if (n > 0 && n < MAX_PATH) return (std::filesystem::path(path) / L"Downloads").wstring();
  return L"";
}
void Set(CefRefPtr<CefRequestContext> c, const char* key, CefRefPtr<CefValue> v) {
  CefString error;
  c->SetPreference(key, v, error);
}
}
void ApplyLocalBrowserPreferences(CefRefPtr<CefRequestContext> context) {
  if (!context) return;
  auto v = CefValue::Create();
  v->SetBool(true);
  Set(context, "download.prompt_for_download", v);

  const auto downloads = DownloadsDirectory();
  if (!downloads.empty()) {
    v = CefValue::Create();
    v->SetString(downloads);
    Set(context, "download.default_directory", v);
  }

  v = CefValue::Create();
  v->SetBool(true);
  Set(context, "browser.show_home_button", v);
}
}
