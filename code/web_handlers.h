#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include <Arduino.h>
#include <WebServer.h>
#include "config.h"

// 外部依赖
extern WebServer server;

// 函数声明
bool checkAuth();
void handleRoot();
void handleToolsPage();
void handleQuery();
void handleSendSms();
void handlePing();
void handleTimer();
void handleSave();

// AT 命令相关
String sendATCommand(const char* cmd, unsigned long timeout);
bool sendATandWaitOK(const char* cmd, unsigned long timeout);
bool waitCGATT1();
void resetModule();
void blink_short(unsigned long gap_time = 500);

#endif // WEB_HANDLERS_H
