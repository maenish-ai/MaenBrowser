#include <cassert>
#include "src/protection/security_snapshot.h"
using namespace maenbrowser::protection;
int main(){uint64_t n=0;assert(ParseSecuritySequence("2026110601",n));assert(n==2026110601);
for(const auto& bad:{"0","01","-2","2junk","9007199254740992",""})assert(!ParseSecuritySequence(bad,n));
DomainRules rules;assert(ParseSecurityDomains("a.example\nb.example\n",2,rules));assert(rules.Contains("a.example"));
assert(!ParseSecurityDomains("a.example\na.example\n",2,rules));assert(!ParseSecurityDomains("b.example\na.example\n",2,rules));
assert(!ParseSecurityDomains("a.example",1,rules));assert(!ParseSecurityDomains("a.example\n",2,rules));assert(!ParseSecurityDomains("https://evil.example\n",1,rules));}
