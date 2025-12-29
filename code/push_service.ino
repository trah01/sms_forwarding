/*
 * push_service.ino - 推送服务函数实现
 * 
 * 支持 HTTPS 请求（跳过证书验证）
 * 支持钉钉加签验证
 */

#include <mbedtls/md.h>
#include <mbedtls/base64.h>

// URL 编码辅助函数
String urlEncode(const String& str) {
  String encoded = "";
  char c;
  char code0;
  char code1;
  for (unsigned int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encoded += '+';
    } else if (isalnum(c)) {
      encoded += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) code1 = (c & 0xf) - 10 + 'A';
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) code0 = c - 10 + 'A';
      encoded += '%';
      encoded += code0;
      encoded += code1;
    }
  }
  return encoded;
}

// JSON 转义函数
String jsonEscape(const String& str) {
  String result = "";
  for (unsigned int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (c == '"') result += "\\\"";
    else if (c == '\\') result += "\\\\";
    else if (c == '\n') result += "\\n";
    else if (c == '\r') result += "\\r";
    else if (c == '\t') result += "\\t";
    else result += c;
  }
  return result;
}

// Telegram Markdown 转义函数
String telegramEscape(const String& str) {
  String result = "";
  for (unsigned int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    // 转义 Markdown 特殊字符: _ * [ ] ( ) ~ ` > # + - = | { } . !
    if (c == '_' || c == '*' || c == '[' || c == ']' || c == '(' || c == ')' ||
        c == '~' || c == '`' || c == '>' || c == '#' || c == '+' || c == '-' ||
        c == '=' || c == '|' || c == '{' || c == '}' || c == '.' || c == '!') {
      result += '\\';
    }
    result += c;
  }
  return result;
}

// HMAC-SHA256 签名（用于钉钉加签）
String hmacSha256Base64(const String& secret, const String& data) {
  unsigned char hmacResult[32];
  
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
  mbedtls_md_hmac_starts(&ctx, (unsigned char*)secret.c_str(), secret.length());
  mbedtls_md_hmac_update(&ctx, (unsigned char*)data.c_str(), data.length());
  mbedtls_md_hmac_finish(&ctx, hmacResult);
  mbedtls_md_free(&ctx);
  
  // Base64 编码
  unsigned char base64Result[64];
  size_t outLen;
  mbedtls_base64_encode(base64Result, sizeof(base64Result), &outLen, hmacResult, 32);
  base64Result[outLen] = 0;
  
  return String((char*)base64Result);
}

// 发送 HTTP/HTTPS 请求的通用函数
int sendHttpRequest(const String& url, const String& method, const String& contentType, const String& body) {
  HTTPClient http;
  
  // 判断是否为 HTTPS
  if (url.startsWith("https://")) {
    // 使用全局的 ssl_client（在 code.ino 中已设置 setInsecure）
    http.begin(ssl_client, url);
  } else {
    http.begin(url);
  }
  
  if (contentType.length() > 0) {
    http.addHeader("Content-Type", contentType);
  }
  
  int httpCode;
  esp_task_wdt_reset(); // 发送前喂狗
  if (method == "GET") {
    httpCode = http.GET();
  } else {
    httpCode = http.POST(body);
  }
  esp_task_wdt_reset(); // 收到响应后喂狗
  
  if (httpCode > 0) {
    Serial.printf("HTTP响应码: %d\n", httpCode);
    // 所有 2xx 状态码都视为成功 (200 OK, 201 Created, 202 Accepted, 204 No Content 等)
    if (httpCode >= 200 && httpCode < 300) {
      String response = http.getString();
      Serial.println("响应: " + response.substring(0, 200));  // 限制输出长度
      stats.pushSuccess++;
    } else {
      Serial.println("HTTP错误响应");
      stats.pushFailed++;
    }
  } else {
    Serial.printf("HTTP请求失败: %s\n", http.errorToString(httpCode).c_str());
    stats.pushFailed++;
  }
  
  http.end();
  return httpCode;
}

// 发送单个推送通道
void sendToChannel(const PushChannel& channel, const char* sender, const char* message, const char* timestamp) {
  if (!channel.enabled) return;
  if (channel.url.length() == 0) return;
  
  String channelName = channel.name.length() > 0 ? channel.name : ("通道" + String(channel.type));
  Serial.println("发送到推送通道: " + channelName);
  
  String senderEscaped = jsonEscape(String(sender));
  String messageEscaped = jsonEscape(String(message));
  String timestampEscaped = jsonEscape(String(timestamp));
  
  switch (channel.type) {
    case PUSH_TYPE_POST_JSON: {
      // 标准 POST JSON 格式
      String jsonData = "{";
      jsonData += "\"sender\":\"" + senderEscaped + "\",";
      jsonData += "\"message\":\"" + messageEscaped + "\",";
      jsonData += "\"timestamp\":\"" + timestampEscaped + "\"";
      jsonData += "}";
      Serial.println("POST JSON: " + jsonData);
      sendHttpRequest(channel.url, "POST", "application/json", jsonData);
      break;
    }
    
    case PUSH_TYPE_BARK: {
      // Bark 推送格式
      String jsonData = "{";
      jsonData += "\"title\":\"" + senderEscaped + "\",";
      jsonData += "\"body\":\"" + messageEscaped + "\"";
      jsonData += "}";
      Serial.println("BARK: " + jsonData);
      sendHttpRequest(channel.url, "POST", "application/json", jsonData);
      break;
    }
    
    case PUSH_TYPE_GET: {
      // GET 请求，参数放 URL 里
      String getUrl = channel.url;
      if (getUrl.indexOf('?') == -1) {
        getUrl += "?";
      } else {
        getUrl += "&";
      }
      getUrl += "sender=" + urlEncode(String(sender));
      getUrl += "&message=" + urlEncode(String(message));
      getUrl += "&timestamp=" + urlEncode(String(timestamp));
      Serial.println("GET: " + getUrl);
      sendHttpRequest(getUrl, "GET", "", "");
      break;
    }
    
    case PUSH_TYPE_CUSTOM: {
      // 自定义模板
      if (channel.customBody.length() == 0) {
        Serial.println("自定义模板为空，跳过");
        return;
      }
      String body = channel.customBody;
      body.replace("{sender}", senderEscaped);
      body.replace("{message}", messageEscaped);
      body.replace("{timestamp}", timestampEscaped);
      Serial.println("自定义: " + body);
      sendHttpRequest(channel.url, "POST", "application/json", body);
      break;
    }
    
    case PUSH_TYPE_TELEGRAM: {
      // Telegram Bot 推送
      // URL格式: https://api.telegram.org/bot<TOKEN>/sendMessage
      // 使用 MarkdownV2 模式，需要转义特殊字符
      String senderTg = telegramEscape(String(sender));
      String messageTg = telegramEscape(String(message));
      String timestampTg = telegramEscape(String(timestamp));
      String text = "📱 *来自: " + senderTg + "*\n" + messageTg + "\n\n_" + timestampTg + "_";
      String jsonData = "{";
      jsonData += "\"chat_id\":\"" + channel.key1 + "\",";
      jsonData += "\"text\":\"" + jsonEscape(text) + "\",";
      jsonData += "\"parse_mode\":\"MarkdownV2\"";
      jsonData += "}";
      Serial.println("Telegram: " + jsonData);
      sendHttpRequest(channel.url, "POST", "application/json", jsonData);
      break;
    }
    
    case PUSH_TYPE_WECOM: {
      // 企业微信机器人 (Webhook)
      // URL格式: https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key=xxx
      String content = "📱 来自: " + String(sender) + "\n" + String(message) + "\n\n" + String(timestamp);
      String jsonData = "{";
      jsonData += "\"msgtype\":\"text\",";
      jsonData += "\"text\":{\"content\":\"" + jsonEscape(content) + "\"}";
      jsonData += "}";
      Serial.println("企业微信: " + jsonData);
      sendHttpRequest(channel.url, "POST", "application/json", jsonData);
      break;
    }
    
    case PUSH_TYPE_DINGTALK: {
      // 钉钉机器人 (Webhook)
      // URL格式: https://oapi.dingtalk.com/robot/send?access_token=xxx
      // 如果配置了加签密钥（key1），则需要添加签名
      
      String requestUrl = channel.url;
      
      // 检查是否需要加签
      if (channel.key1.length() > 0) {
        // 获取当前时间戳（毫秒）- 使用 millis() 补充真实毫秒精度
        unsigned long long timestampMs = (unsigned long long)time(nullptr) * 1000ULL + (millis() % 1000);
        // 格式化为 13 位时间戳字符串
        char timestampBuf[16];
        snprintf(timestampBuf, sizeof(timestampBuf), "%llu", timestampMs);
        String timestampStr = String(timestampBuf);
        
        // 构造签名字符串
        String stringToSign = timestampStr + "\n" + channel.key1;
        
        // 计算 HMAC-SHA256 签名
        String sign = hmacSha256Base64(channel.key1, stringToSign);
        sign = urlEncode(sign);
        
        // 添加签名参数到 URL
        if (requestUrl.indexOf('?') == -1) {
          requestUrl += "?";
        } else {
          requestUrl += "&";
        }
        requestUrl += "timestamp=" + timestampStr;
        requestUrl += "&sign=" + sign;
        
        Serial.println("钉钉加签URL: " + requestUrl);
      }
      
      String content = "📱 来自: " + String(sender) + "\n" + String(message) + "\n\n" + String(timestamp);
      String jsonData = "{";
      jsonData += "\"msgtype\":\"text\",";
      jsonData += "\"text\":{\"content\":\"" + jsonEscape(content) + "\"}";
      jsonData += "}";
      Serial.println("钉钉: " + jsonData);
      sendHttpRequest(requestUrl, "POST", "application/json", jsonData);
      break;
    }
    
    default:
      Serial.println("未知推送类型");
      return;
  }
}

// 发送短信到所有启用的推送通道
void sendSMSToServer(const char* sender, const char* message, const char* timestamp) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi未连接，跳过推送");
    return;
  }
  
  bool hasEnabledChannel = false;
  for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
    if (isPushChannelValid(config.pushChannels[i])) {
      hasEnabledChannel = true;
      break;
    }
  }
  
  if (!hasEnabledChannel) {
    Serial.println("无HTTP推送通道");
    return;
  }
  
  Serial.println("\n=== 开始多通道推送 ===");
  for (int i = 0; i < MAX_PUSH_CHANNELS; i++) {
    if (isPushChannelValid(config.pushChannels[i])) {
      esp_task_wdt_reset(); // 每个通道推送前喂狗
      sendToChannel(config.pushChannels[i], sender, message, timestamp);
      delay(100); // 短暂延迟避免请求过快
    }
  }
  Serial.println("=== 多通道推送完成 ===\n");
  
  // 保存推送统计
  saveStats();
}

// 发送邮件通知函数
void sendEmailNotification(const char* subject, const char* body) {
  if (!config.emailEnabled) return;
  
  if (config.smtpServer.length() == 0 || config.smtpUser.length() == 0 || 
      config.smtpPass.length() == 0 || config.smtpSendTo.length() == 0) {
    Serial.println("邮件配置不完整，跳过发送");
    return;
  }
  
  auto statusCallback = [](SMTPStatus status) {
    Serial.println(status.text);
  };
  smtp.connect(config.smtpServer.c_str(), config.smtpPort, statusCallback);
  if (smtp.isConnected()) {
    smtp.authenticate(config.smtpUser.c_str(), config.smtpPass.c_str(), readymail_auth_password);

    SMTPMessage msg;
    String from = "sms notify <"; from += config.smtpUser; from += ">";
    msg.headers.add(rfc822_from, from.c_str());
    String to = "your_email <"; to += config.smtpSendTo; to += ">";
    msg.headers.add(rfc822_to, to.c_str());
    msg.headers.add(rfc822_subject, subject);
    msg.text.body(body);
    configTime(0, 0, "ntp.ntsc.ac.cn");
    // 等待 NTP 同步，最多 10 秒，防止看门狗超时
    unsigned long ntpStart = millis();
    while (time(nullptr) < 100000 && millis() - ntpStart < 10000) {
      delay(100);
      esp_task_wdt_reset();  // 喂狗
    }
    msg.timestamp = time(nullptr);
    smtp.send(msg);
    Serial.println("邮件发送完成");
  } else {
    Serial.println("邮件服务器连接失败");
  }
}
