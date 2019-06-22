# esp-jumpstart-college

## 基于ESP32的便携式示波器小项目
***
使用ESP32内置ADC，简单实现模拟信号采集、WIFI传输、图形绘制、数据保存与分享功能。
- 12位AD
- 采样率10KHz
- voltage 0-3300mV
***
### 1. 端口配置
- 红灯： GPIO21
- 绿灯： GPIO22
- 蓝灯： GPIO23
- ADC1： GPIO34
- TCP PORT：6666

### 2. LED 状态显示
- 等待配网 红色闪烁
- 正在配网 蓝色闪烁
- 配网成功 绿色
- 配网失败 红色
- AD采集&数据传输 红色绿色交替

### 3. Button 重置设备
- 长按Boot 重置设备

### 4. Android上位机源码
'https://github.com/qljz1993/EEGViewer.git'
    

