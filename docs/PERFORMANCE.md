# Current resource policy — 1.9.6

See RELEASE_1.9.6.md for exact thresholds, exclusions and mode selection, and
ACCEPTANCE_1.9.6.md for the before/after Windows measurement protocol.

UltraLite targets 2 GB-class systems; Lite targets constrained hardware. Both
retain GPU acceleration, hardware video decode, sandbox and site isolation.
Cache ceilings are not RAM caps. Background tab inspection is bounded and only
runs when the user enables idle release. No measured improvement is claimed until
the same workload has been compared on Windows 2 GB / 4 GB devices.
