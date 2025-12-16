#ifndef WEB_PAGES_H
#define WEB_PAGES_H

// 精简版 CSS
const char* commonCss = R"(<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:system-ui,sans-serif;background:#f5f5f5;color:#333;padding:16px;line-height:1.5}
.c{max-width:480px;margin:0 auto}
h1{text-align:center;color:#4f46e5;margin-bottom:16px;font-size:1.4em}
.nav{display:flex;gap:4px;margin-bottom:16px}
.nav a{flex:1;text-align:center;padding:10px;background:#e5e5e5;text-decoration:none;color:#333;border-radius:6px}
.nav a.on{background:#4f46e5;color:#fff}
.card{background:#fff;border-radius:8px;padding:16px;margin-bottom:12px;border:1px solid #ddd}
.card-t{font-weight:600;margin-bottom:12px;padding-bottom:8px;border-bottom:1px solid #eee}
.fg{margin-bottom:12px}
label{display:block;font-size:.9em;color:#666;margin-bottom:4px}
input,select,textarea{width:100%;padding:10px;border:1px solid #ddd;border-radius:6px;font-size:.95em}
input:focus,select:focus,textarea:focus{border-color:#4f46e5;outline:none}
textarea{min-height:70px;resize:vertical}
button{width:100%;padding:12px;background:#4f46e5;color:#fff;border:none;border-radius:6px;font-size:1em;cursor:pointer}
button:hover{background:#4338ca}
button:disabled{background:#aaa}
.btn-s{background:#10b981}
.btn-g{background:#6b7280}
.btn-o{background:#f59e0b}
.row{display:flex;gap:8px;margin-bottom:8px}
.row button{flex:1;margin:0}
.ch{border:1px solid #ddd;border-radius:6px;margin-bottom:8px;background:#fafafa;transition:all .2s}
.ch.en{border-color:#4f46e5;background:#fff}
.ch-h{padding:12px;display:flex;align-items:center;gap:10px;cursor:pointer;user-select:none}
.ch-h input[type=checkbox]{width:18px;height:18px;flex-shrink:0}
.ch-h span{flex:1;font-weight:500}
.ch-h:after{content:'▼';font-size:.7em;color:#999;transition:transform .2s}
.ch.open .ch-h:after{transform:rotate(180deg)}
.ch-b{padding:0 12px 12px;display:none}
.ch.open .ch-b{display:block}
.badge{padding:2px 8px;border-radius:10px;font-size:.8em}
.on{background:#dcfce7;color:#166534}
.off{background:#fee2e2;color:#991b1b}
.msg{padding:10px;border-radius:6px;margin-top:8px;display:none}
.msg.ok{display:block;background:#dcfce7;color:#166534}
.msg.err{display:block;background:#fee2e2;color:#991b1b}
.msg.info{display:block;background:#e0f2fe;color:#0369a1}
.it td{padding:6px 0;border-bottom:1px solid #eee}
.it td:first-child{color:#666;width:40%}
.timer-box{background:#e0f2fe;border-radius:6px;padding:12px;margin-bottom:12px;text-align:center}
.timer-box .status{font-size:.9em;color:#0369a1}
.timer-box .countdown{font-size:1.3em;font-weight:600;color:#0369a1;margin-top:4px}
</style>)";

// 通用 JS
const char* commonJs = R"(<script>
function $(id){return document.getElementById(id)}
function post(url,data,cb){
  fetch(url,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:data})
  .then(r=>r.json()).then(cb).catch(e=>cb({success:false,message:String(e)}));
}
function msg(id,ok,txt){var m=$(id);m.className='msg '+(ok?'ok':'err');m.textContent=txt;m.style.display='block'}
</script>)";

// 配置页面
const char* htmlPage = R"rawliteral(<!DOCTYPE html>
<html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>短信转发配置</title>
%COMMON_CSS%</head><body><div class="c">
<h1>短信转发器</h1>
<div class="nav"><a href="/" class="on">配置</a><a href="/tools">工具</a></div>
<div class="card" style="display:flex;justify-content:space-between;align-items:center">
<div><small>IP</small><br><b>%IP%</b></div>
<div><span class="badge %MQTT_CLASS%">MQTT %MQTT_STATUS%</span></div>
</div>
<form id="f" onsubmit="return save(event)">
<div class="card">
<div class="card-t">管理账号</div>
<div class="fg"><label>账号</label><input name="webUser" value="%WEB_USER%"></div>
<div class="fg"><label>密码</label><input type="password" name="webPass" value="%WEB_PASS%"></div>
</div>
<div class="card">
<div class="card-t">通知方式</div>

<div class="ch%SMTP_EN_CLASS%" id="ch_email">
<div class="ch-h" onclick="fold('_email')">
<input type="checkbox" name="smtpEn" id="en_email" onclick="event.stopPropagation();en('_email')" %SMTP_CHECKED%>
<span>邮件通知</span>
</div>
<div class="ch-b">
<div class="fg"><label>SMTP服务器</label><input name="smtpServer" value="%SMTP_SERVER%"></div>
<div class="fg"><label>端口</label><input type="number" name="smtpPort" value="%SMTP_PORT%"></div>
<div class="fg"><label>发件账号</label><input name="smtpUser" value="%SMTP_USER%"></div>
<div class="fg"><label>授权码</label><input type="password" name="smtpPass" value="%SMTP_PASS%"></div>
<div class="fg"><label>收件邮箱</label><input name="smtpSendTo" value="%SMTP_SEND_TO%"></div>
</div></div>

<div class="ch%MQTT_EN_CLASS%" id="ch_mqtt">
<div class="ch-h" onclick="fold('_mqtt')">
<input type="checkbox" name="mqttEn" id="en_mqtt" onclick="event.stopPropagation();en('_mqtt')" %MQTT_CHECKED%>
<span>MQTT</span>
</div>
<div class="ch-b">
<div class="fg"><label>服务器</label><input name="mqttServer" value="%MQTT_SERVER%" placeholder="mqtt.example.com"></div>
<div class="fg"><label>端口</label><input type="number" name="mqttPort" value="%MQTT_PORT%" placeholder="1883"></div>
<div class="fg"><label>用户名</label><input name="mqttUser" value="%MQTT_USER%"></div>
<div class="fg"><label>密码</label><input type="password" name="mqttPass" value="%MQTT_PASS%"></div>
<div class="fg"><label>主题前缀</label><input name="mqttPrefix" value="%MQTT_PREFIX%" placeholder="sms"></div>
<div class="fg" style="display:flex;align-items:center;gap:8px;padding:8px 0">
<input type="checkbox" name="mqttCtrlOnly" id="mqttCtrlOnly" style="width:18px;height:18px" %MQTT_CTRL_ONLY%>
<label for="mqttCtrlOnly" style="margin:0;flex:1">仅控制模式 <small style="color:#999">(不推送短信内容)</small></label>
</div>
</div></div>

%PUSH_CHANNELS%
</div>
<button type="submit">保存配置</button>
<div class="msg" id="saveMsg"></div>
</form>
</div>
%COMMON_JS%
<script>
function fold(i){var c=$('ch'+i);if(c)c.classList.toggle('open')}
function en(i){var c=$('ch'+i),b=$('en'+i);if(b&&c){if(b.checked)c.classList.add('en');else c.classList.remove('en')}}
function upd(i){if(typeof i==='string')return;var t=$('tp'+i),cf=$('cf'+i);if(t&&cf)cf.style.display=t.value==4?'block':'none'}
function save(e){e.preventDefault();var f=$('f'),btn=f.querySelector('button[type=submit]');btn.disabled=true;btn.textContent='保存中...';
post('/save',new URLSearchParams(new FormData(f)).toString(),function(d){
btn.disabled=false;btn.textContent='保存配置';msg('saveMsg',d.success,d.message||'操作完成')});return false}
document.addEventListener('DOMContentLoaded',function(){en('_email');en('_mqtt');for(var i=0;i<5;i++){en(i);upd(i)}})
</script></body></html>)rawliteral";

// 工具箱页面
const char* htmlToolsPage = R"rawliteral(<!DOCTYPE html>
<html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>工具箱</title>
%COMMON_CSS%</head><body><div class="c">
<h1>短信转发器</h1>
<div class="nav"><a href="/">配置</a><a href="/tools" class="on">工具</a></div>
<div class="card" style="display:flex;justify-content:space-between;align-items:center">
<div><small>IP</small><br><b>%IP%</b></div>
<div><span class="badge %MQTT_CLASS%">MQTT %MQTT_STATUS%</span></div>
</div>
<div class="card">
<div class="card-t">发送短信</div>
<div class="fg"><label>目标号码</label><input id="smsPhone" placeholder="13800138000"></div>
<div class="fg"><label>内容</label><textarea id="smsTxt" placeholder="短信内容"></textarea></div>
<button onclick="sendSms()">发送</button>
<div class="msg" id="smsMsg"></div>
</div>
<div class="card">
<div class="card-t">定时任务</div>
<div class="timer-box">
<div class="status" id="timerStatus">%TIMER_STATUS%</div>
<div class="countdown" id="countdown">%TIMER_COUNTDOWN%</div>
</div>
<div class="ch-h" style="padding:8px 0" onclick="var cb=$('timerEn');cb.checked=!cb.checked">
<input type="checkbox" id="timerEn" %TIMER_CHECKED% onclick="event.stopPropagation()">
<span>启用定时任务</span>
</div>
<div class="fg"><label>类型</label><select id="timerType" onchange="$('smsF').style.display=this.value==1?'block':'none'">
<option value="0" %TIMER_TYPE0%>定时Ping</option><option value="1" %TIMER_TYPE1%>定时短信</option></select></div>
<div class="fg"><label>间隔(天)</label><input type="number" id="timerInt" value="%TIMER_INTERVAL%" min="1" max="365"></div>
<div id="smsF" style="display:none">
<div class="fg"><label>号码</label><input id="timerPhone" value="%TIMER_PHONE%"></div>
<div class="fg"><label>内容</label><input id="timerMsgIn" value="%TIMER_MSG%"></div>
</div>
<button class="btn-s" onclick="saveTimer()">保存定时设置</button>
<div class="msg" id="timerMsg2"></div>
</div>
<div class="card">
<div class="card-t">模组查询</div>
<div class="row"><button class="btn-g" onclick="q('ati')">固件</button><button class="btn-g" onclick="q('signal')">信号</button><button class="btn-g" onclick="q('siminfo')">SIM卡</button></div>
<div class="row"><button class="btn-g" onclick="q('network')">网络</button><button class="btn-g" onclick="q('wifi')">WiFi</button></div>
<div class="msg info" id="qr" style="display:none"></div>
</div>
<div class="card">
<div class="card-t">网络测试</div>
<button class="btn-o" id="pingBtn" onclick="confirmPing()">Ping 8.8.8.8 (消耗流量)</button>
<div class="msg" id="pingR"></div>
</div>
</div>
%COMMON_JS%
<script>
var timerRemain=%TIMER_REMAIN%;
function formatTime(s){if(s<=0)return'--';var d=Math.floor(s/86400),h=Math.floor((s%86400)/3600),m=Math.floor((s%3600)/60);
return (d>0?d+'天':'')+(h>0?h+'时':'')+(m>0?m+'分':'')+'后执行'}
function updateCountdown(){$('countdown').textContent=formatTime(timerRemain);if(timerRemain>0){timerRemain--;setTimeout(updateCountdown,1000)}}
function sendSms(){var p=$('smsPhone').value,t=$('smsTxt').value;if(!p||!t){msg('smsMsg',false,'请填写号码和内容');return}
post('/sendsms','phone='+encodeURIComponent(p)+'&content='+encodeURIComponent(t),function(d){msg('smsMsg',d.success,d.message)})}
function saveTimer(){var d={enabled:$('timerEn').checked,type:+$('timerType').value,interval:+$('timerInt').value,
phone:$('timerPhone').value,message:$('timerMsgIn').value};
fetch('/timer',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(d)})
.then(r=>r.json()).then(r=>{msg('timerMsg2',r.success,r.success?'已保存':'失败');
if(r.success){timerRemain=r.remain;$('timerStatus').textContent=d.enabled?(d.type==0?'定时Ping':'定时短信'):'已禁用';updateCountdown()}
}).catch(e=>msg('timerMsg2',false,e))}
function q(t){var r=$('qr');r.className='msg info';r.style.display='block';r.innerHTML='查询中...';
fetch('/query?type='+t).then(r=>r.json()).then(d=>{r.innerHTML=d.success?d.message:('失败:'+d.message)}).catch(e=>{r.innerHTML='错误:'+e})}
function confirmPing(){if(confirm('Ping测试会消耗少量移动数据流量，确定执行吗？')){doPing()}}
function doPing(){var b=$('pingBtn'),r=$('pingR');b.disabled=true;b.textContent='Ping中...';r.className='msg info';r.style.display='block';r.textContent='请稍候(最长30秒)...';
fetch('/ping',{method:'POST'}).then(x=>x.json()).then(d=>{b.disabled=false;b.textContent='Ping 8.8.8.8 (消耗流量)';
r.className='msg '+(d.success?'ok':'err');r.textContent=d.message}).catch(e=>{b.disabled=false;b.textContent='Ping 8.8.8.8 (消耗流量)';r.className='msg err';r.textContent=e})}
$('timerType').onchange();updateCountdown();
</script></body></html>)rawliteral";

#endif
