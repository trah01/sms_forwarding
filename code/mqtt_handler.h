#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

// 外部 MQTT 变量声明
extern WiFiClient mqttWifiClient;
extern PubSubClient mqttClient;

extern String mqttDeviceId;
extern String mqttTopicStatus;
extern String mqttTopicSmsReceived;
extern String mqttTopicSmsSent;
extern String mqttTopicPingResult;
extern String mqttTopicSmsSend;
extern String mqttTopicPing;
extern String mqttTopicCmd;

extern unsigned long lastMqttReconnectAttempt;
extern unsigned long lastMqttStatusReport;
extern const unsigned long MQTT_RECONNECT_INTERVAL;
extern const unsigned long MQTT_STATUS_INTERVAL;

// MQTT 函数声明
void mqttCallback(char* topic, byte* payload, unsigned int length);
void mqttReconnect();
void initMqttTopics();
String getMacSuffix();
void publishMqttSmsReceived(const char* sender, const char* message, const char* timestamp);
void publishMqttSmsSent(const char* phone, const char* message, bool success);
void publishMqttPingResult(const char* host, bool success, const char* result);
void publishMqttStatus(const char* status);
void publishMqttDeviceStatus();

#endif // MQTT_HANDLER_H
