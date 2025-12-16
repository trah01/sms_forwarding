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
  html.replace("%COMMON_JS%", commonJs);
  html.replace("%IP%", WiFi.localIP().toString());
  
  // MQTT 状态
  String mqttStatusText;
  String mqttClass;
  if (config.mqttEnabled) {
    if (mqttClient.connected()) {
      mqttStatusText = "已连接 " + config.mqttPrefix + "/" + mqttDeviceId + "/";
      mqttClass = "on";
    } else {
      mqttStatusText = "未连接";
      mqttClass = "off";
    }
  } else {
    mqttStatusText = "未启用";
    mqttClass = "off";
  }
  html.replace("%MQTT_STATUS%", mqttStatusText);
  html.replace("%MQTT_CLASS%", mqttClass);
  
  html.replace("%WEB_USER%", config.webUser);
  html.replace("%WEB_PASS%", config.webPass);
  html.replace("%SMTP_SERVER%", config.smtpServer);
  html.replace("%SMTP_PORT%", String(config.smtpPort));
  html.replace("%SMTP_USER%", config.smtpUser);
  html.replace("%SMTP_PASS%", config.smtpPass);
  html.replace("%SMTP_SEND_TO%", config.smtpSendTo);
  html.replace("%SMTP_EN_CLASS%", config.emailEnabled ? " en" : "");
  html.replace("%SMTP_CHECKED%", config.emailEnabled ? "checked" : "");
  
  // MQTT 配置占位符
  html.replace("%MQTT_EN_CLASS%", config.mqttEnabled ? " en" : "");
  html.replace("%MQTT_CHECKED%", config.mqttEnabled ? "checked" : "");
  html.replace("%MQTT_SERVER%", config.mqttServer);
  html.replace("%MQTT_PORT%", String(config.mqttPort));
  html.replace("%MQTT_USER%", config.mqttUser);
  html.replace("%MQTT_PASS%", config.mqttPass);
  html.replace("%MQTT_PREFIX%", config.mqttPrefix);
  html.replace("%MQTT_CTRL_ONLY%", config.mqttControlOnly ? "checked" : "");
  
  // 生成推送通道 HTML
  String channelsHtml = "";
  for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
    String idx = String(i);
    String enabledClass = config.pushChannels[i].enabled ? " en" : "";
    String checked = config.pushChannels[i].enabled ? " checked" : "";
    
    channelsHtml += "<div class=\"ch" + enabledClass + "\" id=\"ch" + idx + "\">";
    channelsHtml += "<div class=\"ch-h\" onclick=\"fold(" + idx + ")\">";
    channelsHtml += "<input type=\"checkbox\" name=\"push" + idx + "en\" id=\"en" + idx + "\" onclick=\"event.stopPropagation();en(" + idx + ")\"" + checked + ">";
    channelsHtml += "<span>Webhook " + String(i + 1) + "</span></div>";
    channelsHtml += "<div class=\"ch-b\">";
    
    // 通道名称
    channelsHtml += "<div class=\"fg\"><label>名称</label><input name=\"push" + idx + "name\" value=\"" + config.pushChannels[i].name + "\" placeholder=\"自定义名称\"></div>";
    
    // 推送类型
    channelsHtml += "<div class=\"fg\"><label>推送方式</label>";
    channelsHtml += "<select name=\"push" + idx + "type\" id=\"tp" + idx + "\" onchange=\"upd(" + idx + ")\">";
    channelsHtml += "<option value=\"1\"" + String(config.pushChannels[i].type == PUSH_TYPE_POST_JSON ? " selected" : "") + ">POST JSON</option>";
    channelsHtml += "<option value=\"2\"" + String(config.pushChannels[i].type == PUSH_TYPE_BARK ? " selected" : "") + ">Bark</option>";
    channelsHtml += "<option value=\"3\"" + String(config.pushChannels[i].type == PUSH_TYPE_GET ? " selected" : "") + ">GET请求</option>";
    channelsHtml += "<option value=\"4\"" + String(config.pushChannels[i].type == PUSH_TYPE_CUSTOM ? " selected" : "") + ">自定义模板</option>";
    channelsHtml += "</select></div>";
    
    // URL
    channelsHtml += "<div class=\"fg\"><label>推送URL</label><input name=\"push" + idx + "url\" value=\"" + config.pushChannels[i].url + "\" placeholder=\"http://...\"></div>";
    
    // 自定义模板区域
    channelsHtml += "<div id=\"cf" + idx + "\" style=\"display:none\"><div class=\"fg\"><label>请求体模板</label><textarea name=\"push" + idx + "body\" rows=\"3\">" + config.pushChannels[i].customBody + "</textarea></div></div>";
    
    channelsHtml += "</div></div>";
  }
  html.replace("%PUSH_CHANNELS%", channelsHtml);
  
  server.send(200, "text/html", html);
}

// 处理工具箱页面请求
void handleToolsPage() {
  if (!checkAuth()) return;
  
  String html = String(htmlToolsPage);
  html.replace("%COMMON_CSS%", commonCss);
  html.replace("%COMMON_JS%", commonJs);
  html.replace("%IP%", WiFi.localIP().toString());
  
  // MQTT 状态
  String mqttStatusText2;
  String mqttClass2;
  if (config.mqttEnabled) {
    if (mqttClient.connected()) {
      mqttStatusText2 = "已连接 " + config.mqttPrefix + "/" + mqttDeviceId + "/";
      mqttClass2 = "on";
    } else {
      mqttStatusText2 = "未连接";
      mqttClass2 = "off";
    }
  } else {
    mqttStatusText2 = "未启用";
    mqttClass2 = "off";
  }
  html.replace("%MQTT_STATUS%", mqttStatusText2);
  html.replace("%MQTT_CLASS%", mqttClass2);
  
  // 定时任务状态（使用秒计算）
  unsigned long remainSec = 0;
  if (config.timerEnabled && timerIntervalSec > 0) {
    unsigned long elapsedSec = (millis() - lastTimerExec) / 1000;
    if (elapsedSec < timerIntervalSec) {
      remainSec = timerIntervalSec - elapsedSec;
    }
  }
  
  html.replace("%TIMER_BOX_CLASS%", config.timerEnabled ? "" : "timer-off");
  html.replace("%TIMER_STATUS%", config.timerEnabled ? (config.timerType == 0 ? "定时Ping" : "定时短信") : "已禁用");
  
  // 格式化剩余时间
  String countdown = "--";
  if (config.timerEnabled && remainSec > 0) {
    int d = remainSec / 86400;
    int h = (remainSec % 86400) / 3600;
    int m = (remainSec % 3600) / 60;
    countdown = "";
    if (d > 0) countdown += String(d) + "天";
    if (h > 0) countdown += String(h) + "时";
    if (m > 0) countdown += String(m) + "分";
    countdown += "后执行";
  }
  html.replace("%TIMER_COUNTDOWN%", countdown);
  html.replace("%TIMER_REMAIN%", String(remainSec));
  html.replace("%TIMER_CHECKED%", config.timerEnabled ? "checked" : "");
  html.replace("%TIMER_TYPE0%", config.timerType == 0 ? "selected" : "");
  html.replace("%TIMER_TYPE1%", config.timerType == 1 ? "selected" : "");
  html.replace("%TIMER_INTERVAL%", String(config.timerInterval));
  html.replace("%TIMER_PHONE%", config.timerPhone);
  html.replace("%TIMER_MSG%", config.timerMessage);
  
  server.send(200, "text/html", html);
}

// 处理保存配置请求
void handleSave() {
  if (!checkAuth()) return;
  
  // 获取新的 Web 账号密码
  String newWebUser = server.arg("webUser");
  String newWebPass = server.arg("webPass");
  
  // 验证 Web 账号密码不能为空
  if (newWebUser.length() == 0) newWebUser = DEFAULT_WEB_USER;
  if (newWebPass.length() == 0) newWebPass = DEFAULT_WEB_PASS;
  
  config.webUser = newWebUser;
  config.webPass = newWebPass;
  config.emailEnabled = server.arg("smtpEn") == "on";
  config.smtpServer = server.arg("smtpServer");
  config.smtpPort = server.arg("smtpPort").toInt();
  if (config.smtpPort == 0) config.smtpPort = 465;
  config.smtpUser = server.arg("smtpUser");
  config.smtpPass = server.arg("smtpPass");
  config.smtpSendTo = server.arg("smtpSendTo");
  
  // 保存推送通道配置
  for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
    String idx = String(i);
    config.pushChannels[i].enabled = server.arg("push" + idx + "en") == "on";
    config.pushChannels[i].type = (PushType)server.arg("push" + idx + "type").toInt();
    config.pushChannels[i].url = server.arg("push" + idx + "url");
    config.pushChannels[i].name = server.arg("push" + idx + "name");
    config.pushChannels[i].key1 = server.arg("push" + idx + "key1");
    config.pushChannels[i].key2 = server.arg("push" + idx + "key2");
    config.pushChannels[i].customBody = server.arg("push" + idx + "body");
    if (config.pushChannels[i].name.length() == 0) {
      config.pushChannels[i].name = "通道" + String(i + 1);
    }
  }
  
  // 保存 MQTT 配置
  bool mqttWasEnabled = config.mqttEnabled;
  config.mqttEnabled = server.arg("mqttEn") == "on";
  config.mqttServer = server.arg("mqttServer");
  config.mqttPort = server.arg("mqttPort").toInt();
  if (config.mqttPort == 0) config.mqttPort = 1883;
  config.mqttUser = server.arg("mqttUser");
  config.mqttPass = server.arg("mqttPass");
  config.mqttPrefix = server.arg("mqttPrefix");
  if (config.mqttPrefix.length() == 0) config.mqttPrefix = "sms";
  config.mqttControlOnly = server.arg("mqttCtrlOnly") == "on";
  
  saveConfig();
  configValid = isConfigValid();
  
  String json = "{\"success\":true,\"message\":\"配置已保存\"}";
  server.send(200, "application/json", json);
  
  // 如果配置有效，发送启动通知
  if (configValid) {
    Serial.println("配置有效，发送启动通知...");
    String subject = "短信转发器配置已更新";
    String body = "设备配置已更新\n设备地址: " + getDeviceUrl();
    sendEmailNotification(subject.c_str(), body.c_str());
  }
}

// 处理发送短信请求
void handleSendSms() {
  if (!checkAuth()) return;
  
  String phone = server.arg("phone");
  String content = server.arg("content");
  
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
    resultMsg = success ? "短信发送成功！" : "短信发送失败，请检查模组状态";
  }
  
  String json = "{";
  json += "\"success\":" + String(success ? "true" : "false") + ",";
  json += "\"message\":\"" + resultMsg + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

// 处理定时任务配置保存
void handleTimer() {
  if (!checkAuth()) return;
  
  String body = server.arg("plain");
  Serial.println("收到定时任务配置: " + body);
  
  // 简单解析 JSON
  bool enabled = body.indexOf("\"enabled\":true") >= 0;
  
  int typeIdx = body.indexOf("\"type\":");
  int timerType = 0;
  if (typeIdx >= 0) {
    timerType = body.substring(typeIdx + 7, typeIdx + 8).toInt();
  }
  
  int intervalIdx = body.indexOf("\"interval\":");
  int interval = 30;  // 默认 30 天
  if (intervalIdx >= 0) {
    int endIdx = body.indexOf(",", intervalIdx + 11);
    if (endIdx < 0) endIdx = body.indexOf("}", intervalIdx + 11);
    interval = body.substring(intervalIdx + 11, endIdx).toInt();
    if (interval < 1) interval = 1;
    if (interval > 365) interval = 365;
  }
  
  int phoneIdx = body.indexOf("\"phone\":\"");
  String phone = "";
  if (phoneIdx >= 0) {
    int endIdx = body.indexOf("\"", phoneIdx + 9);
    phone = body.substring(phoneIdx + 9, endIdx);
  }
  
  int msgIdx = body.indexOf("\"message\":\"");
  String message = "保号短信";
  if (msgIdx >= 0) {
    int endIdx = body.indexOf("\"", msgIdx + 11);
    message = body.substring(msgIdx + 11, endIdx);
  }
  
  // 更新配置
  config.timerEnabled = enabled;
  config.timerType = timerType;
  config.timerInterval = interval;
  config.timerPhone = phone;
  config.timerMessage = message;
  
  // 更新定时间隔（天转秒）
  timerIntervalSec = (unsigned long)interval * 24UL * 60UL * 60UL;
  
  // 重置执行时间
  lastTimerExec = millis();
  
  // 保存配置
  saveConfig();
  
  // 返回剩余秒数
  String json = "{\"success\":true,\"remain\":" + String(timerIntervalSec) + "}";
  server.send(200, "application/json", json);
  
  Serial.println("定时任务配置已保存: " + String(enabled ? "启用" : "禁用") + 
                 ", 类型: " + String(timerType) + 
                 ", 间隔: " + String(interval) + "天");
}
