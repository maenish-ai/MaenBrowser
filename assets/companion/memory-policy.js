export function idlePolicy(preferences={},sample={}) {
  const mode=sample.resourceMode||'Balanced';
  const base=Number(preferences.sleepMinutes)||30;
  const pressure=Number(sample.memoryLoadPercent)>=85 ||
    (Number.isFinite(sample.availableMemoryMiB)&&sample.availableMemoryMiB<384);
  return {minutes:pressure?(mode==='UltraLite'?5:10):base,
    batch:pressure?2:1,pressure};
}
