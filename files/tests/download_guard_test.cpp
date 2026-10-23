#include "src/protection/download_guard.h"
#include <stdexcept>
using namespace maenbrowser::protection;
void check(bool x) { if (!x) throw std::runtime_error("download guard regression"); }
int main() {
  check(RiskyDownloadName(L"invoice.pdf.exe"));
  check(RiskyDownloadName(L"C:\\Downloads\\setup.MSI"));
  check(RiskyDownloadName(L"script.PS1 "));
  check(RiskyDownloadName(L"file.scr."));
  check(RiskyDownloadName(L"update.JS"));
  check(!RiskyDownloadName(L"picture.jpg"));
  check(!RiskyDownloadName(L"document.pdf"));
  check(!RiskyDownloadName(L"picture.exe.jpg"));
  check(!RiskyDownloadName(L".hidden"));
  check(!RiskyDownloadName(L"archive.zip"));
  check(!RiskyDownloadName(L""));
}
