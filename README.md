# esp-jumpstart-college

## 课后作业2

1. LED 状态显示
    - 等待配网 红色闪烁
    - 正在配网 蓝色闪烁
    - 配网成功 绿色
    - 配网失败 红色

2. Button 重置设备
    - 单击 开/关 灯
    - 长按 重置设备

3. 手机控制
   - 使用手机 APP 控制 灯的 开/关



## 提交代码

1. 下载工程

```
git clone https://github.com/zhanzhaocheng/esp-jumpstart-college.git
```

2. 创建分支

```
git checkout -b <name>
```

3. 添加代码
将自己的代码复制到这工程中

4. 提交工程
```
git add .
git commint -m <commint>
git push origin <name>
```
EXAMPLE:

git init

cd .git

git clone  https://github.com/zhanzhaocheng/esp-jumpstart-college.git

cd esp-jumpstart-college

git checkout -b newbranch

//change the target project

git add README.md

git commit -m 'newbranch'

git checkout master

git merge newbranch

git remote add origin  https://github.com/zhanzhaocheng/esp-jumpstart-college.git

git push -u origin master
