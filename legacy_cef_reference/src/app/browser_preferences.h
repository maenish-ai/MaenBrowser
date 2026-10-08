#pragma once
#include "include/cef_preference.h"

namespace maenbrowser::preferences {
void ApplyLocalBrowserPreferences(CefRefPtr<CefPreferenceManager> manager);
}
