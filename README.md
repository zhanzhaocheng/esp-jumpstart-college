R、G、BLED引脚定义如下：
#define CONFIG_RLED_GPIO 25
#define CONFIG_GLED_GPIO 26
#define CONFIG_BLED_GPIO 27

1.新建三个LED初始化函数和三个LED闪烁线程任务

2.在event_handler(void *ctx, system_event_t *event)函数中编写配网中、配网成功、配网失败功能
注：在这些状态切换过程中注意不要遗漏情况，考虑需全面

3.tcp服务器支持如下指令：
all_on:打开所有灯；
all_off：关闭所有灯；
r_on:打开红色LED；
r_off:关闭红色LED；
g_on:打开绿色LED；
g_off:关闭绿色LED;
b_on:打开蓝色LED；
b_off:关闭蓝色LED;
