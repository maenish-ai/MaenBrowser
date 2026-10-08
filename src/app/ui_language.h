#pragma once
#include <windows.h>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <string>
#include "src/storage/local_profile.h"

namespace maenbrowser::ui {
inline std::atomic<int> language{-1};
inline std::filesystem::path LanguagePath() {
  return std::filesystem::path(storage::GetAppDataRoot()) / L"ui-language.txt";
}
inline bool Arabic() {
  int value=language.load();
  if(value<0){std::ifstream file(LanguagePath());std::string stored;file>>stored;
    value=stored=="ar"?1:0;language.store(value);}
  return value==1;
}
inline const char* Locale(){return Arabic()?"ar":"en-US";}
inline const wchar_t* Text(const wchar_t* english,const wchar_t* arabic){return Arabic()?arabic:english;}
inline bool SaveLanguage(const std::string& value){
  if(value!="ar"&&value!="en")return false;
  auto dest=LanguagePath();auto temp=dest;temp+=L".tmp";
  {std::ofstream out(temp,std::ios::binary|std::ios::trunc);out<<value;out.flush();if(!out.good())return false;}
  if(!MoveFileExW(temp.c_str(),dest.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))return false;
  language.store(value=="ar"?1:0);return true;
}
}
