#include "src/protection/filter_snapshot.h"
#include <cassert>
#include <iomanip>
int main(){
 using namespace maenbrowser::protection;
 DomainRules rules;
 assert(!ParseAdSnapshot("example.com\n",rules));
 assert(!ParseAdSnapshot("<html>error</html>",rules));
 std::ostringstream out;
 for(int i=0;i<10000;++i)out << "ad" << std::setw(5) << std::setfill('0') << i << ".example\n";
 auto text=out.str();
 assert(ParseAdSnapshot(text,rules));assert(rules.Size()==10000);
 assert(rules.Contains("sub.ad00001.example"));
 assert(!ParseAdSnapshot(text+"ad00001.example\n",rules));
 assert(rules.Size()==10000); // Rejected snapshot must not replace previous rules.
 assert(!ParseAdSnapshot(text+"invalid domain\n",rules));
 assert(!ParseAdSnapshot(std::string(4*1024*1024+1,'a'),rules));
}
