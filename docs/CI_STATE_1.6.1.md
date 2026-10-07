# CI state 1.6.1

The primary GitHub Actions workflow again produces one Windows Setup artifact on a standard `windows-2022` runner. It downloads pinned stock CEF 152 plus Microsoft's small WebView2 SDK NuGet package. No self-hosted Chromium build and no `MAEN_CEF_ARCHIVE_*` repository variables are required.
