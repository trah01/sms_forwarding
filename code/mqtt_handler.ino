/*
 * mqtt_handler.ino - MQTT 功能实现
 * 
 * 支持两类主题:
 * 1. 用户自定义前缀 (如 sms/device_id/...)
 * 2. Home Assistant MQTT 自动发现 (homeassistant/...)
 */

#include <PubSubClient.h>

// 获取 MAC 地址后缀作为设备唯一 ID
String getMacSuffix() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  return mac.substring(6);  // 取后 6 位
}

// 初始化 MQTT 主题
void initMqttTopics() {
  mqttDeviceId = getMacSuffix();
  String prefix = config.mqttPrefix + "/" + mqttDeviceId;
  
  // 用户自定义前缀 - 发布主题
  mqttTopicStatus = prefix + "/status";
  mqttTopicSmsReceived = prefix + "/sms/received";
  mqttTopicSmsSent = prefix + "/sms/sent";
  mqttTopicPingResult = prefix + "/ping/result";
  
  // 用户自定义前缀 - 订阅主题
  mqttTopicSmsSend = prefix + "/sms/send";
  mqttTopicPing = prefix + "/ping";
  mqttTopicCmd = prefix + "/cmd";
  
  // Home Assistant 自动发现主题（状态数据发布位置）
  String haPrefix = config.mqttHaPrefix;
  if (haPrefix.length() == 0) haPrefix = "homeassistant";
  mqttHaStatusTopic = haPrefix + "/sensor/sms_forwarder_" + mqttDeviceId + "/state";
  mqttHaSmsReceivedTopic = haPrefix + "/event/sms_forwarder_" + mqttDeviceId + "_sms/event";
  
  Serial.println("MQTT设备ID: " + mqttDeviceId);
  Serial.println("用户主题前缀: " + prefix);
  if (config.mqttHaDiscovery) {
    Serial.println("HA自动发现: 已启用 (前缀: " + haPrefix + ")");
  }
}

// 发布 Home Assistant MQTT 自动发现配置
void publishHaDiscoveryConfig() {
  if (!config.mqttHaDiscovery || !mqttClient.connected()) return;
  
  String haPrefix = config.mqttHaPrefix;
  if (haPrefix.length() == 0) haPrefix = "homeassistant";
  String nodeId = "sms_forwarder_" + mqttDeviceId;
  
  // 设备信息（所有实体共享）
  String deviceInfo = "\"device\":{";
  deviceInfo += "\"identifiers\":[\"" + nodeId + "\"],";
  deviceInfo += "\"name\":\"短信转发器 " + mqttDeviceId + "\",";
  deviceInfo += "\"manufacturer\":\"DIY\",";
  deviceInfo += "\"model\":\"ESP32-C3 SMS Forwarder\",";
  deviceInfo += "\"sw_version\":\"1.0\"";
  deviceInfo += "}";
  
  // 1. 设备状态传感器
  String statusConfigTopic = haPrefix + "/sensor/" + nodeId + "_status/config";
  String statusConfig = "{";
  statusConfig += "\"name\":\"状态\",";
  statusConfig += "\"unique_id\":\"" + nodeId + "_status\",";
  statusConfig += "\"state_topic\":\"" + mqttHaStatusTopic + "\",";
  statusConfig += "\"value_template\":\"{{ value_json.status }}\",";
  statusConfig += "\"icon\":\"mdi:message-text\",";
  statusConfig += deviceInfo;
  statusConfig += "}";
  mqttClient.publish(statusConfigTopic.c_str(), statusConfig.c_str(), true);
  
  // 2. WiFi 信号传感器
  String wifiConfigTopic = haPrefix + "/sensor/" + nodeId + "_wifi/config";
  String wifiConfig = "{";
  wifiConfig += "\"name\":\"WiFi信号\",";
  wifiConfig += "\"unique_id\":\"" + nodeId + "_wifi\",";
  wifiConfig += "\"state_topic\":\"" + mqttHaStatusTopic + "\",";
  wifiConfig += "\"value_template\":\"{{ value_json.wifi_rssi }}\",";
  wifiConfig += "\"unit_of_measurement\":\"dBm\",";
  wifiConfig += "\"device_class\":\"signal_strength\",";
  wifiConfig += "\"icon\":\"mdi:wifi\",";
  wifiConfig += deviceInfo;
  wifiConfig += "}";
  mqttClient.publish(wifiConfigTopic.c_str(), wifiConfig.c_str(), true);
  
  // 3. 4G 信号传感器
  String lteConfigTopic = haPrefix + "/sensor/" + nodeId + "_lte/config";
  String lteConfig = "{";
  lteConfig += "\"name\":\"4G信号\",";
  lteConfig += "\"unique_id\":\"" + nodeId + "_lte\",";
  lteConfig += "\"state_topic\":\"" + mqttHaStatusTopic + "\",";
  lteConfig += "\"value_template\":\"{{ value_json.lte_rsrp }}\",";
  lteConfig += "\"unit_of_measurement\":\"dBm\",";
  lteConfig += "\"device_class\":\"signal_strength\",";
  lteConfig += "\"icon\":\"mdi:signal-4g\",";
  lteConfig += deviceInfo;
  lteConfig += "}";
  mqttClient.publish(lteConfigTopic.c_str(), lteConfig.c_str(), true);
  
  // 4. IP 地址传感器
  String ipConfigTopic = haPrefix + "/sensor/" + nodeId + "_ip/config";
  String ipConfig = "{";
  ipConfig += "\"name\":\"IP地址\",";
  ipConfig += "\"unique_id\":\"" + nodeId + "_ip\",";
  ipConfig += "\"state_topic\":\"" + mqttHaStatusTopic + "\",";
  ipConfig += "\"value_template\":\"{{ value_json.ip }}\",";
  ipConfig += "\"icon\":\"mdi:ip-network\",";
  ipConfig += deviceInfo;
  ipConfig += "}";
  mqttClient.publish(ipConfigTopic.c_str(), ipConfig.c_str(), true);
  
  // 5. 运行时间传感器
  String uptimeConfigTopic = haPrefix + "/sensor/" + nodeId + "_uptime/config";
  String uptimeConfig = "{";
  uptimeConfig += "\"name\":\"运行时间\",";
  uptimeConfig += "\"unique_id\":\"" + nodeId + "_uptime\",";
  uptimeConfig += "\"state_topic\":\"" + mqttHaStatusTopic + "\",";
  uptimeConfig += "\"value_template\":\"{{ (value_json.uptime | int / 3600) | round(1) }}\",";
  uptimeConfig += "\"unit_of_measurement\":\"小时\",";
  uptimeConfig += "\"icon\":\"mdi:clock-outline\",";
  uptimeConfig += deviceInfo;
  uptimeConfig += "}";
  mqttClient.publish(uptimeConfigTopic.c_str(), uptimeConfig.c_str(), true);
  
  // 6. 在线状态二值传感器
  String onlineConfigTopic = haPrefix + "/binary_sensor/" + nodeId + "_online/config";
  String onlineConfig = "{";
  onlineConfig += "\"name\":\"在线\",";
  onlineConfig += "\"unique_id\":\"" + nodeId + "_online\",";
  onlineConfig += "\"state_topic\":\"" + mqttHaStatusTopic + "\",";
  onlineConfig += "\"value_template\":\"{{ value_json.status }}\",";
  onlineConfig += "\"payload_on\":\"online\",";
  onlineConfig += "\"payload_off\":\"offline\",";
  onlineConfig += "\"device_class\":\"connectivity\",";
  onlineConfig += deviceInfo;
  onlineConfig += "}";
  mqttClient.publish(onlineConfigTopic.c_str(), onlineConfig.c_str(), true);
  
  // 7. 重启按钮
  String restartConfigTopic = haPrefix + "/button/" + nodeId + "_restart/config";
  String restartConfig = "{";
  restartConfig += "\"name\":\"重启\",";
  restartConfig += "\"unique_id\":\"" + nodeId + "_restart\",";
  restartConfig += "\"command_topic\":\"" + mqttTopicCmd + "\",";
  restartConfig += "\"payload_press\":\"{\\\"action\\\":\\\"restart\\\"}\",";
  restartConfig += "\"icon\":\"mdi:restart\",";
  restartConfig += deviceInfo;
  restartConfig += "}";
  mqttClient.publish(restartConfigTopic.c_str(), restartConfig.c_str(), true);
  
  // 8. 最近短信发送者传感器
  String senderConfigTopic = haPrefix + "/sensor/" + nodeId + "_last_sender/config";
  String senderConfig = "{";
  senderConfig += "\"name\":\"最近短信发送者\",";
  senderConfig += "\"unique_id\":\"" + nodeId + "_last_sender\",";
  senderConfig += "\"state_topic\":\"" + mqttTopicSmsReceived + "\",";
  senderConfig += "\"value_template\":\"{{ value_json.sender }}\",";
  senderConfig += "\"icon\":\"mdi:account\",";
  senderConfig += deviceInfo;
  senderConfig += "}";
  mqttClient.publish(senderConfigTopic.c_str(), senderConfig.c_str(), true);
  
  // 9. 最近短信内容传感器
  String messageConfigTopic = haPrefix + "/sensor/" + nodeId + "_last_message/config";
  String messageConfig = "{";
  messageConfig += "\"name\":\"最近短信内容\",";
  messageConfig += "\"unique_id\":\"" + nodeId + "_last_message\",";
  messageConfig += "\"state_topic\":\"" + mqttTopicSmsReceived + "\",";
  messageConfig += "\"value_template\":\"{{ value_json.message[:50] }}{% if value_json.message | length > 50 %}...{% endif %}\",";
  messageConfig += "\"json_attributes_topic\":\"" + mqttTopicSmsReceived + "\",";
  messageConfig += "\"icon\":\"mdi:message-text\",";
  messageConfig += deviceInfo;
  messageConfig += "}";
  mqttClient.publish(messageConfigTopic.c_str(), messageConfig.c_str(), true);
  
  // 10. 号码过滤模式选择器
  String filterStatusTopic = config.mqttPrefix + "/" + mqttDeviceId + "/filter/status";
  String filterModeConfigTopic = haPrefix + "/select/" + nodeId + "_filter_mode/config";
  String filterModeConfig = "{";
  filterModeConfig += "\"name\":\"号码过滤模式\",";
  filterModeConfig += "\"unique_id\":\"" + nodeId + "_filter_mode\",";
  filterModeConfig += "\"command_topic\":\"" + mqttTopicCmd + "\",";
  filterModeConfig += "\"command_template\":\"{\\\"action\\\":\\\"set_filter_mode\\\",\\\"mode\\\":\\\"{{ value }}\\\"}\",";
  filterModeConfig += "\"state_topic\":\"" + filterStatusTopic + "\",";
  filterModeConfig += "\"value_template\":\"{{ value_json.filter_mode }}\",";
  filterModeConfig += "\"options\":[\"disabled\",\"whitelist\",\"blacklist\"],";
  filterModeConfig += "\"icon\":\"mdi:filter\",";
  filterModeConfig += deviceInfo;
  filterModeConfig += "}";
  mqttClient.publish(filterModeConfigTopic.c_str(), filterModeConfig.c_str(), true);
  
  // 11. 内容过滤模式选择器
  String contentFilterConfigTopic = haPrefix + "/select/" + nodeId + "_content_filter_mode/config";
  String contentFilterConfig = "{";
  contentFilterConfig += "\"name\":\"内容过滤模式\",";
  contentFilterConfig += "\"unique_id\":\"" + nodeId + "_content_filter_mode\",";
  contentFilterConfig += "\"command_topic\":\"" + mqttTopicCmd + "\",";
  contentFilterConfig += "\"command_template\":\"{\\\"action\\\":\\\"set_content_filter_mode\\\",\\\"mode\\\":\\\"{{ value }}\\\"}\",";
  contentFilterConfig += "\"state_topic\":\"" + filterStatusTopic + "\",";
  contentFilterConfig += "\"value_template\":\"{{ value_json.content_filter_mode }}\",";
  contentFilterConfig += "\"options\":[\"disabled\",\"whitelist\",\"blacklist\"],";
  contentFilterConfig += "\"icon\":\"mdi:text-search\",";
  contentFilterConfig += deviceInfo;
  contentFilterConfig += "}";
  mqttClient.publish(contentFilterConfigTopic.c_str(), contentFilterConfig.c_str(), true);
  
  // 12. 定时过滤开关
  String schedFilterStatusTopic = config.mqttPrefix + "/" + mqttDeviceId + "/sched_filter/status";
  String schedFilterSwitchTopic = haPrefix + "/switch/" + nodeId + "_sched_filter/config";
  String schedFilterSwitch = "{";
  schedFilterSwitch += "\"name\":\"定时过滤\",";
  schedFilterSwitch += "\"unique_id\":\"" + nodeId + "_sched_filter\",";
  schedFilterSwitch += "\"command_topic\":\"" + mqttTopicCmd + "\",";
  schedFilterSwitch += "\"payload_on\":\"{\\\"action\\\":\\\"toggle_sched_filter\\\",\\\"enabled\\\":\\\"true\\\"}\",";
  schedFilterSwitch += "\"payload_off\":\"{\\\"action\\\":\\\"toggle_sched_filter\\\",\\\"enabled\\\":\\\"false\\\"}\",";
  schedFilterSwitch += "\"state_topic\":\"" + schedFilterStatusTopic + "\",";
  schedFilterSwitch += "\"value_template\":\"{{ 'ON' if value_json.enabled else 'OFF' }}\",";
  schedFilterSwitch += "\"icon\":\"mdi:clock-outline\",";
  schedFilterSwitch += deviceInfo;
  schedFilterSwitch += "}";
  mqttClient.publish(schedFilterSwitchTopic.c_str(), schedFilterSwitch.c_str(), true);
  
  // 13. 定时过滤时段A模式选择
  String schedModeAConfigTopic = haPrefix + "/select/" + nodeId + "_sched_mode_a/config";
  String schedModeAConfig = "{";
  schedModeAConfig += "\"name\":\"时段A模式(夜间)\",";
  schedModeAConfig += "\"unique_id\":\"" + nodeId + "_sched_mode_a\",";
  schedModeAConfig += "\"command_topic\":\"" + mqttTopicCmd + "\",";
  schedModeAConfig += "\"command_template\":\"{\\\"action\\\":\\\"set_sched_filter\\\",\\\"mode_a\\\":\\\"{% if value == 'disabled' %}0{% elif value == 'whitelist' %}1{% else %}2{% endif %}\\\"}\",";
  schedModeAConfig += "\"state_topic\":\"" + schedFilterStatusTopic + "\",";
  schedModeAConfig += "\"value_template\":\"{{ value_json.mode_a }}\",";
  schedModeAConfig += "\"options\":[\"disabled\",\"whitelist\",\"blacklist\"],";
  schedModeAConfig += "\"icon\":\"mdi:moon-waning-crescent\",";
  schedModeAConfig += deviceInfo;
  schedModeAConfig += "}";
  mqttClient.publish(schedModeAConfigTopic.c_str(), schedModeAConfig.c_str(), true);
  
  // 14. 定时过滤时段B模式选择
  String schedModeBConfigTopic = haPrefix + "/select/" + nodeId + "_sched_mode_b/config";
  String schedModeBConfig = "{";
  schedModeBConfig += "\"name\":\"时段B模式(白天)\",";
  schedModeBConfig += "\"unique_id\":\"" + nodeId + "_sched_mode_b\",";
  schedModeBConfig += "\"command_topic\":\"" + mqttTopicCmd + "\",";
  schedModeBConfig += "\"command_template\":\"{\\\"action\\\":\\\"set_sched_filter\\\",\\\"mode_b\\\":\\\"{% if value == 'disabled' %}0{% elif value == 'whitelist' %}1{% else %}2{% endif %}\\\"}\",";
  schedModeBConfig += "\"state_topic\":\"" + schedFilterStatusTopic + "\",";
  schedModeBConfig += "\"value_template\":\"{{ value_json.mode_b }}\",";
  schedModeBConfig += "\"options\":[\"disabled\",\"whitelist\",\"blacklist\"],";
  schedModeBConfig += "\"icon\":\"mdi:white-balance-sunny\",";
  schedModeBConfig += deviceInfo;
  schedModeBConfig += "}";
  mqttClient.publish(schedModeBConfigTopic.c_str(), schedModeBConfig.c_str(), true);
  
  Serial.println("HA自动发现配置已发布");
}

// MQTT 重连函数
void mqttReconnect() {
  if (!config.mqttEnabled) return;
  if (config.mqttServer.length() == 0) return;
  if (mqttClient.connected()) return;
  
  // 配置服务器（可能配置变更了）
  mqttClient.setServer(config.mqttServer.c_str(), config.mqttPort);
  
  String clientId = "sms_" + mqttDeviceId;
  Serial.println("连接MQTT服务器: " + config.mqttServer);
  Serial.println("客户端ID: " + clientId);
  
  bool connected = false;
  esp_task_wdt_reset(); // 连接前喂狗
  
  // 配置遗嘱消息（设备离线时自动发送）- 同时发送到两类主题
  String willMessage = "{\"status\":\"offline\",\"device\":\"" + mqttDeviceId + "\"}";
  
  if (config.mqttUser.length() > 0) {
    connected = mqttClient.connect(
      clientId.c_str(),
      config.mqttUser.c_str(),
      config.mqttPass.c_str(),
      mqttTopicStatus.c_str(),
      1,  // QoS
      true,  // retain
      willMessage.c_str()
    );
  } else {
    connected = mqttClient.connect(
      clientId.c_str(),
      mqttTopicStatus.c_str(),
      1,  // QoS
      true,  // retain
      willMessage.c_str()
    );
  }
  
  esp_task_wdt_reset(); // 无论连接成功与否都喂狗
  
  if (connected) {
    Serial.println("MQTT连接成功");
    
    // 订阅命令主题
    mqttClient.subscribe(mqttTopicSmsSend.c_str());
    mqttClient.subscribe(mqttTopicPing.c_str());
    mqttClient.subscribe(mqttTopicCmd.c_str());
    Serial.println("已订阅主题:");
    Serial.println("  - " + mqttTopicSmsSend);
    Serial.println("  - " + mqttTopicPing);
    Serial.println("  - " + mqttTopicCmd);
    
    // 发布 HA 自动发现配置
    publishHaDiscoveryConfig();
    
    // 发布上线状态
    publishMqttStatus("online");
    
    // 发布过滤配置状态（初始化 HA Select 实体）
    publishFilterStatus();
    
    // 发布定时过滤状态
    publishSchedFilterStatus();
  } else {
    Serial.print("MQTT连接失败, 错误码: ");
    Serial.println(mqttClient.state());
  }
}

// 提取 JSON 字符串值
String extractJsonString(const String& json, const String& key) {
  String searchKey = "\"" + key + "\"";
  int keyStart = json.indexOf(searchKey);
  if (keyStart < 0) return "";
  
  int colonIdx = json.indexOf(":", keyStart);
  if (colonIdx < 0) return "";
  
  int valStart = json.indexOf("\"", colonIdx + 1);
  if (valStart < 0) return "";
  
  int valEnd = json.indexOf("\"", valStart + 1);
  if (valEnd < 0) return "";
  
  return json.substring(valStart + 1, valEnd);
}

// 发布过滤状态
void publishFilterStatus() {
  if (!config.mqttEnabled || !mqttClient.connected()) return;
  
  String filterMode = config.filterEnabled ? 
    (config.filterIsWhitelist ? "whitelist" : "blacklist") : "disabled";
  String contentFilterMode = config.contentFilterEnabled ? 
    (config.contentFilterIsWhitelist ? "whitelist" : "blacklist") : "disabled";
  
  String json = "{";
  json += "\"filter_mode\":\"" + filterMode + "\",";
  json += "\"filter_enabled\":" + String(config.filterEnabled ? "true" : "false") + ",";
  json += "\"filter_list\":\"" + jsonEscape(config.filterList) + "\",";
  json += "\"content_filter_mode\":\"" + contentFilterMode + "\",";
  json += "\"content_filter_enabled\":" + String(config.contentFilterEnabled ? "true" : "false") + ",";
  json += "\"content_filter_list\":\"" + jsonEscape(config.contentFilterList) + "\"";
  json += "}";
  
  // 发布到 filter 状态主题
  String filterTopic = config.mqttPrefix + "/" + mqttDeviceId + "/filter/status";
  mqttClient.publish(filterTopic.c_str(), json.c_str(), true);
  
  Serial.println("已发布过滤状态");
}

// 发布定时过滤状态
void publishSchedFilterStatus() {
  if (!config.mqttEnabled || !mqttClient.connected()) return;
  
  String modeAStr = config.schedFilterModeA == 0 ? "disabled" : 
                    (config.schedFilterModeA == 1 ? "whitelist" : "blacklist");
  String modeBStr = config.schedFilterModeB == 0 ? "disabled" : 
                    (config.schedFilterModeB == 1 ? "whitelist" : "blacklist");
  
  // 构建时间字符串
  char startTimeStr[6], endTimeStr[6];
  sprintf(startTimeStr, "%02d:%02d", config.schedFilterStartHour, config.schedFilterStartMin);
  sprintf(endTimeStr, "%02d:%02d", config.schedFilterEndHour, config.schedFilterEndMin);
  
  String json = "{";
  json += "\"enabled\":" + String(config.schedFilterEnabled ? "true" : "false") + ",";
  json += "\"start_time\":\"" + String(startTimeStr) + "\",";
  json += "\"end_time\":\"" + String(endTimeStr) + "\",";
  json += "\"start_hour\":" + String(config.schedFilterStartHour) + ",";
  json += "\"start_min\":" + String(config.schedFilterStartMin) + ",";
  json += "\"end_hour\":" + String(config.schedFilterEndHour) + ",";
  json += "\"end_min\":" + String(config.schedFilterEndMin) + ",";
  json += "\"mode_a\":\"" + modeAStr + "\",";
  json += "\"mode_b\":\"" + modeBStr + "\",";
  json += "\"mode_a_num\":" + String(config.schedFilterModeA) + ",";
  json += "\"mode_b_num\":" + String(config.schedFilterModeB);
  json += "}";
  
  // 发布到定时过滤状态主题
  String schedTopic = config.mqttPrefix + "/" + mqttDeviceId + "/sched_filter/status";
  mqttClient.publish(schedTopic.c_str(), json.c_str(), true);
  
  Serial.println("已发布定时过滤状态");
}

// MQTT 消息回调处理
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // 转换 payload 为字符串
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("=== MQTT消息接收 ===");
  Serial.println("主题: " + String(topic));
  Serial.println("内容: " + message);
  Serial.println("====================");
  
  // 处理发送短信命令
  if (String(topic) == mqttTopicSmsSend) {
    int phoneStart = message.indexOf("\"phone\"");
    int msgStart = message.indexOf("\"message\"");
    
    if (phoneStart >= 0 && msgStart >= 0) {
      int phoneValStart = message.indexOf(":", phoneStart) + 1;
      int phoneValEnd = message.indexOf(",", phoneValStart);
      if (phoneValEnd < 0) phoneValEnd = message.indexOf("}", phoneValStart);
      String phoneRaw = message.substring(phoneValStart, phoneValEnd);
      phoneRaw.trim();
      if (phoneRaw.startsWith("\"")) phoneRaw = phoneRaw.substring(1);
      if (phoneRaw.endsWith("\"")) phoneRaw = phoneRaw.substring(0, phoneRaw.length() - 1);
      
      int msgValStart = message.indexOf(":", msgStart) + 1;
      int msgValEnd = message.lastIndexOf("\"");
      String msgRaw = message.substring(msgValStart, msgValEnd + 1);
      msgRaw.trim();
      if (msgRaw.startsWith("\"")) msgRaw = msgRaw.substring(1);
      if (msgRaw.endsWith("\"")) msgRaw = msgRaw.substring(0, msgRaw.length() - 1);
      
      Serial.println("MQTT发送短信命令:");
      Serial.println("  目标: " + phoneRaw);
      Serial.println("  内容: " + msgRaw);
      
      bool success = sendSMS(phoneRaw.c_str(), msgRaw.c_str());
      if (success) {
        stats.smsSent++;
        saveStats();
      }
      publishMqttSmsSent(phoneRaw.c_str(), msgRaw.c_str(), success);
    } else {
      Serial.println("短信命令格式错误");
      publishMqttSmsSent("", "", false);
    }
  }
  // 处理 Ping 命令
  else if (String(topic) == mqttTopicPing) {
    String host = "8.8.8.8";  // 默认目标
    
    int hostStart = message.indexOf("\"host\"");
    if (hostStart >= 0) {
      int hostValStart = message.indexOf(":", hostStart) + 1;
      int hostValEnd = message.indexOf("\"", hostValStart + 2);
      if (hostValEnd > hostValStart) {
        String hostRaw = message.substring(hostValStart, hostValEnd + 1);
        hostRaw.trim();
        if (hostRaw.startsWith("\"")) hostRaw = hostRaw.substring(1);
        if (hostRaw.endsWith("\"")) hostRaw = hostRaw.substring(0, hostRaw.length() - 1);
        if (hostRaw.length() > 0) host = hostRaw;
      }
    }
    
    Serial.println("MQTT Ping命令: " + host);
    
    String activateResp = sendATCommand("AT+CGACT=1,1", 10000);
    delay(500);
    
    String pingCmd = "AT+MPING=\"" + host + "\",30,1";
    while (Serial1.available()) Serial1.read();
    Serial1.println(pingCmd);
    
    unsigned long start = millis();
    String resp = "";
    bool gotResult = false;
    String resultMsg = "";
    bool pingSuccess = false;
    
    while (millis() - start < 35000) {
      while (Serial1.available()) {
        char c = Serial1.read();
        resp += c;
        
        int mpingIdx = resp.indexOf("+MPING:");
        if (mpingIdx >= 0) {
          int lineEnd = resp.indexOf('\n', mpingIdx);
          if (lineEnd >= 0) {
            String mpingLine = resp.substring(mpingIdx, lineEnd);
            mpingLine.trim();
            
            int colonIdx = mpingLine.indexOf(':');
            if (colonIdx >= 0) {
              String params = mpingLine.substring(colonIdx + 1);
              params.trim();
              
              int commaIdx = params.indexOf(',');
              int result = params.substring(0, commaIdx > 0 ? commaIdx : params.length()).toInt();
              
              gotResult = true;
              pingSuccess = (result == 0 || result == 1) || (params.indexOf(',') >= 0 && params.length() > 5);
              
              if (pingSuccess && commaIdx > 0) {
                resultMsg = params;
              } else {
                resultMsg = "错误码: " + String(result);
              }
            }
            break;
          }
        }
        
        if (resp.indexOf("ERROR") >= 0) {
          gotResult = true;
          pingSuccess = false;
          resultMsg = "模组错误";
          break;
        }
      }
      if (gotResult) break;
      delay(10);
      esp_task_wdt_reset();  // 喂狗，等待 ping 可能需要 35 秒
    }
    
    sendATCommand("AT+CGACT=0,1", 5000);
    
    if (!gotResult) {
      resultMsg = "超时";
    }
    
    publishMqttPingResult(host.c_str(), pingSuccess, resultMsg.c_str());
  }
  // 处理控制命令
  else if (String(topic) == mqttTopicCmd) {
    int actionStart = message.indexOf("\"action\"");
    if (actionStart >= 0) {
      int actionValStart = message.indexOf(":", actionStart) + 1;
      int actionValEnd = message.indexOf("\"", actionValStart + 2);
      String actionRaw = message.substring(actionValStart, actionValEnd + 1);
      actionRaw.trim();
      if (actionRaw.startsWith("\"")) actionRaw = actionRaw.substring(1);
      if (actionRaw.endsWith("\"")) actionRaw = actionRaw.substring(0, actionRaw.length() - 1);
      
      Serial.println("MQTT控制命令: " + actionRaw);
      
      if (actionRaw == "restart" || actionRaw == "reset") {
        Serial.println("执行重启命令...");
        publishMqttStatus("restarting");
        delay(500);
        ESP.restart();
      }
      else if (actionRaw == "status") {
        String statusJson = "{";
        statusJson += "\"status\":\"online\",";
        statusJson += "\"device\":\"" + mqttDeviceId + "\",";
        statusJson += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
        statusJson += "\"wifi_rssi\":" + String(WiFi.RSSI()) + ",";
        statusJson += "\"uptime\":" + String(millis() / 1000) + ",";
        statusJson += "\"free_heap\":" + String(ESP.getFreeHeap());
        statusJson += "}";
        mqttClient.publish(mqttTopicStatus.c_str(), statusJson.c_str(), true);
        // 同步到 HA 主题
        if (config.mqttHaDiscovery) {
          mqttClient.publish(mqttHaStatusTopic.c_str(), statusJson.c_str(), true);
        }
        Serial.println("已发送状态信息");
      }
      // 设置号码过滤模式
      else if (actionRaw == "set_filter_mode") {
        String mode = extractJsonString(message, "mode");
        Serial.println("设置号码过滤模式: " + mode);
        
        if (mode == "whitelist") {
          config.filterEnabled = true;
          config.filterIsWhitelist = true;
          Serial.println("已切换到白名单模式");
        } else if (mode == "blacklist") {
          config.filterEnabled = true;
          config.filterIsWhitelist = false;
          Serial.println("已切换到黑名单模式");
        } else if (mode == "disabled" || mode == "disable") {
          config.filterEnabled = false;
          Serial.println("已禁用号码过滤");
        } else {
          Serial.println("无效的模式: " + mode);
        }
        saveConfig();
        publishFilterStatus();
      }
      // 设置内容过滤模式
      else if (actionRaw == "set_content_filter_mode") {
        String mode = extractJsonString(message, "mode");
        Serial.println("设置内容过滤模式: " + mode);
        
        if (mode == "whitelist") {
          config.contentFilterEnabled = true;
          config.contentFilterIsWhitelist = true;
          Serial.println("已切换到内容白名单模式");
        } else if (mode == "blacklist") {
          config.contentFilterEnabled = true;
          config.contentFilterIsWhitelist = false;
          Serial.println("已切换到内容黑名单模式");
        } else if (mode == "disabled" || mode == "disable") {
          config.contentFilterEnabled = false;
          Serial.println("已禁用内容过滤");
        } else {
          Serial.println("无效的模式: " + mode);
        }
        saveConfig();
        publishFilterStatus();
      }
      // 获取过滤状态
      else if (actionRaw == "get_filter_status") {
        Serial.println("获取过滤状态...");
        publishFilterStatus();
      }
      // 设置定时过滤
      else if (actionRaw == "set_sched_filter") {
        String enabled = extractJsonString(message, "enabled");
        String startH = extractJsonString(message, "start_hour");
        String startM = extractJsonString(message, "start_min");
        String endH = extractJsonString(message, "end_hour");
        String endM = extractJsonString(message, "end_min");
        String modeA = extractJsonString(message, "mode_a");
        String modeB = extractJsonString(message, "mode_b");
        
        if (enabled.length() > 0) {
          config.schedFilterEnabled = (enabled == "true" || enabled == "1");
        }
        if (startH.length() > 0) config.schedFilterStartHour = startH.toInt();
        if (startM.length() > 0) config.schedFilterStartMin = startM.toInt();
        if (endH.length() > 0) config.schedFilterEndHour = endH.toInt();
        if (endM.length() > 0) config.schedFilterEndMin = endM.toInt();
        if (modeA.length() > 0) config.schedFilterModeA = modeA.toInt();
        if (modeB.length() > 0) config.schedFilterModeB = modeB.toInt();
        
        saveConfig();
        Serial.printf("定时过滤已配置: %s, %02d:%02d-%02d:%02d, A=%d, B=%d\n",
          config.schedFilterEnabled ? "启用" : "禁用",
          config.schedFilterStartHour, config.schedFilterStartMin,
          config.schedFilterEndHour, config.schedFilterEndMin,
          config.schedFilterModeA, config.schedFilterModeB);
        
        publishSchedFilterStatus();
      }
      // 获取定时过滤状态
      else if (actionRaw == "get_sched_filter") {
        Serial.println("获取定时过滤状态...");
        publishSchedFilterStatus();
      }
      // 启用/禁用定时过滤
      else if (actionRaw == "toggle_sched_filter") {
        String enabled = extractJsonString(message, "enabled");
        config.schedFilterEnabled = (enabled == "true" || enabled == "1" || enabled == "on");
        saveConfig();
        Serial.println(config.schedFilterEnabled ? "定时过滤已启用" : "定时过滤已禁用");
        publishSchedFilterStatus();
      }
      else {
        Serial.println("未知命令: " + actionRaw);
      }
    }
  }
}

// 发布收到短信通知（双主题）
void publishMqttSmsReceived(const char* sender, const char* message, const char* timestamp) {
  if (!config.mqttEnabled) {
    return;
  }
  
  if (!mqttClient.connected()) {
    Serial.println("MQTT未连接，跳过短信推送");
    return;
  }
  
  // 仅控制模式下不推送短信内容
  if (config.mqttControlOnly) {
    Serial.println("MQTT仅控制模式，跳过短信推送");
    return;
  }
  
  Serial.println("MQTT推送短信...");
  
  String json = "{";
  json += "\"sender\":\"" + jsonEscape(String(sender)) + "\",";
  json += "\"message\":\"" + jsonEscape(String(message)) + "\",";
  json += "\"timestamp\":\"" + jsonEscape(String(timestamp)) + "\",";
  json += "\"device\":\"" + mqttDeviceId + "\"";
  json += "}";
  
  // 发布到用户自定义主题
  Serial.println(" 主题1: " + mqttTopicSmsReceived);
  bool success1 = mqttClient.publish(mqttTopicSmsReceived.c_str(), json.c_str());
  
  // 发布到 HA 事件主题（如果启用）
  if (config.mqttHaDiscovery) {
    Serial.println(" 主题2 (HA): " + mqttHaSmsReceivedTopic);
    mqttClient.publish(mqttHaSmsReceivedTopic.c_str(), json.c_str());
  }
  
  if (success1) {
    Serial.println("MQTT短信推送完成");
  } else {
    Serial.println("MQTT短信推送失败");
  }
}

// 发布发送短信结果
void publishMqttSmsSent(const char* phone, const char* message, bool success) {
  if (!config.mqttEnabled || !mqttClient.connected()) return;
  
  String json = "{";
  json += "\"success\":" + String(success ? "true" : "false") + ",";
  json += "\"phone\":\"" + jsonEscape(String(phone)) + "\",";
  json += "\"message\":\"" + jsonEscape(String(message)) + "\",";
  json += "\"device\":\"" + mqttDeviceId + "\"";
  json += "}";
  
  mqttClient.publish(mqttTopicSmsSent.c_str(), json.c_str());
  Serial.println("MQTT发布发送短信结果: " + String(success ? "成功" : "失败"));
}

// 发布 Ping 测试结果
void publishMqttPingResult(const char* host, bool success, const char* result) {
  if (!config.mqttEnabled || !mqttClient.connected()) return;
  
  String json = "{";
  json += "\"success\":" + String(success ? "true" : "false") + ",";
  json += "\"host\":\"" + String(host) + "\",";
  json += "\"result\":\"" + jsonEscape(String(result)) + "\",";
  json += "\"device\":\"" + mqttDeviceId + "\"";
  json += "}";
  
  mqttClient.publish(mqttTopicPingResult.c_str(), json.c_str());
  Serial.println("MQTT发布Ping结果: " + String(success ? "成功" : "失败"));
}

// 发布设备状态（双主题）
void publishMqttStatus(const char* status) {
  if (!config.mqttEnabled) return;
  if (!mqttClient.connected() && String(status) != "online") return;
  
  String json = "{";
  json += "\"status\":\"" + String(status) + "\",";
  json += "\"device\":\"" + mqttDeviceId + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += "}";
  
  // 发布到用户自定义主题
  mqttClient.publish(mqttTopicStatus.c_str(), json.c_str(), true);
  
  // 发布到 HA 状态主题（如果启用）
  if (config.mqttHaDiscovery) {
    mqttClient.publish(mqttHaStatusTopic.c_str(), json.c_str(), true);
  }
  
  Serial.println("MQTT发布状态: " + String(status));
}

// 定期发布设备详细状态（双主题，用于 Home Assistant 等平台）
void publishMqttDeviceStatus() {
  if (!config.mqttEnabled || !mqttClient.connected()) return;
  
  // 获取信号质量
  String cesqResp = sendATCommand("AT+CESQ", 2000);
  int rxlev = -1, rsrp = -1, rsrq = -1;
  int cesqIdx = cesqResp.indexOf("+CESQ:");
  if (cesqIdx >= 0) {
    String params = cesqResp.substring(cesqIdx + 6);
    params.trim();
    // 格式: rxlev,ber,rscp,ecno,rsrq,rsrp
    int vals[6] = {0};
    int vi = 0;
    int start = 0;
    for (int i = 0; i <= params.length() && vi < 6; i++) {
      if (i == params.length() || params[i] == ',') {
        vals[vi++] = params.substring(start, i).toInt();
        start = i + 1;
      }
    }
    rxlev = vals[0];  // 0-63, 99=unknown
    rsrq = vals[4];   // 0-34
    rsrp = vals[5];   // 0-97
  }
  
  // 计算 dBm
  int rsrpDbm = (rsrp != 255 && rsrp <= 97) ? (rsrp - 141) : -999;
  int rsrqDb = (rsrq != 255 && rsrq <= 34) ? ((rsrq / 2) - 20) : -999;
  
  // WiFi 信号评价
  int wifiRssi = WiFi.RSSI();
  String wifiStatus = "未知";
  if (wifiRssi >= -50) wifiStatus = "极好";
  else if (wifiRssi >= -60) wifiStatus = "很好";
  else if (wifiRssi >= -70) wifiStatus = "良好";
  else if (wifiRssi >= -80) wifiStatus = "一般";
  else if (wifiRssi >= -90) wifiStatus = "较弱";
  else wifiStatus = "很差";
  
  // 4G 信号评价
  String lteStatus = "未知";
  if (rsrpDbm != -999) {
    if (rsrpDbm >= -80) lteStatus = "极好";
    else if (rsrpDbm >= -90) lteStatus = "良好";
    else if (rsrpDbm >= -100) lteStatus = "一般";
    else if (rsrpDbm >= -110) lteStatus = "较弱";
    else lteStatus = "很差";
  }
  
  // 获取 APN
  String apn = "";
  String cgdcontResp = sendATCommand("AT+CGDCONT?", 2000);
  int cgdIdx = cgdcontResp.indexOf("+CGDCONT:");
  if (cgdIdx >= 0) {
    int idx0 = cgdcontResp.indexOf(",\"", cgdIdx);
    if (idx0 >= 0) {
      idx0 = cgdcontResp.indexOf(",\"", idx0 + 2);
      if (idx0 >= 0) {
        int endIdx0 = cgdcontResp.indexOf("\"", idx0 + 2);
        if (endIdx0 > idx0) {
          apn = cgdcontResp.substring(idx0 + 2, endIdx0);
        }
      }
    }
  }
  
  // 构建 JSON
  String json = "{";
  json += "\"status\":\"online\",";
  json += "\"device\":\"" + mqttDeviceId + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"wifi_rssi\":" + String(wifiRssi) + ",";
  json += "\"wifi_status\":\"" + wifiStatus + "\",";
  json += "\"wifi_ssid\":\"" + WiFi.SSID() + "\",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"lte_rsrp\":" + String(rsrpDbm) + ",";
  json += "\"lte_rsrq\":" + String(rsrqDb) + ",";
  json += "\"lte_status\":\"" + lteStatus + "\",";
  json += "\"apn\":\"" + apn + "\"";
  json += "}";
  
  // 发布到用户自定义主题
  mqttClient.publish(mqttTopicStatus.c_str(), json.c_str(), true);
  
  // 发布到 HA 状态主题（如果启用）
  if (config.mqttHaDiscovery) {
    mqttClient.publish(mqttHaStatusTopic.c_str(), json.c_str(), true);
  }
  
  Serial.println("MQTT上报设备状态");
}
