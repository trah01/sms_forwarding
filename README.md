# 低成本短信转发器
用低成本硬件实现短信自动转发，只需要提供供电和WiFi即可，收到短信后自动推送到手机/邮箱/智能家居，再也不怕错过验证码！

> 该项目可能不支持电信卡，具体请自测。

本项目改自从[sms_forwarding](https://github.com/chenxuuu/sms_forwarding),删除了钉钉机器人\PushPlus\Server酱,有需要的可以使用原版,或者自定义http推送通道。

本项目主要拆分了功能区，优化了ui界面，接入了mqtt，实现远程控制和推送，同时可以接入Home Assistant

> 原项目固件的视频教程：[B站视频](https://www.bilibili.com/video/BV1cSmABYEiX),少部分不适用于当前修改版，大部分通用。

<img src="assets/photo.png" width="200" />

## 功能特点

- 📱 收到短信自动转发到手机/邮箱
- 🔔 支持多种推送方式同时启用
- 🌐 网页配置，无需改代码
- 📡 支持 MQTT，可接入 Home Assistant 智能家居
- ⏰ 定时Ping或者发送短信保号，避免卡被销号回收
- 💬 网页,mqtt发短信

## 通知方式

所有配置都在网页界面完成，支持同时开启多个：

| 方式 | 说明 |
|------|------|
| **邮件** | 收到短信发邮件通知 |
| **MQTT** | 接入智能家居（如 Home Assistant） |
| **Bark** | iPhone 推送通知 |
| **Webhook** | 推送到任意服务器 |

## 硬件准备

总成本约 **28元**：

| 硬件 | 价格 | 链接 |
|------|------|------|
| ESP32C3 Super Mini | ¥9.5 | [淘宝](https://item.taobao.com/item.htm?id=852057780489&skuId=5813710390565) |
| ML307R-DC 核心板 | ¥16.3 | [淘宝](https://item.taobao.com/item.htm?id=797466121802&skuId=5722077108045) |
| 4G 天线 | ¥2 | 同上链接 |

## 接线方式
### 注意,rx和tx的针脚和 GPIO3  GPIO4不是锤子对应,要掰弯针脚,我是建议买焊接好的esp32c3,然后使用杜邦线连接,然后ML307R的 EN 要和 5V(VCC) 连接,否则无法正常运行

<img src="assets/connect.png" width="200" />
```

简单说就是：
- ESP32 的 **GPIO3** 接 ML307 的 **RX**
- ESP32 的 **GPIO4** 接 ML307 的 **TX**
- **GND** 接 **GND**
- **5V** 接 **VCC** 和 **EN**（让模块自动开机）

## 烧录步骤

### 1. 安装 Arduino IDE

下载安装 [Arduino IDE](https://www.arduino.cc/en/software)

### 2. 添加 ESP32 支持

1. 打开 Arduino IDE，点击 **文件 → 首选项**
2. 在"附加开发板管理器网址"填入：
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. 点击 **工具 → 开发板 → 开发板管理器**
4. 搜索 `esp32`，安装 **esp32 by Espressif Systems**

### 3. 安装依赖库

点击 **工具 → 管理库**，搜索并安装：
- **ReadyMail** by Mobizt
- **pdulib** by David Henry
- **PubSubClient** by Nick O'Leary

### 4. 配置 WiFi

打开 `code/wifi_config.h`，修改成你的 WiFi：

```cpp
#define WIFI_SSID "你的WiFi名"
#define WIFI_PASS "你的WiFi密码"
```

### 5. 上传程序

1. 用 USB 线连接 ESP32C3 到电脑
2. 选择开发板：**工具 → 开发板 → MakerGO ESP32 C3 SuperMini**
3. 选择端口：**工具 → 端口 → （选择出现的COM口）**
4. 点击上传按钮 ➡️

### 6. 开始使用

1. 插入 SIM 卡到 ML307 模块
2. 用 USB 给 ESP32 供电
3. 打开手机连接同一个 WiFi
4. 浏览器访问 ESP32 的 IP 地址（串口会打印）
5. 默认账号密码：`admin` / `admin123`
6. 在网页配置你想要的推送方式

## MQTT 功能（接入智能家居）

如果你用 Home Assistant 或其他智能家居平台，可以通过 MQTT 实现：

- 📥 收到短信自动推送到 HA
- 📤 通过 HA 远程发短信
- 📊 在 HA 显示信号强度、在线状态

### 配置方法

1. 在网页界面展开"MQTT"
2. 填入你的 MQTT 服务器信息
3. 勾选启用，保存

### MQTT 主题说明

设备会用 MAC 地址后6位作为 ID，比如设备 ID 是 `a1b2c3`，主题前缀是 `sms`：

**设备上报的主题：**
| 主题 | 说明 |
|------|------|
| `sms/a1b2c3/status` | 设备状态（每60秒更新） |
| `sms/a1b2c3/sms/received` | 收到的短信内容 |

**发送命令到设备：**
| 主题 | 消息 | 说明 |
|------|------|------|
| `sms/a1b2c3/sms/send` | `{"phone":"138xxx","message":"内容"}` | 发短信 |
| `sms/a1b2c3/ping` | `{}` | 执行 Ping 测试 |
| `sms/a1b2c3/cmd` | `{"action":"restart"}` | 重启设备 |

### 状态信息

设备每60秒上报一次状态，包含：

```json
{
  "status": "online",
  "ip": "192.168.1.100",
  "wifi_rssi": -45,
  "lte_rsrp": -85,
  "network_status": "registered_home"
}
```

| 字段 | 说明 |
|------|------|
| `status` | 在线/离线 |
| `wifi_rssi` | WiFi 信号强度，越接近0越好 |
| `lte_rsrp` | 4G 信号强度，-80以上算好，-100以下较差 |
| `network_status` | 网络状态（已注册/搜索中/未注册） |

### Home Assistant 配置示例

在 `configuration.yaml` 添加：

```yaml
mqtt:
  sensor:
    - name: "短信转发器"
      state_topic: "sms/a1b2c3/status"
      value_template: "{{ value_json.status }}"
      
    - name: "4G信号强度"
      state_topic: "sms/a1b2c3/status"
      value_template: "{{ value_json.lte_rsrp }}"
      unit_of_measurement: "dBm"
```

### 仅控制模式

如果你用的是公共 MQTT 服务器，担心短信内容泄露，可以勾选"仅控制模式"：
- ✅ 可以远程发短信、Ping、重启
- ✅ 会上报设备状态
- ❌ 不会上传收到的短信内容

## 常见问题
**Q: 提示检测不到AT?**
- 是否有接好5v和en
- 检查esp32和ML307的连接

**Q: 提示检测CGATT 附着?**
- 检查4G天线插好没

**Q: 收不到短信？**

- 检查 SIM 卡是否正确插入
- 检查天线是否连接
- 在串口看有没有 +CMTI 提示

**Q: 网页打不开？**
- 检查 ESP32 和手机是否在同一个 WiFi
- 在串口查看 ESP32 的 IP 地址

**Q: MQTT 连不上？**
- 检查服务器地址和端口
- 检查用户名密码是否正确
- 确认服务器允许你的 IP 连接

## 文件说明

```
code/
├── code.ino          # 主程序
├── wifi_config.h     # WiFi 配置（需要修改）
├── config.h/.ino     # 配置管理
├── web_pages.h       # 网页界面
├── web_handlers.h/.ino  # 网页处理
├── sms_handler.h/.ino   # 短信处理
├── push_service.h/.ino  # 推送服务
└── mqtt_handler.h/.ino  # MQTT 功能
```

## License

MIT
