from pathlib import Path
r=Path(__file__).resolve().parents[1]
s=(r/'src/app/maen_client.cpp').read_text(encoding='utf-8')
popup=s[s.index('bool MaenClient::OnBeforePopup'):s.index('bool MaenClient::DoClose')]
assert 'protection::BlockNavigation(target_url.ToString())' in popup
assert 'OpenUrlInNewTab(' not in popup and '->LoadURL(' not in popup
updated=s[s.index('void MaenClient::OnDownloadUpdated'):]
assert 'item->IsInProgress()' in updated and '!state.path.empty()' in updated
assert 'blank && isolated' in updated and 'has_other_window' in updated
assert "item->IsComplete() && blank && isolated" in updated
assert "if(close_popup && popup && popup->IsValid())" in updated
assert 'if(show_window)ShowWindow(show_window,SW_SHOWNOACTIVATE)' in updated
assert 'another' in updated and 'downloads_.erase(id)' in updated
before=s[s.index('bool MaenClient::OnBeforeDownload'):s.index('void MaenClient::OnDownloadUpdated')]
assert 'if (!callback || !download_item) return false;' in before
assert before.count('callback->Continue(download_path, ask)')==1
assert 'chrome.downloads.cancel' not in (r/'assets/companion/background.js').read_text(encoding='utf-8')
created=s[s.index('void MaenClient::OnAfterCreated'):s.index('void MaenClient::OnGotFocus')]
assert 'created_root' in created and 'hidden_download_popups_.erase(it)' in created
assert 'SW_SHOWNOACTIVATE' in created
print('Download ownership: original popup request preserved, one continuation, only isolated blank popups hidden in progress; close after success, restore on failure')

embedded=(r/'src/media/webview2_embedded.cpp').read_text(encoding='utf-8')
assert 'add_DownloadStarting(' not in embedded and 'remove_DownloadStarting(' not in embedded
print('Native media fallback: default WebView2 download manager preserved, no unconditional download cancellation')
