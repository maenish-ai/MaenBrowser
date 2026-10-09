#include "src/protection/protection.h"
#include "src/app/ui_language.h"
#include <windows.h>
#include <bcrypt.h>
#include <wincrypt.h>
#include <shlobj.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cwctype>
#include <fstream>
#include <iterator>
#include <mutex>
#include <sstream>
#include "include/cef_parser.h"
#include "include/cef_resource_handler.h"
#include "include/cef_resource_request_handler.h"
#include "include/cef_request_context.h"
#include "include/cef_scheme.h"
#include "src/media/webview2_media_router.h"
#include "src/protection/identity.h"
#include "src/storage/local_profile.h"

namespace maenbrowser::protection {
namespace {
std::atomic<std::shared_ptr<const Settings>> g_settings{std::make_shared<const Settings>()};
DomainRules g_ads, g_adult, g_violence;
std::atomic<uint64_t> g_blocked{0};
std::mutex g_write;
std::chrono::steady_clock::time_point g_retry{};
std::string g_list_error;

std::string ReadFile(const std::filesystem::path& path, size_t max) {
  std::error_code ec;
  const auto size = std::filesystem::file_size(path, ec);
  if (ec || size > max) return {};
  std::ifstream f(path, std::ios::binary);
  return std::string(std::istreambuf_iterator<char>(f), {});
}
std::string Json(CefRefPtr<CefDictionaryValue> d) {
  auto v = CefValue::Create(); v->SetDictionary(d);
  return CefWriteJSON(v, JSON_WRITER_DEFAULT).ToString();
}
CefRefPtr<CefDictionaryValue> Dictionary(const std::string& text) {
  auto v = CefParseJSON(text, JSON_PARSER_RFC);
  return v && v->GetType() == VTYPE_DICTIONARY ? v->GetDictionary() : nullptr;
}
std::filesystem::path SettingsPath() {
  return std::filesystem::path(storage::GetAppDataRoot()) / L"controls.dat";
}
std::string Hex(const unsigned char* p, size_t n) {
  static const char chars[] = "0123456789abcdef";
  std::string r; r.reserve(n * 2);
  for (size_t i = 0; i < n; ++i) { r += chars[p[i] >> 4]; r += chars[p[i] & 15]; }
  return r;
}
std::string PinHash(const std::string& pin, const std::string& salt) {
  BCRYPT_ALG_HANDLE alg = nullptr;
  if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG) < 0) return {};
  unsigned char out[32]{};
  const NTSTATUS status = BCryptDeriveKeyPBKDF2(alg,
      reinterpret_cast<PUCHAR>(const_cast<char*>(pin.data())), static_cast<ULONG>(pin.size()),
      reinterpret_cast<PUCHAR>(const_cast<char*>(salt.data())), static_cast<ULONG>(salt.size()),
      210000, out, sizeof(out), 0);
  BCryptCloseAlgorithmProvider(alg, 0);
  return status < 0 ? "" : Hex(out, sizeof(out));
}
bool VerifyPin(const Settings& s, const std::string& pin) {
  if (pin.size() > 128 || s.pin_hash.empty()) return false;
  const auto h = PinHash(pin, s.salt);
  if (h.size() != s.pin_hash.size()) return false;
  unsigned char diff = 0;
  for (size_t i = 0; i < h.size(); ++i) diff |= h[i] ^ s.pin_hash[i];
  return diff == 0;
}
CefRefPtr<CefListValue> List(const DomainRules& r) {
  auto v = CefListValue::Create(); size_t i = 0;
  for (const auto& s : r.Values()) v->SetString(i++, s);
  return v;
}
bool ReadRules(CefRefPtr<CefDictionaryValue> d, const char* key, DomainRules& out) {
  if (!d->HasKey(key)) return true;
  auto list = d->GetList(key);
  if (!list || list->GetSize() > 1000) return false;
  std::vector<std::string> values;
  for (size_t i = 0; i < list->GetSize(); ++i) {
    if (list->GetType(i) != VTYPE_STRING) return false;
    auto s = CanonicalHost(list->GetString(i).ToString());
    if (!ValidDomain(s) || s.find('.') == std::string::npos) return false;
    values.push_back(s);
  }
  out.Assign(std::move(values)); return true;
}
CefRefPtr<CefDictionaryValue> Export(const Settings& s, bool secret) {
  auto d = CefDictionaryValue::Create();
  d->SetInt("schema", 1);
  d->SetString("language", ui::Arabic()?"ar":"en");
  d->SetBool("ads", s.ads); d->SetBool("family", s.family);
  d->SetBool("allowOnly", s.allow_only); d->SetBool("adult", s.adult);
  d->SetBool("violence", s.violence); d->SetBool("safeSearch", s.safe_search);
  d->SetBool("askDownload", s.ask_download); d->SetBool("httpsOnly", s.https_only);
  d->SetString("downloadDirectory", s.download_directory);
  d->SetList("exceptions", List(s.exceptions)); d->SetList("allowed", List(s.allowed));
  d->SetList("blocked", List(s.blocked));
  if (secret) { d->SetString("salt", s.salt); d->SetString("pinHash", s.pin_hash); }
  else {
    d->SetBool("hasPin", !s.pin_hash.empty()); d->SetBool("lockedFile", s.corrupt);
    d->SetString("blockedRequests", std::to_string(g_blocked.load()));
    d->SetInt("adDomains", static_cast<int>(g_ads.Size()));
    d->SetInt("adultDomains", static_cast<int>(g_adult.Size()));
    d->SetInt("violenceDomains", static_cast<int>(g_violence.Size()));
    d->SetString("listError", g_list_error);
    d->SetString("version", "1.8.2");
  }
  return d;
}
bool Import(CefRefPtr<CefDictionaryValue> d, Settings& s) {
  const auto b = [&](const char* key, bool& value) {
    if (!d->HasKey(key)) return true;
    if (d->GetType(key) != VTYPE_BOOL) return false;
    value = d->GetBool(key); return true;
  };
  if (!b("ads", s.ads) || !b("family", s.family) || !b("allowOnly", s.allow_only) ||
      !b("adult", s.adult) || !b("violence", s.violence) || !b("safeSearch", s.safe_search) ||
      !b("askDownload", s.ask_download) || !b("httpsOnly", s.https_only)) return false;
  if (d->HasKey("downloadDirectory")) {
    if (d->GetType("downloadDirectory") != VTYPE_STRING) return false;
    auto path = d->GetString("downloadDirectory").ToWString();
    if (path.size() > 240 || path.find(L'\0') != std::wstring::npos || (!path.empty() && !std::filesystem::path(path).is_absolute())) return false;
    s.download_directory = path;
  }
  return ReadRules(d, "exceptions", s.exceptions) && ReadRules(d, "allowed", s.allowed) && ReadRules(d, "blocked", s.blocked);
}
bool Save(const Settings& s) {
  auto json = Json(Export(s, true));
  DATA_BLOB input{static_cast<DWORD>(json.size()), reinterpret_cast<BYTE*>(json.data())}, out{};
  if (!CryptProtectData(&input, L"MaenBrowser local controls", nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &out)) return false;
  auto tmp = SettingsPath(); tmp += L".tmp";
  bool ok = false;
  {
    std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
    if (f) { f.write(reinterpret_cast<char*>(out.pbData), out.cbData); f.flush(); ok = f.good(); }
  }
  SecureZeroMemory(out.pbData, out.cbData); LocalFree(out.pbData);
  if (ok) ok = MoveFileExW(tmp.c_str(), SettingsPath().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE;
  if (!ok) { std::error_code ec; std::filesystem::remove(tmp, ec); }
  return ok;
}
void LoadDomains(const char* name, DomainRules& rules) {
  const auto path = InstallDirectory() / L"filters" / name;
  auto text = ReadFile(path, 16 * 1024 * 1024);
  std::istringstream in(text); std::string line; std::vector<std::string> values;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (ValidDomain(line)) values.push_back(std::move(line));
  }
  rules.Assign(std::move(values));
  if (rules.Size() == 0) g_list_error += std::string(name) + " missing or empty. ";
}
std::string Error(const char* message) {
  auto d = CefDictionaryValue::Create(); d->SetBool("ok", false); d->SetString("error", message); return Json(d);
}
bool Own(const Url& u) { return u.scheme == "chrome-extension" && u.host == kExtensionId; }

class Response final : public CefResourceHandler {
 public:
  Response(std::string data, std::string mime, int status = 200, bool api = false, bool cors = false)
      : data_(std::move(data)), mime_(std::move(mime)), status_(status), api_(api), cors_(cors) {}
  bool Open(CefRefPtr<CefRequest>, bool& handled, CefRefPtr<CefCallback>) override {
    if (api_) { data_ = Api(data_); api_ = false; }
    handled = true; return true;
  }
  void GetResponseHeaders(CefRefPtr<CefResponse> r, int64_t& length, CefString&) override {
    r->SetStatus(status_); r->SetMimeType(mime_); length = static_cast<int64_t>(data_.size());
    r->SetHeaderByName("Cache-Control", "no-store", true);
    r->SetHeaderByName("X-Content-Type-Options", "nosniff", true);
    r->SetHeaderByName("Content-Security-Policy", "default-src 'none'; style-src 'unsafe-inline'; frame-ancestors 'none'", true);
    if (cors_) {
      r->SetHeaderByName("Access-Control-Allow-Origin", kExtensionOrigin, true);
      r->SetHeaderByName("Access-Control-Allow-Headers", "Content-Type", true);
      r->SetHeaderByName("Access-Control-Allow-Methods", "POST, OPTIONS", true);
    }
  }
  bool Read(void* out, int count, int& read, CefRefPtr<CefResourceReadCallback>) override {
    read = static_cast<int>(std::min<size_t>(std::max(0, count), data_.size() - offset_));
    if (read) { std::memcpy(out, data_.data() + offset_, read); offset_ += read; }
    return read > 0;
  }
  void Cancel() override {}
 private:
  std::string data_, mime_; int status_; bool api_, cors_; size_t offset_ = 0;
  IMPLEMENT_REFCOUNTING(Response);
};
std::string BlockedPage() {
  return ui::Arabic()
    ? "<!doctype html><html lang=ar dir=rtl><meta charset=utf-8><title>حماية المتصفح</title><h1>حماية معن براوزر</h1><p>تم حجب هذا العنوان وفق إعدادات الحماية. افتح إعدادات الحماية لمراجعته. تغيير إعدادات الأسرة يحتاج رمز الوالدين.</p></html>"
    : "<!doctype html><html lang=en><meta charset=utf-8><title>Browser protection</title><h1>MaenBrowser protection</h1><p>This address is blocked by your protection settings. Open protection settings to review it. Family changes need the parent PIN.</p></html>";
}

class Resource final : public CefResourceRequestHandler {
 public:
  Resource(std::string source, bool document, bool internal, bool trusted, bool media)
      : source_(std::move(source)), document_(document), internal_(internal), trusted_(trusted), media_(media) {}
  ReturnValue OnBeforeResourceLoad(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>, CefRefPtr<CefRequest> r, CefRefPtr<CefCallback>) override {
    if (internal_) return RV_CONTINUE;
    denied_ = Block(r->GetURL().ToString(), source_, document_);
    if (denied_) return document_ ? RV_CONTINUE : RV_CANCEL;
    auto url = Rewrite(r->GetURL().ToString());
    if (url != r->GetURL().ToString()) r->SetURL(url);
    return RV_CONTINUE;
  }
  CefRefPtr<CefResourceHandler> GetResourceHandler(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>, CefRefPtr<CefRequest> r) override {
    if (internal_) {
      const auto u = ParseUrl(r->GetURL().ToString());
      if (!trusted_ || u.scheme != "https" || u.path != "/api") return new Response("Forbidden", "text/plain", 403);
      if (r->GetMethod() == "OPTIONS") return new Response("", "text/plain", 204, false, true);
      if (r->GetMethod() != "POST") return new Response("Method not allowed", "text/plain", 405, false, true);
      auto post = r->GetPostData();
      if (!post) return new Response(Error("Missing request"), "application/json", 400, false, true);
      CefPostData::ElementVector elements; post->GetElements(elements); std::string body;
      for (const auto& e : elements) {
        if (e->GetType() != PDE_TYPE_BYTES || e->GetBytesCount() > 65536 || body.size() + e->GetBytesCount() > 65536)
          return new Response(Error("Request too large"), "application/json", 413, false, true);
        const auto n = body.size(); body.resize(n + e->GetBytesCount()); e->GetBytes(e->GetBytesCount(), body.data() + n);
      }
      return new Response(body, "application/json", 200, true, true);
    }
    if (denied_) return new Response(BlockedPage(), "text/html", 403);
    // Commit a lightweight CEF document at the real media URL. This gives
    // Chrome's address bar/back stack the correct URL before attaching WebView2.
    if (media_) return new Response(ui::Arabic()?"<!doctype html><meta charset=utf-8><title>الوسائط</title><p dir=rtl>جارٍ فتح محرك الوسائط… إذا تعذر الفتح، أعد تحميل الصفحة أو شغّل التثبيت لإصلاح محرك الوسائط.</p>":"<!doctype html><meta charset=utf-8><title>Media</title><p>Opening the media engine… If it does not open, reload the page or run Setup to repair the media engine.</p>", "text/html");
    return nullptr;
  }
  void OnProtocolExecution(CefRefPtr<CefBrowser>, CefRefPtr<CefFrame>, CefRefPtr<CefRequest>, bool& allow) override { allow = false; }
 private:
  std::string source_; bool document_, internal_, trusted_, media_, denied_ = false;
  IMPLEMENT_REFCOUNTING(Resource);
};
class Context final : public CefRequestContextHandler {
 public:
  CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(CefRefPtr<CefBrowser> browser,
      CefRefPtr<CefFrame>, CefRefPtr<CefRequest> r, bool navigation, bool download,
      const CefString& initiator, bool& disable) override {
    const auto u = ParseUrl(r->GetURL().ToString());
    const bool internal = u.host == "maen.browser";
    if (internal) disable = true;  // Never send control traffic to the public network.
    std::string source = initiator.ToString();
    const bool trusted = source == kExtensionOrigin;
    if (browser && browser->GetMainFrame()) source = browser->GetMainFrame()->GetURL().ToString();
    const bool document = navigation || r->GetResourceType() == RT_MAIN_FRAME || r->GetResourceType() == RT_SUB_FRAME;
    const bool media = browser && !download && r->GetMethod() == "GET" &&
        r->GetResourceType() == RT_MAIN_FRAME &&
        !browser->GetHost()->GetRequestContext()->GetCachePath().empty() &&
        media::UsesEmbeddedMedia(r->GetURL().ToWString());
    return new Resource(source, document, internal, trusted, media);
  }
  IMPLEMENT_REFCOUNTING(Context);
};
// A profile-level fallback prevents control requests from ever reaching DNS,
// including early extension/service-worker requests without a frame handler.
// The context path verifies the initiator; worker fallback verifies Fetch Origin.
class NoNetworkFactory final : public CefSchemeHandlerFactory {
 public:
  CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
      const CefString&, CefRefPtr<CefRequest> request) override {
    // Service workers may have no frame-associated CefRequestContextHandler.
    // Origin is a browser-controlled forbidden Fetch header. No web page can
    // select the companion's chrome-extension origin. There is no HTTP server.
    const bool trusted = request->GetHeaderByName("Origin") == kExtensionOrigin;
    CefRefPtr<Resource> handler = new Resource("", false, true, trusted, false);
    return handler->GetResourceHandler(browser, frame, request);
  }
  IMPLEMENT_REFCOUNTING(NoNetworkFactory);
};
}  // namespace

Url ParseUrl(const std::string& url) {
  CefURLParts p; Url out;
  if (!CefParseURL(url, p)) return out;
  out.valid = true; out.scheme = CefString(&p.scheme).ToString();
  out.host = CanonicalHost(CefString(&p.host).ToString());
  out.path = CefString(&p.path).ToString(); out.query = CefString(&p.query).ToString();
  return out;
}
std::filesystem::path InstallDirectory() {
  wchar_t buffer[32768]{};
  DWORD n = GetModuleFileNameW(nullptr, buffer, 32768);
  return std::filesystem::path(std::wstring(buffer, n)).parent_path();
}
std::string ControlsUrl(const char* page) { return std::string(kExtensionOrigin) + "/" + page; }
std::shared_ptr<const Settings> Current() { return g_settings.load(); }
bool FamilyEnabled() { return Current()->family; }
void Initialize() {
  CefRegisterSchemeHandlerFactory("https", "maen.browser", new NoNetworkFactory());
  LoadDomains("ads.txt", g_ads); LoadDomains("adult.txt", g_adult); LoadDomains("violence.txt", g_violence);
  auto s = std::make_shared<Settings>();
  PWSTR dir = nullptr;
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Downloads, 0, nullptr, &dir))) { s->download_directory = dir; CoTaskMemFree(dir); }
  std::error_code ec;
  if (std::filesystem::exists(SettingsPath(), ec)) {
    auto bytes = ReadFile(SettingsPath(), 1024 * 1024);
    DATA_BLOB in{static_cast<DWORD>(bytes.size()), reinterpret_cast<BYTE*>(bytes.data())}, out{};
    bool valid = CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &out) != FALSE;
    if (valid) {
      auto d = Dictionary(std::string(reinterpret_cast<char*>(out.pbData), out.cbData));
      valid = d && d->GetInt("schema") == 1 && Import(d, *s);
      if (valid) { s->salt = d->GetString("salt").ToString(); s->pin_hash = d->GetString("pinHash").ToString(); }
      SecureZeroMemory(out.pbData, out.cbData); LocalFree(out.pbData);
    }
    if (!valid || (s->family && s->pin_hash.empty())) { s = std::make_shared<Settings>(); s->family = true; s->corrupt = true; }
  }
  g_settings.store(s);
}
bool BlockNavigation(const std::string& url) {
  const auto s = Current(); if (!s->family) return false;
  const auto u = ParseUrl(url);
  if (Own(u)) return false;
  // An empty placeholder is needed by Chrome Runtime for newly-created tabs.
  if (url == "about:blank") return false;
  return Block(url, "", true);
}
bool Block(const std::string& url, const std::string& source, bool document) {
  const auto s = Current();
  if (!s->family && !s->ads) return false;
  const auto u = ParseUrl(url);
  if (Own(u)) return false;
  bool denied = false;
  if (s->family) {
    if (document && u.scheme != "https" && u.scheme != "http" && url != "about:blank") denied = true;
    if (s->blocked.Contains(u.host)) denied = true;
    if (!s->allowed.Contains(u.host)) {
      if (document && s->allow_only) denied = true;
      if ((s->adult && g_adult.Contains(u.host)) || (s->violence && g_violence.Contains(u.host))) denied = true;
    }
    if (!g_list_error.empty() && !s->allow_only && document) denied = true;
  }
  // Most resources are not advertising domains. Avoid parsing the source URL
  // or searching exceptions unless the ad list actually matches.
  if (!denied && s->ads && g_ads.Contains(u.host) && !s->exceptions.Contains(u.host))
    denied = s->exceptions.Size() == 0 || !s->exceptions.Contains(ParseUrl(source).host);
  if (denied) ++g_blocked;
  return denied;
}
std::string Rewrite(const std::string& url) {
  const auto s = Current();
  if (!s->https_only && !(s->family && s->safe_search)) return url;
  auto u = ParseUrl(url); std::string target = url;
  if (s->https_only && u.scheme == "http") target.replace(0, 4, "https");
  if (!(s->family && s->safe_search) || (u.scheme != "http" && u.scheme != "https")) return target;
  std::string key, val;
  if ((u.host == "www.google.com" || u.host == "google.com") && u.path == "/search") { key = "safe"; val = "active"; }
  else if ((u.host == "www.bing.com" || u.host == "bing.com") && u.path == "/search") { key = "adlt"; val = "strict"; }
  else if (u.host == "duckduckgo.com" || u.host == "www.duckduckgo.com") { key = "kp"; val = "1"; }
  else return target;
  const auto hash = target.find('#'); const auto fragment = hash == std::string::npos ? "" : target.substr(hash);
  if (hash != std::string::npos) target.resize(hash);
  const auto q = target.find('?'); std::string prefix = target.substr(0, q), args;
  if (q != std::string::npos) {
    std::istringstream in(target.substr(q + 1)); std::string item;
    while (std::getline(in, item, '&')) {
      auto raw = item.substr(0, item.find('='));
      auto decoded = CefURIDecode(raw, true, static_cast<cef_uri_unescape_rule_t>(UU_NORMAL | UU_SPACES | UU_URL_SPECIAL_CHARS_EXCEPT_PATH_SEPARATORS)).ToString();
      if (CanonicalHost(decoded) == key) continue;
      if (!item.empty()) args += item + "&";
    }
  }
  return prefix + "?" + args + key + "=" + val + fragment;
}
std::string Api(const std::string& body) {
  if (body.size() > 65536) return Error("Request too large");
  auto d = Dictionary(body); if (!d) return Error("Invalid JSON");
  const auto op = d->GetString("op").ToString();
  if (op == "get") {
    auto out = Export(*Current(), false); out->SetBool("ok", true); return Json(out);
  }
  if (op == "setLanguage") {
    std::lock_guard<std::mutex> lock(g_write);
    if (!ui::SaveLanguage(d->GetString("language").ToString())) return Error("Could not save language.");
    auto out = Export(*Current(), false); out->SetBool("ok", true); return Json(out);
  }
  if (op != "save") return Error("Unknown operation");
  std::lock_guard<std::mutex> lock(g_write);
  auto old = Current();
  if (old->corrupt) return Error("Protected settings cannot be read. Restore your profile backup or reinstall with a new profile.");
  auto patch = d->GetDictionary("settings"); if (!patch) return Error("Missing settings");
  if (old->family || (!old->pin_hash.empty() && (patch->HasKey("family") || d->HasKey("newPin")))) {
    if (std::chrono::steady_clock::now() < g_retry) return Error("Please wait before trying your PIN again.");
    if (!VerifyPin(*old, d->GetString("pin").ToString())) {
      g_retry = std::chrono::steady_clock::now() + std::chrono::seconds(5); return Error("Incorrect parent PIN.");
    }
  }
  auto next = std::make_shared<Settings>(*old);
  if (!Import(patch, *next)) return Error("Invalid settings. Use domain names only and an absolute download folder.");
  if (d->HasKey("newPin")) {
    auto pin = d->GetString("newPin").ToString();
    if (pin.size() < 6 || pin.size() > 128) return Error("Use a parent PIN or passphrase of 6 to 128 characters.");
    unsigned char salt[16]{};
    if (BCryptGenRandom(nullptr, salt, sizeof(salt), BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0) return Error("Windows random generator failed.");
    next->salt = Hex(salt, sizeof(salt)); next->pin_hash = PinHash(pin, next->salt);
    if (next->pin_hash.empty()) return Error("PIN protection failed.");
  }
  if (next->family && next->pin_hash.empty()) return Error("Set a parent PIN before enabling family mode.");
  if (next->family && !next->allow_only && !g_list_error.empty()) return Error("Family category lists are missing. Repair installation or use allowed-sites-only mode.");
  if (!Save(*next)) return Error("Could not save protected settings. No changes were applied.");
  g_settings.store(next); auto out = Export(*next, false); out->SetBool("ok", true); return Json(out);
}
CefRefPtr<CefRequestContextHandler> ContextHandler() { return new Context(); }
std::wstring DownloadPath(const std::wstring& suggested) {
  const auto s = Current(); if (s->ask_download || s->download_directory.empty()) return L"";
  const auto directory = std::filesystem::path(s->download_directory);
  std::error_code ec; std::filesystem::create_directories(directory, ec); if (ec) return L"";
  auto clean = std::filesystem::path(suggested).filename().wstring();
  for (auto& c : clean) if (c < 32 || std::wstring(L"<>:\"/\\|?*").find(c) != std::wstring::npos) c = L'_';
  while (!clean.empty() && (clean.back() == L'.' || clean.back() == L' ')) clean.pop_back();
  if (clean.empty()) clean = L"download";
  // Reserve enough room for collision suffixes and avoid device path aliases.
  if (clean.size() > 160) clean.resize(160);
  auto stem = std::filesystem::path(clean).stem().wstring();
  std::transform(stem.begin(), stem.end(), stem.begin(), ::towupper);
  if (stem == L"CON" || stem == L"PRN" || stem == L"AUX" || stem == L"NUL" ||
      (stem.size() == 4 && (stem.starts_with(L"COM") || stem.starts_with(L"LPT")) && stem.back() >= L'1' && stem.back() <= L'9')) clean = L"_" + clean;
  const std::filesystem::path name(clean);
  // Do not overwrite an existing file when automatic saving is selected.
  for (int n = 0; n < 10000; ++n) {
    const auto candidate = directory / (n == 0 ? name : std::filesystem::path(name.stem().wstring() + L" (" + std::to_wstring(n) + L")" + name.extension().wstring()));
    if (!std::filesystem::exists(candidate, ec) && !ec) return candidate.wstring();
  }
  return L"";
}
}  // namespace maenbrowser::protection
