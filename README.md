# X波段相控阵金属球及太阳法标定

[![CI](https://github.com/Steven927c/x-band-phased-array-calibration/actions/workflows/build.yml/badge.svg)](https://github.com/Steven927c/x-band-phased-array-calibration/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**X-Band Phased-Array Metal-Sphere and Solar Calibration**

一个从工程实践中提取、重新组织并去除设备私有协议依赖的 Qt/C++ 雷达标定工具包，包含：

- 无人机吊挂金属球标定：几何计算、光学区 RCS、理论等效反射率、距离库定位、`T-1/T/T+1` 提取、RHI/PPI 峰识别。
- 太阳法定北：太阳理论方位/俯仰计算、大气折射修正、线性功率平滑、自适应门限和峰值质心拟合。
- 设备适配接口：主程序自行连接伺服、DSP、DBF 或回放数据，核心算法不依赖任何命令字布局。
- Qt Widgets 演示程序和无需硬件的核心测试。

> 本仓库是算法与集成示例，不是安全关键设备的完整控制软件。演示数据明确为合成数据；真实标定结果必须由有资质的人员结合现场条件复核。

## 目录结构

```text
app/                    Qt Widgets 独立演示程序
src/core/               金属球和太阳法核心算法
src/device/             与真实设备对接的抽象接口
src/widgets/            无第三方依赖的轻量绘图控件
tests/                  核心算法测试
examples/               设备适配示例
docs/                   架构、集成和发布说明
.github/                GitHub Actions 与协作模板
```

## 构建

要求：C++17、CMake 3.16+、Qt 5.12+ 或 Qt 6.x。

```bash
cmake -S . -B build -DRCT_BUILD_DEMO=ON -DRCT_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure -C Release
```

Qt 5/qmake 的快速测试构建：

```bash
qmake radar-calibration-toolkit.pro
make        # Windows MinGW 可用 mingw32-make
./radar-calibration-tests
```

qmake 构建演示程序：

```bash
qmake radar-calibration-demo.pro
make
./radar-calibration-demo
```

## 最小用法

```cpp
#include "core/metal_ball_calibrator.h"

radarcal::MetalBallConfig config;
config.radar = {30.50, 103.95, 502.0};
config.uav = {30.51, 103.95, 620.0};
config.ropeLengthM = 20.0;
config.sphereDiameterCm = 30.0;
config.frequencyGHz = 9.4;

const auto geometry = radarcal::MetalBallCalibrator::calculateGeometry(config);
// geometry.azimuthDeg / elevationDeg / slantRangeM / equivalentReflectivityDbz
```

太阳位置：

```cpp
#include "core/solar_calibrator.h"

radarcal::GeoPoint station{30.50, 103.95, 502.0};
const auto sun = radarcal::SolarCalibrator::calculatePosition(
    QDateTime::currentDateTime(), station);
```

真实设备接入见 [docs/integration.zh-CN.md](docs/integration.zh-CN.md)。

## 重要算法约定

- 经纬度采用 WGS-84 语义，角度单位为度，高度和距离单位为米。
- 金属球位置由无人机高度减吊绳长度得到；地面距离与斜距分开计算。
- `T` 是目标距离库，`T-1`/`T+1` 是相邻距离库，不做复制或补造。
- RHI 识别优先选择金属球理论俯仰附近的低侧峰；不强制要求无人机峰必定分离。
- 太阳峰先在相邻三点的线性功率域平滑，再在主峰连续区域按超噪声功率求质心，不直接采用孤立最大点。
- 推荐修正量为测量误差的相反数。真正写入伺服零位前必须由宿主应用再次确认。

## 从原工程拆分时做了什么

本项目保留了两个模块中的通用数学和判定逻辑，并做了以下开源化处理：

- 删除内部 DSP/DBF 命令字偏移、网络地址、全局变量和主窗口依赖。
- 以 `CalibrationDevice` 抽象设备动作与实时观测。
- 使用自有 `QPainter` 绘图控件，未复制 QCustomPlot 等第三方源码。
- 不包含现场参数、日志、原始扫描文件、测试回放或模拟真实业务的隐藏开关。
- 所有演示数据均明确标识为 synthetic，仅用于验证 UI 和算法调用。

## 贡献、安全与许可

- 贡献规范：[CONTRIBUTING.md](CONTRIBUTING.md)
- 安全问题：[SECURITY.md](SECURITY.md)
- 发布前检查：[OPEN_SOURCE_CHECKLIST.md](OPEN_SOURCE_CHECKLIST.md)
- 原工程职责映射：[docs/source-map.zh-CN.md](docs/source-map.zh-CN.md)
- GitHub 发布流程：[docs/publishing.zh-CN.md](docs/publishing.zh-CN.md)
- 许可证：[MIT](LICENSE)

