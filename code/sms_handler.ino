/*
 * sms_handler.ino - 短信处理函数实现
 */

// 清理短信内容（去除自动填充标识符）
String cleanSmsContent(const String& text) {
  String cleaned = text;
  
  // 去除开头的 <#> 标记
  if (cleaned.startsWith("<#>")) {
    cleaned = cleaned.substring(3);
    cleaned.trim();
  }
  
  // 去除末尾的 hash 标识（以 / 开头的随机字符串）
  int slashIdx = cleaned.lastIndexOf('/');
  if (slashIdx > 0) {
    String suffix = cleaned.substring(slashIdx + 1);
    suffix.trim();
    // 检查是否是自动填充标识（全是字母数字，长度在6-15之间）
    bool isAutoFillCode = suffix.length() >= 6 && suffix.length() <= 15;
    for (unsigned int i = 0; i < suffix.length() && isAutoFillCode; i++) {
      char c = suffix.charAt(i);
      if (!isalnum(c)) isAutoFillCode = false;
    }
    if (isAutoFillCode) {
      cleaned = cleaned.substring(0, slashIdx);
      cleaned.trim();
    }
  }
  
  // 去除末尾单独的随机字符串（换行后的字母数字串）
  int lastNewline = cleaned.lastIndexOf('\n');
  if (lastNewline > 0) {
    String lastLine = cleaned.substring(lastNewline + 1);
    lastLine.trim();
    // 检查是否是自动填充标识
    bool isAutoFillCode = lastLine.length() >= 6 && lastLine.length() <= 15;
    for (unsigned int i = 0; i < lastLine.length() && isAutoFillCode; i++) {
      char c = lastLine.charAt(i);
      if (!isalnum(c)) isAutoFillCode = false;
    }
    if (isAutoFillCode) {
      cleaned = cleaned.substring(0, lastNewline);
      cleaned.trim();
    }
  }
  
  return cleaned;
}

// 格式化时间戳（从 PDU 格式转为可读格式）
// 输入: 25121615142600 (YYMMDDHHMMSSZZ)
// 输出: 2025-12-16 15:14:26
String formatTimestamp(const String& pduTimestamp) {
  if (pduTimestamp.length() < 12) return pduTimestamp;
  
  String year = "20" + pduTimestamp.substring(0, 2);
  String month = pduTimestamp.substring(2, 4);
  String day = pduTimestamp.substring(4, 6);
  String hour = pduTimestamp.substring(6, 8);
  String minute = pduTimestamp.substring(8, 10);
  String second = pduTimestamp.substring(10, 12);
  
  return year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second;
}

// 提取验证码（用于邮件主题）
String extractVerifyCode(const String& text) {
  // 查找4-8位连续数字
  String code = "";
  for (unsigned int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);
    if (c >= '0' && c <= '9') {
      code += c;
    } else if (code.length() >= 4 && code.length() <= 8) {
      break;  // 找到了有效验证码
    } else {
      code = "";  // 重新开始
    }
  }
  // 最后检查
  if (code.length() >= 4 && code.length() <= 8) {
    return code;
  }
  return "";
}

// 初始化长短信缓存
void initConcatBuffer() {
  for (int i = 0; i < MAX_CONCAT_MESSAGES; i++) {
    concatBuffer[i].inUse = false;
    concatBuffer[i].receivedParts = 0;
    for (int j = 0; j < MAX_CONCAT_PARTS; j++) {
      concatBuffer[i].parts[j].valid = false;
      concatBuffer[i].parts[j].text = "";
    }
  }
}

// 查找或创建长短信缓存槽位
int findOrCreateConcatSlot(int refNumber, const char* sender, int totalParts) {
  // 先查找是否已存在
  for (int i = 0; i < MAX_CONCAT_MESSAGES; i++) {
    if (concatBuffer[i].inUse && 
        concatBuffer[i].refNumber == refNumber &&
        concatBuffer[i].sender.equals(sender)) {
      return i;
    }
  }
  
  // 查找空闲槽位
  for (int i = 0; i < MAX_CONCAT_MESSAGES; i++) {
    if (!concatBuffer[i].inUse) {
      concatBuffer[i].inUse = true;
      concatBuffer[i].refNumber = refNumber;
      concatBuffer[i].sender = String(sender);
      concatBuffer[i].totalParts = totalParts;
      concatBuffer[i].receivedParts = 0;
      concatBuffer[i].firstPartTime = millis();
      for (int j = 0; j < MAX_CONCAT_PARTS; j++) {
        concatBuffer[i].parts[j].valid = false;
        concatBuffer[i].parts[j].text = "";
      }
      return i;
    }
  }
  
  // 没有空闲槽位，查找最老的槽位覆盖
  int oldestSlot = 0;
  unsigned long oldestTime = concatBuffer[0].firstPartTime;
  for (int i = 1; i < MAX_CONCAT_MESSAGES; i++) {
    if (concatBuffer[i].firstPartTime < oldestTime) {
      oldestTime = concatBuffer[i].firstPartTime;
      oldestSlot = i;
    }
  }
  
  // 覆盖最老的槽位
  Serial.println("长短信缓存已满，覆盖最老的槽位");
  concatBuffer[oldestSlot].inUse = true;
  concatBuffer[oldestSlot].refNumber = refNumber;
  concatBuffer[oldestSlot].sender = String(sender);
  concatBuffer[oldestSlot].totalParts = totalParts;
  concatBuffer[oldestSlot].receivedParts = 0;
  concatBuffer[oldestSlot].firstPartTime = millis();
  for (int j = 0; j < MAX_CONCAT_PARTS; j++) {
    concatBuffer[oldestSlot].parts[j].valid = false;
    concatBuffer[oldestSlot].parts[j].text = "";
  }
  return oldestSlot;
}

// 合并长短信各分段
String assembleConcatSms(int slot) {
  String result = "";
  for (int i = 0; i < concatBuffer[slot].totalParts; i++) {
    if (concatBuffer[slot].parts[i].valid) {
      result += concatBuffer[slot].parts[i].text;
    } else {
      result += "[缺失分段" + String(i + 1) + "]";
    }
  }
  return result;
}

// 清空长短信槽位
void clearConcatSlot(int slot) {
  concatBuffer[slot].inUse = false;
  concatBuffer[slot].receivedParts = 0;
  concatBuffer[slot].sender = "";
  concatBuffer[slot].timestamp = "";
  for (int j = 0; j < MAX_CONCAT_PARTS; j++) {
    concatBuffer[slot].parts[j].valid = false;
    concatBuffer[slot].parts[j].text = "";
  }
}

// 检查长短信超时并转发
void checkConcatTimeout() {
  unsigned long now = millis();
  for (int i = 0; i < MAX_CONCAT_MESSAGES; i++) {
    if (concatBuffer[i].inUse) {
      if (now - concatBuffer[i].firstPartTime >= CONCAT_TIMEOUT_MS) {
        Serial.println("长短信超时，强制转发不完整消息");
        Serial.printf("  参考号: %d, 已收到: %d/%d\n", 
                      concatBuffer[i].refNumber,
                      concatBuffer[i].receivedParts,
                      concatBuffer[i].totalParts);
        
        // 合并已收到的分段
        String fullText = assembleConcatSms(i);
        
        // 处理短信内容
        processSmsContent(concatBuffer[i].sender.c_str(), 
                         fullText.c_str(), 
                         concatBuffer[i].timestamp.c_str());
        
        // 清空槽位
        clearConcatSlot(i);
      }
    }
  }
}

// 发送短信（PDU 模式）
bool sendSMS(const char* phoneNumber, const char* message) {
  Serial.println("准备发送短信...");
  Serial.print("目标号码: "); Serial.println(phoneNumber);
  Serial.print("短信内容: "); Serial.println(message);

  // 使用 pdulib 编码 PDU
  pdu.setSCAnumber();  // 使用默认短信中心
  int pduLen = pdu.encodePDU(phoneNumber, message);
  
  if (pduLen < 0) {
    Serial.print("PDU编码失败，错误码: ");
    Serial.println(pduLen);
    return false;
  }
  
  Serial.print("PDU数据: "); Serial.println(pdu.getSMS());
  Serial.print("PDU长度: "); Serial.println(pduLen);
  
  // 发送 AT+CMGS 命令
  String cmgsCmd = "AT+CMGS=";
  cmgsCmd += pduLen;
  
  while (Serial1.available()) Serial1.read();
  Serial1.println(cmgsCmd);
  
  // 等待 > 提示符
  unsigned long start = millis();
  bool gotPrompt = false;
  while (millis() - start < 5000) {
    if (Serial1.available()) {
      char c = Serial1.read();
      Serial.print(c);
      if (c == '>') {
        gotPrompt = true;
        break;
      }
    }
  }
  
  if (!gotPrompt) {
    Serial.println("未收到>提示符");
    return false;
  }
  
  // 发送 PDU 数据
  Serial1.print(pdu.getSMS());
  Serial1.write(0x1A);  // Ctrl+Z 结束
  
  // 等待响应
  start = millis();
  String resp = "";
  while (millis() - start < 30000) {
    while (Serial1.available()) {
      char c = Serial1.read();
      resp += c;
      Serial.print(c);
      if (resp.indexOf("OK") >= 0) {
        Serial.println("\n短信发送成功");
        return true;
      }
      if (resp.indexOf("ERROR") >= 0) {
        Serial.println("\n短信发送失败");
        return false;
      }
    }
  }
  Serial.println("短信发送超时");
  return false;
}

// 处理最终的短信内容（管理员命令检查和转发）
void processSmsContent(const char* sender, const char* text, const char* timestamp) {
  // 清理短信内容和格式化时间
  String cleanedText = cleanSmsContent(String(text));
  String formattedTime = formatTimestamp(String(timestamp));
  String verifyCode = extractVerifyCode(cleanedText);
  
  Serial.println("=== 处理短信 ===");
  Serial.printf("发送者: %s\n", sender);
  Serial.printf("时间: %s\n", formattedTime.c_str());
  
  // 添加到短信历史
  addSmsToHistory(sender, cleanedText.c_str(), formattedTime.c_str());
  
  // 检查黑白名单
  if (isNumberFiltered(sender)) {
    Serial.println("号码被过滤，跳过推送");
    return;
  }
  
  if (verifyCode.length() > 0) {
    Serial.printf("验证码: %s\n", verifyCode.c_str());
  }

  // 发送通知 http
  sendSMSToServer(sender, cleanedText.c_str(), formattedTime.c_str());
  
  // 发送 MQTT 通知
  publishMqttSmsReceived(sender, cleanedText.c_str(), formattedTime.c_str());
  
  // 发送通知邮件
  String subject;
  if (verifyCode.length() > 0) {
    subject = "[" + verifyCode + "] " + String(sender);
  } else {
    String preview = cleanedText.substring(0, 30);
    if (cleanedText.length() > 30) preview += "...";
    subject = String(sender) + ": " + preview;
  }
  
  String body = "发送者: " + String(sender) + "\n";
  body += "时间: " + formattedTime + "\n";
  body += "内容:\n" + cleanedText;
  
  sendEmailNotification(subject.c_str(), body.c_str());
}

// 处理来电通知
void processIncomingCall(const char* callerNumber) {
  Serial.println("=== 来电通知 ===");
  Serial.printf("来电号码: %s\n", callerNumber);
  
  // 获取当前时间（简单格式）
  unsigned long uptimeSec = millis() / 1000;
  String timestamp = "运行 " + String(uptimeSec / 3600) + ":" + 
                     String((uptimeSec % 3600) / 60) + ":" + 
                     String(uptimeSec % 60);
  
  // 构建通知内容
  String message = "📞 来电通知\n来电号码: " + String(callerNumber);
  
  // HTTP 推送
  if (WiFi.status() == WL_CONNECTED) {
    for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
      if (isPushChannelValid(config.pushChannels[i])) {
        sendToChannel(config.pushChannels[i], callerNumber, "📞 来电中...", timestamp.c_str());
      }
    }
  }
  
  // MQTT 推送
  if (config.mqttEnabled && mqttClient.connected() && !config.mqttControlOnly) {
    String json = "{";
    json += "\"type\":\"incoming_call\",";
    json += "\"caller\":\"" + String(callerNumber) + "\",";
    json += "\"timestamp\":\"" + timestamp + "\"";
    json += "}";
    mqttClient.publish((config.mqttPrefix + "/call").c_str(), json.c_str());
    Serial.println("MQTT来电通知已发送");
  }
  
  // 邮件通知
  if (config.emailEnabled) {
    String subject = "📞 来电: " + String(callerNumber);
    String body = "来电号码: " + String(callerNumber) + "\n";
    body += "时间: " + timestamp;
    sendEmailNotification(subject.c_str(), body.c_str());
  }
  
  Serial.println("来电通知已发送");
}

// 读取串口一行（含回车换行），返回行字符串，无新行时返回空
String readSerialLine(HardwareSerial& port) {
  static char lineBuf[SERIAL_BUFFER_SIZE];
  static int linePos = 0;

  while (port.available()) {
    char c = port.read();
    if (c == '\n') {
      lineBuf[linePos] = 0;
      String res = String(lineBuf);
      linePos = 0;
      return res;
    } else if (c != '\r') {  // 跳过 \r
      if (linePos < SERIAL_BUFFER_SIZE - 1)
        lineBuf[linePos++] = c;
      else
        linePos = 0;  // 超长报错保护，重头计
    }
  }
  return "";
}

// 检查字符串是否为有效的十六进制 PDU 数据
bool isHexString(const String& str) {
  if (str.length() == 0) return false;
  for (unsigned int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
      return false;
    }
  }
  return true;
}

// 处理 URC 和 PDU
void checkSerial1URC() {
  static enum { IDLE,
                WAIT_PDU } state = IDLE;

  String line = readSerialLine(Serial1);
  if (line.length() == 0) return;

  // 打印到调试串口
  Serial.println("Debug> " + line);

  if (state == IDLE) {
    // 检测来电 RING
    if (line.startsWith("RING") || line.indexOf("RING") >= 0) {
      Serial.println("检测到来电振铃...");
      // 继续等待 +CLIP 获取号码
    }
    
    // 检测来电号码显示 +CLIP
    if (line.startsWith("+CLIP:")) {
      Serial.println("检测到来电: " + line);
      // 解析来电号码 +CLIP: "号码",类型
      int quoteStart = line.indexOf('"');
      int quoteEnd = line.indexOf('"', quoteStart + 1);
      if (quoteStart >= 0 && quoteEnd > quoteStart) {
        String callerNumber = line.substring(quoteStart + 1, quoteEnd);
        Serial.println("来电号码: " + callerNumber);
        
        // 发送来电通知
        processIncomingCall(callerNumber.c_str());
      }
    }
    
    // 检测到短信上报 URC 头
    if (line.startsWith("+CMT:")) {
      Serial.println("检测到+CMT，等待PDU数据...");
      state = WAIT_PDU;
    }
  } else if (state == WAIT_PDU) {
    // 跳过空行
    if (line.length() == 0) {
      return;
    }
    
    // 如果是十六进制字符串，认为是 PDU 数据
    if (isHexString(line)) {
      Serial.println("收到PDU数据: " + line);
      Serial.println("PDU长度: " + String(line.length()) + " 字符");
      
      // 解析 PDU
      if (!pdu.decodePDU(line.c_str())) {
        Serial.println("PDU解析失败！");
      } else {
        Serial.println("PDU解析成功");
        Serial.println("=== 短信内容 ===");
        Serial.println("发送者: " + String(pdu.getSender()));
        Serial.println("时间戳: " + String(pdu.getTimeStamp()));
        Serial.println("内容: " + String(pdu.getText()));
        
        // 获取长短信信息
        int* concatInfo = pdu.getConcatInfo();
        int refNumber = concatInfo[0];
        int partNumber = concatInfo[1];
        int totalParts = concatInfo[2];
        
        Serial.printf("长短信信息: 参考号=%d, 当前=%d, 总计=%d\n", refNumber, partNumber, totalParts);
        Serial.println("===============");

        // 判断是否为长短信
        if (totalParts > 1 && partNumber > 0) {
          // 这是长短信的一部分
          Serial.printf("收到长短信分段 %d/%d\n", partNumber, totalParts);
          
          // 查找或创建缓存槽位
          int slot = findOrCreateConcatSlot(refNumber, pdu.getSender(), totalParts);
          
          // 存储该分段（partNumber 从 1 开始，数组从 0 开始）
          int partIndex = partNumber - 1;
          if (partIndex >= 0 && partIndex < MAX_CONCAT_PARTS) {
            if (!concatBuffer[slot].parts[partIndex].valid) {
              concatBuffer[slot].parts[partIndex].valid = true;
              concatBuffer[slot].parts[partIndex].text = String(pdu.getText());
              concatBuffer[slot].receivedParts++;
              
              // 如果是第一个收到的分段，保存时间戳
              if (concatBuffer[slot].receivedParts == 1) {
                concatBuffer[slot].timestamp = String(pdu.getTimeStamp());
              }
              
              Serial.printf("  已缓存分段 %d，当前已收到 %d/%d\n", 
                           partNumber, 
                           concatBuffer[slot].receivedParts, 
                           totalParts);
            } else {
              Serial.printf("  分段 %d 已存在，跳过\n", partNumber);
            }
          }
          
          // 检查是否已收齐所有分段
          if (concatBuffer[slot].receivedParts >= totalParts) {
            Serial.println("长短信已收齐，开始合并转发");
            
            // 合并所有分段
            String fullText = assembleConcatSms(slot);
            
            // 处理完整短信
            processSmsContent(concatBuffer[slot].sender.c_str(), 
                             fullText.c_str(), 
                             concatBuffer[slot].timestamp.c_str());
            
            // 清空槽位
            clearConcatSlot(slot);
          }
        } else {
          // 普通短信，直接处理
          processSmsContent(pdu.getSender(), pdu.getText(), pdu.getTimeStamp());
        }
      }
      
      // 返回 IDLE 状态
      state = IDLE;
    } 
    // 如果是其他内容（OK、ERROR 等），也返回 IDLE
    else {
      Serial.println("收到非PDU数据，返回IDLE状态");
      state = IDLE;
    }
  }
}
