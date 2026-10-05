#pragma once
#include "include/cef_request_context.h"
namespace maenbrowser::preferences {
void ApplyLocalBrowserPreferences(CefRefPtr<CefRequestContext> context);
}
