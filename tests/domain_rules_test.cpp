#include "src/protection/domain_rules.h"
#include <iostream>
#include <stdexcept>
using namespace maenbrowser::protection;
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
int main() {
  DomainRules r; r.Assign({"example.com", "bad.example", "example.com", "invalid/path", "", ".com"});
  check(r.Size() == 2, "validation/dedup");
  check(r.Contains("example.com"), "exact match");
  check(r.Contains("a.b.example.com"), "subdomains");
  check(!r.Contains("notexample.com"), "label boundary");
  check(!r.Contains("example.com.evil.test"), "suffix spoof");
  check(!r.Contains("example.com@evil.test"), "userinfo spoof");
  check(CanonicalHost("WEB.WhatsApp.COM.") == "web.whatsapp.com", "canonicalization");
  check(HostMatches("www.example.com", "example.com"), "suffix match");
  check(!HostMatches("example.com.evil.test", "example.com"), "host boundary");
  check(!ValidDomain("https://example.com"), "URL not domain");
  check(!ValidDomain("example..com"), "empty label");
  check(!ValidDomain(std::string(64, 'a') + ".test"), "long label");
  std::cout << "Domain boundary and normalization tests passed\n";
}
