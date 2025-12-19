#ifndef WEB_PAGES_H
#define WEB_PAGES_H

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

/* 模式切换按钮组 */
.mode-toggle{display:flex;gap:8px;margin-bottom:12px}
.mode-btn{flex:1;padding:12px 8px;border:2px solid var(--border);border-radius:10px;background:#fff;cursor:pointer;text-align:center;transition:all .2s;font-size:0.9em}
.mode-btn:hover{border-color:#cbd5e1;background:#f8fafc}
.mode-btn.active{border-color:var(--primary);background:#eef2ff}
.mode-btn .mode-icon{font-size:1.4em;margin-bottom:4px}
.mode-btn .mode-title{font-weight:600;color:#1e293b}
.mode-btn .mode-desc{font-size:0.8em;color:var(--text-light);margin-top:2px}
.mode-btn.active .mode-title{color:var(--primary)}

/* 开关行 */
.sw-row{display:flex;justify-content:space-between;align-items:center;padding:8px 0;cursor:pointer}
.sw{width:44px;height:24px;background:#cbd5e1;border-radius:24px;position:relative;transition:0.3s}
.sw:after{content:'';width:20px;height:20px;background:#fff;border-radius:50%;position:absolute;top:2px;left:2px;transition:0.3s;box-shadow:0 1px 2px rgba(0,0,0,0.2)}
.sw.on{background:var(--primary)}
.sw.on:after{left:22px}

/* 历史记录 - 聊天界面风格 */
.sms-container{display:flex;height:calc(100vh - 190px);gap:0;overflow:hidden;border:1px solid var(--border);background:#fff}
.contact-list{width:250px;min-width:180px;border-right:1px solid var(--border);overflow-y:auto;background:#fafbfc;flex-shrink:0}
.contact-list::-webkit-scrollbar{width:4px}
.contact-list::-webkit-scrollbar-thumb{background:#cbd5e1;border-radius:2px}
.contact-item{padding:12px 14px;cursor:pointer;border-bottom:1px solid #f1f5f9;transition:all .2s;position:relative;display:flex;align-items:center;gap:12px}
.contact-item:hover{background:#f1f5f9}
.contact-item.active{background:#eff6ff}
.contact-avatar{width:40px;height:40px;border-radius:50%;background:#e2e8f0;color:#fff;display:flex;align-items:center;justify-content:center;font-weight:700;font-size:1em;flex-shrink:0;text-transform:uppercase}
.contact-info{flex:1;overflow:hidden;display:flex;flex-direction:column;justify-content:center}
.contact-name-row{display:flex;justify-content:space-between;align-items:center;margin-bottom:2px}
.contact-name{font-size:0.95em;font-weight:600;color:#334155;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.contact-time{font-size:0.75em;color:#94a3b8;flex-shrink:0;margin-left:8px}
.contact-preview-row{display:flex;justify-content:space-between;align-items:center}
.contact-preview{font-size:0.8em;color:#94a3b8;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;flex:1}
.contact-badge{background:#f1f5f9;color:#64748b;border-radius:99px;font-size:0.75em;padding:2px 8px;margin-left:8px;flex-shrink:0;font-weight:500}

.chat-area{flex:1;display:flex;flex-direction:column;min-width:0;background:#fff}
.chat-header{padding:12px 16px;border-bottom:1px solid var(--border);background:#fff;display:flex;align-items:center;justify-content:space-between;z-index:10}
.chat-title{font-weight:700;font-size:1em;color:#1e293b;display:flex;align-items:center;gap:8px}
.chat-msg-count{font-size:0.75em;color:#94a3b8;background:#f1f5f9;padding:2px 8px;border-radius:12px}
.chat-messages{flex:1;overflow-y:auto;padding:16px;background:#fff}
.chat-messages::-webkit-scrollbar{width:4px}
.chat-messages::-webkit-scrollbar-thumb{background:#cbd5e1;border-radius:2px}
.chat-empty{text-align:center;padding:60px 20px;color:#94a3b8;font-size:0.9em;display:flex;flex-direction:column;align-items:center;gap:12px}
.chat-day{text-align:center;margin:20px 0 16px;position:relative}
.chat-day::before{content:'';position:absolute;top:50%;left:0;right:0;height:1px;background:#f1f5f9;z-index:0}
.chat-day span{position:relative;background:#fff;padding:4px 12px;font-size:0.75em;color:#cbd5e1;border:1px solid #f1f5f9;border-radius:99px;z-index:1}
.msg-bubble{max-width:80%;padding:10px 14px;margin-bottom:12px;border-radius:18px;position:relative;font-size:0.95em;line-height:1.6;word-break:break-word;box-shadow:0 1px 2px rgba(0,0,0,0.02)}
.msg-in{background:#f1f5f9;color:#334155;border-bottom-left-radius:4px;margin-right:auto}
.msg-time{font-size:0.7em;color:rgba(0,0,0,0.3);margin-top:4px;display:block;text-align:right}

/* 消息提示 */
.toast{position:fixed;top:20px;left:50%;transform:translateX(-50%);background:rgba(0,0,0,0.8);color:#fff;padding:8px 16px;border-radius:99px;font-size:0.85em;z-index:9999;transition:all .3s cubic-bezier(0.18, 0.89, 0.32, 1.28);opacity:0;transform:translate(-50%, -20px);pointer-events:none}
.toast.show{opacity:1;transform:translate(-50%, 0)}

/* 信息表格 */
.info-table{width:100%;font-size:0.85em;border-collapse:collapse}
.info-table td{padding:8px 0;border-bottom:1px solid #f8fafc}
.info-table td:first-child{color:var(--text-light);width:35%}

/* 移动端适配 */
.chat-back{display:none}
@media(max-width:600px){
  .sms-container{position:relative}
  .contact-list{width:100%;border-right:none}
  .chat-area{position:absolute;top:0;left:100%;width:100%;height:100%;transition:left .3s ease;z-index:20}
  .chat-area.show{left:0}
  .chat-back{display:flex;align-items:center;justify-content:center;width:32px;height:32px;margin-right:4px;cursor:pointer;border-radius:50%}
  .chat-back:active{background:#f1f5f9}
  .contact-item.active{background:transparent} /* 移动端取消列表选中高亮 */
}
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
    <div class="card-t" style="color:var(--danger)">危险操作</div>
    <button class="btn btn-d" onclick="if(confirm('确定重启?'))act('reboot')">重启设备 (Reboot)</button>
  </div>
</div>

<!-- ============ 历史页 ============ -->
<div class="page">
  <div class="card" style="padding:0;overflow:hidden">
    <div style="padding:12px 16px;border-bottom:1px solid var(--border);display:flex;justify-content:space-between;align-items:center;background:#fafbfc">
      <span style="font-weight:700;font-size:1.1em">短信历史</span>
      <div class="badge b-wait" onclick="loadHist()" style="cursor:pointer">刷新</div>
    </div>
    <div class="sms-container" id="smsContainer">
      <div class="contact-list" id="contactList">
        <div style="text-align:center;padding:20px;color:#94a3b8;font-size:0.85em">加载中...</div>
      </div>
      <div class="chat-area">
        <div class="chat-header" id="chatHeader" style="display:none">
          <div>
            <div class="chat-title" id="chatTitle">
               <div class="chat-back" onclick="backToList()">
                 <svg viewBox="0 0 24 24" width="24" height="24"><path d="M20 11H7.83l5.59-5.59L12 4l-8 8 8 8 1.41-1.41L7.83 13H20v-2z" fill="currentColor"/></svg>
               </div>
               <span id="chatName">选择联系人</span>
            </div>
            <div class="chat-msg-count" id="chatCount" style="margin-left:36px"></div>
          </div>
        </div>
        <div class="chat-messages" id="chatMessages">
          <div class="chat-empty">👈 选择左侧号码查看短信</div>
        </div>
      </div>
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
         
         <div style="border-top:1px solid var(--border);margin:12px 0 8px;padding-top:12px">
           <div style="font-weight:600;margin-bottom:8px">Home Assistant 自动发现</div>
           <div class="sw-row" onclick="xToggle('mqHa')">
              <span>启用 HA 自动发现</span>
              <div id="mqHaSw" class="sw %MQTT_HA_SW%"></div>
              <input type="hidden" id="mqHa" name="mqttHaDiscovery" value="%MQTT_HA_VAL%">
           </div>
           <div id="mqHaBox" style="display:%MQTT_HA_DISP%">
             <div class="fg"><label>HA 发现前缀</label><input name="mqttHaPrefix" value="%MQTT_HA_PREFIX%" placeholder="homeassistant"></div>
             <div style="font-size:0.85em;color:var(--text-light);margin-top:4px">
               💡 启用后设备将自动注册到 Home Assistant，无需手动配置 YAML 文件。
             </div>
           </div>
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
    <summary>号码黑白名单过滤 (Max 100) <span id="ftModeBadge" class="badge b-wait" style="margin-left:8px">未启用</span></summary>
    <div class="det-body">
      <div class="sw-row" onclick="xToggle('ftEn');updFtMode()">
         <span>启用过滤</span>
         <div id="ftEnSw" class="sw"></div>
         <input type="hidden" id="ftEn" name="filterEn" value="%FILTER_EN_VAL%">
      </div>
      <div id="ftModeBox" style="margin-top:12px">
        <label style="margin-bottom:8px">过滤模式</label>
        <div class="mode-toggle">
          <div class="mode-btn" id="modeBl" onclick="setFtMode(false)">
            <div class="mode-icon">🚫</div>
            <div class="mode-title">黑名单模式</div>
            <div class="mode-desc">拦截列表中的号码</div>
          </div>
          <div class="mode-btn" id="modeWl" onclick="setFtMode(true)">
            <div class="mode-icon">✅</div>
            <div class="mode-title">仅白名单</div>
            <div class="mode-desc">只接收列表中的号码</div>
          </div>
        </div>
        <input type="hidden" id="ftWl" name="filterIsWhitelist" value="%FILTER_WL_VAL%">
        <div id="ftModeHint" style="background:#f0f9ff;border:1px solid #bae6fd;border-radius:8px;padding:10px;margin-bottom:12px;font-size:0.85em"></div>
        <div class="fg">
          <label id="ftListLabel">号码列表</label>
          <textarea id="ftList" name="filterList" rows="5" placeholder="输入号码，多个号码用逗号分隔，例如: 10086,13800000000"></textarea>
        </div>
      </div>
      <button type="button" class="btn btn-w" onclick="saveFilter()">仅保存名单设置</button>
    </div>
  </details>

  <details>
    <summary>内容关键词过滤 <span id="cfModeBadge" class="badge b-wait" style="margin-left:8px">未启用</span></summary>
    <div class="det-body">
      <div class="sw-row" onclick="xToggle('cfEn');updCfMode()">
         <span>启用内容过滤</span>
         <div id="cfEnSw" class="sw"></div>
         <input type="hidden" id="cfEn" name="contentFilterEn" value="%CF_EN_VAL%">
      </div>
      <div id="cfModeBox" style="margin-top:12px">
        <label style="margin-bottom:8px">过滤模式</label>
        <div class="mode-toggle">
          <div class="mode-btn" id="cfModeBl" onclick="setCfMode(false)">
            <div class="mode-icon">🚫</div>
            <div class="mode-title">黑名单模式</div>
            <div class="mode-desc">拦截包含关键词的短信</div>
          </div>
          <div class="mode-btn" id="cfModeWl" onclick="setCfMode(true)">
            <div class="mode-icon">🔍</div>
            <div class="mode-title">仅白名单</div>
            <div class="mode-desc">只转发包含关键词的短信</div>
          </div>
        </div>
        <input type="hidden" id="cfWl" name="contentFilterIsWhitelist" value="%CF_WL_VAL%">
        <div id="cfModeHint" style="background:#fef9c3;border:1px solid #fcd34d;border-radius:8px;padding:10px;margin-bottom:12px;font-size:0.85em"></div>
        <div class="fg">
          <label id="cfListLabel">关键词列表</label>
          <textarea id="cfList" name="contentFilterList" rows="4" placeholder="输入关键词，多个关键词用逗号分隔，例如: 验证码,快递,银行"></textarea>
        </div>
        <div style="font-size:0.85em;color:var(--text-light);margin-bottom:12px">
          💡 关键词匹配不区分大小写。可用于过滤广告短信或只接收重要短信。
        </div>
      </div>
      <button type="button" class="btn btn-w" onclick="saveContentFilter()">仅保存关键词设置</button>
    </div>
  </details>

  <details>
    <summary>定时任务 <span id="tmBadge" class="badge b-wait" style="margin-left:8px">未启用</span></summary>
    <div class="det-body">
      <div class="sw-row" onclick="xToggle('tmEn');updTmInfo()">
         <span>启用任务</span>
         <div id="tmEnSw" class="sw"></div>
         <input type="hidden" id="tmEn" name="timerEn" value="%TIMER_EN_VAL%">
      </div>
      <div class="fg"><label>类型 & 间隔(天)</label>
        <div class="grid-2">
          <select id="tmType" name="timerType" onchange="$('tmSms').style.display=this.value==1?'block':'none';updTmInfo()"><option value="0">Ping保活</option><option value="1">发送短信</option></select>
          <input type="number" id="tmInt" name="timerInterval" min="1" onchange="updTmInfo()">
        </div>
      </div>
      <div id="tmSms" style="display:none">
        <div class="fg"><label>对方号码</label><input id="tmPh" name="timerPhone"></div>
        <div class="fg"><label>短信内容</label><input id="tmMsg" name="timerMessage"></div>
      </div>
      <div style="background:#f0f9ff;border:1px solid #bae6fd;border-radius:8px;padding:12px;margin-bottom:12px" id="tmInfo"></div>
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
var cf={en:%CF_EN_BOOL%,wl:%CF_WL_BOOL%,ls:'%CF_LIST%'};
var tm={en:%TIMER_EN_BOOL%,tp:%TIMER_TP%,int:%TIMER_INT%,ph:'%TIMER_PH%',ms:'%TIMER_MS%',rm:%TIMER_RM%};

// 状态初始化 - 号码过滤
if(ft.en){$('ftEnSw').className='sw on';$('ftEn').value='true'}
$('ftWl').value=ft.wl?'true':'false';
$('ftList').value=ft.ls;

// 状态初始化 - 内容过滤
if(cf.en){$('cfEnSw').className='sw on';$('cfEn').value='true'}
$('cfWl').value=cf.wl?'true':'false';
$('cfList').value=cf.ls;

if(tm.en){$('tmEnSw').className='sw on';$('tmEn').value='true'}
$('tmType').value=tm.tp;$('tmInt').value=tm.int;$('tmPh').value=tm.ph;$('tmMsg').value=tm.ms;
if(tm.tp==1)$('tmSms').style.display='block';

// 更新定时任务信息显示
function updTmInfo(){
  var en=$('tmEn').value==='true';
  var tp=+$('tmType').value;
  var days=+$('tmInt').value||0;
  var badge=$('tmBadge');
  var info=$('tmInfo');
  var typeName=tp===0?'Ping保活':'发送短信';
  
  if(!en){
    badge.className='badge b-wait';badge.innerText='未启用';
    info.innerHTML='<div style="color:#64748b">⏸ 定时任务已禁用</div>';
    return;
  }
  badge.className='badge b-ok';badge.innerText=typeName;
  
  // 计算剩余时间（天+小时）
  var rmSec=tm.rm;
  var rmDays=Math.floor(rmSec/86400);
  var rmHours=Math.floor((rmSec%86400)/3600);
  var rmStr=rmDays>0?(rmDays+'天'):'';rmStr+=(rmHours>0?(rmHours+'小时'):'');
  if(!rmStr)rmStr='即将执行';
  
  var html='<div style="font-weight:600;color:#0369a1;margin-bottom:6px">✅ 定时任务已启用</div>';
  html+='<div style="display:grid;grid-template-columns:1fr 1fr;gap:8px;font-size:0.9em">';
  html+='<div><span style="color:#64748b">任务类型:</span> <b>'+typeName+'</b></div>';
  html+='<div><span style="color:#64748b">执行间隔:</span> <b>每 '+days+' 天</b></div>';
  html+='<div><span style="color:#64748b">下次执行:</span> <b>'+rmStr+'后</b></div>';
  if(tp===1){html+='<div><span style="color:#64748b">发送至:</span> <b>'+($('tmPh').value||'未设置')+'</b></div>';}
  html+='</div>';
  info.innerHTML=html;
}
updTmInfo();

// 设置过滤模式
function setFtMode(isWhitelist){
  $('ftWl').value=isWhitelist?'true':'false';
  updFtMode();
}

// 更新过滤模式 UI
function updFtMode(){
  var en=$('ftEn').value==='true';
  var wl=$('ftWl').value==='true';
  var badge=$('ftModeBadge');
  var hint=$('ftModeHint');
  var modeBox=$('ftModeBox');
  
  // 更新按钮选中状态
  $('modeBl').className='mode-btn'+(wl?'':' active');
  $('modeWl').className='mode-btn'+(wl?' active':'');
  
  // 更新徽章
  if(!en){
    badge.className='badge b-wait';badge.innerText='未启用';
    modeBox.style.opacity='0.5';
  }else{
    modeBox.style.opacity='1';
    if(wl){
      badge.className='badge b-ok';badge.innerText='仅白名单';
    }else{
      badge.className='badge b-err';badge.innerText='黑名单';
    }
  }
  
  // 更新提示文字和标签
  if(wl){
    hint.innerHTML='<span style="color:#0369a1">✅ 仅白名单模式</span>：只有列表中的号码发送的短信会被转发，其他号码将被<b>拦截</b>。';
    $('ftListLabel').innerText='允许的号码 (只接收这些)';
  }else{
    hint.innerHTML='<span style="color:#b91c1c">🚫 黑名单模式</span>：列表中的号码发送的短信会被<b>拦截</b>，其他号码正常转发。';
    $('ftListLabel').innerText='拦截的号码 (屏蔽这些)';
  }
}
updFtMode();

// 设置内容过滤模式
function setCfMode(isWhitelist){
  $('cfWl').value=isWhitelist?'true':'false';
  updCfMode();
}

// 更新内容过滤模式 UI
function updCfMode(){
  var en=$('cfEn').value==='true';
  var wl=$('cfWl').value==='true';
  var badge=$('cfModeBadge');
  var hint=$('cfModeHint');
  var modeBox=$('cfModeBox');
  
  // 更新按钮选中状态
  $('cfModeBl').className='mode-btn'+(wl?'':' active');
  $('cfModeWl').className='mode-btn'+(wl?' active':'');
  
  // 更新徽章
  if(!en){
    badge.className='badge b-wait';badge.innerText='未启用';
    modeBox.style.opacity='0.5';
  }else{
    modeBox.style.opacity='1';
    if(wl){
      badge.className='badge b-ok';badge.innerText='仅白名单';
    }else{
      badge.className='badge b-warn';badge.innerText='黑名单';
    }
  }
  
  // 更新提示文字和标签
  if(wl){
    hint.innerHTML='<span style="color:#0369a1">🔍 仅白名单模式</span>：只有包含关键词的短信会被转发，其他短信将被<b>拦截</b>。适合只接收验证码等重要短信。';
    $('cfListLabel').innerText='必须包含的关键词 (只转发这些)';
  }else{
    hint.innerHTML='<span style="color:#a16207">🚫 黑名单模式</span>：包含关键词的短信会被<b>拦截</b>，其他短信正常转发。适合过滤广告短信。';
    $('cfListLabel').innerText='拦截的关键词 (屏蔽这些)';
  }
}
updCfMode();

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
  if(id==='mqHa')$('mqHaBox').style.display=i.value==='true'?'block':'none';
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
  // Telegram(5) 和 钉钉(7) 需要显示 Key1 输入框
  var showK1=(t=='5'||t=='7');
  $('k1'+i).style.display=showK1?'block':'none';
  // 动态更新标签
  var lbl=$('k1l'+i);
  if(lbl){lbl.innerText=(t=='5')?'Chat ID':'加签密钥 (可选)';}
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


// 短信历史数据和当前选中的联系人
var smsData=[];
var curContact=null;
// 颜色池
const colors=['#3b82f6','#10b981','#f59e0b','#ef4444','#8b5cf6','#ec4899','#6366f1','#14b8a6'];
function getAvatarColor(name){
  let hash=0;
  for(let i=0;i<name.length;i++)hash=name.charCodeAt(i)+((hash<<5)-hash);
  return colors[Math.abs(hash)%colors.length];
}

function loadHist(){
  $('contactList').innerHTML='<div style="text-align:center;padding:20px;color:#94a3b8;font-size:0.85em">加载中...</div>';
  $('chatMessages').innerHTML='<div class="chat-empty"><div style="font-size:2em;margin-bottom:10px">💬</div><div>加载中...</div></div>';
  fetch('/history').then(r=>r.json()).then(d=>{
    smsData=d.history||[];
    if(smsData.length==0){
      $('contactList').innerHTML='<div style="text-align:center;padding:30px 10px;color:#94a3b8;font-size:0.85em">暂无短信</div>';
      $('chatMessages').innerHTML='<div class="chat-empty"><div style="font-size:2em;margin-bottom:10px">📭</div><div>暂无短信记录</div></div>';
      return;
    }
    // 按发送者分组
    var contacts={};
    smsData.forEach(i=>{
      if(!contacts[i.s])contacts[i.s]={name:i.s,msgs:[],lastTime:i.t};
      contacts[i.s].msgs.push(i);
      if(i.t>contacts[i.s].lastTime)contacts[i.s].lastTime=i.t;
    });
    // 按最新消息时间排序联系人
    var sortedContacts=Object.values(contacts).sort((a,b)=>b.lastTime.localeCompare(a.lastTime));
    // 渲染联系人列表
    var h='';
    sortedContacts.forEach((c,idx)=>{
      var preview=c.msgs[0].m.substring(0,20)+(c.msgs[0].m.length>20?'...':'');
      var shortTime=c.lastTime.substring(5,16);
      var avColor=getAvatarColor(c.name);
      var avText=c.name.replace('+','').substring(0,1);
      avText = c.name.length > 2 ? c.name.substring(0,1) : c.name;

      h+=`<div class="contact-item${idx===0?' active':''}" onclick="selContact('${c.name.replace(/'/g,"\\'")}')" data-name="${c.name}">
        <div class="contact-avatar" style="background:${avColor}">${avText}</div>
        <div class="contact-info">
          <div class="contact-name-row">
             <div class="contact-name">${c.name}</div>
             <div class="contact-time">${shortTime}</div>
          </div>
          <div class="contact-preview-row">
             <div class="contact-preview">${preview}</div>
             ${c.msgs.length>1 ? '<div class="contact-badge">'+c.msgs.length+'</div>' : ''}
          </div>
        </div>
      </div>`;
    });
    $('contactList').innerHTML=h;
    // 默认选中第一个联系人
    if(sortedContacts.length>0)selContact(sortedContacts[0].name);
  }).catch(e=>{
    console.error('加载短信失败',e);
    $('contactList').innerHTML='<div style="text-align:center;padding:20px;color:#b91c1c;font-size:0.85em">加载失败</div>';
  });
}

function selContact(name){
  curContact=name;
  // 更新联系人列表高亮
  document.querySelectorAll('.contact-item').forEach(e=>{
    e.classList.toggle('active',e.dataset.name===name);
  });
  // 过滤该联系人的消息
  var msgs=smsData.filter(i=>i.s===name);
  if(msgs.length===0){
    $('chatMessages').innerHTML='<div class="chat-empty">无消息</div>';
    return;
  }
  // 按时间正序排列（旧的在上）
  msgs.sort((a,b)=>a.t.localeCompare(b.t));
  
  // 显示聊天头部
  $('chatHeader').style.display='flex';
  var avColor=getAvatarColor(name);
  var avText=name.length > 2 ? name.substring(0,1) : name;
  
  // 移动端显示返回按钮和名字
  var backBtn='<div class="chat-back" onclick="backToList()"><svg viewBox="0 0 24 24" width="24" height="24"><path d="M20 11H7.83l5.59-5.59L12 4l-8 8 8 8 1.41-1.41L7.83 13H20v-2z" fill="#64748b"/></svg></div>';
  $('chatTitle').innerHTML=`${backBtn}<div class="contact-avatar" style="width:28px;height:28px;font-size:0.8em;background:${avColor}">${avText}</div> ${name}`;
  $('chatCount').innerText=msgs.length+'条短信';
  $('chatCount').style.marginLeft='36px'; // 对齐调整
  
  // 移动端切换视图
  if(window.innerWidth<=600){
    $s('.chat-area').classList.add('show');
    $s('.contact-list').style.display='none'; // 隐藏列表防止滚动冲突
  }
  
  // 渲染消息，按日期分组
  var h='';
  var lastDay='';
  msgs.forEach(m=>{
    var day=m.t.substring(0,10);
    if(day!==lastDay){
      h+='<div class="chat-day"><span>'+day+'</span></div>';
      lastDay=day;
    }
    var time=m.t.substring(11,16);
    h+=`<div class="msg-bubble msg-in">
      <div>${m.m}</div>
      <span class="msg-time">${time}</span>
    </div>`;
  });
  $('chatMessages').innerHTML=h;
  // 滚动到底部
  var chatBox=$('chatMessages');
  requestAnimationFrame(()=>chatBox.scrollTop=chatBox.scrollHeight);
}

function backToList(){
  $s('.chat-area').classList.remove('show');
  setTimeout(()=>$s('.contact-list').style.display='block', 300); // 动画结束后显示列表
}

function saveFilter(){
  var ls=$('ftList').value.split(/[,，\n]/).map(s=>s.trim()).filter(s=>s).join(',');
  postJ('/filter',{
    enabled: $('ftEn').value==='true',
    whitelist: $('ftWl').value==='true',
    numbers: ls.split(',') //后端兼容数组格式
  }, d=>toast('已保存号码过滤设置'));
}

function saveContentFilter(){
  var ls=$('cfList').value.split(/[,，\n]/).map(s=>s.trim()).filter(s=>s).join(',');
  postJ('/contentfilter',{
    enabled: $('cfEn').value==='true',
    whitelist: $('cfWl').value==='true',
    keywords: ls
  }, d=>toast('已保存关键词过滤设置'));
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
  // 使用 URLSearchParams 而非 FormData（ESP32 处理更稳定）
  var fd=new FormData(e.target);
  var params=new URLSearchParams();
  for(var p of fd.entries()){params.append(p[0],p[1])}
  fetch('/save',{
    method:'POST',
    headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:params.toString()
  })
  .then(r=>r.json()).then(d=>{
    if(d.success){
      toast('配置已保存');
      if(d.needRestart && confirm('配置已保存成功！\n\n是否立即重启设备使配置生效？')){
        toast('设备重启中...');
        postJ('/restart',{},()=>{});
        setTimeout(()=>location.reload(),3000);
      }
    }else{
      toast(d.message||'保存失败');
    }
  }).catch(e=>{console.error(e);toast('保存失败，请检查网络')});
  return false;
}

// 启动加载
autoLoad();
</script></body></html>)rawliteral";

// 旧兼容
const char* htmlToolsPage = htmlPage;

#endif
