// Only fields intentionally changed by this panel may be written.
export function changedSettings(previous, current) {
  return Object.fromEntries(Object.entries(current).filter(([key,value]) =>
    JSON.stringify(value) !== JSON.stringify(previous[key])));
}
