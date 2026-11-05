export function coordinates(lat,lon){if(!Number.isFinite(lat)||!Number.isFinite(lon)||Math.abs(lat)>90||Math.abs(lon)>180)throw new Error('Invalid coordinates');return {lat:Number(lat.toFixed(2)),lon:Number(lon.toFixed(2))};}
export function weatherUrl(point){const p=coordinates(point.lat,point.lon);return 'https://api.met.no/weatherapi/locationforecast/2.0/compact?lat='+p.lat+'&lon='+p.lon;}
export function forecast(data,now=Date.now()){
 const series=data?.properties?.timeseries;if(!Array.isArray(series))throw new Error('Invalid weather response');
 const item=series.find(x=>Number.isFinite(Date.parse(x.time))&&Math.abs(Date.parse(x.time)-now)<=90*60000);
 const d=item?.data?.instant?.details;
 if(!d||!Number.isFinite(d.air_temperature)||!Number.isFinite(d.wind_speed)||d.air_temperature< -100||d.air_temperature>70)throw new Error('Forecast unavailable');
 const symbol=String(item.data.next_1_hours?.summary?.symbol_code||item.data.next_6_hours?.summary?.symbol_code||'');
 const condition=symbol.includes('thunder')?'Thunderstorms':symbol.includes('sleet')?'Sleet':symbol.includes('snow')?'Snow':symbol.includes('rain')?'Rain':symbol.includes('fog')?'Fog':symbol.includes('partlycloudy')||symbol.includes('fair')?'Partly cloudy':symbol.includes('cloudy')?'Cloudy':symbol.includes('clearsky')?'Clear sky':'Forecast';
 return {temperature:Math.round(d.air_temperature),wind:Math.round(d.wind_speed*3.6),time:item.time,condition};
}
