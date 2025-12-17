#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>
#include <vector>

// 串口映射
#define TXD 3
#define RXD 4

// mDNS 主机名（访问地址为 http://sms.local）
#define MDNS_HOSTNAME "sms"

// 推送通道类型
enum PushType {
  PUSH_TYPE_NONE = 0,      // 未启用
  PUSH_TYPE_POST_JSON = 1, // POST JSON格式 {"sender":"xxx","message":"xxx","timestamp":"xxx"}
  PUSH_TYPE_BARK = 2,      // Bark格式 POST {"title":"xxx","body":"xxx"}
  PUSH_TYPE_GET = 3,       // GET请求，参数放URL中
  PUSH_TYPE_CUSTOM = 4,    // 自定义模板
  PUSH_TYPE_TELEGRAM = 5,  // Telegram Bot
  PUSH_TYPE_WECOM = 6,     // 企业微信机器人
  PUSH_TYPE_DINGTALK = 7   // 钉钉机器人
};

// 最大推送通道数
#define MAX_PUSH_CHANNELS 3

// 短信历史记录设置
#define MAX_SMS_HISTORY 100

// 黑白名单设置
#define MAX_FILTER_NUMBERS 100  // 扩容到100个

// 推送通道配置
struct PushChannel {
  bool enabled;
  PushType type;
  String name;
  String url;
  String key1;
  String key2;
  String customBody;
};

// 短信历史记录结构 (旧结构保留，实际上改用 SPIFFS)
struct SmsRecord {
  String sender;
  String message;
  String timestamp;
  bool valid;
};

// 统计计数器结构
struct Statistics {
  unsigned long smsReceived;    // 收到短信数
  unsigned long smsSent;        // 发送短信数
  unsigned long pushSuccess;    // 推送成功数
  unsigned long pushFailed;     // 推送失败数
  unsigned long bootCount;      // 启动次数
};

// WiFi 配置结构
#define MAX_WIFI_NETWORKS 3
struct WifiNetwork {
  String ssid;
  String password;
  bool enabled;
};

// 配置参数结构体
struct Config {
  // WiFi 配置（支持多个网络）
  WifiNetwork wifiNetworks[MAX_WIFI_NETWORKS];
  
  String smtpServer;
  int smtpPort;
  String smtpUser;
  String smtpPass;
  String smtpSendTo;

  bool emailEnabled;
  PushChannel pushChannels[MAX_PUSH_CHANNELS];
  String webUser;
  String webPass;
  
  // 定时任务配置
  bool timerEnabled;
  int timerType;
  int timerInterval;
  String timerPhone;
  String timerMessage;
  
  // MQTT 配置
  bool mqttEnabled;
  bool mqttControlOnly;
  String mqttServer;
  int mqttPort;
  String mqttUser;
  String mqttPass;
  String mqttPrefix;
  
  // 黑白名单配置
  bool filterEnabled;          // 是否启用过滤
  bool filterIsWhitelist;      // true=白名单, false=黑名单
  // 使用单个长字符串存储，内存中逗号分隔，避免定义大数组占用栈空间
  String filterList;  
};

// 默认Web管理账号密码
#define DEFAULT_WEB_USER "admin"
#define DEFAULT_WEB_PASS "admin123"

// 长短信合并相关定义
#define MAX_CONCAT_PARTS 10
#define CONCAT_TIMEOUT_MS 30000
#define MAX_CONCAT_MESSAGES 5

#define SERIAL_BUFFER_SIZE 500
#define MAX_PDU_LENGTH 300

// 长短信分段结构
struct SmsPart {
  bool valid;
  String text;
};

// 长短信缓存结构
struct ConcatSms {
  bool inUse;
  int refNumber;
  String sender;
  String timestamp;
  int totalParts;
  int receivedParts;
  unsigned long firstPartTime;
  SmsPart parts[MAX_CONCAT_PARTS];
};

// 全局变量声明 (extern)
extern Config config;
extern Preferences preferences;
extern bool configValid;
extern unsigned long lastPrintTime;
extern unsigned long lastTimerExec;
extern unsigned long timerIntervalSec;
extern ConcatSms concatBuffer[MAX_CONCAT_MESSAGES];
extern char serialBuf[SERIAL_BUFFER_SIZE];
extern int serialBufLen;

// 短信历史和统计
extern SmsRecord smsHistory[MAX_SMS_HISTORY];
extern int smsHistoryIndex;
extern Statistics stats;

// 函数声明
void saveConfig();
void loadConfig();
bool isPushChannelValid(const PushChannel& ch);
bool isConfigValid();
String getDeviceUrl();
void initSmsStorage();
void addSmsToHistory(const char* sender, const char* message, const char* timestamp);
String getSmsHistory();
bool isNumberFiltered(const char* number);
void saveStats();
void loadStats();

#endif // CONFIG_H
