#include "src/protection/control_origin.h"
#include <cassert>
#include <string>
using namespace maenbrowser::protection;
int main() {
 const auto page=std::string(kExtensionOrigin)+"/start.html";
 assert(TrustedControlSource(kExtensionOrigin, "", "null"));
 assert(TrustedControlSource("", page, "null"));
 assert(TrustedControlSource("null", page, ""));
 assert(TrustedControlSource("", "", kExtensionOrigin));
 for(const auto* origin : {"", "null", "https://evil.example", "chrome-extension://other"})
   assert(!TrustedControlSource("", "", origin));
 assert(!TrustedControlSource("https://evil.example",page,kExtensionOrigin));
 assert(!TrustedControlSource("null","https://evil.example",kExtensionOrigin));
 for(const auto& url : {std::string(kExtensionOrigin)+".evil/start.html", std::string(kExtensionOrigin)+"@evil/start.html", std::string("https://evil.example/?url=")+page})
   assert(!TrustedControlSource("",url,"null"));
}
