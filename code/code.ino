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
#include <Preferences.h>
#include <pdulib.h>
#define ENABLE_SMTP
#define ENABLE_DEBUG
#include <ReadyMail.h>
#include <HTTPClient.h>
#include <base64.h>

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
String mqttTopicStatus = "";
String mqttTopicSmsReceived = "";
String mqttTopicSmsSent = "";
String mqttTopicPingResult = "";
String mqttTopicSmsSend = "";
String mqttTopicPing = "";
String mqttTopicCmd = "";

unsigned long lastMqttReconnectAttempt = 0;
unsigned long lastMqttStatusReport = 0;
const unsigned long MQTT_RECONNECT_INTERVAL = 5000;
const unsigned long MQTT_STATUS_INTERVAL = 60000;  // 每60秒上报一次状态

// ========== setup 函数 ==========
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, RXD, TXD);
  Serial1.setRxBufferSize(SERIAL_BUFFER_SIZE);
  
  // 初始化长短信缓存
  initConcatBuffer();
  
  // 加载配置
  loadConfig();
  configValid = isConfigValid();
  
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
  Serial.println("连接wifi");
  Serial.println(WIFI_SSID);
  while (WiFiMulti.run() != WL_CONNECTED) blink_short();
  Serial.println("wifi已连接");
  Serial.print("IP地址: ");
  Serial.println(WiFi.localIP());
  
  // 启动 HTTP 服务器
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/tools", handleToolsPage);
  server.on("/sms", handleToolsPage);  // 兼容旧链接
  server.on("/sendsms", HTTP_POST, handleSendSms);
  server.on("/ping", HTTP_POST, handlePing);
  server.on("/timer", HTTP_POST, handleTimer);
  server.on("/query", handleQuery);
  server.begin();
  Serial.println("HTTP服务器已启动");
  
  ssl_client.setInsecure();
  while (!sendATandWaitOK("AT", 1000)) {
    Serial.println("AT未响应，重试...");
    blink_short();
  }
  Serial.println("模组AT响应正常");
  
  // 设置短信自动上报
  while (!sendATandWaitOK("AT+CNMI=2,2,0,0,0", 1000)) {
    Serial.println("设置CNMI失败，重试...");
    blink_short();
  }
  Serial.println("CNMI参数设置完成");
  
  // 配置 PDU 模式
  while (!sendATandWaitOK("AT+CMGF=0", 1000)) {
    Serial.println("设置PDU模式失败，重试...");
    blink_short();
  }
  Serial.println("PDU模式设置完成");
  
  // 等待 CGATT 附着
  while (!waitCGATT1()) {
    Serial.println("等待CGATT附着...");
    blink_short();
  }
  Serial.println("CGATT已附着");
  digitalWrite(LED_BUILTIN, LOW);
  
  // 如果配置有效，发送启动通知
  if (configValid) {
    Serial.println("配置有效，发送启动通知...");
    String subject = "短信转发器已启动";
    String body = "设备已启动\n设备地址: " + getDeviceUrl();
    sendEmailNotification(subject.c_str(), body.c_str());
  }
  
  // ========== MQTT 初始化 ==========
  Serial.println("初始化MQTT...");
  initMqttTopics();
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(1024);
  
  // 如果 MQTT 已启用，首次连接
  if (config.mqttEnabled && config.mqttServer.length() > 0) {
    mqttClient.setServer(config.mqttServer.c_str(), config.mqttPort);
    mqttReconnect();
  }
  Serial.println("MQTT初始化完成");
}

// ========== loop 函数 ==========
void loop() {
  // 处理 HTTP 请求
  server.handleClient();
  
  // 如果配置无效，每 30 秒打印一次提示
  if (!configValid) {
    if (millis() - lastPrintTime >= 30000) {
      lastPrintTime = millis();
      Serial.println("请访问 " + getDeviceUrl() + " 配置系统参数");
    }
  }
  
  // 检查定时任务执行（使用秒比较避免溢出）
  if (config.timerEnabled && timerIntervalSec > 0 && configValid) {
    unsigned long elapsedSec = (millis() - lastTimerExec) / 1000;
    if (elapsedSec >= timerIntervalSec) {
      Serial.println("执行定时任务...");
      lastTimerExec = millis();
      
      if (config.timerType == 0) {
        // 定时 Ping
        Serial.println("开始定时Ping...");
        if (sendATandWaitOK("AT+CGACT=1,1", 10000)) {
          sendATandWaitOK("AT+MPING=1,\"8.8.8.8\",4,32,255", 30000);
          delay(2000);
          sendATandWaitOK("AT+CGACT=0,1", 5000);
          Serial.println("定时Ping完成");
          
          #ifdef ENABLE_MQTT
          publishMqttStatus("active_ping");
          #endif
        }
      } else if (config.timerType == 1 && config.timerPhone.length() > 0 && config.timerMessage.length() > 0) {
        // 定时发送短信
        Serial.println("发送保号短信...");
        sendSMS(config.timerPhone.c_str(), config.timerMessage.c_str());
        
        #ifdef ENABLE_MQTT
        publishMqttSmsSent(config.timerPhone.c_str(), config.timerMessage.c_str(), true);
        #endif
      }
    }
  }
  
  // ========== MQTT 处理 ==========
  if (config.mqttEnabled && WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      unsigned long now = millis();
      if (now - lastMqttReconnectAttempt > MQTT_RECONNECT_INTERVAL) {
        lastMqttReconnectAttempt = now;
        mqttReconnect();
      }
    } else {
      mqttClient.loop();
      
      // 定期上报设备状态
      unsigned long now = millis();
      if (now - lastMqttStatusReport > MQTT_STATUS_INTERVAL) {
        lastMqttStatusReport = now;
        publishMqttDeviceStatus();
      }
    }
  }
  
  // 检查长短信超时
  checkConcatTimeout();
  
  // 本地透传
  if (Serial.available()) Serial1.write(Serial.read());
  
  // 检查 URC 和解析
  checkSerial1URC();
}
