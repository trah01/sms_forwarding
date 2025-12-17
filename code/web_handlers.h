#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include <Arduino.h>
#include <WebServer.h>
#include "config.h"

extern WebServer server;

// Web 页面处理
bool checkAuth();
void handleRoot();
void handleToolsPage();
void handleQuery();
void handleSendSms();
void handlePing();
void handleTimer();
void handleSave();

// 新功能
void handleRestart();
void handleSmsHistory();
void handleStats();
void handleFilterSave();

// AT 命令
String sendATCommand(const char* cmd, unsigned long timeout);
bool sendATandWaitOK(const char* cmd, unsigned long timeout);
bool waitCGATT1();
void blink_short(unsigned long gap_time = 500);

#endif
