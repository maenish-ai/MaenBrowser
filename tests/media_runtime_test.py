"""Runtime selection, archive safety, layout and tamper tests with synthetic files."""
from pathlib import Path
import importlib.util
import io
import json
import struct
import tarfile
import tempfile
import zipfile

r=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('media_runtime', r/'tools/media-runtime.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
lock=m.load_lock()
assert m.selection(lock)['kind']=='locked-upstream'

def rejected(action):
    try: action()
    except (ValueError,OSError): return
    raise AssertionError('Invalid runtime was accepted')

for url,sha in [('https://example.org/cef.zip',None),(None,'a'*64),('http://example.org/cef.zip','a'*64),('https://user:password@example.org/cef.zip','a'*64),('https://example.org/cef.zip','garbage')]:
    rejected(lambda:m.selection(lock,url,sha))
for name in ['../escape','/absolute','C:/windows/file','dir\\..\\file']:
    rejected(lambda:m.safe_member(name))
with tempfile.TemporaryDirectory() as d:
    root=Path(d);content={p:b'fixture' for p in m.REQUIRED}
    pe=bytearray(128);pe[:2]=b'MZ';struct.pack_into('<I',pe,0x3c,64);pe[64:70]=b'PE\0\0\x64\x86'
    content['Release/libcef.dll']=pe
    content['include/cef_version.h']=f'#define CEF_VERSION "{lock["cef_version"]}"\n'.encode()
    marker='\n'.join([f'cef_version={lock["cef_version"]}',*lock['codec_build_flags']]).encode()
    archive=root/'custom.zip'
    with zipfile.ZipFile(archive,'w') as z:
        for path,data in content.items():z.writestr('cef/'+path,data)
        z.writestr('cef/MAEN_MEDIA_RUNTIME.txt',marker)
    selected=m.selection(lock,'https://example.org/custom.zip?signature=private',m.digest(archive))
    result=m.prepare(archive,root/'output',selected)
    m.verify(result,selected)
    assert 'private' not in (result/m.RECEIPT).read_text()
    (result/'Release/libcef.dll').write_bytes(b'changed')
    rejected(lambda:m.verify(result,selected))
    # Preparing again restores from the pinned archive instead of trusting cache.
    result=m.prepare(archive,root/'output',selected);m.verify(result,selected)
    bad={**selected,'sha256':'0'*64};rejected(lambda:m.prepare(archive,root/'other',bad))
    (result/'include/cef_version.h').write_text('#define CEF_VERSION "old"')
    rejected(lambda:m.verify(result,selected))
    unsafe=root/'unsafe.tar.bz2'
    with tarfile.open(unsafe,'w:bz2') as t:
        entry=tarfile.TarInfo('../escape');entry.size=1;t.addfile(entry,io.BytesIO(b'x'))
    rejected(lambda:m.unpack(unsafe,root/'unsafe'))
    assert not (root/'escape').exists()
print('Media runtime: default codec source, incomplete overrides, checksum, tamper, version and traversal guards passed')
