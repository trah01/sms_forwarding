/*
 * web_handlers.ino - Web 服务器处理函数实现
 */

// 检查 HTTP Basic 认证
bool checkAuth() {
  if (!server.authenticate(config.webUser.c_str(), config.webPass.c_str())) {
    server.requestAuthentication(BASIC_AUTH, "SMS Forwarding", "请输入管理员账号密码");
    return false;
  }
  return true;
}

// 发送 AT 命令并获取响应
String sendATCommand(const char* cmd, unsigned long timeout) {
  while (Serial1.available()) Serial1.read();
  Serial1.println(cmd);
  
  unsigned long start = millis();
  String resp = "";
  while (millis() - start < timeout) {
    while (Serial1.available()) {
      char c = Serial1.read();
      resp += c;
      if (resp.indexOf("OK") >= 0 || resp.indexOf("ERROR") >= 0) {
        delay(50);  // 等待剩余数据
        while (Serial1.available()) resp += (char)Serial1.read();
        return resp;
      }
    }
  }
  return resp;
}

// AT 命令并等待 OK
bool sendATandWaitOK(const char* cmd, unsigned long timeout) {
  while (Serial1.available()) Serial1.read();
  Serial1.println(cmd);
  unsigned long start = millis();
  String resp = "";
  while (millis() - start < timeout) {
    while (Serial1.available()) {
      char c = Serial1.read();
      resp += c;
      if (resp.indexOf("OK") >= 0) return true;
      if (resp.indexOf("ERROR") >= 0) return false;
    }
  }
  return false;
}

// 等待 CGATT 附着
bool waitCGATT1() {
  Serial1.println("AT+CGATT?");
  unsigned long start = millis();
  String resp = "";
  while (millis() - start < 2000) {
    while (Serial1.available()) {
      char c = Serial1.read();
      resp += c;
      if (resp.indexOf("+CGATT: 1") >= 0) return true;
      if (resp.indexOf("+CGATT: 0") >= 0) return false;
    }
  }
  return false;
}

// 重启模组
void resetModule() {
  Serial.println("正在重启模组...");
  Serial1.println("AT+CFUN=1,1");
  delay(3000);
}

// LED 闪烁
void blink_short(unsigned long gap_time) {
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(gap_time);
}

// 处理配置页面请求
void handleRoot() {
  if (!checkAuth()) return;
  
  String html = String(htmlPage);
  html.replace("%COMMON_CSS%", commonCss);
  // html.replace("%COMMON_JS%", commonJs); // JS 已合并
  html.replace("%IP%", WiFi.localIP().toString());
  
  // 1. MQTT 状态 (概览页用)
  String mqttStatusText = config.mqttEnabled ? (mqttClient.connected() ? "已连接" : "未连接") : "未启用";
  String mqttClass = config.mqttEnabled ? (mqttClient.connected() ? "b-ok" : "b-err") : "b-wait";
  html.replace("%MQTT_STATUS%", mqttStatusText);
  html.replace("%MQTT_CLASS%", mqttClass);
  html.replace("%MQTT_PREFIX%", config.mqttPrefix);
  html.replace("%MQTT_ENABLED%", config.mqttEnabled ? "true" : "false");
  
  // MQTT 主题列表
  String mqttTopicsHtml = "";
  if (config.mqttEnabled && config.mqttServer.length() > 0) {
    mqttTopicsHtml = mqttTopicStatus + "<br>" + mqttTopicSmsReceived;
  } else {
    mqttTopicsHtml = "未配置";
  }
  html.replace("%MQTT_TOPICS%", mqttTopicsHtml);
  
  // 1.5 WiFi 网络配置 HTML
  String wifiHtml = "";
  for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
    String idx = String(i);
    String enabledSw = config.wifiNetworks[i].enabled ? "on" : "";
    String enabledVal = config.wifiNetworks[i].enabled ? "true" : "false";
    String currentSsid = WiFi.SSID();
    bool isCurrent = config.wifiNetworks[i].ssid.length() > 0 && config.wifiNetworks[i].ssid == currentSsid;
    
    wifiHtml += "<div style=\"padding:10px;background:#f8fafc;border-radius:8px;margin-bottom:8px;border:1px solid " + String(isCurrent ? "var(--success)" : "#e2e8f0") + "\">";
    wifiHtml += "<div class=\"sw-row\" onclick=\"wfTog(" + idx + ")\">";
    wifiHtml += "<span style=\"font-weight:600\">网络 " + String(i + 1);
    if (isCurrent) wifiHtml += " <span class=\"badge b-ok\">当前</span>";
    wifiHtml += "</span>";
    wifiHtml += "<div id=\"wfs" + idx + "\" class=\"sw " + enabledSw + "\"></div>";
    wifiHtml += "<input type=\"hidden\" id=\"wfe" + idx + "\" name=\"wifi" + idx + "en\" value=\"" + enabledVal + "\">";
    wifiHtml += "</div>";
    wifiHtml += "<div class=\"grid-2\" style=\"margin-top:8px\">";
    wifiHtml += "<div class=\"fg\" style=\"margin-bottom:0\"><label>SSID</label><input name=\"wifi" + idx + "ssid\" value=\"" + config.wifiNetworks[i].ssid + "\" placeholder=\"WiFi名称\"></div>";
    // 密码不回显明文，使用占位符提示
    String passPlaceholder = config.wifiNetworks[i].password.length() > 0 ? "（已保存，留空则保留）" : "WiFi密码";
    wifiHtml += "<div class=\"fg\" style=\"margin-bottom:0\"><label>密码</label><input name=\"wifi" + idx + "pass\" type=\"password\" placeholder=\"" + passPlaceholder + "\"></div>";
    wifiHtml += "</div></div>";
  }
  html.replace("%WIFI_NETWORKS%", wifiHtml);

  // 2. 基础配置填写
  html.replace("%WEB_USER%", config.webUser);
  html.replace("%WEB_PASS%", config.webPass);

  // 3. SMTP 配置
  html.replace("%SMTP_EN_VAL%", config.emailEnabled ? "true" : "false");
  html.replace("%SMTP_EN_SW%", config.emailEnabled ? "on" : "");
  html.replace("%SMTP_DISP%", config.emailEnabled ? "block" : "none");
  html.replace("%SMTP_SERVER%", config.smtpServer);
  html.replace("%SMTP_PORT%", String(config.smtpPort));
  html.replace("%SMTP_USER%", config.smtpUser);
  html.replace("%SMTP_PASS%", config.smtpPass);
  html.replace("%SMTP_SEND_TO%", config.smtpSendTo);

  // 4. MQTT 配置
  html.replace("%MQTT_EN_VAL%", config.mqttEnabled ? "true" : "false");
  html.replace("%MQTT_EN_SW%", config.mqttEnabled ? "on" : "");
  html.replace("%MQTT_DISP%", config.mqttEnabled ? "block" : "none");
  html.replace("%MQTT_SERVER%", config.mqttServer);
  html.replace("%MQTT_PORT%", String(config.mqttPort));
  html.replace("%MQTT_USER%", config.mqttUser);
  html.replace("%MQTT_PASS%", config.mqttPass);
  
  html.replace("%MQTT_CO_VAL%", config.mqttControlOnly ? "true" : "false");
  html.replace("%MQTT_CO_SW%", config.mqttControlOnly ? "on" : "");
  
  // 4.5 Home Assistant 自动发现配置
  html.replace("%MQTT_HA_VAL%", config.mqttHaDiscovery ? "true" : "false");
  html.replace("%MQTT_HA_SW%", config.mqttHaDiscovery ? "on" : "");
  html.replace("%MQTT_HA_DISP%", config.mqttHaDiscovery ? "block" : "none");
  html.replace("%MQTT_HA_PREFIX%", config.mqttHaPrefix.length() > 0 ? config.mqttHaPrefix : "homeassistant");
  
  // 5. 黑白名单配置 (Js 对象初始化)
  html.replace("%FILTER_EN_VAL%", config.filterEnabled ? "true" : "false");
  html.replace("%FILTER_EN_BOOL%", config.filterEnabled ? "true" : "false"); // JS bool
  html.replace("%FILTER_WL_BOOL%", config.filterIsWhitelist ? "true" : "false"); // JS bool
  html.replace("%FILTER_WL_VAL%", config.filterIsWhitelist ? "true" : "false"); // hidden input value
  
  // 处理 filterList 换行符，避免破坏 JS 字符串
  String safeFilterList = config.filterList;
  safeFilterList.replace("\n", "");
  safeFilterList.replace("\r", "");
  html.replace("%FILTER_LIST%", safeFilterList);

  // 5.5 内容关键词过滤配置
  html.replace("%CF_EN_VAL%", config.contentFilterEnabled ? "true" : "false");
  html.replace("%CF_EN_BOOL%", config.contentFilterEnabled ? "true" : "false");
  html.replace("%CF_WL_BOOL%", config.contentFilterIsWhitelist ? "true" : "false");
  html.replace("%CF_WL_VAL%", config.contentFilterIsWhitelist ? "true" : "false");
  
  String safeCfList = config.contentFilterList;
  safeCfList.replace("\n", "");
  safeCfList.replace("\r", "");
  html.replace("%CF_LIST%", safeCfList);

  // 6. 定时任务配置 (Js 对象初始化)
  html.replace("%TIMER_EN_VAL%", config.timerEnabled ? "true" : "false");
  html.replace("%TIMER_EN_BOOL%", config.timerEnabled ? "true" : "false"); // JS bool
  html.replace("%TIMER_TP%", String(config.timerType));
  html.replace("%TIMER_INT%", String(config.timerInterval));
  html.replace("%TIMER_PH%", config.timerPhone);
  html.replace("%TIMER_MS%", config.timerMessage);

  // 计算剩余时间
  unsigned long remainSec = 0;
  if (config.timerEnabled && timerIntervalSec > 0) {
    unsigned long elapsedSec = (millis() - lastTimerExec) / 1000;
    if (elapsedSec < timerIntervalSec) {
      remainSec = timerIntervalSec - elapsedSec;
    }
  }
  html.replace("%TIMER_RM%", String(remainSec));

  // 7. 生成推送通道 HTML
  String channelsHtml = "";
  for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
    String idx = String(i);
    String enabledSw = config.pushChannels[i].enabled ? "on" : "";
    String enabledVal = config.pushChannels[i].enabled ? "true" : "false";
    
    // 动态生成折叠卡片（虽然放在 details 下，但每个通道本身也是个小块）
    // 为了美观，这里每个通道用一个带开关的面板
    channelsHtml += "<div class=\"card\" style=\"border:1px solid #e2e8f0;padding:12px;margin-bottom:8px;box-shadow:none\">";
    
    // 头部开关行
    channelsHtml += "<div class=\"sw-row\" onclick=\"chTog(" + idx + ")\">";
    channelsHtml += "<span style=\"font-weight:600\">通道 " + String(i + 1) + "</span>";
    channelsHtml += "<div id=\"chs" + idx + "\" class=\"sw " + enabledSw + "\"></div>";
    channelsHtml += "<input type=\"hidden\" id=\"che" + idx + "\" name=\"push" + idx + "en\" value=\"" + enabledVal + "\">";
    channelsHtml += "</div>";

    // 详情区域 (点击名称展开/点击折叠按钮? 简化处理，直接显示)
    // 这里使用一个简单的折叠按钮
    channelsHtml += "<div style=\"font-size:0.85em;color:var(--primary);text-align:right;cursor:pointer\" onclick=\"fd(" + idx + ")\">展开/收起 <span id=\"chi" + idx + "\" style=\"display:inline-block;transition:.2s\">></span></div>";
    
    channelsHtml += "<div id=\"chb" + idx + "\" style=\"display:none;margin-top:12px;border-top:1px solid #f1f5f9;padding-top:12px\">";
    
    channelsHtml += "<div class=\"fg\"><label>名称</label><input name=\"push" + idx + "name\" value=\"" + config.pushChannels[i].name + "\"></div>";
    
    channelsHtml += "<div class=\"fg\"><label>类型</label>";
    channelsHtml += "<select name=\"push" + idx + "type\" id=\"tp" + idx + "\" onchange=\"upd(" + idx + ")\">";
    channelsHtml += "<option value=\"1\"" + String(config.pushChannels[i].type == PUSH_TYPE_POST_JSON ? " selected" : "") + ">POST JSON</option>";
    channelsHtml += "<option value=\"2\"" + String(config.pushChannels[i].type == PUSH_TYPE_BARK ? " selected" : "") + ">Bark</option>";
    channelsHtml += "<option value=\"3\"" + String(config.pushChannels[i].type == PUSH_TYPE_GET ? " selected" : "") + ">GET请求</option>";
    channelsHtml += "<option value=\"4\"" + String(config.pushChannels[i].type == PUSH_TYPE_CUSTOM ? " selected" : "") + ">自定义模板</option>";
    channelsHtml += "<option value=\"5\"" + String(config.pushChannels[i].type == PUSH_TYPE_TELEGRAM ? " selected" : "") + ">Telegram Bot</option>";
    channelsHtml += "<option value=\"6\"" + String(config.pushChannels[i].type == PUSH_TYPE_WECOM ? " selected" : "") + ">企业微信</option>";
    channelsHtml += "<option value=\"7\"" + String(config.pushChannels[i].type == PUSH_TYPE_DINGTALK ? " selected" : "") + ">钉钉</option>";
    channelsHtml += "</select></div>";
    
    channelsHtml += "<div class=\"fg\"><label>URL</label><input name=\"push" + idx + "url\" value=\"" + config.pushChannels[i].url + "\" placeholder=\"https://...\"></div>";
    
    // Key1 输入框 (Telegram=Chat ID, 钉钉=加签密钥)
    bool showK1 = (config.pushChannels[i].type == PUSH_TYPE_TELEGRAM || config.pushChannels[i].type == PUSH_TYPE_DINGTALK);
    String k1Label = (config.pushChannels[i].type == PUSH_TYPE_TELEGRAM) ? "Chat ID" : "加签密钥 (可选)";
    String k1Hint = (config.pushChannels[i].type == PUSH_TYPE_TELEGRAM) ? "如 123456789" : "SEC开头的密钥";
    channelsHtml += "<div id=\"k1" + idx + "\" style=\"display:" + (showK1 ? "block" : "none") + "\"><div class=\"fg\"><label id=\"k1l" + idx + "\">" + k1Label + "</label><input name=\"push" + idx + "k1\" value=\"" + config.pushChannels[i].key1 + "\" placeholder=\"" + k1Hint + "\"></div></div>";
    
    // 自定义模板 Body (显示条件: type == 4)
    channelsHtml += "<div id=\"cf" + idx + "\" style=\"display:" + (config.pushChannels[i].type == PUSH_TYPE_CUSTOM ? "block" : "none") + "\"><div class=\"fg\"><label>Body模板</label><textarea name=\"push" + idx + "body\" rows=\"3\">" + config.pushChannels[i].customBody + "</textarea></div></div>";
    
    channelsHtml += "</div></div>";
  }
  html.replace("%PUSH_CHANNELS%", channelsHtml);
  
  
  server.send(200, "text/html", html);
}

// 处理工具箱页面请求 (重定向到首页)
void handleToolsPage() {
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

// 处理保存配置请求 (全量保存)
void handleSave() {
  if (!checkAuth()) return;
  
  // 喂狗防止超时
  esp_task_wdt_reset();
  yield();
  
  // WiFi 网络配置
  for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
    String prefix = "wifi" + String(i);
    String newSsid = server.arg(prefix + "ssid");
    String newPass = server.arg(prefix + "pass");
    String newEn = server.arg(prefix + "en");
    
    // 只有当 SSID 非空时才更新（避免清空已保存的配置）
    if (newSsid.length() > 0) {
      config.wifiNetworks[i].ssid = newSsid;
    }
    // 密码为空时保留原密码
    if (newPass.length() > 0) {
      config.wifiNetworks[i].password = newPass;
    }
    // enabled 状态始终更新
    config.wifiNetworks[i].enabled = (newEn == "true");
    
    // 调试输出
    Serial.printf("WiFi%d: SSID=%s, Pass=%s, En=%d\n", 
      i, config.wifiNetworks[i].ssid.c_str(), 
      config.wifiNetworks[i].password.length() > 0 ? "***" : "(empty)",
      config.wifiNetworks[i].enabled);
  }
  
  esp_task_wdt_reset();
  yield();
  
  // 获取新的 Web 账号密码
  String newWebUser = server.arg("webUser");
  String newWebPass = server.arg("webPass");
  if (newWebUser.length() > 0 && newWebPass.length() > 0) {
    config.webUser = newWebUser;
    config.webPass = newWebPass;
  }
  
  // SMTP 配置
  config.emailEnabled = server.arg("smtpEn") == "true";
  config.smtpServer = server.arg("smtpServer");
  config.smtpPort = server.arg("smtpPort").toInt();
  if (config.smtpPort == 0) config.smtpPort = 465;
  config.smtpUser = server.arg("smtpUser");
  config.smtpPass = server.arg("smtpPass");
  config.smtpSendTo = server.arg("smtpSendTo");
  
  esp_task_wdt_reset();
  yield();
  
  // 推送通道配置
  for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
    String prefix = "push" + String(i);
    config.pushChannels[i].enabled = server.arg(prefix + "en") == "true";
    config.pushChannels[i].type = (PushType)server.arg(prefix + "type").toInt();
    config.pushChannels[i].url = server.arg(prefix + "url");
    config.pushChannels[i].name = server.arg(prefix + "name");
    config.pushChannels[i].key1 = server.arg(prefix + "k1"); // Telegram Chat ID
    config.pushChannels[i].customBody = server.arg(prefix + "body");
  }
  
  esp_task_wdt_reset();
  yield();
  
  // MQTT 配置
  config.mqttEnabled = server.arg("mqttEn") == "true";
  config.mqttServer = server.arg("mqttServer");
  config.mqttPort = server.arg("mqttPort").toInt();
  if (config.mqttPort == 0) config.mqttPort = 1883;
  config.mqttUser = server.arg("mqttUser");
  config.mqttPass = server.arg("mqttPass");
  config.mqttPrefix = server.arg("mqttPrefix");
  if (config.mqttPrefix.length() == 0) config.mqttPrefix = "sms";
  config.mqttControlOnly = server.arg("mqttCtrlOnly") == "on" || server.arg("mqttCtrlOnly") == "true"; 
  
  // HA 自动发现配置
  config.mqttHaDiscovery = server.arg("mqttHaDiscovery") == "true";
  config.mqttHaPrefix = server.arg("mqttHaPrefix");
  if (config.mqttHaPrefix.length() == 0) config.mqttHaPrefix = "homeassistant";
  
  esp_task_wdt_reset();
  yield();
  
  // 黑白名单配置 (全量保存时也更新)
  if (server.hasArg("filterEn")) { // 只有当前端提交了这些字段才更新
      config.filterEnabled = server.arg("filterEn") == "true";
      config.filterIsWhitelist = server.arg("filterIsWhitelist") == "true";
      config.filterList = server.arg("filterList");
      config.filterList.replace("\r", "");
      config.filterList.replace("\n", ","); // 换行转逗号
  }

  // 定时任务配置 (全量保存时)
  if (server.hasArg("timerEn")) {
      config.timerEnabled = server.arg("timerEn") == "true";
      config.timerType = server.arg("timerType").toInt();
      config.timerInterval = server.arg("timerInterval").toInt();
      if (config.timerInterval < 1) config.timerInterval = 1;
      config.timerPhone = server.arg("timerPhone");
      config.timerMessage = server.arg("timerMessage");
      timerIntervalSec = (unsigned long)config.timerInterval * 24UL * 60UL * 60UL;
  }
  
  // 喂狗并保存配置
  esp_task_wdt_reset();
  yield();
  saveConfig();
  esp_task_wdt_reset();
  yield();
  configValid = isConfigValid();
  
  // 返回成功响应，让前端询问用户是否重启
  String json = "{\"success\":true,\"message\":\"配置已保存\",\"needRestart\":true}";
  server.send(200, "application/json", json);
}

// 处理发送短信请求
void handleSendSms() {
  if (!checkAuth()) return;
  
  // 解析 JSON body (前端 postJ 发送 JSON 格式)
  String body = server.arg("plain");
  String phone = "";
  String content = "";
  
  // 简易 JSON 解析
  int phoneIdx = body.indexOf("\"phone\"");
  if (phoneIdx >= 0) {
    int colonIdx = body.indexOf(":", phoneIdx);
    int startQuote = body.indexOf("\"", colonIdx);
    int endQuote = body.indexOf("\"", startQuote + 1);
    if (startQuote >= 0 && endQuote > startQuote) {
      phone = body.substring(startQuote + 1, endQuote);
    }
  }
  
  int contentIdx = body.indexOf("\"content\"");
  if (contentIdx >= 0) {
    int colonIdx = body.indexOf(":", contentIdx);
    int startQuote = body.indexOf("\"", colonIdx);
    int endQuote = body.indexOf("\"", startQuote + 1);
    if (startQuote >= 0 && endQuote > startQuote) {
      content = body.substring(startQuote + 1, endQuote);
    }
  }
  
  phone.trim();
  content.trim();
  
  bool success = false;
  String resultMsg = "";
  
  if (phone.length() == 0) {
    resultMsg = "错误：请输入目标号码";
  } else if (content.length() == 0) {
    resultMsg = "错误：请输入短信内容";
  } else {
    Serial.println("网页端发送短信请求");
    Serial.println("目标号码: " + phone);
    Serial.println("短信内容: " + content);
    
    success = sendSMS(phone.c_str(), content.c_str());
    if (success) {
      stats.smsSent++;
      saveStats();
    }
    resultMsg = success ? "短信发送成功！" : "短信发送失败，请检查模组状态";
  }
  
  String json = "{";
  json += "\"success\":" + String(success ? "true" : "false") + ",";
  json += "\"message\":\"" + resultMsg + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

// 处理定时任务配置保存 (API: /timer)
void handleTimer() {
  if (!checkAuth()) return;
  
  String body = server.arg("plain");
  
  config.timerEnabled = body.indexOf("\"enabled\":true") >= 0;
  
  // 简易解析 JSON (假设标准格式)
  // type
  int typeIdx = body.indexOf("\"type\":");
  if (typeIdx > 0) config.timerType = body.substring(typeIdx+7).toInt();
  
  // interval
  int intIdx = body.indexOf("\"interval\":");
  if (intIdx > 0) config.timerInterval = body.substring(intIdx+11).toInt();
  if (config.timerInterval < 1) config.timerInterval = 1;
  
  // phone
  int phIdx = body.indexOf("\"phone\":\"");
  if (phIdx > 0) {
      int end = body.indexOf("\"", phIdx+9);
      config.timerPhone = body.substring(phIdx+9, end);
  }
  
  // message
  int msgIdx = body.indexOf("\"message\":\"");
  if (msgIdx > 0) {
      int end = body.indexOf("\"", msgIdx+11);
      config.timerMessage = body.substring(msgIdx+11, end);
  }
  
  timerIntervalSec = (unsigned long)config.timerInterval * 24UL * 60UL * 60UL;
  lastTimerExec = millis();
  saveConfig();
  
  String json = "{\"success\":true,\"remain\":" + String(timerIntervalSec) + "}";
  server.send(200, "application/json", json);
}

// 处理重启请求
void handleRestart() {
  if (!checkAuth()) return;
  server.send(200, "application/json", "{\"success\":true,\"message\":\"设备将在2秒后重启\"}");
  delay(2000);
  ESP.restart();
}

// 获取短信历史记录
void handleSmsHistory() {
  if (!checkAuth()) return;
  
  String historyJson = getSmsHistory();
  String json = "{\"history\":" + historyJson + "}";
  
  server.send(200, "application/json", json);
}

// 获取统计信息
void handleStats() {
  if (!checkAuth()) return;
  
  String json = "{";
  json += "\"received\":" + String(stats.smsReceived) + ",";
  json += "\"sent\":" + String(stats.smsSent) + ",";
  json += "\"pushOk\":" + String(stats.pushSuccess) + ",";
  json += "\"pushFail\":" + String(stats.pushFailed) + ",";
  json += "\"boots\":" + String(stats.bootCount) + ",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"wifiRssi\":" + String(WiFi.RSSI());
  json += "}";
  
  server.send(200, "application/json", json);
}

// 保存黑白名单配置
void handleFilterSave() {
  if (!checkAuth()) return;
  
  String body = server.arg("plain");
  // 简单 JSON 解析
  config.filterEnabled = body.indexOf("\"enabled\":true") >= 0;
  config.filterIsWhitelist = body.indexOf("\"whitelist\":true") >= 0;
  
  // 解析 numbers 数组或字符串
  int numStart = body.indexOf("\"numbers\":");
  if (numStart > 0) {
      int arrStart = body.indexOf("[", numStart);
      int arrEnd = body.indexOf("]", numStart);
      if (arrStart > 0 && arrEnd > arrStart) {
          String arrStr = body.substring(arrStart + 1, arrEnd);
          // 将 JSON 数组 "a","b" 转换为 a,b 存入 filterList
          config.filterList = "";
          int lastQ = -1;
          while (true) {
              int startQ = arrStr.indexOf("\"", lastQ + 1);
              if (startQ < 0) break;
              int endQ = arrStr.indexOf("\"", startQ + 1);
              if (endQ < 0) break;
              if (config.filterList.length() > 0) config.filterList += ",";
              config.filterList += arrStr.substring(startQ + 1, endQ);
              lastQ = endQ;
          }
      }
  }
  
  saveConfig();
  server.send(200, "application/json", "{\"success\":true,\"message\":\"号码名单已更新\"}");
}

// 保存内容关键词过滤配置
void handleContentFilterSave() {
  if (!checkAuth()) return;
  
  String body = server.arg("plain");
  // 简单 JSON 解析
  config.contentFilterEnabled = body.indexOf("\"enabled\":true") >= 0;
  config.contentFilterIsWhitelist = body.indexOf("\"whitelist\":true") >= 0;
  
  // 解析 keywords 字符串
  int kwStart = body.indexOf("\"keywords\":\"");
  if (kwStart > 0) {
    int valStart = kwStart + 12;  // 跳过 "keywords":"
    int valEnd = body.indexOf("\"", valStart);
    if (valEnd > valStart) {
      config.contentFilterList = body.substring(valStart, valEnd);
    }
  }
  
  saveConfig();
  server.send(200, "application/json", "{\"success\":true,\"message\":\"关键词过滤已更新\"}");
}
