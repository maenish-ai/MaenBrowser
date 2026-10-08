#pragma once

#include <windows.h>
#include <string>
#include <functional>

namespace maenbrowser::media {

// Opens a WebView2 surface inside the current CEF tab's native content host.
// The WebView2 runtime is created lazily only when a routed media site is used.
bool OpenEmbeddedWebView2(int browser_id, HWND cef_window, const std::wstring& url,
                         std::function<void(const std::wstring&)> navigate);

// Tears down the on-demand WebView2 surface for a tab/browser. Safe to call
// repeatedly and when no WebView2 surface exists.
void CloseEmbeddedWebView2(int browser_id);

}  // namespace maenbrowser::media
