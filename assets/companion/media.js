import {appendError} from './error-journal.js';
import {mediaError} from './media-errors.js';
import {t,errorText} from './i18n.js';
import {$,message} from './api.js';
const formats={'H.264 / MP4':'video/mp4; codecs="avc1.42E01E"','HEVC / H.265':'video/mp4; codecs="hvc1"','VP8':'video/webm; codecs="vp8"','VP9':'video/webm; codecs="vp09.00.10.08"','AV1':'video/mp4; codecs="av01.0.05M.08"','AAC':'audio/mp4; codecs="mp4a.40.2"','Opus':'audio/webm; codecs="opus"','Vorbis':'audio/ogg; codecs="vorbis"','FLAC':'audio/flac','MP3':'audio/mpeg','WAV / PCM':'audio/wav; codecs="1"'};
function refreshDiagnostics(){
$('results').replaceChildren();
for(const [name,type]of Object.entries(formats)){const p=document.createElement('p');p.textContent=name+': '+t($('player').canPlayType(type)||'Not reported');$('results').append(p);}
for(const [name,value]of Object.entries({MediaSource:!!window.MediaSource,WebRTC:!!window.RTCPeerConnection,WebCodecs:!!window.VideoDecoder,WebGL2:!!document.createElement('canvas').getContext('webgl2')})){const p=document.createElement('p');p.textContent=name+': '+t(value?'Available':'Unavailable');$('results').append(p);}
}
refreshDiagnostics();
document.addEventListener('maen-language',refreshDiagnostics);
let objectUrl;
$('file').onchange=()=>{if(objectUrl)URL.revokeObjectURL(objectUrl);const file=$('file').files[0];if(file){objectUrl=URL.createObjectURL(file);$('player').src=objectUrl;message($('status'),t('Selected {name} — press Play.',{name:file.name}));}};
$('player').onplaying=()=>message($('status'),'Playback started successfully.');$('player').onerror=()=>{const [code,reason]=mediaError($('player').error?.code);void appendError(code,'media');$('status').textContent=code+' — '+t(reason);$('status').classList.add('error');};
$('devices').onclick=async()=>{try{const devices=await navigator.mediaDevices.enumerateDevices();$('output').replaceChildren(new Option(t('System default'),''));for(const d of devices.filter(d=>d.kind==='audiooutput'))$('output').append(new Option(d.label||t('Audio output'),d.deviceId));}catch(e){message($('status'),e);}};
$('output').onchange=async()=>{try{if(!$('player').setSinkId)throw new Error('Output selection is unavailable. Use Windows sound settings.');await $('player').setSinkId($('output').value);}catch(e){message($('status'),e);}};
window.addEventListener('pagehide',()=>{if(objectUrl)URL.revokeObjectURL(objectUrl);});
