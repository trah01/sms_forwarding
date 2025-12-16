#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>

// 串口映射
#define TXD 3
#define RXD 4

// 推送通道类型
enum PushType {
  PUSH_TYPE_NONE = 0,      // 未启用
  PUSH_TYPE_POST_JSON = 1, // POST JSON格式 {"sender":"xxx","message":"xxx","timestamp":"xxx"}
  PUSH_TYPE_BARK = 2,      // Bark格式 POST {"title":"xxx","body":"xxx"}
  PUSH_TYPE_GET = 3,       // GET请求，参数放URL中
  PUSH_TYPE_CUSTOM = 4     // 自定义模板
};

// 最大推送通道数
#define MAX_PUSH_CHANNELS 3

// 推送通道配置（通用设计，支持多种推送方式）
struct PushChannel {
  bool enabled;           // 是否启用
  PushType type;          // 推送类型
  String name;            // 通道名称（用于显示）
  String url;             // 推送URL（webhook地址）
  String key1;            // 额外参数1（如：钉钉secret、pushplus token等）
  String key2;            // 额外参数2（备用）
  String customBody;      // 自定义请求体模板（使用 {sender} {message} {timestamp} 占位符）
};

// 配置参数结构体
struct Config {
  String smtpServer;
  int smtpPort;
  String smtpUser;
  String smtpPass;
  String smtpSendTo;

  bool emailEnabled;
  PushChannel pushChannels[MAX_PUSH_CHANNELS];  // 多推送通道
  String webUser;      // Web管理账号
  String webPass;      // Web管理密码
  
  // 定时任务配置
  bool timerEnabled;        // 是否启用定时任务
  int timerType;            // 0=Ping, 1=短信
  int timerInterval;        // 间隔时间（天）
  String timerPhone;        // 定时短信目标号码
  String timerMessage;      // 定时短信内容
  
  // MQTT 配置
  bool mqttEnabled;         // 是否启用 MQTT
  bool mqttControlOnly;     // 仅控制模式（不推送短信内容）
  String mqttServer;        // MQTT 服务器地址
  int mqttPort;             // MQTT 端口
  String mqttUser;          // MQTT 用户名
  String mqttPass;          // MQTT 密码
  String mqttPrefix;        // MQTT 主题前缀
};

// 默认Web管理账号密码
#define DEFAULT_WEB_USER "admin"
#define DEFAULT_WEB_PASS "admin123"

// 长短信合并相关定义
#define MAX_CONCAT_PARTS 10       // 最大支持的长短信分段数
#define CONCAT_TIMEOUT_MS 30000   // 长短信等待超时时间(毫秒)
#define MAX_CONCAT_MESSAGES 5     // 最多同时缓存的长短信组数

#define SERIAL_BUFFER_SIZE 500
#define MAX_PDU_LENGTH 300

// 长短信分段结构
struct SmsPart {
  bool valid;           // 该分段是否有效
  String text;          // 分段内容
};

// 长短信缓存结构
struct ConcatSms {
  bool inUse;                           // 是否正在使用
  int refNumber;                        // 参考号
  String sender;                        // 发送者
  String timestamp;                     // 时间戳（使用第一个收到的分段的时间戳）
  int totalParts;                       // 总分段数
  int receivedParts;                    // 已收到的分段数
  unsigned long firstPartTime;          // 收到第一个分段的时间
  SmsPart parts[MAX_CONCAT_PARTS];      // 各分段内容
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

// 函数声明
void saveConfig();
void loadConfig();
bool isPushChannelValid(const PushChannel& ch);
bool isConfigValid();
String getDeviceUrl();

#endif // CONFIG_H
