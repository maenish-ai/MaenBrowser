#include "src/media/media_policy.h"
#include <stdexcept>
#include <iostream>
using maenbrowser::media::IsDirectMediaPath;
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
int main() {
  for (const char* path : {"/movie.mp4", "/MOVIE.MP4", "/track.m4a", "/video.m4v", "/sound.aac"})
    check(IsDirectMediaPath("https", path), "HTTPS media file should route");
  for (const char* path : {"/watch", "/watch?next=movie.mp4", "/movie.mp4/", "/movie.mp4.exe", "/mp4", "/", "/movie.webm", "/video.m3u8"})
    check(!IsDirectMediaPath("https", path), "ordinary page must not route");
  for (const char* scheme : {"http", "file", "blob", "data", "javascript", ""})
    check(!IsDirectMediaPath(scheme, "/movie.mp4"), "non-HTTPS scheme must not route");
  std::cout << "Direct media routing tests passed\n";
}
