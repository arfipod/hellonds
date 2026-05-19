#!/usr/bin/env bash
set -euo pipefail

if [ -f /opt/wonderful/bin/wf-env ]; then
  # shellcheck disable=SC1091
  source /opt/wonderful/bin/wf-env
fi

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "== Building plain HTTP project =="
cd "$ROOT/dsi_rest_http"
make clean
make

echo
echo "== Checking HTTPS dependencies =="
HAVE_CURL=0
HAVE_MBEDTLS=0
HAVE_ZLIB=0

if [ -n "${BLOCKSDSEXT:-}" ] && find "$BLOCKSDSEXT" -name 'libcurl.a' -print -quit 2>/dev/null | grep -q .; then
  HAVE_CURL=1
fi

if [ -n "${BLOCKSDSEXT:-}" ] && find "$BLOCKSDSEXT" -name 'libmbedtls.a' -print -quit 2>/dev/null | grep -q .; then
  HAVE_MBEDTLS=1
fi

if find /opt/wonderful/toolchain/gcc-arm-none-eabi/arm-none-eabi/lib -name 'libz.a' -print -quit 2>/dev/null | grep -q .; then
  HAVE_ZLIB=1
fi

if [ "$HAVE_CURL" -eq 1 ] && [ "$HAVE_MBEDTLS" -eq 1 ] && [ "$HAVE_ZLIB" -eq 1 ]; then
  echo "== Building HTTPS/libcurl project =="
  cd "$ROOT/dsi_rest_https_curl"
  make clean
  make
else
  echo "Skipping HTTPS/libcurl build because dependencies are missing."
  echo "Install them with:"
  echo "  wf-pacman -Sy toolchain-gcc-arm-none-eabi-zlib blocksds-mbedtls blocksds-libcurl"
fi

echo
echo "Done."
