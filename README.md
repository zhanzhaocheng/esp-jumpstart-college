# esp-jumpstart-college

## 课后作业&小项目

使用ESP32内置ADC，简单实现模拟信号采集、WIFI传输、图形绘制、数据保存与分享功能。12位AD，采样率10KHz，11dB attenuation，voltage 0-3.9V。

1. 端口配置

    -红灯： GPIO21
    
    -绿灯： GPIO22
    
    -蓝灯： GPIO23
    
    -ADC1： ADC_CHANNEL_6 GPIO34
    
    -TCP/IP PORT：6666
    

2. LED 状态显示
    - 等待配网 红色闪烁
    - 正在配网 蓝色闪烁
    - 配网成功 绿色
    - 配网失败 红色
    - AD采集&数据传输 红色绿色交替

3. Button 重置设备
    - 单击 开/关 灯
    - 长按 重置设备
    

