# 1.5.35 CI state correction

GitHub run 37676733133 proved that source preflight and the security gate both pass.
The red result came only from an intentionally thrown exception when the repository
variables for the custom media CEF archive were absent.

1.5.35 treats that condition correctly:
- source/security validation stays green;
- no stock CEF fallback is allowed;
- no Setup is produced without the verified custom media runtime;
- the workflow emits a warning rather than manufacturing a red source failure.

This does not manufacture H.264/AAC support. A real Media Setup still requires the
Windows x64 CEF media archive and its SHA256.
