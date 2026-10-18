# 集成到真实雷达软件

## 1. 引入核心库

将本仓库作为 Git submodule、CMake `FetchContent` 依赖或源码子目录：

```cmake
add_subdirectory(third_party/x-band-phased-array-calibration)
target_link_libraries(your_app PRIVATE radar_calibration_core)
```

## 2. 实现设备适配器

继承 `radarcal::CalibrationDevice`，把方法连接到现有设备控制链：

- `loadAndSendParameterFile`：读取用户选择的参数并原样下发。
- `position`：机械方位/俯仰定位。
- `scanAzimuth`：方位扇扫。
- `scanElevation`：机械或电扫俯仰扫描；绝对角和相对 DBF 角的换算应由适配器明确完成。
- `stop`：停止当前扫描。
- `setSoftwareZero`：设置伺服软件零位，宿主必须先弹出确认。

适配器将实时数据转为 `RadarObservation`，再发出 `observationAvailable`。不要在设备回调线程直接操作 Qt 界面；可以通过 queued connection 把已经拥有所有权的数据对象送到界面线程。

## 3. 距离库和三条曲线

```cpp
const int bin = radarcal::MetalBallCalibrator::expectedRangeBin(
    geometry.slantRangeM, observation.gateSpacingM,
    observation.reflectivityDbz.size());

const auto point = radarcal::MetalBallCalibrator::extractTriplet(
    observation.elevationDeg,
    observation.reflectivityDbz,
    bin,
    QDateTime::fromMSecsSinceEpoch(observation.timestampMs));
```

距离存在误差时，可以由宿主在理论库附近的小窗口内寻找一次目标库并锁定；同一扫描文件必须保持锁定，不能每帧追逐最强回波，否则曲线会跳到不同目标。`T-1/T/T+1` 必须始终来自相邻的三个真实库。

## 4. 角度语义

建议数据结构同时保存：

- 机械俯仰；
- DBF 相对电扫角；
- 合成后的绝对波束俯仰。

公共库只使用你明确传入的绝对扫描轴。不要用固定 `0~90°` 过滤 DBF 相对角；也不要默认“DBF 参考角就是机械法向”，除非设备协议明确如此。

## 5. 线程和生命周期

- 回调线程只复制形成完整值对象，所有容器均由接收方拥有。
- 绘图读取不可变快照，不持有 DSP 原始缓冲区指针。
- 页面切换或鼠标事件不能访问已经清空的曲线/径向对象。
- 停止和析构时断开回调并等待工作线程退出。

## 6. 现场验证清单

1. 用已知经纬高验证理论方位、仰角、地面距离和斜距。
2. 用静态回放验证距离库索引及 `T-1/T/T+1` 顺序。
3. 不接天线时验证停止、超时和页面切换无崩溃。
4. 低功率数据仍应绘制真实值，不将小于 `-30 dB` 当作无效。
5. RHI/PPI 未检测到峰时仍保存完整采集数据，但不得声明“已找到目标”。
6. 太阳法只在扫描覆盖两侧且峰不在边界时给出可用修正。
7. 写零位操作必须有人工确认和审计日志。

