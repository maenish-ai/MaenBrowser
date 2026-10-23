#include "src/protection/always_on_shield.h"
#include <stdexcept>
#include <iostream>
using namespace maenbrowser::protection;
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
int main() {
  DomainRules optional;
  optional.Assign({"example-threat.test"});
  check(SecurityThreatUrl("fearevolve.com", "/", optional), "always-on known IOC");
  check(SecurityThreatUrl("events.myweb3id.io", "/cplatform/getAd", optional), "loader");
  check(SecurityThreatUrl("example-threat.test", "/", optional), "extensible IOC");
  check(!SecurityThreatUrl("example.com", "/", optional), "normal site");
  check(!SecurityThreatUrl("noas.mtrtb.com", "/normal-ad", optional), "ordinary ad not blocked");
  check(!SecurityThreatUrl("noas.mtrtb.com.evil.test", "/getAd", optional), "host spoof");
  check(BlockUnsolicitedPopup(false), "automatic popup is blocked");
  check(!BlockUnsolicitedPopup(true), "user popup permitted");
  // These APIs deliberately do not receive an ad-blocking setting. A user
  // may turn ads off/on without changing any of these results.
  std::cout << "Independent security shield passed\n";
}
