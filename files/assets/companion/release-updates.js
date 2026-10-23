// Update discovery only: GitHub Releases is a notice source, never an automatic installer.
export const RELEASES_API = 'https://api.github.com/repos/maenish-ai/MaenBrowser/releases/latest';
export const RELEASES_PAGE = 'https://github.com/maenish-ai/MaenBrowser/releases/latest';
export const UPDATE_NOTIFICATION_ID = 'maenbrowser-release-available';
export const UPDATE_ALARM = 'maen-release-check';

export function parseVersion(input) {
  const match = /^v?(\d+)\.(\d+)\.(\d+)$/.exec(String(input ?? ''));
  if (!match) return null;
  const numbers = match.slice(1).map(Number);
  return numbers.every(n => Number.isSafeInteger(n)) ? numbers : null;
}

export function newerVersion(candidate, installed) {
  const next = parseVersion(candidate);
  const current = parseVersion(installed);
  if (!next || !current) return false;
  for (let i = 0; i < 3; i++) {
    if (next[i] !== current[i]) return next[i] > current[i];
  }
  return false;
}

export function verifiedRelease(payload, installed) {
  if (!payload || payload.draft || payload.prerelease) return null;
  const tag = String(payload.tag_name ?? '');
  const v = parseVersion(tag);
  if (!v || !newerVersion(tag, installed)) return null;
  const version = v.join('.');
  // Do not notify on GitHub source-only releases, previews or missing installers.
  const name = `MaenBrowser-${version}-Setup.exe`;
  const prefix = `https://github.com/maenish-ai/MaenBrowser/releases/download/${encodeURIComponent(tag)}/`;
  const hasSetup = Array.isArray(payload.assets) && payload.assets.some(asset =>
    asset && asset.name === name && asset.browser_download_url === prefix + name &&
    asset.state !== 'deleted');
  if (!hasSetup) return null;
  return { version, url: `https://github.com/maenish-ai/MaenBrowser/releases/tag/${encodeURIComponent(tag)}` };
}

export async function checkForRelease({
  fetcher = fetch,
  installed = chrome.runtime.getManifest().version,
  storage = chrome.storage.local,
  notifications = chrome.notifications,
} = {}) {
  const response = await fetcher(RELEASES_API, {
    method: 'GET', cache: 'no-store', credentials: 'omit',
    headers: { Accept: 'application/vnd.github+json', 'X-GitHub-Api-Version': '2022-11-28' },
  });
  if (!response.ok) return null; // Offline, rate limited, or no release published yet.
  const update = verifiedRelease(await response.json(), installed);
  if (!update) return null;
  const { lastNotifiedRelease } = await storage.get('lastNotifiedRelease');
  if (lastNotifiedRelease === update.version) return update;
  // Only mark notified after Chromium accepts the notification.
  await notifications.create(UPDATE_NOTIFICATION_ID, {
    type: 'basic', iconUrl: 'icon128.png',
    title: 'MaenBrowser update available',
    message: `Version ${update.version} is ready. Click to open the official GitHub release.`,
    priority: 1,
  });
  await storage.set({ lastNotifiedRelease: update.version });
  return update;
}
