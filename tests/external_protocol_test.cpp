#include "src/app/external_protocol.h"
#include <cassert>
int main(){
 using maenbrowser::AllowedExternalUrl;
 assert(AllowedExternalUrl("mailto:example@example.com"));
 assert(AllowedExternalUrl("zoommtg://zoom.us/join?confno=123"));
 assert(!AllowedExternalUrl("file:///C:/Windows/system32/cmd.exe"));
 assert(!AllowedExternalUrl("powershell:Start-Process"));
 assert(!AllowedExternalUrl("msteams:\nunsafe"));
 assert(!AllowedExternalUrl("mailto:x\" --flag"));
}
