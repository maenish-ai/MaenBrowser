#pragma once

namespace maenbrowser::updater {

class UpdateManager {
 public:
  static void Initialize();
  static void CheckNow();
  static void Shutdown();
};

}  // namespace maenbrowser::updater
