/*
 * mqtt_handler.ino - MQTT 功能实现
 * 
 * MQTT 现在通过 Web 界面配置，不再使用 mqtt_config.h
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
  
  // 发布主题
  mqttTopicStatus = prefix + "/status";
  mqttTopicSmsReceived = prefix + "/sms/received";
  mqttTopicSmsSent = prefix + "/sms/sent";
  mqttTopicPingResult = prefix + "/ping/result";
  
  // 订阅主题
  mqttTopicSmsSend = prefix + "/sms/send";
  mqttTopicPing = prefix + "/ping";
  mqttTopicCmd = prefix + "/cmd";
  
  Serial.println("MQTT设备ID: " + mqttDeviceId);
  Serial.println("MQTT主题前缀: " + prefix);
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
  
  // 配置遗嘱消息（设备离线时自动发送）
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
    
    // 发布上线状态
    publishMqttStatus("online");
  } else {
    Serial.print("MQTT连接失败, 错误码: ");
    Serial.println(mqttClient.state());
  }
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
        Serial.println("已发送状态信息");
      }
      else {
        Serial.println("未知命令: " + actionRaw);
      }
    }
  }
}

// 发布收到短信通知
void publishMqttSmsReceived(const char* sender, const char* message, const char* timestamp) {
  if (!config.mqttEnabled || !mqttClient.connected()) return;
  
  // 仅控制模式下不推送短信内容
  if (config.mqttControlOnly) {
    Serial.println("MQTT仅控制模式，跳过短信推送");
    return;
  }
  
  String json = "{";
  json += "\"sender\":\"" + jsonEscape(String(sender)) + "\",";
  json += "\"message\":\"" + jsonEscape(String(message)) + "\",";
  json += "\"timestamp\":\"" + jsonEscape(String(timestamp)) + "\",";
  json += "\"device\":\"" + mqttDeviceId + "\"";
  json += "}";
  
  mqttClient.publish(mqttTopicSmsReceived.c_str(), json.c_str());
  Serial.println("MQTT发布收到短信通知");
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

// 发布设备状态
void publishMqttStatus(const char* status) {
  if (!config.mqttEnabled) return;
  if (!mqttClient.connected() && String(status) != "online") return;
  
  String json = "{";
  json += "\"status\":\"" + String(status) + "\",";
  json += "\"device\":\"" + mqttDeviceId + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += "}";
  
  mqttClient.publish(mqttTopicStatus.c_str(), json.c_str(), true);
  Serial.println("MQTT发布状态: " + String(status));
}

// 定期发布设备详细状态（用于 Home Assistant 等平台）
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
  
  // 获取网络注册状态
  String cregResp = sendATCommand("AT+CREG?", 2000);
  int regStatus = 0;
  int cregIdx = cregResp.indexOf("+CREG:");
  if (cregIdx >= 0) {
    int comma = cregResp.indexOf(',', cregIdx);
    if (comma > 0) {
      regStatus = cregResp.substring(comma + 1, comma + 2).toInt();
    }
  }
  
  String regStatusStr = "unknown";
  switch(regStatus) {
    case 1: regStatusStr = "registered_home"; break;
    case 5: regStatusStr = "registered_roaming"; break;
    case 2: regStatusStr = "searching"; break;
    case 3: regStatusStr = "denied"; break;
    case 0: regStatusStr = "not_registered"; break;
  }
  
  // 构建 JSON
  String json = "{";
  json += "\"status\":\"online\",";
  json += "\"device\":\"" + mqttDeviceId + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"wifi_rssi\":" + String(WiFi.RSSI()) + ",";
  json += "\"wifi_ssid\":\"" + WiFi.SSID() + "\",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"lte_rsrp\":" + String(rsrpDbm) + ",";
  json += "\"lte_rsrq\":" + String(rsrqDb) + ",";
  json += "\"network_status\":\"" + regStatusStr + "\"";
  json += "}";
  
  mqttClient.publish(mqttTopicStatus.c_str(), json.c_str(), true);
  Serial.println("MQTT上报设备状态");
}
