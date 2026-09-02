(()=>{
  const params=new URLSearchParams(location.search);
  const demo=params.get('demo')==='1'||params.get('demo')==='true';
  const cloud=location.protocol==='https:'&&!demo;

  window.SmartRadarConfig={
    demoMode:demo,
    cloudMode:cloud,
    get deviceId(){return localStorage.getItem('smartRadar.deviceId')||'smart-radar-01'},
    get token(){return localStorage.getItem('smartRadar.token')||'smart-radar-demo-2026'},
    saveCloudIdentity(id,token){
      if(id)localStorage.setItem('smartRadar.deviceId',id);
      if(token)localStorage.setItem('smartRadar.token',token);
    },
    websocketUrl(){
      if(cloud)return `wss://${location.host}/api/ws?role=viewer&deviceId=${encodeURIComponent(this.deviceId)}&token=${encodeURIComponent(this.token)}`;
      return `ws://${location.host}/ws`;
    }
  };
})();
