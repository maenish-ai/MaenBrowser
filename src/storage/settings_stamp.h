#pragma once
#include <filesystem>
namespace maenbrowser::storage {
struct SettingsStamp {
  bool valid=true, exists=false;
  std::filesystem::file_time_type modified{};
  std::uintmax_t bytes=0;
  bool operator==(const SettingsStamp&) const = default;
};
inline SettingsStamp ReadSettingsStamp(const std::filesystem::path& path) {
  SettingsStamp stamp;std::error_code ec;
  stamp.exists=std::filesystem::exists(path,ec);
  if(ec){stamp.valid=false;return stamp;}
  if(!stamp.exists)return stamp;
  stamp.modified=std::filesystem::last_write_time(path,ec);
  if(ec){stamp.valid=false;return stamp;}
  stamp.bytes=std::filesystem::file_size(path,ec);
  stamp.valid=!ec;return stamp;
}
inline bool SettingsChanged(const SettingsStamp& previous,const SettingsStamp& current) {
  return !previous.valid||!current.valid||previous!=current;
}
}
