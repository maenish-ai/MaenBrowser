#include "src/protection/ad_shim_policy.h"
#include <cassert>
#include "src/protection/response_cors.h"
#include <string_view>
int main(){
 using namespace maenbrowser::protection;
 assert(std::string_view(ResponseCorsOrigin(true,false))==kExtensionOrigin);
 assert(std::string_view(ResponseCorsOrigin(true,true))==kExtensionOrigin);
 assert(ResponseCorsOrigin(false,false)==nullptr);
 assert(std::string_view(ResponseCorsOrigin(false,true))=="*");
 const auto choose=[](bool family,bool ads,bool blocked,bool script,const char* host,const char* path){return SelectAdShim(family,ads,blocked,script,"https",host,path);};
 assert(choose(false,true,true,true,"www.google-analytics.com","/analytics.js")==AdShim::Analytics);
 assert(choose(false,true,true,true,"securepubads.g.doubleclick.net","/tag/js/gpt.js")==AdShim::PublisherTag);
 assert(choose(true,true,true,true,"www.google-analytics.com","/analytics.js")==AdShim::None);
 assert(choose(false,false,true,true,"www.google-analytics.com","/analytics.js")==AdShim::None);
 assert(choose(false,true,false,true,"www.google-analytics.com","/analytics.js")==AdShim::None);
 assert(choose(false,true,true,false,"www.google-analytics.com","/analytics.js")==AdShim::None);
 assert(choose(false,true,true,true,"www.google-analytics.com.evil","/analytics.js")==AdShim::None);
 assert(choose(false,true,true,true,"www.google-analytics.com","/other.js")==AdShim::None);
}
