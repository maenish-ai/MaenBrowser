# Performance scope

No new browser engine, background decoder/transcoder, cloud classifier, persistent
site observer or normal-mode polling was added. The two local ad shims total about
9 KiB. Ad lookups use one merged sorted table; family categories remain lazy.
The opt-in agent mode alone polls one protected-file timestamp/size every second
to stop browsing when parent policy changes. It does not rescan all domain lists.

A synthetic Linux -O2 C++ check ran 1,000,000 lookups over the same five hostnames:
72,524 domains: 201.657 ns/lookup; 145,368 domains: 195.326 ns/lookup; both 400,000
hits. This single noisy microbenchmark establishes neither a speedup nor Windows
browser performance. It only checks that the merged lookup remains inexpensive
for that narrow workload. It excludes parsing, renderer CPU/RAM, chat load and GPU.

Use tools/measure-windows.ps1 on the same device/pages before and after, including
chat while typing, WhatsApp calls, Adobe image editing, a 1080p clip and background
idle tabs. Record total browser-tree memory/CPU and typing/video responsiveness.
Do not claim the world's lightest browser or a 2 GB performance guarantee from
source checks or this synthetic measurement.
