#pragma once
#include <string_view>
namespace maenbrowser::protection {
enum class AdShim { None, Analytics, PublisherTag };
inline AdShim SelectAdShim(bool family,bool ads,bool blocked,bool script,
                          std::string_view scheme,std::string_view host,std::string_view path) {
  // Only replace an already blocked script. Exceptions and all family rules
  // take precedence. No URL regexes, impression pings, or arbitrary redirects.
  if(family||!ads||!blocked||!script||scheme!="https")return AdShim::None;
  if((host=="www.google-analytics.com"||host=="google-analytics.com"||host=="ssl.google-analytics.com")&&path=="/analytics.js")return AdShim::Analytics;
  if((host=="securepubads.g.doubleclick.net"||host=="www.googletagservices.com")&&path=="/tag/js/gpt.js")return AdShim::PublisherTag;
  return AdShim::None;
}
}
