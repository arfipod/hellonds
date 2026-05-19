// SPDX-License-Identifier: CC0-1.0
//
// Nintendo DSi + TWiLight Menu++ HTTPS REST demo.
// Build with BlocksDS + DSWiFi + libcurl + Mbed TLS.
//
// Install dependencies:
//   wf-pacman -Sy toolchain-gcc-arm-none-eabi-zlib blocksds-mbedtls blocksds-libcurl

#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

#include <nds.h>
#include <dswifi9.h>

#define API_URL "https://httpbin.org/get?hello=nintendo_dsi"

// First hardware demo convenience.
// Set to 0 for real security and provide a proper CA/certificate strategy.
#define DEMO_INSECURE_TLS 1

#define RESPONSE_MAX (32 * 1024)

static PrintConsole topScreen;
static PrintConsole bottomScreen;

static char response_buffer[RESPONSE_MAX];
static size_t response_size = 0;

static void wait_for_start(void)
{
    printf("\nPress START to exit.\n");

    while (1)
    {
        cothread_yield_irq(IRQ_VBLANK);
        scanKeys();

        if (keysDown() & KEY_START)
            break;
    }
}

static bool wait_for_wifi_connection(void)
{
    consoleSelect(&topScreen);
    printf("Waiting for Wi-Fi association...\n");

    int old_status = -1;
    int timeout_frames = 60 * 30; // ~30 seconds

    while (timeout_frames-- > 0)
    {
        cothread_yield_irq(IRQ_VBLANK);

        int status = Wifi_AssocStatus();

        if (status != old_status)
        {
            printf("Wi-Fi: %s\n", ASSOCSTATUS_STRINGS[status]);
            old_status = status;
        }

        if (status == ASSOCSTATUS_ASSOCIATED)
            return true;

        if (status == ASSOCSTATUS_CANNOTCONNECT)
            return false;
    }

    return false;
}

static size_t curl_write_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    (void)userdata;

    size_t incoming = size * nmemb;
    size_t available = RESPONSE_MAX - 1 - response_size;

    if (incoming > available)
        incoming = available;

    if (incoming > 0)
    {
        memcpy(&response_buffer[response_size], ptr, incoming);
        response_size += incoming;
        response_buffer[response_size] = '\0';

        consoleSelect(&bottomScreen);
        printf("%.*s", (int)incoming, ptr);
        consoleSelect(&topScreen);
    }

    cothread_yield();

    return size * nmemb;
}

static int https_get_with_curl(const char *url)
{
    CURLcode rc;

    response_size = 0;
    response_buffer[0] = '\0';

    printf("\nInitializing curl...\n");

    rc = curl_global_init(CURL_GLOBAL_ALL);
    if (rc != CURLE_OK)
    {
        printf("curl_global_init() failed: %d\n", (int)rc);
        return -1;
    }

    CURL *curl = curl_easy_init();
    if (curl == NULL)
    {
        printf("curl_easy_init() failed.\n");
        curl_global_cleanup();
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Nintendo-DSi-BlocksDS/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

#if DEMO_INSECURE_TLS
    // Demo only: makes first hardware test easier.
    // Do not use this for private APIs, tokens, passwords, or production.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
#else
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
#endif

    consoleSelect(&bottomScreen);
    consoleClear();
    printf("HTTPS response:\n\n");
    consoleSelect(&topScreen);

    printf("Performing HTTPS GET...\n");

    rc = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (rc != CURLE_OK)
    {
        printf("curl failed: %s\n", curl_easy_strerror(rc));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }

    printf("HTTP status: %ld\n", http_code);
    printf("Received: %lu bytes\n", (unsigned long)response_size);

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return (int)response_size;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    defaultExceptionHandler();

    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);

    consoleInit(&topScreen, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
    consoleInit(&bottomScreen, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 3, false, true);

    consoleSelect(&topScreen);
    consoleClear();

    printf("DSi REST HTTPS/libcurl demo\n");
    printf("Target:\n%s\n\n", API_URL);
    printf("Initializing Wi-Fi...\n");

    if (!Wifi_InitDefault(WFC_CONNECT | WIFI_ATTEMPT_DSI_MODE))
    {
        printf("Wifi_InitDefault() failed.\n");
        wait_for_start();
        return 1;
    }

    printf("Wi-Fi initialized.\n");

    if (!wait_for_wifi_connection())
    {
        printf("Wi-Fi connection failed.\n");
        Wifi_DisableWifi();
        wait_for_start();
        return 1;
    }

    printf("Wi-Fi associated.\n");

    struct in_addr gateway = {0}, mask = {0}, dns1 = {0}, dns2 = {0};
    struct in_addr ip = Wifi_GetIPInfo(&gateway, &mask, &dns1, &dns2);

    printf("IP:  %s\n", inet_ntoa(ip));
    printf("DNS: %s\n", inet_ntoa(dns1));

#if DEMO_INSECURE_TLS
    printf("\nWARNING: TLS verification disabled.\n");
#endif

    https_get_with_curl(API_URL);

    if (Wifi_DisconnectAP() != 0)
        printf("Wifi_DisconnectAP() returned error.\n");

    Wifi_DisableWifi();

    wait_for_start();
    return 0;
}
