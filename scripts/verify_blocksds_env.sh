#!/usr/bin/env bash
set -u

echo "== BlocksDS environment verification =="
echo

if [ -f /opt/wonderful/bin/wf-env ]; then
  # shellcheck disable=SC1091
  source /opt/wonderful/bin/wf-env
fi

echo "PATH=$PATH"
echo "BLOCKSDS=${BLOCKSDS:-}"
echo "BLOCKSDSEXT=${BLOCKSDSEXT:-}"
echo

fail=0

check_path() {
  local p="$1"
  local label="$2"
  if [ -e "$p" ]; then
    echo "[OK] $label: $p"
  else
    echo "[FAIL] $label missing: $p"
    fail=1
  fi
}

check_cmd() {
  local c="$1"
  if command -v "$c" >/dev/null 2>&1; then
    echo "[OK] command found: $c -> $(command -v "$c")"
  else
    echo "[FAIL] command missing: $c"
    fail=1
  fi
}

check_cmd make
check_cmd wf-pacman
check_cmd arm-none-eabi-gcc

if [ -z "${BLOCKSDS:-}" ]; then
  echo "[FAIL] BLOCKSDS is not set"
  fail=1
else
  check_path "$BLOCKSDS" "BLOCKSDS root"
  check_path "$BLOCKSDS/sys/arm7/main_core/arm7_dswifi.elf" "DSWiFi ARM7 core"
  check_path "$BLOCKSDS/libs/dswifi/lib/libdswifi9.a" "DSWiFi ARM9 library"
  check_path "$BLOCKSDS/libs/libnds/lib/libnds9.a" "libnds ARM9 library"
fi

echo
echo "Optional HTTPS dependencies:"
if [ -n "${BLOCKSDSEXT:-}" ]; then
  find "$BLOCKSDSEXT" -name 'libcurl.a' -print -quit 2>/dev/null | grep -q . \
    && echo "[OK] libcurl found" || echo "[INFO] libcurl not found; install: wf-pacman -Sy blocksds-libcurl"
  find "$BLOCKSDSEXT" -name 'libmbedtls.a' -print -quit 2>/dev/null | grep -q . \
    && echo "[OK] mbedtls found" || echo "[INFO] mbedtls not found; install: wf-pacman -Sy blocksds-mbedtls"
else
  echo "[INFO] BLOCKSDSEXT not set"
fi

find /opt/wonderful/toolchain/gcc-arm-none-eabi/arm-none-eabi/lib -name 'libz.a' -print -quit 2>/dev/null | grep -q . \
  && echo "[OK] zlib found" || echo "[INFO] zlib not found; install: wf-pacman -Sy toolchain-gcc-arm-none-eabi-zlib"

echo
if [ "$fail" -eq 0 ]; then
  echo "Environment looks ready for the HTTP DSi REST project."
else
  echo "Some required items are missing."
fi

exit "$fail"
