import assert from 'node:assert/strict';
import { newerVersion, parseVersion, verifiedRelease, checkForRelease } from '../assets/companion/release-updates.js';
assert.deepEqual(parseVersion('v1.9.7'), [1, 9, 7]);
assert.equal(parseVersion('v1.9.7-rc1'), null);
assert.equal(newerVersion('v1.10.0', '1.9.99'), true);
assert.equal(newerVersion('1.9.6', '1.9.6'), false);
assert.equal(newerVersion('1.9.5', '1.9.6'), false);
const payload = {
  tag_name: 'v1.9.7', draft: false, prerelease: false,
  assets: [{ name: 'MaenBrowser-1.9.7-Setup.exe',
    browser_download_url: 'https://github.com/maenish-ai/MaenBrowser/releases/download/v1.9.7/MaenBrowser-1.9.7-Setup.exe' }]
};
assert.equal(verifiedRelease(payload, '1.9.6').version, '1.9.7');
assert.equal(verifiedRelease(payload, '1.9.7'), null);
assert.equal(verifiedRelease({ ...payload, prerelease: true }, '1.9.6'), null);
assert.equal(verifiedRelease({ ...payload, assets: [] }, '1.9.6'), null);
let notifications = 0;
const memory = {};
const storage = { get: async k => ({ [k]: memory[k] }), set: async v => Object.assign(memory, v) };
const options = {
  installed: '1.9.6', storage,
  fetcher: async () => ({ ok: true, json: async () => payload }),
  notifications: { create: async () => { notifications++; } },
};
await checkForRelease(options);
await checkForRelease(options);
assert.equal(notifications, 1);
assert.equal(memory.lastNotifiedRelease, '1.9.7');
console.log('Version compare, verified asset and release-notification deduplication passed');
