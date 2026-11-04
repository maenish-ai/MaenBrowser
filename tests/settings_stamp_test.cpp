#include "src/storage/settings_stamp.h"
#include <cassert>
#include <fstream>
int main(){
 namespace fs=std::filesystem;using namespace maenbrowser::storage;
 const auto path=fs::temp_directory_path()/"maen-stamp-test.dat";
 fs::remove(path);
 const auto missing=ReadSettingsStamp(path);
 assert(!SettingsChanged(missing,ReadSettingsStamp(path)));
 {std::ofstream f(path);f<<"initial";}
 const auto first=ReadSettingsStamp(path);
 assert(SettingsChanged(missing,first));
 assert(!SettingsChanged(first,ReadSettingsStamp(path)));
 {std::ofstream f(path);f<<"parent policy changed";}
 assert(SettingsChanged(first,ReadSettingsStamp(path)));
 SettingsStamp denied;denied.valid=false;
 assert(SettingsChanged(first,denied));
 fs::remove(path);assert(SettingsChanged(first,ReadSettingsStamp(path)));
}
