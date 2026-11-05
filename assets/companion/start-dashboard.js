import {t,currentLanguage} from './i18n.js';
import {coordinates,weatherUrl,forecast} from './weather-policy.js';
const $=id=>document.getElementById(id);
let clockTimer,controller,activePoint=null,busy=false,locationRequest=0;
const read=key=>{try{return JSON.parse(localStorage.getItem(key)||'null');}catch{return null;}};
const write=(key,value)=>{try{if(value===null)localStorage.removeItem(key);else localStorage.setItem(key,JSON.stringify(value));}catch{}};
function clock(){
 clearTimeout(clockTimer);if(document.hidden)return;
 const now=new Date(),locale=currentLanguage()==='ar'?'ar':navigator.language;
 $('localClock').textContent=new Intl.DateTimeFormat(locale,{hour:'2-digit',minute:'2-digit'}).format(now);
 $('localDate').textContent=new Intl.DateTimeFormat(locale,{weekday:'long',year:'numeric',month:'long',day:'numeric'}).format(now);
 $('localZone').textContent=Intl.DateTimeFormat().resolvedOptions().timeZone;
 clockTimer=setTimeout(clock,60000-Date.now()%60000);
}
const cities=[['Amman',31.95,35.93],['Jerusalem',31.77,35.21],['Ramallah',31.9,35.2],['Cairo',30.04,31.24],['Beirut',33.89,35.5],['Damascus',33.51,36.29],['Baghdad',33.32,44.37],['Riyadh',24.71,46.68],['Dubai',25.2,55.27],['Doha',25.29,51.53],['Kuwait City',29.38,47.98],['Muscat',23.59,58.41],['Manama',26.22,50.59],['Sanaa',15.37,44.19],['Rabat',34.02,-6.84],['Algiers',36.75,3.06],['Tunis',36.81,10.18],['Tripoli',32.89,13.19],['Khartoum',15.5,32.56],['Istanbul',41.01,28.98],['London',51.51,-0.13],['Paris',48.86,2.35],['Berlin',52.52,13.41],['Oslo',59.91,10.75],['New York',40.71,-74.01],['Los Angeles',34.05,-118.24],['Toronto',43.65,-79.38],['Sao Paulo',-23.55,-46.63],['Cape Town',-33.93,18.42],['Nairobi',-1.29,36.82],['Delhi',28.61,77.21],['Beijing',39.9,116.41],['Tokyo',35.68,139.69],['Singapore',1.35,103.82],['Sydney',-33.87,151.21]];
function cityOptions(){const value=$('weatherCity').value;$('weatherCity').replaceChildren();for(const [name]of cities){const o=document.createElement('option');o.value=name;o.textContent=t(name);$('weatherCity').append(o);}$('weatherCity').value=cities.some(c=>c[0]===value)?value:cities[0][0];}
function show(value){$('weatherValue').textContent=t('{temperature} °C · {condition} · wind {wind} km/h',{...value,condition:t(value.condition)});$('weatherStatus').textContent=t('Forecast time: {time}',{time:new Date(value.time).toLocaleString(currentLanguage()==='ar'?'ar':navigator.language)});$('weatherAttribution').hidden=false;}
function off(){locationRequest++;controller?.abort();activePoint=null;write('maen.weather.enabled',null);write('maen.weather.cache',null);$('weatherPlace').hidden=true;$('weatherDisable').hidden=true;$('weatherAttribution').hidden=true;$('weatherValue').textContent=t('Choose a city to see the forecast');$('weatherStatus').textContent=t('Optional: enabling weather shares the selected city coordinates and your IP address with MET Norway.');}
async function load(point,remember=true){
 if(busy)return;busy=true;activePoint=point;$('weatherPlace').textContent=t(point.name||'Your approximate location');$('weatherPlace').hidden=false;controller=new AbortController();$('weatherEnable').disabled=true;$('weatherLocate').disabled=true;$('weatherDisable').hidden=false;
 let timeout;
 if(remember)write('maen.weather.enabled',point);
 try{
  const url=weatherUrl(point),cached=read('maen.weather.cache');
  if(cached?.url===url&&Number.isFinite(cached.expires)&&cached.expires>Date.now()&&cached.expires<Date.now()+2*3600000){show(forecast(cached.data));return;}
  $('weatherStatus').textContent=t('Loading weather…');
  timeout=setTimeout(()=>controller.abort(),10000);
  const response=await fetch(url,{credentials:'omit',redirect:'error',signal:controller.signal});
  if(!response.ok)throw new Error('Weather service unavailable');
  const text=await response.text();if(text.length>500000)throw new Error('Weather response too large');
  const data=JSON.parse(text);const value=forecast(data);
  if(!activePoint)return;
  write('maen.weather.cache',{url,data,expires:Date.now()+3600000});show(value);
 }catch{if(activePoint){$('weatherValue').textContent=t('Weather unavailable');$('weatherStatus').textContent=t('Try again later or choose another city. Date, time and search still work.');}}
 finally{clearTimeout(timeout);busy=false;$('weatherEnable').disabled=false;$('weatherLocate').disabled=false;}
}
$('weatherEnable').onclick=()=>{const [name,lat,lon]=cities.find(c=>c[0]===$('weatherCity').value)||cities[0];load({...coordinates(lat,lon),name});};
$('weatherDisable').onclick=off;
$('weatherLocate').onclick=()=>{
 const request=++locationRequest;
 if(!navigator.geolocation){$('weatherStatus').textContent=t('Location unavailable. Choose a city instead.');return;}
 $('weatherStatus').textContent=t('Waiting for location permission…');
 navigator.geolocation.getCurrentPosition(p=>{if(request===locationRequest)load({...coordinates(p.coords.latitude,p.coords.longitude),name:'Your approximate location'});},()=>{if(request===locationRequest)$('weatherStatus').textContent=t('Location unavailable. Choose a city instead.');},{enableHighAccuracy:false,maximumAge:3600000,timeout:10000});
};
document.addEventListener('visibilitychange',clock);
document.addEventListener('maen-language',()=>{clock();cityOptions();if(activePoint)$('weatherPlace').textContent=t(activePoint.name||'Your approximate location');const c=read('maen.weather.cache');if(activePoint&&c)try{show(forecast(c.data));}catch{}});
window.addEventListener('storage',event=>{if(event.key==='maen.weather.enabled'&&!event.newValue)off();});
window.addEventListener('pagehide',()=>{clearTimeout(clockTimer);controller?.abort();});
cityOptions();clock();
const enabled=read('maen.weather.enabled');if(enabled&&Number.isFinite(enabled.lat)&&Number.isFinite(enabled.lon)){
 try{coordinates(enabled.lat,enabled.lon);if(cities.some(c=>c[0]===enabled.name))$('weatherCity').value=enabled.name;load(enabled,false);}catch{off();}
}
