// Local, recent, browser-wide categories only. No VPN provider, URL, host or IP.
export function describeNetwork(data,t){
  if(data?.networkDiagnosticsVersion!==1)return t('Network diagnostics require the matching browser version.');
  const age=data.recentNetworkAgeSeconds;
  if(!Number.isInteger(age)||age<0||age>300)return t('No recent page network error was recorded. This does not prove that the VPN or internet is working.');
  const descriptions={
    network_changed:'The network changed while the page was loading.',
    offline:'The system reported that the network was disconnected.',
    dns:'The website name could not be resolved (DNS).',
    proxy:'The proxy or tunnel connection failed.',
    timeout:'The connection timed out.',
    certificate:'A certificate error occurred; the secure connection was not bypassed.',
    other:'Another page loading error occurred.'
  };
  const reason=Object.hasOwn(descriptions,data.recentNetworkCategory)?descriptions[data.recentNetworkCategory]:descriptions.other;
  return t('Recent page error across all normal tabs ({age} seconds ago): {reason} This does not prove the VPN caused it.',{age,reason:t(reason)});
}
