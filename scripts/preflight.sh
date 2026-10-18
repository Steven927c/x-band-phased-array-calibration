#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

if find . -path './build*' -prune -o -type f \
    \( -name '*.AR2' -o -name '*.log' -o -name '*.bin' -o -name '*.key' -o -name '*.pem' \) \
    -print | grep -q .; then
  echo "Forbidden field-data or credential file found." >&2
  exit 1
fi

if grep -RInE --exclude-dir='build*' --exclude='preflight.sh' --exclude='preflight.ps1' \
    'BEGIN (RSA|OPENSSH|EC) PRIVATE KEY|AKIA[0-9A-Z]{16}|192\.168\.|g_Radar_Constant|g_Servo.*Code|Cmd\[' .; then
  echo "Potential private content found. Review before publishing." >&2
  exit 1
fi

cmake -S . -B build-preflight -DRCT_BUILD_DEMO=ON -DRCT_BUILD_TESTS=ON
cmake --build build-preflight
ctest --test-dir build-preflight --output-on-failure

git diff --check || true
git status --short || true
echo "Preflight completed. Manually finish OPEN_SOURCE_CHECKLIST.md before publishing."

