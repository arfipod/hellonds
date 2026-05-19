#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

usage() {
  cat <<'USAGE'
Usage:
  scripts/capture_melonds.sh [--output path.png] [--delay seconds] gospel_nds
  scripts/capture_melonds.sh [--output path.png] [--delay seconds] flappy_test
  scripts/capture_melonds.sh [--output path.png] [--delay seconds] path/to/file.nds

Captures the melonDS window as a PNG. The script forces Qt to use XCB because
native Wayland windows are not visible to simple X11 capture code.
USAGE
}

output=""
delay="2"

while [ $# -gt 0 ]; do
  case "$1" in
    --output)
      if [ -z "${2:-}" ]; then
        echo "--output requires a path" >&2
        exit 2
      fi
      output="$2"
      shift 2
      ;;
    --delay)
      if [ -z "${2:-}" ]; then
        echo "--delay requires a number of seconds" >&2
        exit 2
      fi
      delay="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      break
      ;;
    -*)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
    *)
      break
      ;;
  esac
done

target="${1:-gospel_nds}"

if [ -z "$output" ]; then
  target_name="$(basename "$target" .nds)"
  output="$ROOT/captures/${target_name}_melonds.png"
fi

mkdir -p "$(dirname "$output")"
log_file="$(mktemp)"
launch_pid=""

cleanup() {
  if [ -n "$launch_pid" ] && kill -0 "$launch_pid" >/dev/null 2>&1; then
    kill -TERM "$launch_pid" >/dev/null 2>&1 || true
  fi
  if pgrep -f "melonDS.*$target" >/dev/null 2>&1; then
    pkill -TERM -f "melonDS.*$target" >/dev/null 2>&1 || true
    sleep 0.2
  fi
  if pgrep -f "melonDS.*$target" >/dev/null 2>&1; then
    pkill -KILL -f "melonDS.*$target" >/dev/null 2>&1 || true
  fi
  rm -f "$log_file"
}
trap cleanup EXIT

env QT_QPA_PLATFORM=xcb "$ROOT/scripts/run_melonds.sh" --foreground "$target" >"$log_file" 2>&1 &
launch_pid="$!"

python3 - "$output" "$delay" <<'PY'
import binascii
import ctypes
import os
import struct
import sys
import time
import zlib

out = sys.argv[1]
delay = float(sys.argv[2])
deadline = time.time() + max(delay, 0.0) + 15.0

lib = ctypes.cdll.LoadLibrary("libX11.so.6")


class XImage(ctypes.Structure):
    _fields_ = [
        ("width", ctypes.c_int),
        ("height", ctypes.c_int),
        ("xoffset", ctypes.c_int),
        ("format", ctypes.c_int),
        ("data", ctypes.c_void_p),
        ("byte_order", ctypes.c_int),
        ("bitmap_unit", ctypes.c_int),
        ("bitmap_bit_order", ctypes.c_int),
        ("bitmap_pad", ctypes.c_int),
        ("depth", ctypes.c_int),
        ("bytes_per_line", ctypes.c_int),
        ("bits_per_pixel", ctypes.c_int),
        ("red_mask", ctypes.c_ulong),
        ("green_mask", ctypes.c_ulong),
        ("blue_mask", ctypes.c_ulong),
        ("obdata", ctypes.c_void_p),
        ("f", ctypes.c_void_p),
    ]


class XWindowAttributes(ctypes.Structure):
    _fields_ = [
        ("x", ctypes.c_int),
        ("y", ctypes.c_int),
        ("width", ctypes.c_int),
        ("height", ctypes.c_int),
        ("border_width", ctypes.c_int),
        ("depth", ctypes.c_int),
        ("visual", ctypes.c_void_p),
        ("root", ctypes.c_ulong),
        ("class", ctypes.c_int),
        ("bit_gravity", ctypes.c_int),
        ("win_gravity", ctypes.c_int),
        ("backing_store", ctypes.c_int),
        ("backing_planes", ctypes.c_ulong),
        ("backing_pixel", ctypes.c_ulong),
        ("save_under", ctypes.c_int),
        ("colormap", ctypes.c_ulong),
        ("map_installed", ctypes.c_int),
        ("map_state", ctypes.c_int),
        ("all_event_masks", ctypes.c_long),
        ("your_event_mask", ctypes.c_long),
        ("do_not_propagate_mask", ctypes.c_long),
        ("override_redirect", ctypes.c_int),
        ("screen", ctypes.c_void_p),
    ]


lib.XOpenDisplay.argtypes = [ctypes.c_char_p]
lib.XOpenDisplay.restype = ctypes.c_void_p
lib.XDefaultRootWindow.argtypes = [ctypes.c_void_p]
lib.XDefaultRootWindow.restype = ctypes.c_ulong
lib.XQueryTree.argtypes = [
    ctypes.c_void_p,
    ctypes.c_ulong,
    ctypes.POINTER(ctypes.c_ulong),
    ctypes.POINTER(ctypes.c_ulong),
    ctypes.POINTER(ctypes.POINTER(ctypes.c_ulong)),
    ctypes.POINTER(ctypes.c_uint),
]
lib.XQueryTree.restype = ctypes.c_int
lib.XFetchName.argtypes = [ctypes.c_void_p, ctypes.c_ulong, ctypes.POINTER(ctypes.c_char_p)]
lib.XFetchName.restype = ctypes.c_int
lib.XGetWindowAttributes.argtypes = [ctypes.c_void_p, ctypes.c_ulong, ctypes.POINTER(XWindowAttributes)]
lib.XGetWindowAttributes.restype = ctypes.c_int
lib.XGetImage.argtypes = [
    ctypes.c_void_p,
    ctypes.c_ulong,
    ctypes.c_int,
    ctypes.c_int,
    ctypes.c_uint,
    ctypes.c_uint,
    ctypes.c_ulong,
    ctypes.c_int,
]
lib.XGetImage.restype = ctypes.POINTER(XImage)
lib.XGetPixel.argtypes = [ctypes.POINTER(XImage), ctypes.c_int, ctypes.c_int]
lib.XGetPixel.restype = ctypes.c_ulong
lib.XDestroyImage.argtypes = [ctypes.POINTER(XImage)]
lib.XCloseDisplay.argtypes = [ctypes.c_void_p]
lib.XFree.argtypes = [ctypes.c_void_p]


def find_melonds_window(display, root):
    found = []

    def walk(window, depth=0):
        name = ctypes.c_char_p()
        if lib.XFetchName(display, window, ctypes.byref(name)) and name.value:
            title = name.value.decode(errors="replace")
            if "melonDS" in title and "Selection Owner" not in title:
                attrs = XWindowAttributes()
                if lib.XGetWindowAttributes(display, window, ctypes.byref(attrs)):
                    if attrs.map_state == 2 and attrs.width > 100 and attrs.height > 100:
                        found.append((depth, attrs.width * attrs.height, window))
            lib.XFree(name)

        root_return = ctypes.c_ulong()
        parent_return = ctypes.c_ulong()
        children = ctypes.POINTER(ctypes.c_ulong)()
        nchildren = ctypes.c_uint()
        if lib.XQueryTree(
            display,
            window,
            ctypes.byref(root_return),
            ctypes.byref(parent_return),
            ctypes.byref(children),
            ctypes.byref(nchildren),
        ):
            for index in range(nchildren.value):
                walk(children[index], depth + 1)
            if children:
                lib.XFree(children)

    walk(root)
    if not found:
        return None
    found.sort(reverse=True)
    return found[0][2]


def scale_channel(pixel, mask):
    if not mask:
        return 0
    shift = (mask & -mask).bit_length() - 1
    maximum = mask >> shift
    return int(((pixel & mask) >> shift) * 255 / maximum)


def png_chunk(kind, data):
    return (
        struct.pack(">I", len(data))
        + kind
        + data
        + struct.pack(">I", binascii.crc32(kind + data) & 0xFFFFFFFF)
    )


display_name = os.environ.get("DISPLAY", ":0").encode()
display = lib.XOpenDisplay(display_name)
if not display:
    raise SystemExit("Could not open X display")

root = lib.XDefaultRootWindow(display)
window = None
while time.time() < deadline:
    window = find_melonds_window(display, root)
    if window:
        break
    time.sleep(0.25)

if not window:
    lib.XCloseDisplay(display)
    raise SystemExit("Could not find a visible melonDS X11 window")

time.sleep(max(delay, 0.0))

attrs = XWindowAttributes()
if not lib.XGetWindowAttributes(display, window, ctypes.byref(attrs)):
    lib.XCloseDisplay(display)
    raise SystemExit("Could not inspect the melonDS window")

image = lib.XGetImage(display, window, 0, 0, attrs.width, attrs.height, (1 << 32) - 1, 2)
if not image:
    lib.XCloseDisplay(display)
    raise SystemExit("Could not capture the melonDS window")

masks = (image.contents.red_mask, image.contents.green_mask, image.contents.blue_mask)
raw = bytearray()
for y in range(attrs.height):
    raw.append(0)
    for x in range(attrs.width):
        pixel = lib.XGetPixel(image, x, y)
        raw.extend(
            (
                scale_channel(pixel, masks[0]),
                scale_channel(pixel, masks[1]),
                scale_channel(pixel, masks[2]),
            )
        )

with open(out, "wb") as handle:
    handle.write(b"\x89PNG\r\n\x1a\n")
    handle.write(png_chunk(b"IHDR", struct.pack(">IIBBBBB", attrs.width, attrs.height, 8, 2, 0, 0, 0)))
    handle.write(png_chunk(b"IDAT", zlib.compress(bytes(raw), 6)))
    handle.write(png_chunk(b"IEND", b""))

lib.XDestroyImage(image)
lib.XCloseDisplay(display)
print(f"Captured {attrs.width}x{attrs.height}: {out}")
PY

echo "melonDS log:"
sed -n '1,80p' "$log_file"
