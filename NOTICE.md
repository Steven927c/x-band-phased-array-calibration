# Source provenance and scope

This project was extracted and reorganized in 2026 from two calibration modules
in a larger Qt radar application: UAV-suspended metal-sphere calibration and
solar pointing calibration.

The public repository contains newly separated, hardware-neutral implementations
of the reusable mathematics, peak-selection logic, data types, integration
boundary, tests, and demonstration UI. It intentionally does not contain the
original application's proprietary command layouts, global state, device
addresses, customer data, field captures, parameters, or bundled third-party
plotting source.

The MIT license applies only to files published in this repository. It does not
grant rights to any external radar firmware, protocol, data file, paper,
standard, SDK, or dependency that is not included here.

