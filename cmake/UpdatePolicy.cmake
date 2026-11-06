# A dependency path is not an authenticated update configuration. Keep the
# dormant WinSparkle path locked until trust roots, signed metadata, a security
# floor and transactional install/rollback are implemented and tested together.
if(MAEN_ENABLE_UPDATER)
  message(FATAL_ERROR "Automatic updater is not release-ready: signed feed/key provisioning, anti-rollback security floor and verified install/rollback are required. Keep MAEN_ENABLE_UPDATER=OFF; do not ship the dormant unsigned path.")
endif()
