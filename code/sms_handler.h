#ifndef SMS_HANDLER_H
#define SMS_HANDLER_H

#include <Arduino.h>
#include "config.h"

// 前向声明 (PDU 类型在 code.ino 中通过 pdulib.h 定义)
class PDU;

// 外部依赖变量声明
extern PDU pdu;
extern ConcatSms concatBuffer[MAX_CONCAT_MESSAGES];

// 函数声明
void initConcatBuffer();
int findOrCreateConcatSlot(int refNumber, const char* sender, int totalParts);
String assembleConcatSms(int slot);
void clearConcatSlot(int slot);
void checkConcatTimeout();
bool sendSMS(const char* phoneNumber, const char* message);
void processSmsContent(const char* sender, const char* text, const char* timestamp);
void processIncomingCall(const char* callerNumber);
void checkSerial1URC();
String readSerialLine(HardwareSerial& port);
bool isHexString(const String& str);

#endif // SMS_HANDLER_H
