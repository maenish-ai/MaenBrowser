import {readErrorJournal} from './error-journal.js';
import {processSample} from './process-sample.js';
import {downloadReport} from './report-download.js';
import {$,api,nativeApi,message} from './api.js';
import {t} from './i18n.js';
import {diagnosticReport} from './diagnostics.js';
let snapshot=null;
let previous=null;
function render(){
  if(!snapshot)return;
  $('details').replaceChildren();
  const n=snapshot.native;
  const rows={'Version':n.version,'Engine':n.engine,'Media engine':n.mediaEngine,'Resource mode':n.resourceMode,'System memory (MiB)':n.physicalMemoryMiB,'Logical processors':n.logicalProcessors,'Browser process memory (MiB)':n.browserProcessMemoryMiB,'Browser process CPU seconds':n.browserProcessCpuSeconds,'Network hint':t(snapshot.online?'Online hint':'Offline hint'),'Ad blocking':t(snapshot.protection.ads?'Enabled':'Disabled'),'Family Protection':t(snapshot.protection.family?'Enabled':'Disabled')};
  for(const [label,value]of Object.entries(rows)){const p=document.createElement('p');p.textContent=t(label)+': '+(value??t('Unavailable'));$('details').append(p);}
  const usage=document.createElement('p');
  usage.textContent=snapshot.processSample
    ? t('Main process CPU: {percent}% over {seconds} seconds',{percent:snapshot.processSample.cpuPercent.toFixed(1),seconds:snapshot.processSample.seconds.toFixed(1)})
    : t('Refresh again after at least one second to measure CPU usage.');
  $('details').append(usage);
}
async function refresh(){
  $('downloadReport').disabled=true;
  $('refresh').disabled=true;$('export').disabled=true;snapshot=null;$('details').replaceChildren();$('report').hidden=true;
  try{
    const native=await nativeApi('technical');
    const state=await api();
    snapshot={schema:1,capturedAt:new Date().toISOString(),native,online:navigator.onLine,protection:{ads:state.ads===true,family:state.family===true,filterError:!!state.listError},recentErrors:await readErrorJournal(),diagnostics:JSON.parse(diagnosticReport())};
    snapshot.processSample=processSample(previous,native);previous=native;
    $('downloadReport').disabled=false;
    render();message($('status'),'Snapshot captured.');$('export').disabled=false;
  }catch(e){previous=null;message($('status'),e);}finally{$('refresh').disabled=false;}
}
$('refresh').onclick=refresh;
$('export').onclick=()=>{if(!snapshot)return;$('report').value=JSON.stringify(snapshot,null,2);$('report').hidden=false;$('report').select();message($('status'),'Technical report ready to copy.');};
$('downloadReport').onclick=()=>{if(snapshot)try{downloadReport(snapshot,'MaenBrowser-technical-report.json');message($('status'),'Technical report download requested.');}catch(e){message($('status'),e);}};
document.addEventListener('maen-language',render);
refresh();

const credits=$("project-credits");
if(credits){const h=document.createElement("h2");h.textContent="Project credits / عن المشروع";credits.append(h);for(const [language,copy] of [["en","MaenBrowser is a free browser project, created from the project owner’s idea and developed with assistance from ChatGPT by OpenAI. Donations are optional. ChatGPT is an AI assistant, not a co-owner or official OpenAI endorsement."],["ar","MaenBrowser مشروع متصفح مجاني للجميع، انطلق من فكرة صاحب المشروع وطُوّر بمساعدة ChatGPT من OpenAI. التبرع اختياري ولا يُشترط لاستخدام المتصفح."]]){const paragraph=document.createElement("p");paragraph.lang=language;if(language==="ar")paragraph.dir="rtl";paragraph.textContent=copy;credits.append(paragraph);}}
