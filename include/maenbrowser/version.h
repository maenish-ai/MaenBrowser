#pragma once

namespace maenbrowser {
inline constexpr char kProductName[] = "MaenBrowser";
#ifndef MAEN_BROWSER_VERSION
#define MAEN_BROWSER_VERSION "1.13.0"
#endif
inline constexpr char kVersion[] = MAEN_BROWSER_VERSION;
inline constexpr char kHomepage[] = "maen://newtab";
inline constexpr char kRepositoryUrl[] = "https://github.com/maenish-ai/MaenBrowser";
}  // namespace maenbrowser
