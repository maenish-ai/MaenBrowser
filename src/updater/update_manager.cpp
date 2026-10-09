#include "src/updater/update_manager.h"
#include "include/maenbrowser/version.h"

#if MAEN_ENABLE_UPDATER
#include <winsparkle.h>
#endif

namespace maenbrowser::updater {

void UpdateManager::Initialize() {
#if MAEN_ENABLE_UPDATER
  // Keep disabled in public CI until an HTTPS appcast and signing key are
  // configured. Never auto-install unsigned browser updates.
  win_sparkle_set_app_details(
      L"MaenBrowser",
      L"MaenBrowser",
      L"1.8.4");
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
