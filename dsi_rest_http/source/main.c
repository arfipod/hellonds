// SPDX-License-Identifier: CC0-1.0
//
// Nintendo DSi + TWiLight Menu++ plain HTTP REST demo.
// Build with BlocksDS. Launch in DSi mode for WPA/WPA2 Wi-Fi support.
//
// This intentionally uses plain HTTP so it needs only DSWiFi + libnds.
// For real public APIs, prefer the HTTPS/libcurl example.

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <nds.h>
#include <dswifi9.h>

#define API_HOST "httpbin.org"
#define API_PORT "80"
#define API_PATH "/get?hello=nintendo_dsi"

#define RESPONSE_BUFFER_SIZE (32 * 1024)
#define READ_CHUNK_SIZE 512

static PrintConsole topScreen;
static PrintConsole bottomScreen;
static char response_buffer[RESPONSE_BUFFER_SIZE];

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

static int http_get(const char *host, const char *port, const char *path)
{
    consoleSelect(&topScreen);
    printf("\nResolving %s:%s...\n", host, port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = AF_INET;       // Keep first demo simple and IPv4-only.
    hints.ai_socktype = SOCK_STREAM; // TCP

    struct addrinfo *result = NULL;
    int err = getaddrinfo(host, port, &hints, &result);
    if (err != 0)
    {
        printf("getaddrinfo() failed: %d\n", err);
        return -1;
    }

    int sfd = -1;
    struct addrinfo *rp = NULL;

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd < 0)
            continue;

        printf("Connecting...\n");

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(sfd);
        sfd = -1;
    }

    freeaddrinfo(result);

    if (sfd < 0)
    {
        printf("Could not connect to server.\n");
        return -1;
    }

    printf("Connected. Sending HTTP GET...\n");

    char request[768];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: Nintendo-DSi-BlocksDS\r\n"
             "Accept: application/json,text/plain,*/*\r\n"
             "Connection: close\r\n"
             "\r\n",
             path, host);

    ssize_t sent = write(sfd, request, strlen(request));
    if (sent < 0)
    {
        printf("write() failed.\n");
        close(sfd);
        return -1;
    }

    printf("Request sent: %ld bytes\n", (long)sent);

    int opt = 1;
    if (ioctl(sfd, FIONBIO, (char *)&opt) < 0)
    {
        printf("Could not set non-blocking mode.\n");
        close(sfd);
        return -1;
    }

    consoleSelect(&bottomScreen);
    consoleClear();
    printf("Response from http://%s%s\n\n", host, path);

    int total = 0;
    int idle_frames = 0;

    while (total < RESPONSE_BUFFER_SIZE - 1)
    {
        ssize_t n = read(sfd, &response_buffer[total], READ_CHUNK_SIZE);

        if (n > 0)
        {
            total += (int)n;
            response_buffer[total] = '\0';
            idle_frames = 0;

            // Stream a compact preview to the lower screen.
            if (total < 4096)
                printf("%.*s", (int)n, &response_buffer[total - n]);
        }
        else if (n == 0)
        {
            break; // Server closed connection.
        }
        else
        {
            // Non-blocking socket: no data right now. Yield to DSWiFi/lwIP.
            idle_frames++;
            if (idle_frames > 60 * 10) // ~10 seconds without data.
            {
                printf("\n\nTimeout waiting for response.\n");
                break;
            }
        }

        cothread_yield();
    }

    response_buffer[total] = '\0';

    shutdown(sfd, 0);
    close(sfd);

    consoleSelect(&topScreen);
    printf("Received %d bytes.\n", total);

    return total;
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

    printf("DSi REST HTTP demo\n");
    printf("Target: http://%s%s\n\n", API_HOST, API_PATH);
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

    http_get(API_HOST, API_PORT, API_PATH);

    if (Wifi_DisconnectAP() != 0)
        printf("Wifi_DisconnectAP() returned error.\n");

    Wifi_DisableWifi();

    wait_for_start();
    return 0;
}
