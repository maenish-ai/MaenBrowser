# 1.9.5 FIX1 — Windows COM declarations

Observed in GitHub Actions run #79:
https://github.com/maenish-ai/MaenBrowser/actions/runs/38209350681

Build Release failed in src/media/media_window.cpp:
- C2065: COINIT_APARTMENTTHREADED undeclared
- C3861: CoInitializeEx identifier not found
- C3861: CoUninitialize identifier not found

Fix: explicitly include <objbase.h> after <windows.h>. The target already links ole32; no linker or runtime-policy change is needed.

This is a source build fix. Product version remains 1.9.5. The manual Ctrl+Shift+M media window and disabled automatic routing are unchanged.
Windows compilation and real playback must still pass GitHub Actions; local checks are not proof of Windows playback.
