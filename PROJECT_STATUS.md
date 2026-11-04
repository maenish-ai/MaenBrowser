# Current candidate: MaenBrowser 1.10.0

Base: the user's successful 1.9.10 (8fc1aa71df0876cf86e75b976dc92da65db73787).
The working CEF 154 archive pin is unchanged. Filtering, media/image coverage,
branding entry points, navigation and opt-in agent support were extended.

See UPLOAD_AND_RELEASE_AR.md and docs/RELEASE_1.10.0.md. A separate patch targets
YourAssistantAgent commit 10dce4b and must be built in that repository.

Linux portable/native-policy tests, PowerShell checks and mocked Agent tests are
recorded with this release. Actual Windows compilation, media/shim/image tests,
agent/CDP login, Adobe and user-device resource acceptance remain CI/device tasks.
No Setup was produced, remote repository modified or release published here.
Archived 1.9.6 patch exporters are historical and not installation procedures.
