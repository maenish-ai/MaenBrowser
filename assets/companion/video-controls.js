// This function is serialized by chrome.scripting. Keep it self-contained.
// User-invoked only: no persistent content script or background DOM scanning.
export function revealVideoControls() {
  let count = 0;
  for (const video of document.querySelectorAll('video')) {
    video.controls = true;
    video.controlsList?.remove('nofullscreen', 'noplaybackrate');
    count++;
  }
  return count;
}
