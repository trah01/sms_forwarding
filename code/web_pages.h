#ifndef WEB_PAGES_H
#define WEB_PAGES_H

// 尝试引入私有功能模块（如果文件存在）
#if __has_include("call_forward_private.h")
  #include "call_forward_private.h"
#endif

// 現代化 CSS 设计
const char* commonCss = R"(<style>
:root{--primary:#6366f1;--primary-dark:#4f46e5;--bg:#f8fafc;--card:#ffffff;--text:#334155;--text-light:#64748b;--border:#e2e8f0;--success:#22c55e;--danger:#ef4444;--warning:#eab308;--info:#3b82f6}
*{box-sizing:border-box;margin:0;padding:0;outline:none;-webkit-tap-highlight-color:transparent}
body{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Helvetica,Arial,sans-serif;background:var(--bg);color:var(--text);line-height:1.5;padding:12px 12px 70px}
.c{max-width:900px;margin:0 auto;padding:0 20px}

/* 导航栏 */
.nav{position:fixed;bottom:0;left:50%;transform:translateX(-50%);width:calc(100% - 24px);max-width:900px;background:rgba(255,255,255,0.95);backdrop-filter:blur(10px);border:1px solid var(--border);display:flex;justify-content:space-around;padding:8px;z-index:999;border-radius:16px;margin-bottom:12px;box-shadow:0 -2px 10px rgba(0,0,0,0.05)}
.nav-item{flex:1;text-align:center;padding:6px;border-radius:12px;color:var(--text-light);font-size:0.75em;cursor:pointer;transition:all .2s;display:flex;flex-direction:column;align-items:center;gap:4px}
.nav-item.active{color:var(--primary);background:#e0e7ff33}
.nav-icon{width:24px;height:24px;background:currentcolor;mask-size:contain;mask-repeat:no-repeat;-webkit-mask-size:contain;-webkit-mask-repeat:no-repeat}

/* 页面 */
.page{display:none;animation:fadeIn .3s ease}.page.active{display:block}
@keyframes fadeIn{from{opacity:0;transform:translateY(5px)}to{opacity:1;transform:translateY(0)}}

/* 卡片 */
.card{background:var(--card);border-radius:16px;padding:16px;margin-bottom:12px;box-shadow:0 1px 3px rgba(0,0,0,0.05);border:1px solid var(--border)}
.card-t{font-size:1.1em;font-weight:700;margin-bottom:12px;color:#1e293b;display:flex;justify-content:space-between;align-items:center}
.card-sub{font-size:0.85em;color:var(--text-light);font-weight:400}

/* 数据看板 */
.grid-2{display:grid;grid-template-columns:1fr 1fr;gap:12px}
.grid-3{display:grid;grid-template-columns:1fr 1fr 1fr;gap:12px}
.stat-box{background:#f8fafc;padding:12px;border-radius:12px;text-align:center;border:1px solid var(--border)}
.stat-num{font-size:1.4em;font-weight:800;color:var(--primary);line-height:1.2}
.stat-tag{font-size:0.8em;color:var(--text-light)}

/* 状态徽章 */
.badge{padding:3px 8px;border-radius:99px;font-size:0.75em;font-weight:600;display:inline-flex;align-items:center;gap:4px}
.b-ok{background:#dcfce7;color:#15803d}
.b-err{background:#fee2e2;color:#b91c1c}
.b-wait{background:#f1f5f9;color:#475569}
.b-warn{background:#fef9c3;color:#a16207}

/* 表单元素 */
.fg{margin-bottom:16px}
label{display:block;margin-bottom:6px;font-size:0.9em;font-weight:600;color:#475569}
input,select,textarea{width:100%;padding:12px;border:1px solid var(--border);border-radius:10px;font-size:1em;color:#1e293b;background:#fff;transition:border-color .2s}
input:focus,textarea:focus{border-color:var(--primary);box-shadow:0 0 0 3px rgba(99,102,241,0.1)}
.btn{width:100%;padding:14px;border:none;border-radius:12px;background:var(--primary);color:#fff;font-weight:600;font-size:1em;cursor:pointer;transition:transform .1s,opacity .2s;display:flex;align-items:center;justify-content:center;gap:8px}
.btn:active{transform:scale(0.98);opacity:0.9}
.btn-w{background:#fff;color:var(--text);border:1px solid var(--border)}
.btn-d{background:var(--danger);color:#fff}

/* 折叠面板 */
details{border:1px solid var(--border);border-radius:12px;margin-bottom:8px;overflow:hidden;background:#fff}
summary{padding:14px;background:#f8fafc;cursor:pointer;font-weight:600;list-style:none;display:flex;justify-content:space-between;align-items:center}
summary::-webkit-details-marker{display:none}
summary:after{content:'+';font-size:1.2em;color:var(--text-light)}
details[open] summary:after{content:'-'}
details[open] summary{border-bottom:1px solid var(--border)}
.det-body{padding:16px}

/* 开关行 */
.sw-row{display:flex;justify-content:space-between;align-items:center;padding:8px 0;cursor:pointer}
.sw{width:44px;height:24px;background:#cbd5e1;border-radius:24px;position:relative;transition:0.3s}
.sw:after{content:'';width:20px;height:20px;background:#fff;border-radius:50%;position:absolute;top:2px;left:2px;transition:0.3s;box-shadow:0 1px 2px rgba(0,0,0,0.2)}
.sw.on{background:var(--primary)}
.sw.on:after{left:22px}

/* 历史记录 */
.log-list{display:flex;flex-direction:column;gap:8px}
.log-item{padding:12px;background:#f8fafc;border-radius:10px;border-left:4px solid var(--primary);font-size:0.9em}
.log-h{display:flex;justify-content:space-between;margin-bottom:4px;color:var(--text-light);font-size:0.85em}
.log-m{line-height:1.4;word-break:break-all}

/* 消息提示 */
.toast{position:fixed;top:20px;left:50%;transform:translateX(-50%);background:rgba(0,0,0,0.8);color:#fff;padding:10px 20px;border-radius:99px;font-size:0.9em;z-index:9999;transition:opacity .3s;opacity:0;pointer-events:none}
.toast.show{opacity:1;top:24px}

/* 信息表格 */
.info-table{width:100%;font-size:0.85em}
.info-table td{padding:4px 0;border-bottom:1px solid #f1f5f9}
.info-table td:first-child{color:var(--text-light);width:40%}
</style>
<script>
const $=id=>document.getElementById(id);
const $s=sel=>document.querySelector(sel);
const postJ=(u,d,cb)=>{
  fetch(u,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(d)})
  .then(r=>r.json()).then(cb).catch(e=>toast('请求失败: '+e))
};
let toastTimer;
function toast(msg){
  let t=$('toast');t.innerText=msg;t.className='toast show';
  clearTimeout(toastTimer);toastTimer=setTimeout(()=>t.className='toast',2000);
}
</script>)";

// 统一 JS
const char* commonJs = "";

// 完整的 HTML
const char* htmlPage = R"rawliteral(<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=0"><title>短信转发器</title>
%COMMON_CSS%
</head><body><div class="c">

<div class="nav">
  <div class="nav-item active" onclick="swTab(0)">
    <div class="nav-icon" style="-webkit-mask-image:url('data:image/svg+xml;utf8,<svg viewBox=\'0 0 24 24\' xmlns=\'http://www.w3.org/2000/svg\'><path d=\'M3 13h8V3H3v10zm0 8h8v-6H3v6zm10 0h8V11h-8v10zm0-18v6h8V3h-8z\'/></svg>')"></div>
    <span>概览</span>
  </div>
  <div class="nav-item" onclick="swTab(1)">
    <div class="nav-icon" style="-webkit-mask-image:url('data:image/svg+xml;utf8,<svg viewBox=\'0 0 24 24\' xmlns=\'http://www.w3.org/2000/svg\'><path d=\'M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm-1 17.93c-3.95-.49-7-3.85-7-7.93 0-.62.08-1.21.21-1.79L9 15v1c0 1.1.9 2 2 2v1.93zm6.9-2.54c-.26-.81-1-1.39-1.9-1.39h-1v-3c0-.55-.45-1-1-1H8v-2h2c.55 0 1-.45 1-1V7h2c1.1 0 2-.9 2-2v-.41c2.93 1.19 5 4.06 5 7.41 0 2.08-.8 3.97-2.1 5.39z\'/></svg>')"></div>
    <span>控制</span>
  </div>
  <div class="nav-item" onclick="swTab(2)">
    <div class="nav-icon" style="-webkit-mask-image:url('data:image/svg+xml;utf8,<svg viewBox=\'0 0 24 24\' xmlns=\'http://www.w3.org/2000/svg\'><path d=\'M13 3c-4.97 0-9 4.03-9 9H1l3.89 3.89.07.14L9 12H6c0-3.87 3.13-7 7-7s7 3.13 7 7-3.13 7-7 7c-1.93 0-3.68-.79-4.94-2.06l-1.42 1.42C8.27 19.99 10.51 21 13 21c4.97 0 9-4.03 9-9s-4.03-9-9-9zm-1 5v5l4.28 2.54.72-1.21-3.5-2.08V8H12z\'/></svg>')"></div>
    <span>历史</span>
  </div>
  <div class="nav-item" onclick="swTab(3)">
    <div class="nav-icon" style="-webkit-mask-image:url('data:image/svg+xml;utf8,<svg viewBox=%270 0 24 24%27 xmlns=%27http://www.w3.org/2000/svg%27><path d=%27M12 15.5A3.5 3.5 0 0 1 8.5 12 3.5 3.5 0 0 1 12 8.5a3.5 3.5 0 0 1 3.5 3.5 3.5 3.5 0 0 1-3.5 3.5m7.43-2.53c.04-.32.07-.64.07-.97s-.03-.66-.07-1l2.11-1.63c.19-.15.24-.42.12-.64l-2-3.46c-.12-.22-.39-.31-.61-.22l-2.49 1c-.52-.39-1.06-.73-1.69-.98l-.37-2.65A.506.506 0 0 0 14 2h-4c-.25 0-.46.18-.5.42l-.37 2.65c-.63.25-1.17.59-1.69.98l-2.49-1c-.22-.09-.49 0-.61.22l-2 3.46c-.13.22-.07.49.12.64L4.57 11c-.04.34-.07.67-.07 1s.03.65.07.97l-2.11 1.66c-.19.15-.25.42-.12.64l2 3.46c.12.22.39.3.61.22l2.49-1.01c.52.4 1.06.74 1.69.99l.37 2.65c.04.24.25.42.5.42h4c.25 0 .46-.18.5-.42l.37-2.65c.63-.26 1.17-.59 1.69-.99l2.49 1.01c.22.08.49 0 .61-.22l2-3.46c.12-.22.07-.49-.12-.64l-2.11-1.66z%27/></svg>');mask-image:url('data:image/svg+xml;utf8,<svg viewBox=%270 0 24 24%27 xmlns=%27http://www.w3.org/2000/svg%27><path d=%27M12 15.5A3.5 3.5 0 0 1 8.5 12 3.5 3.5 0 0 1 12 8.5a3.5 3.5 0 0 1 3.5 3.5 3.5 3.5 0 0 1-3.5 3.5m7.43-2.53c.04-.32.07-.64.07-.97s-.03-.66-.07-1l2.11-1.63c.19-.15.24-.42.12-.64l-2-3.46c-.12-.22-.39-.31-.61-.22l-2.49 1c-.52-.39-1.06-.73-1.69-.98l-.37-2.65A.506.506 0 0 0 14 2h-4c-.25 0-.46.18-.5.42l-.37 2.65c-.63.25-1.17.59-1.69.98l-2.49-1c-.22-.09-.49 0-.61.22l-2 3.46c-.13.22-.07.49.12.64L4.57 11c-.04.34-.07.67-.07 1s.03.65.07.97l-2.11 1.66c-.19.15-.25.42-.12.64l2 3.46c.12.22.39.3.61.22l2.49-1.01c.52.4 1.06.74 1.69.99l.37 2.65c.04.24.25.42.5.42h4c.25 0 .46-.18.5-.42l.37-2.65c.63-.26 1.17-.59 1.69-.99l2.49 1.01c.22.08.49 0 .61-.22l2-3.46c.12-.22.07-.49-.12-.64l-2.11-1.66z%27/></svg>')"></div>
    <span>配置</span>
  </div>
</div>

<div id="toast" class="toast"></div>

<!-- ============ 概览页 ============ -->
<div class="page active">
  <div class="card">
    <div class="card-t">统计数据 <span class="badge b-wait" id="upT">-</span></div>
    <div class="grid-3">
      <div class="stat-box"><div class="stat-num" id="ssRecv">-</div><div class="stat-tag">收信</div></div>
      <div class="stat-box"><div class="stat-num" id="ssSent">-</div><div class="stat-tag">发信</div></div>
      <div class="stat-box"><div class="stat-num" id="ssBoot">-</div><div class="stat-tag">重启</div></div>
    </div>
  </div>

  <div class="card">
    <div class="card-t">状态看板</div>
    <div class="grid-2" style="margin-bottom:12px">
      <div class="stat-box" style="text-align:left">
        <div class="stat-tag">WiFi 信号</div>
        <div style="font-weight:700" id="wifiS">- dBm</div>
        <div class="badge b-ok" id="ipStr">%IP%</div>
      </div>
      <div class="stat-box" style="text-align:left">
        <div class="stat-tag">MQTT 状态</div>
        <div style="font-weight:700" id="mqS">%MQTT_STATUS%</div>
        <div style="font-size:0.8em;color:var(--text-light);margin-top:4px;word-break:break-all" id="mqTopics">%MQTT_TOPICS%</div>
      </div>
    </div>
    
    <div class="stat-box" style="text-align:left;position:relative;margin-bottom:12px">
       <div class="stat-tag">模组网络</div>
       <div style="font-weight:700;font-size:1.1em;margin:4px 0" id="modNet">查询中...</div>
       <div style="font-size:0.85em;color:#64748b" id="modSig">信号强度: -</div>
       <div style="position:absolute;top:10px;right:10px" class="badge b-wait" id="modSim">SIM?</div>
    </div>

    <div class="sw-row">
      <span style="font-weight:600;color:#64748b">来电通知</span>
      <span class="badge %CLIP_CLASS%">%CLIP_STATUS%</span>
    </div>
    <div class="sw-row">
      <span style="font-weight:600;color:#64748b">系统看门狗</span>
      <span class="badge b-ok">30秒自动复位</span>
    </div>
  </div>
</div>

<!-- ============ 控制页 ============ -->
<div class="page">
  <div class="card">
    <div class="card-t">发送短信</div>
    <div class="fg"><label>目标号码</label><input id="sPh" placeholder="10086" type="tel"></div>
    <div class="fg"><label>短信内容</label><textarea id="sTx" placeholder="内容支持中文" rows="4"></textarea></div>
    <button class="btn" onclick="act('sms')">发送 (Send)</button>
  </div>

  <div class="card">
    <div class="card-t">流量保号</div>
    <div class="sw-row"><span id="pStat">Ping 8.8.8.8 (会产生少量流量费用)</span> <button class="btn btn-w" style="width:auto;padding:6px 16px" onclick="if(confirm('确定要消耗流量保号吗？\n此操作会产生少量流量/话费'))act('ping')">消耗流量</button></div>
    <div id="pLog" style="margin-top:8px;font-size:0.85em;color:#64748b;display:none"></div>
  </div>

  <div class="card">
    <div class="card-t">余额查询 <span class="card-sub">USSD免费</span></div>
    <div class="fg" style="margin-bottom:8px">
      <select id="ussdSel" style="padding:10px;width:100%" onchange="ussdChg()">
        <option value="*100#">giffgaff (*100#)</option>
        <option value="*100#">中国移动 (*100#)</option>
        <option value="*100#">中国联通 (*100#)</option>
        <option value="*102#">中国电信 (*102#)</option>
        <option value="custom">自定义代码...</option>
      </select>
    </div>
    <div id="ussdCust" style="display:none;margin-bottom:8px">
      <input id="ussdCode" placeholder="输入USSD代码，如 *100#" style="padding:10px">
    </div>
    <button class="btn btn-w" onclick="queryBalance()">查询余额</button>
    <div id="balLog" style="margin-top:12px;font-size:0.9em;color:#64748b;display:none;padding:10px;background:#f8fafc;border-radius:8px"></div>
  </div>

%CALL_FORWARD%

  <div class="card">
    <div class="card-t" style="color:var(--danger)">危险操作</div>
    <button class="btn btn-d" onclick="if(confirm('确定重启?'))act('reboot')">重启设备 (Reboot)</button>
  </div>
</div>

<!-- ============ 历史页 ============ -->
<div class="page">
  <div class="card">
    <div class="card-t">短信历史 <div class="badge b-wait" onclick="loadHist()">刷新</div></div>
    <div id="hList" class="log-list">
      <div style="text-align:center;padding:20px;color:#94a3b8">加载中...</div>
    </div>
  </div>
</div>

<!-- ============ 配置页 ============ -->
<div class="page">
  <form id="cf" onsubmit="return saveAll(event)">
  
  <details open>
    <summary>WiFi 网络 <span class="card-sub">自动选择信号最强的</span></summary>
    <div class="det-body">
      %WIFI_NETWORKS%
      <div style="font-size:0.85em;color:var(--text-light);margin-top:8px">
        💡 可配置多个WiFi，设备会自动连接信号最强的网络。修改后需重启生效。
      </div>
    </div>
  </details>

  <details>
    <summary>Web 管理 & 邮箱</summary>
    <div class="det-body">
      <div class="grid-2">
        <div class="fg"><label>Web账号</label><input name="webUser" value="%WEB_USER%"></div>
        <div class="fg"><label>Web密码</label><input name="webPass" value="%WEB_PASS%"></div>
      </div>
      <div class="sw-row" onclick="xToggle('smEn')">
         <span>启用 SMTP 邮件推送</span>
         <div id="smEnSw" class="sw %SMTP_EN_SW%"></div>
         <input type="hidden" id="smEn" name="smtpEn" value="%SMTP_EN_VAL%">
      </div>
      <div id="smBox" style="display:%SMTP_DISP%">
        <div class="fg"><label>SMTP 服务器</label><input name="smtpServer" value="%SMTP_SERVER%"></div>
        <div class="grid-2">
           <div class="fg"><label>端口</label><input name="smtpPort" type="number" value="%SMTP_PORT%"></div>
           <div class="fg"><label>发件账号</label><input name="smtpUser" value="%SMTP_USER%"></div>
        </div>
        <div class="fg"><label>授权码/密码</label><input name="smtpPass" type="password" value="%SMTP_PASS%"></div>
        <div class="fg"><label>接收邮箱</label><input name="smtpSendTo" value="%SMTP_SEND_TO%"></div>
      </div>
    </div>
  </details>

  <details>
    <summary>MQTT 设置</summary>
    <div class="det-body">
      <div class="sw-row" onclick="xToggle('mqEn')">
         <span>启用 MQTT</span>
         <div id="mqEnSw" class="sw %MQTT_EN_SW%"></div>
         <input type="hidden" id="mqEn" name="mqttEn" value="%MQTT_EN_VAL%">
      </div>
      <div id="mqBox" style="display:%MQTT_DISP%">
         <div class="grid-2">
            <div class="fg"><label>服务器</label><input name="mqttServer" value="%MQTT_SERVER%"></div>
            <div class="fg"><label>端口</label><input name="mqttPort" type="number" value="%MQTT_PORT%"></div>
         </div>
         <div class="grid-2">
            <div class="fg"><label>账号</label><input name="mqttUser" value="%MQTT_USER%"></div>
            <div class="fg"><label>密码</label><input name="mqttPass" type="password" value="%MQTT_PASS%"></div>
         </div>
         <div class="fg"><label>Topic 前缀</label><input name="mqttPrefix" value="%MQTT_PREFIX%"></div>
         <div class="sw-row" onclick="xToggle('mqCo')">
            <span>仅控制模式 (不推内容)</span>
            <div id="mqCoSw" class="sw %MQTT_CO_SW%"></div>
            <input type="hidden" id="mqCo" name="mqttCtrlOnly" value="%MQTT_CO_VAL%">
         </div>
      </div>
    </div>
  </details>

  <details>
    <summary>Web 通道</summary>
    <div class="det-body">
      %PUSH_CHANNELS%
    </div>
  </details>

  <details>
    <summary>黑白名单过滤 (Max 100)</summary>
    <div class="det-body">
      <div class="sw-row" onclick="xToggle('ftEn')">
         <span>启用过滤</span>
         <div id="ftEnSw" class="sw"></div>
         <input type="hidden" id="ftEn" name="filterEn" value="%FILTER_EN_VAL%">
      </div>
      <div class="fg" style="margin-top:10px">
        <div style="display:flex;gap:12px;margin-bottom:8px">
          <label><input type="radio" name="filterIsWhitelist" value="false" style="width:auto"> 黑名单 (拦截)</label>
          <label><input type="radio" name="filterIsWhitelist" value="true" style="width:auto"> 白名单 (放行)</label>
        </div>
        <textarea id="ftList" name="filterList" rows="5" placeholder="输入号码，多个号码用逗号分隔，例如: 10086,13800000000"></textarea>
      </div>
      <button type="button" class="btn btn-w" onclick="saveFilter()">仅保存名单设置</button>
    </div>
  </details>

  <details>
    <summary>定时任务</summary>
    <div class="det-body">
      <div class="sw-row" onclick="xToggle('tmEn')">
         <span>启用任务</span>
         <div id="tmEnSw" class="sw"></div>
         <input type="hidden" id="tmEn" name="timerEn" value="%TIMER_EN_VAL%">
      </div>
      <div class="fg"><label>类型 & 间隔(天)</label>
        <div class="grid-2">
          <select id="tmType" name="timerType" onchange="$('tmSms').style.display=this.value==1?'block':'none'"><option value="0">Ping保活</option><option value="1">发送短信</option></select>
          <input type="number" id="tmInt" name="timerInterval" min="1">
        </div>
      </div>
      <div id="tmSms" style="display:none">
        <div class="fg"><label>对方号码</label><input id="tmPh" name="timerPhone"></div>
        <div class="fg"><label>短信内容</label><input id="tmMsg" name="timerMessage"></div>
      </div>
      <div style="color:var(--info);font-size:0.9em;margin-bottom:12px" id="tmInfo"></div>
      <button type="button" class="btn btn-w" onclick="saveTimer()">仅保存任务设置</button>
    </div>
  </details>

  <div style="height:60px"></div>
  <div style="position:fixed;bottom:80px;left:0;right:0;padding:12px;pointer-events:none">
    <button class="c btn" style="pointer-events:auto;box-shadow:0 10px 20px rgba(99,102,241,0.3)">保存所有系统配置</button>
  </div>

  </form>
</div>

<script>
// 初始化数据
var ft={en:%FILTER_EN_BOOL%,wl:%FILTER_WL_BOOL%,ls:'%FILTER_LIST%'};
var tm={en:%TIMER_EN_BOOL%,tp:%TIMER_TP%,int:%TIMER_INT%,ph:'%TIMER_PH%',ms:'%TIMER_MS%',rm:%TIMER_RM%};

// 状态初始化
if(ft.en){$('ftEnSw').className='sw on';$('ftEn').value='true'}
if(ft.wl)document.getElementsByName('filterIsWhitelist')[1].checked=true;else document.getElementsByName('filterIsWhitelist')[0].checked=true;
$('ftList').value=ft.ls;

if(tm.en){$('tmEnSw').className='sw on';$('tmEn').value='true'}
$('tmType').value=tm.tp;$('tmInt').value=tm.int;$('tmPh').value=tm.ph;$('tmMsg').value=tm.ms;
if(tm.tp==1)$('tmSms').style.display='block';
$('tmInfo').innerText = tm.en ? ('下次执行: '+(tm.rm/86400).toFixed(1)+'天后') : '任务已禁用';

// 页面切换
function swTab(n){
  document.querySelectorAll('.nav-item').forEach((e,i)=>e.className='nav-item'+(i===n?' active':''));
  document.querySelectorAll('.page').forEach((e,i)=>e.className='page'+(i===n?' active':''));
  if(n===2)loadHist();
}

// 交互函数
function xToggle(id){
  var i=$(id),s=$(id+'Sw');
  if(i.value==='true'){i.value='false';s.className='sw'}
  else{i.value='true';s.className='sw on'}
  // 联动显示
  if(id==='smEn')$('smBox').style.display=i.value==='true'?'block':'none';
  if(id==='mqEn')$('mqBox').style.display=i.value==='true'?'block':'none';
}
function fd(i){
  var b=$('chb'+i),s=$('chi'+i);
  if(b.style.display==='none'){b.style.display='block';s.style.transform='rotate(90deg)'}
  else{b.style.display='none';s.style.transform='rotate(0)'}
}
function chTog(i){
  var v=$('che'+i),s=$('chs'+i);
  if(v.value==='true'){v.value='false';s.className='sw'}
  else{v.value='true';s.className='sw on'}
}
function wfTog(i){
  var v=$('wfe'+i),s=$('wfs'+i);
  if(v.value==='true'){v.value='false';s.className='sw'}
  else{v.value='true';s.className='sw on'}
}
function upd(i){
  var t=$('tp'+i).value;
  $('cf'+i).style.display=(t=='4')?'block':'none'; // 自定义模板
  $('tg'+i).style.display=(t=='5')?'block':'none'; // Telegram Chat ID
}

// 自动加载函数
function autoLoad(){
  fetch('/stats').then(r=>r.json()).then(d=>{
    $('ssRecv').innerText=d.received;$('ssSent').innerText=d.sent;$('ssBoot').innerText=d.boots;
    $('wifiS').innerText=d.wifiRssi+' dBm';
    var h=Math.floor(d.uptime/3600);
    $('upT').innerText='运行 '+h+' 小时 / 内存 '+(d.freeHeap/1024).toFixed(0)+'K';
  }).catch(e=>{console.log('stats error',e)});
  
  // 自动查询模组信息
  fetch('/query?type=network').then(r=>r.json()).then(d=>{
    if(d.success){$('modNet').innerHTML=d.message;$('modNet').style.color='#15803d'}
    else{$('modNet').innerText='未注册网络';$('modNet').style.color='#b91c1c'}
  }).catch(e=>{$('modNet').innerText='查询失败';console.log('network error',e)});
  
  setTimeout(()=>{
    fetch('/query?type=signal').then(r=>r.json()).then(d=>{$('modSig').innerHTML=d.message}).catch(e=>{});
    fetch('/query?type=siminfo').then(r=>r.json()).then(d=>{
        var s=$('modSim');
        if(d.success){s.innerText='SIM OK';s.className='badge b-ok'}else{s.innerText='SIM ERR';s.className='badge b-err'}
    }).catch(e=>{});
  },2000);
}

function act(t){
  if(t==='reboot'){postJ('/restart',{},d=>toast(d.message));return}
  if(t==='sms'){
    var p=$('sPh').value,x=$('sTx').value;
    if(!p||!x)return toast('请填写号码和内容');
    toast('发送中...');
    postJ('/sendsms',{phone:p,content:x},d=>toast(d.message));
  }
  if(t==='ping'){
    var l=$('pLog');l.style.display='block';l.innerText='正在 Ping 8.8.8.8 ...';
    postJ('/ping',{},d=>{
        l.innerText=d.message;l.style.color=d.success?'#15803d':'#b91c1c';
    });
  }
}

function ussdChg(){
  var sel=$('ussdSel').value;
  $('ussdCust').style.display=(sel==='custom')?'block':'none';
}

function queryBalance(){
  var sel=$('ussdSel').value;
  var code=(sel==='custom')?$('ussdCode').value:sel;
  if(!code||code.length<2){return toast('请输入有效的USSD代码');}
  var l=$('balLog');l.style.display='block';l.innerHTML='正在查询 '+code+' ，请稍候...';
  fetch('/query?type=balance&code='+encodeURIComponent(code)).then(r=>r.json()).then(d=>{
    l.innerHTML=d.message;l.style.color=d.success?'#15803d':'#b91c1c';
  }).catch(e=>{
    l.innerText='查询失败: '+e;l.style.color='#b91c1c';
  });
}

%CALL_FORWARD_JS%

function loadHist(){
  $('hList').innerHTML='<div style="text-align:center;padding:20px;color:#94a3b8">加载中...</div>';
  fetch('/history').then(r=>r.json()).then(d=>{
    var h='';
    if(!d.history||d.history.length==0)h='<div style="text-align:center;padding:40px;color:#cbd5e1">暂无记录</div>';
    else{
       d.history.forEach(i=>{
         h+=`<div class="log-item">
           <div class="log-h"><span>${i.s}</span><span>${i.t}</span></div>
           <div class="log-m">${i.m}</div>
         </div>`;
       });
    }
    $('hList').innerHTML=h;
  });
}

function saveFilter(){
  var ls=$('ftList').value.split(/[,，\n]/).map(s=>s.trim()).filter(s=>s).join(',');
  postJ('/filter',{
    enabled: $('ftEn').value==='true',
    whitelist: document.getElementsByName('filterIsWhitelist')[1].checked,
    numbers: ls.split(',') //后端兼容数组格式
  }, d=>toast('已保存过滤名单'));
}

function saveTimer(){
  postJ('/timer',{
    enabled: $('tmEn').value==='true',
    type: +$('tmType').value,
    interval: +$('tmInt').value,
    phone: $('tmPh').value,
    message: $('tmMsg').value
  }, d=>toast('已保存定时任务'));
}

function saveAll(e){
  e.preventDefault();
  toast('正在保存配置...');
  fetch('/save',{method:'POST',body:new FormData(e.target)})
  .then(r=>r.text()).then(t=>{
    toast('保存成功，设备将重启...');
    setTimeout(()=>location.reload(),3000);
  }).catch(()=>toast('保存失败'));
  return false;
}

// 启动加载
autoLoad();
</script></body></html>)rawliteral";

// 旧兼容
const char* htmlToolsPage = htmlPage;

#endif
