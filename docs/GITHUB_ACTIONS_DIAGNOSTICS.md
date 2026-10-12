# GitHub Actions troubleshooting

The Windows Setup job requires a separately produced, verified CEF 152 media archive. A normal source push cannot generate that archive automatically. The `configuration` job checks whether the two repository secrets have a valid format. When either is missing or malformed, the Setup job is **skipped**, not reported as a successful installer build. No Setup artifact is produced.

Configure repository **Settings > Secrets and variables > Actions > Repository secrets**:

- `MAEN_CEF_ARCHIVE_URL`: publicly or runner-accessible HTTPS URL of the media-enabled archive.
- `MAEN_CEF_ARCHIVE_SHA256`: 64-digit hexadecimal SHA-256 of the exact downloaded archive.

The CEF media build workflow requires a registered, online Windows x64 **self-hosted runner**. Without that runner the job stays queued. Its 12-hour timeout and disk/RAM needs are independent of the regular Windows Setup workflow.

If the configuration gate passes but Setup fails, open the failed Actions run and inspect the first failed step and its full logs. The source ZIP alone cannot establish the cause of a later GitHub-hosted run. The package and JS unit tests can be run locally, but Windows native build, H.264 playback, installer generation, and CEF integrity verification require a compatible Windows environment and archive.
