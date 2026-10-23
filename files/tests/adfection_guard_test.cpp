#include "src/protection/adfection_guard.h"
#include <iostream>
#include <stdexcept>
using namespace maenbrowser::protection;
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
int main() {
  check(KnownAdFectionHost("fearevolve.com"), "Candiru C&C");
  check(KnownAdFectionHost("sub.jobmarcher.com"), "C&C subdomain");
  check(KnownAdFectionHost("keyadapter.com"), "validator");
  check(!KnownAdFectionHost("keyadapter.com.evil.test"), "domain boundary");
  check(!KnownAdFectionHost("notfearevolve.com"), "partial domain");
  check(KnownAdFectionLoader("noas.mtrtb.com", "/getAd"), "NOAS loader");
  check(KnownAdFectionLoader("events.platc-tester.com", "/cplatform/getAd"), "platform loader");
  check(!KnownAdFectionLoader("noas.mtrtb.com", "/help"), "do not block other ad routes");
  check(!KnownAdFectionLoader("tracker.mtrtb.com", "/getAd"), "do not block adjacent advertising domains");
  check(!KnownAdFectionHost("volasim.com"), "avoid collateral damage");
  check(!KnownAdFectionHost("akedo.gg"), "avoid collateral damage");
  check(!KnownAdFectionHost("example.com"), "normal pages");
  std::cout << "AdFection scoped indicators passed\n";
}
