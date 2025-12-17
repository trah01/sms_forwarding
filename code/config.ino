/*
 * config.ino - 配置相关函数实现
 */

// 短信历史和统计的全局变量定义
SmsRecord smsHistory[MAX_SMS_HISTORY];
int smsHistoryIndex = 0;
Statistics stats = {0, 0, 0, 0, 0};

// 保存配置到 NVS
void saveConfig() {
  preferences.begin("sms_config", false);
  preferences.putString("smtpServer", config.smtpServer);
  preferences.putInt("smtpPort", config.smtpPort);
  preferences.putString("smtpUser", config.smtpUser);
  preferences.putString("smtpPass", config.smtpPass);
  preferences.putString("smtpSendTo", config.smtpSendTo);
  preferences.putBool("smtpEn", config.emailEnabled);
  preferences.putString("webUser", config.webUser);
  preferences.putString("webPass", config.webPass);
  
  // 保存推送通道配置
  for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
    String prefix = "push" + String(i);
    preferences.putBool((prefix + "en").c_str(), config.pushChannels[i].enabled);
    preferences.putUChar((prefix + "type").c_str(), (uint8_t)config.pushChannels[i].type);
    preferences.putString((prefix + "url").c_str(), config.pushChannels[i].url);
    preferences.putString((prefix + "name").c_str(), config.pushChannels[i].name);
    preferences.putString((prefix + "k1").c_str(), config.pushChannels[i].key1);
    preferences.putString((prefix + "k2").c_str(), config.pushChannels[i].key2);
    preferences.putString((prefix + "body").c_str(), config.pushChannels[i].customBody);
  }
  
  // 保存定时任务配置
  preferences.putBool("timerEn", config.timerEnabled);
  preferences.putInt("timerType", config.timerType);
  preferences.putInt("timerInt", config.timerInterval);
  preferences.putString("timerPhone", config.timerPhone);
  preferences.putString("timerMsg", config.timerMessage);
  
  // 保存 MQTT 配置
  preferences.putBool("mqttEn", config.mqttEnabled);
  preferences.putBool("mqttCtrlOnly", config.mqttControlOnly);
  preferences.putString("mqttServer", config.mqttServer);
  preferences.putInt("mqttPort", config.mqttPort);
  preferences.putString("mqttUser", config.mqttUser);
  preferences.putString("mqttPass", config.mqttPass);
  preferences.putString("mqttPrefix", config.mqttPrefix);
  
  // 保存黑白名单配置
  preferences.putBool("filterEn", config.filterEnabled);
  preferences.putBool("filterWL", config.filterIsWhitelist);
  preferences.putString("filterList", config.filterList);
  
  preferences.end();
  Serial.println("配置已保存");
}

// 从 NVS 加载配置
void loadConfig() {
  preferences.begin("sms_config", true);
  config.smtpServer = preferences.getString("smtpServer", "");
  config.smtpPort = preferences.getInt("smtpPort", 465);
  config.smtpUser = preferences.getString("smtpUser", "");
  config.smtpPass = preferences.getString("smtpPass", "");
  config.smtpSendTo = preferences.getString("smtpSendTo", "");
  config.emailEnabled = preferences.getBool("smtpEn", false);
  config.webUser = preferences.getString("webUser", DEFAULT_WEB_USER);
  config.webPass = preferences.getString("webPass", DEFAULT_WEB_PASS);
  
  // 加载推送通道配置
  for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
    String prefix = "push" + String(i);
    config.pushChannels[i].enabled = preferences.getBool((prefix + "en").c_str(), false);
    config.pushChannels[i].type = (PushType)preferences.getUChar((prefix + "type").c_str(), PUSH_TYPE_POST_JSON);
    config.pushChannels[i].url = preferences.getString((prefix + "url").c_str(), "");
    config.pushChannels[i].name = preferences.getString((prefix + "name").c_str(), "通道" + String(i + 1));
    config.pushChannels[i].key1 = preferences.getString((prefix + "k1").c_str(), "");
    config.pushChannels[i].key2 = preferences.getString((prefix + "k2").c_str(), "");
    config.pushChannels[i].customBody = preferences.getString((prefix + "body").c_str(), "");
  }
  
  // 加载定时任务配置
  config.timerEnabled = preferences.getBool("timerEn", false);
  config.timerType = preferences.getInt("timerType", 0);
  config.timerInterval = preferences.getInt("timerInt", 30);
  config.timerPhone = preferences.getString("timerPhone", "");
  config.timerMessage = preferences.getString("timerMsg", "保号短信");
  timerIntervalSec = (unsigned long)config.timerInterval * 24UL * 60UL * 60UL;
  
  // 加载 MQTT 配置
  config.mqttEnabled = preferences.getBool("mqttEn", false);
  config.mqttControlOnly = preferences.getBool("mqttCtrlOnly", false);
  config.mqttServer = preferences.getString("mqttServer", "");
  config.mqttPort = preferences.getInt("mqttPort", 1883);
  config.mqttUser = preferences.getString("mqttUser", "");
  config.mqttPass = preferences.getString("mqttPass", "");
  config.mqttPrefix = preferences.getString("mqttPrefix", "sms");
  
  // 加载黑白名单配置
  config.filterEnabled = preferences.getBool("filterEn", false);
  config.filterIsWhitelist = preferences.getBool("filterWL", false);
  config.filterList = preferences.getString("filterList", "");
  
  preferences.end();
  
  // 加载统计数据
  loadStats();
  
  // 初始化短信历史
  for (int i = 0; i < MAX_SMS_HISTORY; i++) {
    smsHistory[i].valid = false;
  }
  
  Serial.println("配置已加载");
}

// 检查推送通道是否有效
bool isPushChannelValid(const PushChannel& ch) {
  if (!ch.enabled) return false;
  switch (ch.type) {
    case PUSH_TYPE_POST_JSON:
    case PUSH_TYPE_BARK:
    case PUSH_TYPE_GET:
    case PUSH_TYPE_CUSTOM:
      return ch.url.length() > 0;
    default:
      return false;
  }
}

// 检查配置是否有效
bool isConfigValid() {
  bool emailValid = config.emailEnabled &&
                    config.smtpServer.length() > 0 && 
                    config.smtpUser.length() > 0 && 
                    config.smtpPass.length() > 0 && 
                    config.smtpSendTo.length() > 0;
  
  bool pushValid = false;
  for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
    if (isPushChannelValid(config.pushChannels[i])) {
      pushValid = true;
      break;
    }
  }
  
  return emailValid || pushValid;
}

// 获取设备 URL
String getDeviceUrl() {
  return "http://" + WiFi.localIP().toString() + "/";
}

// 初始化 SPIFFS
void initSmsStorage() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS 初始化失败");
  }
}

// 添加短信到历史记录（SPIFFS 存储）
void addSmsToHistory(const char* sender, const char* message, const char* timestamp) {
  stats.smsReceived++;
  
  // 构建 JSON 行
  String line = "{\"t\":\"" + String(timestamp) + "\",\"s\":\"" + String(sender) + "\",\"m\":\"";
  // 转义消息内容中的引号和换行
  String msg = String(message);
  msg.replace("\\", "\\\\");
  msg.replace("\"", "\\\"");
  msg.replace("\n", "\\n");
  msg.replace("\r", "");
  line += msg.substring(0, 200) + "\"}\n";  // 限制消息长度
  
  // 检查文件大小，超过 50KB 则清理旧数据
  File f = SPIFFS.open("/sms.txt", "r");
  size_t fileSize = f ? f.size() : 0;
  f.close();
  
  if (fileSize > 50000) {
    // 读取后半部分保留
    f = SPIFFS.open("/sms.txt", "r");
    f.seek(fileSize / 2);
    String remaining = f.readString();
    f.close();
    // 从第一个换行开始保留
    int nl = remaining.indexOf('\n');
    if (nl > 0) remaining = remaining.substring(nl + 1);
    f = SPIFFS.open("/sms.txt", "w");
    f.print(remaining);
    f.close();
  }
  
  // 追加新短信
  f = SPIFFS.open("/sms.txt", "a");
  if (f) {
    f.print(line);
    f.close();
  }
}

// 获取短信历史（返回 JSON 数组字符串）
String getSmsHistory() {
  File f = SPIFFS.open("/sms.txt", "r");
  if (!f) return "[]";
  
  String result = "[";
  bool first = true;
  
  // 读取所有行并倒序排列
  std::vector<String> lines;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    if (line.length() > 10) {
      lines.push_back(line);
    }
  }
  f.close();
  
  // 倒序输出最近100条
  int count = 0;
  for (int i = lines.size() - 1; i >= 0 && count < 100; i--, count++) {
    if (!first) result += ",";
    first = false;
    result += lines[i];
  }
  result += "]";
  return result;
}

// 检查号码是否被过滤
bool isNumberFiltered(const char* number) {
  if (!config.filterEnabled) return false;
  if (config.filterList.length() == 0) return false;
  
  String num = String(number);
  // 简单的包含匹配。如果需要更精准，可以在保存时加上分隔符，如 ",num1,num2,"，查找 ",num,"
  // 这里暂时使用简单子串匹配，如果 filterList 包含该号码则命中
  bool found = config.filterList.indexOf(num) >= 0;
  
  if (config.filterIsWhitelist) {
    return !found; // 白名单：未找到则过滤
  } else {
    return found; // 黑名单：找到则过滤
  }
}

// 保存统计数据
void saveStats() {
  preferences.begin("sms_stats", false);
  preferences.putULong("received", stats.smsReceived);
  preferences.putULong("sent", stats.smsSent);
  preferences.putULong("pushOk", stats.pushSuccess);
  preferences.putULong("pushFail", stats.pushFailed);
  preferences.putULong("boots", stats.bootCount);
  preferences.end();
}

// 加载统计数据
void loadStats() {
  preferences.begin("sms_stats", true);
  stats.smsReceived = preferences.getULong("received", 0);
  stats.smsSent = preferences.getULong("sent", 0);
  stats.pushSuccess = preferences.getULong("pushOk", 0);
  stats.pushFailed = preferences.getULong("pushFail", 0);
  stats.bootCount = preferences.getULong("boots", 0) + 1;
  preferences.end();
  
  // 保存更新后的启动次数
  preferences.begin("sms_stats", false);
  preferences.putULong("boots", stats.bootCount);
  preferences.end();
}
