import {t,errorText} from './i18n.js';
import {$,webUrl} from './api.js';
const key=location.hash.slice(1);const data=key.startsWith('reader-')?(await chrome.storage.session.get(key))[key]:null;
if(data){$('title').textContent=data.title;$('text').textContent=data.text;if(webUrl(data.url))$('source').href=data.url;await chrome.storage.session.remove(key);}else $('text').textContent='This temporary reading copy expired. Open the original page and choose Text reader again.';
