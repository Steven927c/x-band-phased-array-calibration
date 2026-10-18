# X-Band Phased-Array Metal-Sphere and Solar Calibration

This repository contains device-independent Qt/C++ algorithms extracted and
reorganized from practical weather-radar calibration work:

- UAV-suspended metal-sphere geometry, optical-region RCS, expected equivalent
  reflectivity, target range-gate triplets, and RHI/PPI peak association.
- Solar position, atmospheric-refraction correction, linear-power smoothing,
  adaptive peak detection, centroid fitting, and pointing-error estimation.
- A hardware-neutral `CalibrationDevice` interface, a Qt Widgets demonstration,
  and core tests.

The public code intentionally excludes proprietary DSP/DBF/servo command
layouts, addresses, field parameters, captures, logs, and production file
extensions. Demonstration samples are explicitly synthetic.

## Build

Requirements: C++17, CMake 3.16+, and Qt 5.12+ or Qt 6.x.

```bash
cmake -S . -B build -DRCT_BUILD_DEMO=ON -DRCT_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure -C Release
```

See the Chinese [README](README.md), [architecture](docs/architecture.zh-CN.md),
and [integration guide](docs/integration.zh-CN.md) for details.

## License

MIT. See [LICENSE](LICENSE).

