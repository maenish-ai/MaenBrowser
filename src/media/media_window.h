#pragma once
#include <string>
namespace maenbrowser::media {
// Called on CEF UI thread. A separate STA owns every WebView2 COM object.
bool OpenMediaWindow(const std::wstring& url);
// Join before CefShutdown: media policy still uses CEF's URL parser.
void ShutdownMediaWindow();
}
