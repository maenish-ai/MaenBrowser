import assert from 'node:assert/strict';
import {shouldPreserveAuthPopup} from '../assets/companion/popup-policy.js';
assert.equal(shouldPreserveAuthPopup('https://accounts.google.com/o/oauth2/v2/auth'), true);
assert.equal(shouldPreserveAuthPopup('https://foo.eu.auth0.com/authorize'), true);
assert.equal(shouldPreserveAuthPopup('https://example.com/connect?client_id=1'), true);
assert.equal(shouldPreserveAuthPopup('https://example.com/document','https://claude.ai/login'), true);
assert.equal(shouldPreserveAuthPopup('https://example.com/document','https://news.example.org'), false);
assert.equal(shouldPreserveAuthPopup('about:blank','https://news.example.org'), true);
console.log('Preserve OAuth and payment popup flows; ordinary links are movable');
