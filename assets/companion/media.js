import {appendError} from './error-journal.js';
import {mediaError} from './media-errors.js';
import {t,errorText} from './i18n.js';
import {$,message} from './api.js';
import {mediaCodecs as formats,performanceCodecs} from './media-codecs.js';
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

$('performanceCheck').onclick=async()=>{
  const button=$('performanceCheck');button.disabled=true;
  const result=$('performanceResults');result.replaceChildren();
  try{
    if(!navigator.mediaCapabilities?.decodingInfo)throw new Error('Decode capability reports are unavailable.');
    for(const [name,codec] of performanceCodecs){
      const mime=codec.startsWith('vp09')?'video/webm':'video/mp4';
      const report=await navigator.mediaCapabilities.decodingInfo({type:'file',video:{contentType:mime+'; codecs="'+codec+'"',width:1920,height:1080,bitrate:5000000,framerate:30}});
      const p=document.createElement('p');p.textContent=name+': '+t('Supported')+' '+t(report.supported?'Yes':'No')+', '+t('Smooth')+' '+t(report.smooth?'Yes':'No')+', '+t('Power efficient')+' '+t(report.powerEfficient?'Yes':'No');result.append(p);
    }
  }catch(error){message(result,error);}finally{button.disabled=false;}
};
