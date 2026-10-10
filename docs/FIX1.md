# MaenBrowser 1.9.0 FIX1

Fixes the reported MSVC C2665 in src/app/maen_client.cpp, OnBeforeBrowse.
The deferred About navigation now uses base::BindOnce with an explicitly bound
CefRefPtr<CefBrowser>, and includes cef_bind.h and cef_callback.h.
The callback retains the browser until execution and checks validity before navigation.
The existing About mapping and media reload fixes are preserved.
Application VERSION remains 1.9.0; FIX1 identifies this source package.

Upload all extracted project contents, including .github, to the repository root,
then run Build MaenBrowser Windows Setup.

Validation: local JavaScript, portable C++ and package/source checks.
A full Windows/CEF build and runtime test cannot be performed in this Linux environment;
GitHub Actions must confirm the Windows build. This archive is source, not an installer.

CEF reference: https://github.com/chromiumembedded/cef/blob/master/include/wrapper/cef_closure_task.h
