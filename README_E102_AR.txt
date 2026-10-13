MaenBrowser 1.9.6 - E102 focused patch

Copy the files into the GitHub repository using the exact paths included in this ZIP.

Changes:
- On a failed local network connection, read-only protection status (get) is retried once after 250ms.
- Save and other write operations are never retried.
- HTTP errors, invalid responses, and application errors are not retried.
- A persistent failure still reports E102. This does not repair a missing/crashed local service.
- Adds regression tests for transient and persistent failures.

Validation: node tests/companion_api_test.mjs; node tests/protection_sync_regression_test.mjs; python tools/verify-package.py all passed locally.
Windows build/runtime not tested. Existing historical E102 records may remain in the journal.
