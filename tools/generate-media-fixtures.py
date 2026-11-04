"""Regenerate the 1.10 synthetic fixtures with FFmpeg; never used at runtime."""
from pathlib import Path
import subprocess,json
r=Path(__file__).resolve().parents[1]/'tests/fixtures';formats=json.loads((r/'formats.json').read_text(encoding='utf-8'))
generated={'h264-main.mp4','h264-high.mp4','opus.ogg','alac.m4a','sample.aiff','sample.wma','h264.mkv','mpeg2.mpg'}
formats=[entry for entry in formats if entry['file'] not in generated]
def ff(*args):subprocess.run(['ffmpeg','-hide_banner','-loglevel','error','-y',*args],check=True)
for name,profile in [('h264-main.mp4','main'),('h264-high.mp4','high')]:
 ff('-f','lavfi','-i','testsrc2=size=160x90:rate=15','-f','lavfi','-i','sine=frequency=440:sample_rate=44100','-t','2','-c:v','libx264','-profile:v',profile,'-pix_fmt','yuv420p','-c:a','aac','-movflags','+faststart',str(r/name))
 formats.append({'file':name,'mime':'video/mp4','video':True})
ff('-f','lavfi','-i','sine=frequency=440:sample_rate=48000','-t','2','-c:a','libopus',str(r/'opus.ogg'))
formats.append({'file':'opus.ogg','mime':'audio/ogg','video':False})
for name,args,mime,video in [
 ('alac.m4a',['-f','lavfi','-i','sine=frequency=440:sample_rate=44100','-t','2','-c:a','alac'],'audio/mp4',False),
 ('sample.aiff',['-f','lavfi','-i','sine=frequency=440:sample_rate=44100','-t','2','-c:a','pcm_s16be'],'audio/aiff',False),
 ('sample.wma',['-f','lavfi','-i','sine=frequency=440:sample_rate=44100','-t','2','-c:a','wmav2'],'audio/x-ms-wma',False),
 ('h264.mkv',['-f','lavfi','-i','testsrc2=size=160x90:rate=15','-t','2','-c:v','libx264','-pix_fmt','yuv420p'],'video/x-matroska',True),
 ('mpeg2.mpg',['-f','lavfi','-i','testsrc2=size=160x90:rate=25','-t','2','-c:v','mpeg2video'],'video/mpeg',True)]:
 ff(*args,str(r/name));formats.append({'file':name,'mime':mime,'video':video,'required':False})
(r/'formats.json').write_text(json.dumps(formats,indent=2)+'\n',encoding='utf-8')
images=[]
for ext,mime in [('png','image/png'),('jpg','image/jpeg'),('webp','image/webp'),('gif','image/gif'),('bmp','image/bmp')]:
 name='pattern.'+ext;ff('-f','lavfi','-i','testsrc2=size=160x90:rate=1','-frames:v','1',str(r/name));images.append({'file':name,'mime':mime,'width':160,'height':90})
(r/'pattern.svg').write_text('<svg xmlns="http://www.w3.org/2000/svg" width="160" height="90"><rect width="160" height="90" fill="teal"/></svg>',encoding='utf-8');images.append({'file':'pattern.svg','mime':'image/svg+xml','width':160,'height':90})
ff('-f','lavfi','-i','testsrc2=size=160x90:rate=1','-frames:v','1','-c:v','libaom-av1','-still-picture','1',str(r/'pattern.avif'));images.append({'file':'pattern.avif','mime':'image/avif','width':160,'height':90})
(r/'images.json').write_text(json.dumps(images,indent=2)+'\n',encoding='utf-8')
