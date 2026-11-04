#pragma once
#include <filesystem>
#include <cwchar>
namespace maenbrowser::storage {
inline std::filesystem::path AgentProfile(const std::filesystem::path& normal,
                                          const std::filesystem::path& requested) {
  const auto fallback=normal.parent_path()/"Agent User Data";
  if(!requested.is_absolute())return fallback;
#ifdef _WIN32
  if(requested.native().starts_with(L"\\\\"))return fallback;
#endif
  std::error_code ec;
  auto target=std::filesystem::weakly_canonical(requested,ec);
  if(ec)return fallback;
  auto primary=std::filesystem::weakly_canonical(normal,ec);
  if(ec)return fallback;
  // Refuse normal-profile overlap, including aliases resolved through links.
  auto a=primary.begin(),b=target.begin();
  for(;a!=primary.end()&&b!=target.end();++a,++b){
#ifdef _WIN32
    if(_wcsicmp(a->native().c_str(),b->native().c_str())!=0)break;
#else
    if(*a!=*b)break;
#endif
  }
  if(a==primary.end()||b==target.end())return fallback;
  return target;
}
}
