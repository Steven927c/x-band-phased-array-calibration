# 原工程到开源工程的对应关系

| 原模块职责 | 开源位置 | 处理方式 |
|---|---|---|
| 金属球经纬高、RCS、期望反射率 | `src/core/metal_ball_calibrator.*` | 保留算法，统一单位和输入结构 |
| 目标距离库及 T-1/T/T+1 | `MetalBallCalibrator::expectedRangeBin/extractTriplet` | 去除 DSP 缓冲区偏移依赖 |
| RHI 金属球/无人机峰识别 | `selectRhiSpherePeak` | 保留理论角关联、宽峰兼容和稳定峰判定 |
| PPI 扇区峰识别 | `selectPpiPeak` | 增加过北向角度展开 |
| 太阳理论位置 | `src/core/solar_calibrator.*` | 保留 UTC 与折射修正 |
| 太阳功率峰拟合 | `SolarCalibrator::fitPeak` | 保留线性功率平滑、自适应门限和质心 |
| 伺服/DSP/DBF 命令 | `src/device/calibration_device.h` | 改为抽象接口，不公开私有协议 |
| 原 `.ui` 和 QCustomPlot 页面 | `app/`、`src/widgets/` | 用独立程序化 Qt UI 和自有轻量绘图替代 |
| 参数文件、AR2、日志 | 不包含 | 防止泄露现场和内部格式；由宿主适配 |

本仓库不是把生产文件机械复制后改到“勉强能编译”，而是将两个模块的可复用边界重新建立。这样公共代码可以被测试、复用和审阅，生产设备细节仍留在私有仓库。

