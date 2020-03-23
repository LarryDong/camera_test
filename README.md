#  摄像头测试程序

## 1. 安装相机驱动

1. 解压RPI.zip
2.  启动IIC通信
```bash
cd RPI
chmod +x ./enable_i2c_vc.sh
./enable_i2c_vc.sh
```
并进行reboot

3. 编译RPI文件
```bash
cd RPI
make install
make clean && make
```
make时可能出现opencv头文件找不到报错，不用理会，不影响下面测试与使用

4.  测试运行程序，将看到图像，证明安装正确
```bash
./preview
```


## 2. 外部触发程序

### 编译

```bash
cd camera_test
mkdir build
cd build
cmake ..
make
```

### 启动

```bash
./bin/trigger
```

### 窗口说明

![程序运行截图](https://raw.githubusercontent.com/LarryDong/camera_test/master/gitpic.bmp)

1. Settings窗口：进行相关设置：

  Window：选择有几个窗口进行工作，开启的窗口越多帧率越低，建议测速时关闭所有窗口，配光时打开图像看配光情况
	Resolution：选择分辨率，0时为640x480分辨率，1时为1080x720分辨率；

2. video窗口：显示实时图像（当1窗口的window数值为1或2时）

3. bin窗口：可以通过调整阈值显示二值化结果（仅当1窗口中window数值为2时工作）

4. 帧率窗口：显示当前分辨率，以及分辨率下帧率。

 ### 关于帧率

分辨率为640，不启动任何窗口，帧率大概在120；启动1或2个窗口，速率会相应下降；

分辨率为1080时，不启动窗口，帧率约50-60。

 ### 引脚

启动后若无输入信号，应该是满速运行；

之后在FSIN输入PWM信号后，跟随输入信号进行拍照；GND也接地。

 ### Opencv 库

若opencv版本问题无法运行，请替换libs下opencv相关链接库。并重新编译
