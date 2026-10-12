# MaenBrowser 1.9.3 — on-demand performance diagnostics

Built on 1.9.2. Automatic WebView2 routing remains disabled; the WhatsApp placeholder rollback and previous diagnostics are retained.

## Changes
- Technical Details can measure average main-process CPU utilization between two user-requested snapshots at least one second apart. Percentages are normalized to total logical processor capacity. Renderer/subprocess usage is excluded; this is not total browser CPU.
- Invalid, short, or different-process samples do not produce a percentage.
- Download the captured technical snapshot as local JSON. No upload, background polling, page addresses, or parent PINs are added.
- New labels support Arabic and English. Existing text export remains available.

## Validation
Local JavaScript regression tests, portable C++ policy tests, package verification and Windows-encoding simulation are required before delivery.
Windows build, installer execution and real-device performance remain unverified here. This update adds measurement, not a claim of reduced resource usage or a fix for unsupported video codecs.

## Windows acceptance
1. Upload the complete source tree including .github and run the Windows setup workflow.
2. Confirm WhatsApp loads without the removed local loading placeholder.
3. Open Technical Details, refresh after at least one second, and check the main-process CPU sample.
4. Download report; confirm valid JSON and expected filename through the browser download interface.
5. Switch Arabic/English and check the new labels.
6. Measure total browser CPU and memory with Windows Task Manager on the same pages before claiming performance improvement.
