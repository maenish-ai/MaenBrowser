#include "src/storage/agent_profile.h"
#include <cassert>
int main(){
 namespace fs=std::filesystem;
 const auto root=fs::temp_directory_path()/"maen-agent-profile-test";
 const auto normal=root/"MaenBrowser"/"User Data";
 const auto fallback=normal.parent_path()/"Agent User Data";
 using maenbrowser::storage::AgentProfile;
 assert(AgentProfile(normal,normal)==fallback);
 assert(AgentProfile(normal,normal/"Default")==fallback);
 assert(AgentProfile(normal,normal.parent_path())==fallback);
 assert(AgentProfile(normal,"relative")==fallback);
 assert(AgentProfile(normal,root/"YourAgent"/"Facebook")==fs::weakly_canonical(root/"YourAgent"/"Facebook"));
}
