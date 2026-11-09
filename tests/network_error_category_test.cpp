#include <cassert>
#include "src/protection/network_error_category.h"
using maenbrowser::protection::NetworkErrorCategory;
int main(){
  assert(NetworkErrorCategory(0).empty());assert(NetworkErrorCategory(-3).empty());assert(NetworkErrorCategory(-20).empty());
  assert(NetworkErrorCategory(-21)=="network_changed");assert(NetworkErrorCategory(-106)=="offline");
  assert(NetworkErrorCategory(-105)=="dns");assert(NetworkErrorCategory(-803)=="dns");assert(NetworkErrorCategory(-130)=="proxy");
  assert(NetworkErrorCategory(-111)=="proxy");assert(NetworkErrorCategory(-118)=="timeout");
  assert(NetworkErrorCategory(-202)=="certificate");assert(NetworkErrorCategory(-5)=="other");
}
