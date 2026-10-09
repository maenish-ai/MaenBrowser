// Injected only into a direct media document in the on-demand Windows engine.
// No URL interpolation, network requests, autoplay, observers or polling.
(() => {
  if (!/^(video|audio)\//i.test(document.contentType || '')) return;
  const media = document.querySelector('video,audio');
  if (!media || document.getElementById('maen-media-tools')) return;
  const ar = document.documentElement.lang === 'ar';
  const text = (en, arabic) => ar ? arabic : en;
  media.controls = true;
  media.style.cssText = 'display:block;width:100%;max-height:calc(100vh - 110px);min-height:60px;object-fit:contain;margin:auto';
  document.body.style.cssText = 'margin:0;padding:16px;box-sizing:border-box;background:#111;color:#eee;font:16px system-ui';
  const tools = document.createElement('div');
  tools.id = 'maen-media-tools';
  tools.dir = ar ? 'rtl' : 'ltr';
  tools.style.cssText = 'display:flex;gap:12px;align-items:center;flex-wrap:wrap;margin-top:12px';
  const label = document.createElement('label');
  label.textContent = text('Playback speed ', 'سرعة التشغيل ');
  const speed = document.createElement('select');
  for (const value of [0.5, 0.75, 1, 1.25, 1.5, 2]) {
    const option = document.createElement('option');
    option.value = String(value); option.textContent = value + '×';
    speed.append(option);
  }
  speed.value = String(media.playbackRate);
  speed.addEventListener('change', () => { media.playbackRate = Number(speed.value); });
  media.addEventListener('ratechange', () => { speed.value = String(media.playbackRate); });
  label.append(speed); tools.append(label);
  const status = document.createElement('span');
  status.setAttribute('role', 'status');
  function reportError() {
    const code = media.error?.code;
    const messages = {
      1: text('Playback was interrupted. Reload to retry.', 'توقف التشغيل. أعد تحميل الصفحة للمحاولة.'),
      2: text('Video could not be fetched. Return to the site to renew the link or sign in.', 'تعذر تحميل الفيديو. ارجع إلى الموقع لتجديد الرابط أو تسجيل الدخول.'),
      3: text('This video could not be decoded on this device.', 'تعذر فك ترميز هذا الفيديو على الجهاز.'),
      4: text('The format or response is unsupported. The link may also have expired or require the site session.', 'الصيغة أو استجابة الموقع غير مدعومة. قد يكون الرابط منتهيًا أو يحتاج جلسة الموقع.')
    };
    status.textContent = messages[code] || '';
  }
  media.addEventListener('error', reportError);
  media.addEventListener('waiting', () => { status.textContent = text('Buffering…', 'جارٍ تحميل الفيديو…'); });
  media.addEventListener('playing', () => { status.textContent = ''; });
  media.addEventListener('loadedmetadata', () => { status.textContent = ''; });
  tools.append(status); document.body.append(tools); reportError();
})();
