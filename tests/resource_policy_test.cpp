#include "src/app/resource_policy.h"
#include <cassert>
int main(){
 using namespace maenbrowser::resource;
 assert(ChooseMode(2048,2,"auto")==Mode::UltraLite);
 assert(ChooseMode(4096,4,"auto")==Mode::Lite);
 assert(ChooseMode(8192,8,"auto")==Mode::Balanced);
 assert(ChooseMode(16384,8,"auto")==Mode::Performance);
 assert(ChooseMode(16384,4,"auto")==Mode::Lite);
 assert(ChooseMode(2048,2,"balanced")==Mode::Balanced);
 assert(ChooseMode(16384,8,"ultra")==Mode::UltraLite);
 assert(ChooseMode(4096,4,"invalid")==Mode::Lite);
}
