#ifndef PUSH_SERVICE_H
#define PUSH_SERVICE_H

#include <Arduino.h>
#include "config.h"

// 函数声明
String urlEncode(const String& str);
String jsonEscape(const String& str);
void sendToChannel(const PushChannel& channel, const char* sender, const char* message, const char* timestamp);
void sendSMSToServer(const char* sender, const char* message, const char* timestamp);
void sendEmailNotification(const char* subject, const char* body);

#endif // PUSH_SERVICE_H
