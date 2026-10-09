#include "src/media/webview2_media_router.h"
#include "src/media/media_policy.h"
#include "src/protection/protection.h"

namespace maenbrowser::media {
bool IsWhatsAppWebUrl(const std::wstring& url) {
  const auto parsed = protection::ParseUrl(CefString(url).ToString());
  return parsed.scheme == "https" &&
         (parsed.host == "web.whatsapp.com" || parsed.host == "whatsapp.com" || parsed.host == "www.whatsapp.com");
}
bool IsDirectMediaUrl(const std::wstring& url) {
  const auto parsed = protection::ParseUrl(CefString(url).ToString());
  return parsed.valid && !parsed.host.empty() && IsDirectMediaPath(parsed.scheme, parsed.path);
}
bool UsesEmbeddedMedia(const std::wstring& url) {
  return IsWhatsAppWebUrl(url) || IsDirectMediaUrl(url);
}
}  // namespace maenbrowser::media
