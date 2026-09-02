(()=>{
  const C=SmartRadarConfig,$=id=>document.getElementById(id),R=new RadarCanvas($('radarCanvas'));
  const e={conn:$('connectionBadge'),dev:$('deviceBadge'),angle:$('angleValue'),dist:$('distanceValue'),fps:$('fpsValue'),rssi:$('rssiValue'),sensor:$('sensorValue'),servo:$('servoValue'),obj:$('objectsValue'),reads:$('readingsValue'),alarms:$('alarmsValue'),min:$('minDistanceValue'),max:$('maxDistanceValue'),up:$('uptimeValue'),ap:$('apIpValue'),sta:$('stationIpValue'),clients:$('apClientsValue'),heap:$('heapValue'),psram:$('psramValue'),cloud:$('cloudValue'),alert:$('radarAlert'),alertText:$('radarAlertText'),log:$('eventLog'),toast:$('toast'),mode:$('modeLabel')};
  const ids=['servoMinAngle','servoMaxAngle','servoStep','servoMoveIntervalMs','maxDistanceCm','alarmDistanceCm','buzzerEnabled','ledEnabled','alarmEnabled','staSsid','staPassword','cloudEnabled','cloudHost','cloudDeviceId','cloudToken'];
  const f=Object.fromEntries(ids.map(i=>[i,$(i)]));
  let ws,retry=800,timer,csv,connected=false;

  const demo={running:true,angle:0,direction:1,readings:0,alarms:0,objects:0,min:Infinity,max:0,start:Date.now(),alarmEnabled:true,buzzerEnabled:true,ledEnabled:true,maxDistanceCm:250,alarmDistanceCm:30,servoMinAngle:0,servoMaxAngle:180,servoStep:3,servoMoveIntervalMs:90,lastAlarm:false};

  e.mode.textContent=C.demoMode?'Modo: DEMO / simulador':C.cloudMode?'Modo: Vercel / remoto':'Modo: ESP32 / local';
  const fmtB=v=>v>1048576?(v/1048576).toFixed(1)+' MB':v>1024?Math.round(v/1024)+' KB':'--';
  const fmtD=v=>Number(v)>0?Math.round(v)+' cm':'--';
  const fmtU=ms=>{let s=Math.floor((+ms||0)/1000),m=Math.floor(s/60),h=Math.floor(m/60),d=Math.floor(h/24);return d?`${d}d ${h%24}h`:h?`${h}h ${m%60}m`:m?`${m}m ${s%60}s`:`${s}s`};

  function event(msg,kind=''){
    const row=document.createElement('div');row.className='event '+kind;
    row.innerHTML=`<time>${new Date().toLocaleTimeString()}</time><span></span>`;
    row.querySelector('span').textContent=msg;e.log.prepend(row);
    while(e.log.children.length>120)e.log.lastElementChild.remove();
  }
  function toast(m){e.toast.textContent=m;e.toast.classList.remove('hidden');clearTimeout(toast.t);toast.t=setTimeout(()=>e.toast.classList.add('hidden'),3000)}
  function state(on){connected=on;e.conn.textContent=on?(C.demoMode?'DEMO ONLINE':'WEBSOCKET CONNECTED'):'WEBSOCKET DISCONNECTED';e.conn.className=on?'on':'off';if(!C.cloudMode)device(on)}
  function device(on){e.dev.textContent=on?'DISPOSITIVO ONLINE':'DISPOSITIVO OFFLINE';e.dev.className=on?'on':'warn'}

  function handleDemoCommand(o){
    if(o.type==='ping')return true;
    if(o.type==='settings'){
      Object.assign(demo,o.settings||{});apply(o.settings||{});toast('Configurações aplicadas no simulador');event('Configurações DEMO atualizadas','ok');return true;
    }
    if(o.type!=='command')return true;
    const c=o.command;
    if(c==='start'){demo.running=true;toast('Radar iniciado');}
    else if(c==='stop'){demo.running=false;toast('Radar parado');}
    else if(c==='center'){demo.angle=90;R.addReading({angle:90,distance:0,valid:false,alarm:false});toast('Servo centralizado');}
    else if(c==='restart'){Object.assign(demo,{running:true,angle:0,direction:1,readings:0,alarms:0,objects:0,min:Infinity,max:0,start:Date.now(),lastAlarm:false});R.clear();toast('Radar reiniciado');}
    else if(c==='alarm_on'){demo.alarmEnabled=true;f.alarmEnabled.checked=true;toast('Alarme ativado');}
    else if(c==='alarm_off'){demo.alarmEnabled=false;f.alarmEnabled.checked=false;toast('Alarme desativado');}
    else if(c==='led_on'){demo.ledEnabled=true;f.ledEnabled.checked=true;toast('LED ativado');}
    else if(c==='led_off'){demo.ledEnabled=false;f.ledEnabled.checked=false;toast('LED desativado');}
    else if(c==='clear_radar'){R.clear();toast('Radar limpo');}
    else if(c==='clear_logs'){demo.readings=demo.alarms=demo.objects=0;demo.min=Infinity;demo.max=0;toast('Logs simulados apagados');}
    else if(c==='export_logs'){
      const rows=['Timestamp,Angulo,Distancia_cm,Alarme,Evento,RSSI'];
      for(let i=0;i<24;i++){const a=(i*7)%181,d=Math.round(15+Math.random()*(demo.maxDistanceCm-15)),al=d<=demo.alarmDistanceCm;rows.push(`${Date.now()-i*180},${a},${d},${al},${al?'OBJECT_DETECTED':'SCAN'},${-38-Math.round(Math.random()*16)}`)}
      download(rows.join('\n'));return true;
    }
    event(`Comando DEMO: ${c}`,'ok');return true;
  }

  function send(o){
    if(C.demoMode)return handleDemoCommand(o);
    if(!ws||ws.readyState!==1){toast('WebSocket desconectado');return false}
    ws.send(JSON.stringify(o));return true;
  }
  const cmd=c=>send({type:'command',command:c});
  function reconnect(){if(C.demoMode){startDemo();return}clearTimeout(timer);try{ws&&ws.close()}catch{}retry=800;connect()}
  function connect(){try{ws=new WebSocket(C.websocketUrl())}catch{return schedule()}ws.onopen=()=>{retry=800;state(true);event('WEBSOCKET CONNECTED','ok');if(C.cloudMode)cmd('get_config')};ws.onclose=()=>{state(false);if(C.cloudMode)device(false);event('WEBSOCKET DISCONNECTED','alert');schedule()};ws.onmessage=x=>{try{handle(JSON.parse(x.data))}catch{}}}
  function schedule(){clearTimeout(timer);timer=setTimeout(connect,retry);retry=Math.min(6000,retry*1.6)}

  function handle(m){
    if(m.type==='relay'){device(m.deviceOnline);e.cloud.textContent=m.redisBackplane?'REDIS + WSS':'WSS'}
    else if(m.type==='radar'){
      device(true);R.addReading(m);e.angle.textContent=Math.round(m.angle);e.dist.textContent=m.valid?Math.round(m.distance):'--';e.rssi.textContent=m.rssi?m.rssi+' dBm':'--';e.sensor.textContent=m.valid?'OK':'SENSOR ERROR';
      if(m.alarm){e.alert.classList.remove('hidden');e.alertText.textContent=`${Math.round(m.distance)} cm • ${Math.round(m.angle)}°`;if(m.event==='OBJECT_DETECTED')event(`⚠ Objeto: ${Math.round(m.distance)} cm em ${Math.round(m.angle)}°`,'alert')}else e.alert.classList.add('hidden')
    }
    else if(m.type==='status'){
      device(true);e.obj.textContent=m.objectsDetected??0;e.reads.textContent=m.readings??0;e.alarms.textContent=m.alarms??0;e.min.textContent=fmtD(m.minDistance);e.max.textContent=fmtD(m.maxDistance);e.up.textContent=fmtU(m.uptime);e.ap.textContent=m.apIp||'--';e.sta.textContent=m.stationIp||'--';e.clients.textContent=m.apClients??0;e.heap.textContent=fmtB(m.freeHeap);e.psram.textContent=fmtB(m.freePsram);e.sensor.textContent=m.sensorOk?'OK':'SENSOR ERROR';e.servo.textContent=m.running?'VARRENDO':'PARADO';e.cloud.textContent=m.cloudConnected?'ONLINE':'OFFLINE'
    }
    else if(m.type==='config')apply(m.config||{});
    else if(m.type==='ack'){toast(m.message||'OK');event(m.message||'OK','ok')}
    else if(m.type==='error'){toast(m.message||'Erro');event(m.message||'Erro','alert')}
    else if(m.type==='csv_begin')csv={id:m.transferId,data:''};
    else if(m.type==='csv_chunk'&&csv&&csv.id===m.transferId)csv.data+=m.data||'';
    else if(m.type==='csv_end'&&csv&&csv.id===m.transferId){download(csv.data);csv=null}
  }

  function apply(v){
    for(const id of ['servoMinAngle','servoMaxAngle','servoStep','servoMoveIntervalMs','maxDistanceCm','alarmDistanceCm','staSsid','cloudHost','cloudDeviceId','cloudToken'])if(v[id]!=null&&f[id])f[id].value=v[id];
    for(const id of ['buzzerEnabled','ledEnabled','alarmEnabled','cloudEnabled'])if(v[id]!=null&&f[id])f[id].checked=!!v[id];
    if(f.staPassword)f.staPassword.value='';
    if(v.maxDistanceCm)R.setMaxDistance(v.maxDistanceCm);
    if(v.cloudDeviceId&&v.cloudToken)C.saveCloudIdentity(v.cloudDeviceId,v.cloudToken);
  }

  function download(content){
    const n=new Date(),p=v=>String(v).padStart(2,'0'),name=`radar_logs_${n.getFullYear()}-${p(n.getMonth()+1)}-${p(n.getDate())}_${p(n.getHours())}-${p(n.getMinutes())}-${p(n.getSeconds())}.csv`;
    const url=URL.createObjectURL(new Blob([content],{type:'text/csv;charset=utf-8'})),a=document.createElement('a');a.href=url;a.download=name;document.body.append(a);a.click();a.remove();setTimeout(()=>URL.revokeObjectURL(url),1000);toast(name)
  }

  function demoReading(){
    if(!demo.running)return;
    demo.angle+=demo.direction*demo.servoStep;
    if(demo.angle>=demo.servoMaxAngle){demo.angle=demo.servoMaxAngle;demo.direction=-1}
    if(demo.angle<=demo.servoMinAngle){demo.angle=demo.servoMinAngle;demo.direction=1}
    const wave=(Math.sin(demo.angle*Math.PI/24)+1)/2;
    const obstacleBand=(demo.angle>58&&demo.angle<82)||(demo.angle>128&&demo.angle<145);
    let distance=obstacleBand?18+Math.random()*28:70+wave*130+Math.random()*25;
    distance=Math.min(demo.maxDistanceCm,Math.max(5,distance));
    const alarm=demo.alarmEnabled&&distance<=demo.alarmDistanceCm;
    demo.readings++;demo.min=Math.min(demo.min,distance);demo.max=Math.max(demo.max,distance);
    if(alarm&&!demo.lastAlarm){demo.alarms++;demo.objects++}
    demo.lastAlarm=alarm;
    handle({type:'radar',angle:demo.angle,distance,valid:true,alarm,event:alarm?'OBJECT_DETECTED':'SCAN',rssi:-42-Math.round(Math.random()*7)});
  }

  function demoStatus(){
    handle({type:'status',objectsDetected:demo.objects,readings:demo.readings,alarms:demo.alarms,minDistance:Number.isFinite(demo.min)?demo.min:0,maxDistance:demo.max,uptime:Date.now()-demo.start,apIp:'192.168.4.1',stationIp:'192.168.1.120',apClients:1,freeHeap:286720,freePsram:7852032,sensorOk:true,running:demo.running,cloudConnected:false});
  }

  function startDemo(){
    connected=true;state(true);device(true);e.cloud.textContent='SIMULADO';
    apply({servoMinAngle:demo.servoMinAngle,servoMaxAngle:demo.servoMaxAngle,servoStep:demo.servoStep,servoMoveIntervalMs:demo.servoMoveIntervalMs,maxDistanceCm:demo.maxDistanceCm,alarmDistanceCm:demo.alarmDistanceCm,buzzerEnabled:demo.buzzerEnabled,ledEnabled:demo.ledEnabled,alarmEnabled:demo.alarmEnabled,cloudEnabled:false,cloudDeviceId:C.deviceId,cloudToken:C.token});
    event('MODO DEMO ATIVO — hardware não necessário','ok');
    clearInterval(startDemo.scan);clearInterval(startDemo.status);
    startDemo.scan=setInterval(demoReading,Math.max(50,demo.servoMoveIntervalMs));
    startDemo.status=setInterval(demoStatus,1000);demoStatus();
  }

  document.querySelectorAll('[data-command]').forEach(b=>b.onclick=()=>cmd(b.dataset.command));
  $('clearRadarButton').onclick=()=>{R.clear();cmd('clear_radar')};
  $('exportButton').onclick=()=>cmd('export_logs');
  $('clearLogsButton').onclick=()=>confirm('Tem certeza que deseja apagar todos os logs?')&&cmd('clear_logs');
  $('clearEventsButton').onclick=()=>e.log.innerHTML='';
  $('reconnectCloudButton').onclick=()=>{C.saveCloudIdentity(f.cloudDeviceId.value.trim(),f.cloudToken.value.trim());reconnect()};
  $('settingsForm').onsubmit=ev=>{
    ev.preventDefault();
    const s={servoMinAngle:+f.servoMinAngle.value,servoMaxAngle:+f.servoMaxAngle.value,servoStep:+f.servoStep.value,servoMoveIntervalMs:+f.servoMoveIntervalMs.value,maxDistanceCm:+f.maxDistanceCm.value,alarmDistanceCm:+f.alarmDistanceCm.value,buzzerEnabled:f.buzzerEnabled.checked,ledEnabled:f.ledEnabled.checked,alarmEnabled:f.alarmEnabled.checked,staSsid:f.staSsid.value.trim(),cloudEnabled:f.cloudEnabled.checked,cloudHost:f.cloudHost.value.trim(),cloudDeviceId:f.cloudDeviceId.value.trim(),cloudToken:f.cloudToken.value.trim()};
    if(f.staPassword.value)s.staPassword=f.staPassword.value;
    if(s.servoMinAngle>=s.servoMaxAngle||s.alarmDistanceCm>s.maxDistanceCm)return toast('Confira limites de ângulo e distância');
    if(send({type:'settings',settings:s})){C.saveCloudIdentity(s.cloudDeviceId,s.cloudToken);R.setMaxDistance(s.maxDistanceCm)}
  };

  window.addEventListener('radar-fps',x=>e.fps.textContent=x.detail);
  f.cloudDeviceId.value=C.deviceId;f.cloudToken.value=C.token;
  state(false);device(!C.cloudMode);
  if(C.demoMode)startDemo();else connect();
  setInterval(()=>connected&&send({type:'ping'}),12000);
})();
