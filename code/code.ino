/*
 * SMS Forwarding - 短信转发器
 * 
 * 文件结构:
 * - code.ino          : 主文件（全局变量、setup、loop）
 * - config.h          : 配置结构体和常量定义
 * - config.ino        : 配置相关函数实现
 * - web_pages.h       : HTML 页面内容
 * - web_handlers.h    : Web 处理函数声明
 * - web_handlers.ino  : Web 处理函数实现
 * - sms_handler.h     : 短信处理函数声明
 * - sms_handler.ino   : 短信处理函数实现
 * - push_service.h    : 推送服务函数声明
 * - push_service.ino  : 推送服务函数实现
 * - mqtt_handler.h    : MQTT 处理函数声明
 * - mqtt_handler.ino  : MQTT 处理函数实现
 * - wifi_config.h     : WiFi 配置
 * - mqtt_config.h     : MQTT 配置
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include <pdulib.h>
#define ENABLE_SMTP
#define ENABLE_DEBUG
#include <ReadyMail.h>
#include <HTTPClient.h>
#include <base64.h>
#include <esp_task_wdt.h>

// 项目配置文件
#include "wifi_config.h"
#include "config.h"
#include "web_pages.h"
#include "web_handlers.h"
#include "sms_handler.h"
#include "push_service.h"
#include "mqtt_handler.h"

// ========== 全局变量定义 ==========
Config config;
Preferences preferences;
WiFiMulti WiFiMulti;
PDU pdu = PDU(4096);
WiFiClientSecure ssl_client;
SMTPClient smtp(ssl_client);
WebServer server(80);

bool configValid = false;
unsigned long lastPrintTime = 0;
unsigned long lastTimerExec = 0;
unsigned long timerIntervalSec = 0;  // 使用秒避免溢出（支持到 136 年）

char serialBuf[SERIAL_BUFFER_SIZE];
int serialBufLen = 0;

ConcatSms concatBuffer[MAX_CONCAT_MESSAGES];

// ========== MQTT 全局变量 ==========
WiFiClient mqttWifiClient;
PubSubClient mqttClient(mqttWifiClient);

String mqttDeviceId = "";
// 用户自定义前缀主题
String mqttTopicStatus = "";
String mqttTopicSmsReceived = "";
String mqttTopicSmsSent = "";
String mqttTopicPingResult = "";
String mqttTopicSmsSend = "";
String mqttTopicPing = "";
String mqttTopicCmd = "";
// Home Assistant 自动发现主题
String mqttHaStatusTopic = "";
String mqttHaSmsReceivedTopic = "";

unsigned long lastMqttReconnectAttempt = 0;
unsigned long lastMqttStatusReport = 0;
const unsigned long MQTT_RECONNECT_INTERVAL = 5000;
const unsigned long MQTT_STATUS_INTERVAL = 60000;  // 每60秒上报一次状态

// 定时过滤检查变量
unsigned long lastSchedFilterCheck = 0;
const unsigned long SCHED_FILTER_CHECK_INTERVAL = 60000;  // 每60秒检查一次
int currentSchedFilterMode = -1;  // 当前应用的模式，-1表示未初始化

// ========== setup 函数 ==========
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RXD, TXD);
  Serial1.setRxBufferSize(SERIAL_BUFFER_SIZE);
  
  // 初始化长短信缓存
  initConcatBuffer();
  
  // 启用看门狗 (60秒超时)
  // 适配 ESP32 Arduino Core v3.0+ (IDF 5.x)
  esp_task_wdt_config_t wdt_config = {
      .timeout_ms = 60000,
      .idle_core_mask = 0, // 不监控 IDLE 任务，只监控显式添加的任务
      .trigger_panic = true
  };
  
  // 尝试重新配置（如果已初始化）或者初始化
  esp_err_t err = esp_task_wdt_reconfigure(&wdt_config);
  if (err != ESP_OK) {
      err = esp_task_wdt_init(&wdt_config);
  }
  
  if (err == ESP_OK) {
      esp_task_wdt_add(NULL); // 监控当前任务 (loopTask)
      esp_task_wdt_reset();
      Serial.println("看门狗已启用(60s)");
  } else {
      Serial.printf("看门狗配置失败: 0x%x\n", err);
  }
  
  esp_task_wdt_reset();
  // 初始化短信存储(SPIFFS)
  initSmsStorage();
  esp_task_wdt_reset();
  
  // 加载配置
  loadConfig();
  esp_task_wdt_reset();
  configValid = isConfigValid();
  
  // 添加所有启用的 WiFi 网络（WiFiMulti 会自动选择信号最强的）
  int wifiCount = 0;
  for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
    if (config.wifiNetworks[i].enabled && config.wifiNetworks[i].ssid.length() > 0) {
      WiFiMulti.addAP(config.wifiNetworks[i].ssid.c_str(), config.wifiNetworks[i].password.c_str());
      Serial.printf("已添加WiFi: %s\n", config.wifiNetworks[i].ssid.c_str());
      wifiCount++;
    }
  }
  
  if (wifiCount == 0) {
    Serial.println("警告: 未配置任何WiFi网络，请检查配置");
  }
  
  Serial.println("连接WiFi...");
  while (WiFiMulti.run() != WL_CONNECTED) blink_short();
  Serial.printf("WiFi已连接: %s, IP: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
  
  // 启动 mDNS 服务
  if (MDNS.begin(MDNS_HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);  // 注册 HTTP 服务
    Serial.println("mDNS 已启动");
    Serial.printf("访问地址: http://%s.local 或 http://%s\n", MDNS_HOSTNAME, WiFi.localIP().toString().c_str());
  } else {
    Serial.println("mDNS 启动失败，请使用 IP 访问");
  }
  
  // 启动 HTTP 服务器
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/tools", handleToolsPage);
  server.on("/sendsms", HTTP_POST, handleSendSms);
  server.on("/ping", HTTP_POST, handlePing);
  server.on("/timer", HTTP_POST, handleTimer);
  server.on("/query", handleQuery);
  server.on("/restart", HTTP_POST, handleRestart);     // 重启
  server.on("/history", handleSmsHistory);             // 短信历史
  server.on("/stats", handleStats);                    // 统计信息
  server.on("/filter", HTTP_POST, handleFilterSave);   // 号码过滤保存
  server.on("/contentfilter", HTTP_POST, handleContentFilterSave);  // 内容过滤保存
  server.on("/schedfilter", HTTP_POST, handleSchedFilterSave);      // 定时过滤保存
  server.begin();
  Serial.println("HTTP服务器已启动");
  
  ssl_client.setInsecure();
  while (!sendATandWaitOK("AT", 1000)) {
    Serial.println("AT未响应，重试...");
    blink_short();
  }
  Serial.println("模组AT响应正常");
  
  // 清除APN配置，让模组自动识别（换卡后自动适配）
  Serial.print("重置APN配置... ");
  if (sendATandWaitOK("AT+CGDCONT=1,\"IP\",\"\"", 2000)) {
    Serial.println("完成");
  } else {
    Serial.println("跳过");
  }
  
  // 设置短信自动上报
  Serial.println("设置短信自动上报(AT+CNMI)...");
  int retryCount = 0;
  while (!sendATandWaitOK("AT+CNMI=2,2,0,0,0", 2000)) {
    blink_short();
    if (++retryCount >= 10) {
      Serial.println("警告: AT+CNMI 设置失败，跳过");
      break;
    }
  }
  if (retryCount < 10) Serial.println("AT+CNMI 设置成功");
  
  // 配置 PDU 模式
  Serial.println("配置PDU模式(AT+CMGF=0)...");
  retryCount = 0;
  while (!sendATandWaitOK("AT+CMGF=0", 2000)) {
    blink_short();
    if (++retryCount >= 10) {
      Serial.println("警告: AT+CMGF 设置失败，跳过");
      break;
    }
  }
  if (retryCount < 10) Serial.println("AT+CMGF 设置成功");
  
  // 等待 CGATT 附着
  Serial.println("等待网络附着(CGATT)...");
  retryCount = 0;
  while (!waitCGATT1()) {
    blink_short();
    if (++retryCount >= 30) {  // 网络附着可能需要更长时间
      Serial.println("警告: 网络附着超时，继续执行");
      break;
    }
  }
  if (retryCount < 30) Serial.println("网络已附着");
  

  Serial.println("模组初始化完成");
  digitalWrite(LED_BUILTIN, LOW);
  
  // 发送启动通知
  if (configValid) {
    String body = "设备已启动\n地址: " + getDeviceUrl() + "\n启动次数: " + String(stats.bootCount);
    sendEmailNotification("短信转发器已启动", body.c_str());
  }
  
  // MQTT 初始化
  initMqttTopics();
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(1024);
  if (config.mqttEnabled && config.mqttServer.length() > 0) {
    mqttClient.setServer(config.mqttServer.c_str(), config.mqttPort);
    mqttReconnect();
  }
  
  // NTP 时间同步（用于定时过滤）
  configTime(8 * 3600, 0, "pool.ntp.org", "ntp.aliyun.com");  // UTC+8 中国时区
  Serial.println("NTP时间同步已配置");
  esp_task_wdt_reset(); // setup 结束前最后一次喂狗
}

// 检查定时过滤模式
void checkScheduledFilter() {
  if (!config.schedFilterEnabled) return;
  esp_task_wdt_reset(); // 检查前喂狗
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // 时间未同步，跳过
    return;
  }
  
  int currentHour = timeinfo.tm_hour;
  int currentMin = timeinfo.tm_min;
  int currentTime = currentHour * 60 + currentMin;  // 转换为分钟
  
  int startTime = config.schedFilterStartHour * 60 + config.schedFilterStartMin;
  int endTime = config.schedFilterEndHour * 60 + config.schedFilterEndMin;
  
  // 判断当前是否在时段A内
  bool inPeriodA = false;
  if (startTime <= endTime) {
    // 正常时段，如 08:00 - 22:00
    inPeriodA = (currentTime >= startTime && currentTime < endTime);
  } else {
    // 跨午夜时段，如 22:00 - 08:00
    inPeriodA = (currentTime >= startTime || currentTime < endTime);
  }
  
  int targetMode = inPeriodA ? config.schedFilterModeA : config.schedFilterModeB;
  
  // 如果模式未变化，无需操作
  if (targetMode == currentSchedFilterMode) return;
  
  // 切换模式
  currentSchedFilterMode = targetMode;
  
  switch (targetMode) {
    case 0:  // 禁用过滤
      config.filterEnabled = false;
      Serial.println("定时过滤: 已禁用号码过滤");
      break;
    case 1:  // 白名单
      config.filterEnabled = true;
      config.filterIsWhitelist = true;
      Serial.println("定时过滤: 已切换到白名单模式");
      break;
    case 2:  // 黑名单
      config.filterEnabled = true;
      config.filterIsWhitelist = false;
      Serial.println("定时过滤: 已切换到黑名单模式");
      break;
  }
  
  // 保存配置（可选，如希望重启后保持）
  // saveConfig();
  
  // 发布状态更新
  if (config.mqttEnabled && mqttClient.connected()) {
    publishFilterStatus();
  }
}

// ========== loop 函数 ==========
void loop() {
  // 处理 HTTP 请求
  server.handleClient();
  
  // 如果配置无效，每 30 秒打印一次提示
  if (!configValid) {
    if (millis() - lastPrintTime >= 30000) {
      lastPrintTime = millis();
      Serial.printf("请访问 %s 配置\n", getDeviceUrl().c_str());
    }
  }
  
  // 检查定时任务
  if (config.timerEnabled && timerIntervalSec > 0 && configValid) {
    unsigned long elapsedSec = (millis() - lastTimerExec) / 1000;
    if (elapsedSec >= timerIntervalSec) {
      lastTimerExec = millis();
      
      if (config.timerType == 0) {
        if (sendATandWaitOK("AT+CGACT=1,1", 10000)) {
          sendATandWaitOK("AT+MPING=1,\"8.8.8.8\",4,32,255", 30000);
          delay(2000);
          sendATandWaitOK("AT+CGACT=0,1", 5000);
          publishMqttStatus("active_ping");
        }
      } else if (config.timerType == 1 && config.timerPhone.length() > 0) {
        sendSMS(config.timerPhone.c_str(), config.timerMessage.c_str());
        stats.smsSent++;
        saveStats();
        publishMqttSmsSent(config.timerPhone.c_str(), config.timerMessage.c_str(), true);
      }
    }
  }
  
  // MQTT 处理
  if (config.mqttEnabled && WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      unsigned long now = millis();
      if (now - lastMqttReconnectAttempt > MQTT_RECONNECT_INTERVAL) {
        lastMqttReconnectAttempt = now;
        mqttReconnect();
      }
    } else {
      mqttClient.loop();
      
      unsigned long now = millis();
      if (now - lastMqttStatusReport > MQTT_STATUS_INTERVAL) {
        lastMqttStatusReport = now;
        publishMqttDeviceStatus();
      }
    }
  }
  
  // 定时过滤检查
  if (config.schedFilterEnabled) {
    unsigned long now = millis();
    if (now - lastSchedFilterCheck > SCHED_FILTER_CHECK_INTERVAL) {
      lastSchedFilterCheck = now;
      checkScheduledFilter();
    }
  }
  
  // 检查长短信超时
  checkConcatTimeout();
  
  // 本地透传
  if (Serial.available()) Serial1.write(Serial.read());
  
  // 检查 URC 和解析
  checkSerial1URC();
  
  // 喂狗
  esp_task_wdt_reset();
}
