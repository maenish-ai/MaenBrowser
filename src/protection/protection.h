#pragma once
#include <atomic>
#include <filesystem>
#include <memory>
#include <string>
#include "include/cef_request_context_handler.h"
#include "include/cef_values.h"
#include "src/protection/domain_rules.h"

namespace maenbrowser::protection {
struct Settings {
  bool ads = true;
  bool family = false;
  bool allow_only = true;
  bool adult = true;
  bool violence = true;
  bool safe_search = true;
  bool ask_download = true;
  bool https_only = false;
  bool corrupt = false;
  std::wstring download_directory;
  DomainRules exceptions, allowed, blocked;
  std::string salt, pin_hash;
};
struct Url {
  std::string scheme, host, path, query;
  bool valid = false;
};
Url ParseUrl(const std::string& url);
std::filesystem::path InstallDirectory();
std::string ControlsUrl(const char* page = "options.html");
void Initialize();
std::shared_ptr<const Settings> Current();
bool Block(const std::string& url, const std::string& source, bool document);
bool BlockNavigation(const std::string& url);
std::string Rewrite(const std::string& url);
std::string Api(const std::string& body);
CefRefPtr<CefRequestContextHandler> ContextHandler();
std::wstring DownloadPath(const std::wstring& suggested_name);
bool FamilyEnabled();
}
