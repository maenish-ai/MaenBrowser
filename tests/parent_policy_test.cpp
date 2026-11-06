#include <cassert>
#include "src/protection/parent_policy.h"
using namespace maenbrowser::protection;
int main(){
 assert(RequiresParentPin(true,true,{"ads"},false));
 assert(!RequiresParentPin(false,true,{"ads","askDownload","exceptions"},false));
 for(const auto& key:{"family","allowOnly","adult","violence","safeSearch","allowed","blocked"})assert(RequiresParentPin(false,true,{key},false));
 assert(RequiresParentPin(false,true,{},true));
 assert(!RequiresParentPin(false,false,{"family"},true));
}
