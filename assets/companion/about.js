import {readErrorJournal} from './error-journal.js';
import {$,api,nativeApi,message} from './api.js';
import {t} from './i18n.js';
import {diagnosticReport} from './diagnostics.js';
let snapshot=null;
function render(){
  if(!snapshot)return;
  $('details').replaceChildren();
  const n=snapshot.native;
  const rows={'Version':n.version,'Engine':n.engine,'Media engine':n.mediaEngine,'Resource mode':n.resourceMode,'System memory (MiB)':n.physicalMemoryMiB,'Logical processors':n.logicalProcessors,'Browser process memory (MiB)':n.browserProcessMemoryMiB,'Browser process CPU seconds':n.browserProcessCpuSeconds,'Network hint':t(snapshot.online?'Online hint':'Offline hint'),'Ad blocking':t(snapshot.protection.ads?'Enabled':'Disabled'),'Family Protection':t(snapshot.protection.family?'Enabled':'Disabled')};
  for(const [label,value]of Object.entries(rows)){const p=document.createElement('p');p.textContent=t(label)+': '+(value??t('Unavailable'));$('details').append(p);}
}
async function refresh(){
  $('refresh').disabled=true;$('export').disabled=true;snapshot=null;$('details').replaceChildren();$('report').hidden=true;
  try{
    const native=await nativeApi('technical');
    const state=await api();
    snapshot={schema:1,capturedAt:new Date().toISOString(),native,online:navigator.onLine,protection:{ads:state.ads===true,family:state.family===true,filterError:!!state.listError},recentErrors:await readErrorJournal(),diagnostics:JSON.parse(diagnosticReport())};
    render();message($('status'),'Snapshot captured.');$('export').disabled=false;
  }catch(e){message($('status'),e);}finally{$('refresh').disabled=false;}
}
$('refresh').onclick=refresh;
$('export').onclick=()=>{if(!snapshot)return;$('report').value=JSON.stringify(snapshot,null,2);$('report').hidden=false;$('report').select();};
document.addEventListener('maen-language',render);
refresh();
