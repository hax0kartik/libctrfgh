#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>

#include <curl/curl.h>
#include <3ds.h>

static PrintConsole top_screen;

static void wait_for_b(void) {
    do {
        hidScanInput();
    } while (!(hidKeysDown() & KEY_B));
}

static char *result_buf = NULL;
static size_t result_sz = 0;
static size_t result_written = 0;

static size_t handle_data(char *ptr, size_t size, size_t nmemb, void *userdata) {
    (void) userdata;
    const size_t bsz = size*nmemb;

    if (result_sz == 0 || !result_buf) {
        result_sz = 0x1000;
        result_buf = malloc(result_sz);
    }

    bool need_realloc = false;
    while (result_written + bsz > result_sz) {
        result_sz <<= 1;
        need_realloc = true;
    }

    if (need_realloc) {
        char *new_buf = realloc(result_buf, result_sz);
        if (!new_buf) {
            return 0;
        }
        result_buf = new_buf;
    }

    if (!result_buf) {
        return 0;
    }

    memcpy(result_buf + result_written, ptr, bsz);
    result_written += bsz;

    return bsz;
}

int main(int argc, char *argv[]) {
    acInit();
    gfxInitDefault();
    gfxSet3D(false);

    consoleInit(GFX_TOP, &top_screen);
    consoleSetWindow(&top_screen, 0, 0, 50, 30);
    consoleSelect(&top_screen);

    printf("Waiting for WiFi... ");
    acWaitInternetConnection();
    printf("OK\n");

    char cerr[CURL_ERROR_SIZE];
    void *socubuf = memalign(0x1000, 0x100000);
    if (!socubuf) {
        printf("malloc (1) failed\n");
        goto exit;
    }

    if (R_FAILED(socInit(socubuf, 0x100000))) {
        printf("socInit failed\n");
        goto exit;
    }

    printf("Press B to start.\n");
    wait_for_b();

    CURL *hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(hnd, CURLOPT_URL, "https://www.howsmyssl.com/a/check");
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "curl/7.58.0");
    curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, handle_data);
    curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, cerr);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(hnd, CURLOPT_STDERR, stdout);

    cerr[0] = 0;
    CURLcode cres = curl_easy_perform(hnd);
    curl_easy_cleanup(hnd);

    if (cres != CURLE_OK) {
        printf("CURL failed: %s\n", cerr);
        goto exit;
    }

    if (result_sz <= result_written) {
        // less isn't really possible, but hey
        char *new_buf = realloc(result_buf, result_written + 1);
        if (!new_buf) {
            printf("realloc failed\n");
            goto exit;
        }
        result_buf = new_buf;
    }

    result_buf[result_written] = '\0';

    // please forgive my JSON parsing
    char *cur = strstr(result_buf, "\"tls_version\"");
    if (cur) {
        cur += 13 /* strlen("\"tls_version\"") */;
        cur = strchr(cur, '"');
        if (cur) {
            cur += 1;
            char *cur_end = strchr(cur, '"');
            if (cur_end) {
                *cur_end = '\0';
                printf("TLS version: %s\n", cur);
            }
        }
    }

exit:
    printf("Press B to exit.\n");
    wait_for_b();

    socExit();

    if (result_buf) {
        free(result_buf);
    }
    if (socubuf) {
        free(socubuf);
    }

    gfxExit();
    acExit();

    return 0;
}
