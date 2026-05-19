# hellonds
A reference project for Nintendo DS and DSi development


```text
BLOCKSDS=/opt/wonderful/thirdparty/blocksds/core
BLOCKSDSEXT=/opt/wonderful/thirdparty/blocksds/external
```

You have already verified that the official BlocksDS DSWiFi example builds:

```bash
cd /opt/wonderful/thirdparty/blocksds/core/examples/dswifi/get_website
make clean
make
```

So your development environment is ready.

---

## Package contents

```text
dsi_twilight_rest_package/
  README.md
  scripts/
    verify_blocksds_env.sh
    build_all.sh
  dsi_rest_http/
    Makefile
    source/main.c
  dsi_rest_https_curl/
    Makefile
    source/main.c
```

There are two independent projects:

1. `dsi_rest_http`
   - Uses `libnds` + `dswifi`.
   - Performs a plain HTTP GET request using sockets.
   - Best first hardware test.
   - Does not require extra packages beyond `blocksds-toolchain`.

2. `dsi_rest_https_curl`
   - Uses `libnds` + `dswifi` + `libcurl` + `mbedtls` + `zlib`.
   - Performs an HTTPS GET request.
   - More useful for modern REST APIs.
   - Requires extra packages.

3. `flappy_test`
   - Uses `libnds`.
   - Complete Flappy Bird-style game for Nintendo DS.
   - Gameplay is on the top screen.
   - The lower touch screen is used to flap and shows status text.
   - Does not require extra packages beyond `blocksds-toolchain`.

---

## Important concepts

`libnds` gives you Nintendo DS/DSi runtime, video, input, console, interrupt, timer, and low-level support.

For Wi-Fi, you also need `DSWiFi`.

For HTTP REST:
- DSWiFi is enough.
- You use normal C socket functions: `getaddrinfo()`, `socket()`, `connect()`, `write()`, `read()`, `close()`.

For HTTPS REST:
- DSWiFi alone is not enough.
- Use `blocksds-mbedtls` or `blocksds-libcurl`.
- This package uses libcurl because it is simpler for REST APIs.

---

## 1. Verify environment

From this package root:

```bash
./scripts/verify_blocksds_env.sh
```

Expected result:
- It prints `BLOCKSDS` and `BLOCKSDSEXT`.
- It finds `arm7_dswifi.elf`.
- It finds `libdswifi9.a`.
- It finds `libnds9.a`.
- It should not report fatal errors.

---

## 2. Build the plain HTTP project

```bash
cd dsi_rest_http
make clean
make
```

Expected output:

```text
dsi_rest_http.nds
```

Copy `dsi_rest_http.nds` to your DSi SD card and launch it from TWiLight Menu++.

The default request is:

```text
http://httpbin.org/get?hello=nintendo_dsi
```

Change it in `dsi_rest_http/source/main.c`:

```c
#define API_HOST "httpbin.org"
#define API_PORT "80"
#define API_PATH "/get?hello=nintendo_dsi"
```

---

## 3. Install HTTPS dependencies

For HTTPS/libcurl:

```bash
wf-pacman -Sy toolchain-gcc-arm-none-eabi-zlib blocksds-mbedtls blocksds-libcurl
```

Then build:

```bash
cd dsi_rest_https_curl
make clean
make
```

Expected output:

```text
dsi_rest_https_curl.nds
```

The default request is:

```text
https://httpbin.org/get?hello=nintendo_dsi
```

Change it in `dsi_rest_https_curl/source/main.c`:

```c
#define API_URL "https://httpbin.org/get?hello=nintendo_dsi"
```

---

## 4. Build both projects from root

From this package root:

```bash
./scripts/build_all.sh
```

The script builds HTTP first, then `flappy_test`, then HTTPS if libcurl/Mbed
TLS/zlib are installed.

---

## 4a. Build the Flappy Test game

```bash
cd flappy_test
make clean
make
```

Expected output:

```text
flappy_test.nds
```

Copy `flappy_test.nds` to your Nintendo DS or DSi SD card and launch it from
TWiLight Menu++ or another homebrew loader.

---

## 5. DSi / TWiLight Menu++ notes

For WPA/WPA2 networks, launch the ROM in DSi mode. DSWiFi can attempt DSi Wi-Fi mode with:

```c
Wifi_InitDefault(WFC_CONNECT | WIFI_ATTEMPT_DSI_MODE);
```

The examples use that.

Your DSi should already have a working Wi-Fi profile configured in system settings. The examples use `WFC_CONNECT`, which means "connect using firmware-stored access point settings".

If your network fails:
- First test with the official BlocksDS `examples/dswifi/get_website`.
- Try a 2.4 GHz Wi-Fi network.
- Avoid hidden SSIDs for first tests.
- If needed, temporarily test with a phone hotspot.
- Remember that DS mode only supports open/WEP, while DSi mode supports WPA/WPA2 through DSWiFi.

---

## 6. Security note for HTTPS demo

The HTTPS/libcurl demo intentionally disables certificate verification:

```c
#define DEMO_INSECURE_TLS 1
```

This makes the first test easier because you don't need to bundle a CA store.

Do not use this mode for tokens, passwords, private APIs, or anything sensitive.

For production:
- Set `DEMO_INSECURE_TLS` to `0`.
- Bundle certificates with NitroFS, or use certificate pinning.
- Use short responses and defensive parsing because the console has limited RAM.

---

## 7. Suggested next development steps

After this works on hardware:

1. Replace `httpbin.org` with your own small REST endpoint.
2. Keep the API response small, ideally under a few KB.
3. Add JSON parsing only after connectivity works.
4. Add a simple menu to configure host/path or API key.
5. Store configuration in SD/NitroFS later if needed.
6. Keep network code non-blocking or at least user-cancellable for real apps.

---

## 8. Typical commands

```bash
# Load environment if needed
source /opt/wonderful/bin/wf-env

# Build HTTP
cd dsi_rest_http
make clean && make

# Build HTTPS
cd ../dsi_rest_https_curl
make clean && make

# Clean both
cd ..
(cd dsi_rest_http && make clean)
(cd dsi_rest_https_curl && make clean)
```
