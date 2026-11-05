from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'src/app/maen_client.cpp').read_text(encoding='utf-8')
popup=s[s.index('bool MaenClient::OnBeforePopup'):s.index('bool MaenClient::DoClose')]
assert 'protection::BlockNavigation(target_url.ToString())' in popup
assert 'OpenUrlInNewTab(' not in popup and '->LoadURL(' not in popup
updated=s[s.index('void MaenClient::OnDownloadUpdated'):]
assert '->CloseBrowser(' not in updated and 'ShowWindow(' not in updated
before=s[s.index('bool MaenClient::OnBeforeDownload'):s.index('void MaenClient::OnDownloadUpdated')]
assert 'if (!callback || !download_item) return false;' in before
assert before.count('callback->Continue(download_path, ask)')==1
assert 'chrome.downloads.cancel' not in (r/'assets/companion/background.js').read_text(encoding='utf-8')
print('Download ownership: original popup request preserved, one continuation, no automatic close/hide/cancel')

embedded=(r/'src/media/webview2_embedded.cpp').read_text(encoding='utf-8')
assert 'add_DownloadStarting(' not in embedded and 'remove_DownloadStarting(' not in embedded
print('Native media fallback: default WebView2 download manager preserved, no unconditional download cancellation')
