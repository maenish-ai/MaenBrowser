#include "src/updater/update_manager.h"

#if MAEN_ENABLE_UPDATER
#include <winsparkle.h>
#endif

namespace maenbrowser::updater {

void UpdateManager::Initialize() {
#if MAEN_ENABLE_UPDATER
  // Production values are injected when a release feed and signing key exist.
  // Never ship an updater feed over plain HTTP.
  win_sparkle_set_app_details(L"MaenBrowser", L"MaenBrowser", L"1.0.0");
  win_sparkle_set_automatic_check_for_updates(1);
  win_sparkle_set_update_check_interval(24 * 60 * 60);
  win_sparkle_init();
#endif
}

void UpdateManager::CheckNow() {
#if MAEN_ENABLE_UPDATER
  win_sparkle_check_update_with_ui();
#endif
}

void UpdateManager::Shutdown() {
#if MAEN_ENABLE_UPDATER
  win_sparkle_cleanup();
#endif
}

}  // namespace maenbrowser::updater
