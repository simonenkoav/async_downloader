#include <iostream>
#include <list>
#include <algorithm>
#include <curl/curl.h>

#ifdef _WIN32
#define WAITMS(x) Sleep(x)
#else
/* Portable sleep for platforms other than Windows. */
#define WAITMS(x)                               \
  struct timeval wait = { 0, (x) * 1000 };      \
  (void)select(0, NULL, NULL, NULL, &wait);
#endif

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

void initCurlOptions() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *http_handle = curl_easy_init();

    static const char *pagefilename = "image";

    curl_easy_setopt(http_handle, CURLOPT_URL, "https://lh4.ggpht.com/M1XTibfCgpi5pgjSDb7kXDh21N8fpn-8evzQVAX-qGFhSyArDmSuCAv1pjVp4jbAt_g=h900");
    curl_easy_setopt(http_handle, CURLOPT_WRITEFUNCTION, write_data);

    FILE *pagefile;
    pagefile = fopen(pagefilename, "wb");
    if(pagefile) {
        /* write the page body to this file handle */
        curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, pagefile);
    }
    CURLM *multi_handle = curl_multi_init();
    curl_multi_add_handle(multi_handle, http_handle);

    int still_running = -1;
    int repeats = 0;
    curl_multi_perform(multi_handle, &still_running);
    do {
        CURLMcode mc; /* curl_multi_wait() return code */
        int numfds;

        /* wait for activity, timeout or "nothing" */
        mc = curl_multi_wait(multi_handle, NULL, 0, 1000, &numfds);

        if(mc != CURLM_OK) {
            fprintf(stderr, "curl_multi_wait() failed, code %d.\n", mc);
            break;
        }

        /* 'numfds' being zero means either a timeout or no file descriptors to
           wait for. Try timeout on first occurrence, then assume no file
           descriptors and no file descriptors to wait for means wait for 100
           milliseconds. */

        if(!numfds) {
            repeats++; /* count number of repeated zero numfds */
            if(repeats > 1) {
                WAITMS(100); /* sleep 100 milliseconds */
            }
        }
        else
            repeats = 0;

        curl_multi_perform(multi_handle, &still_running);
    } while(still_running);

    curl_multi_remove_handle(multi_handle, http_handle);

    curl_easy_cleanup(http_handle);

    curl_multi_cleanup(multi_handle);

    curl_global_cleanup();

    fclose(pagefile);
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Missing argument" << std::endl << "Usage: async_downloader [URL]...";
    } else {
        initCurlOptions();

        std::list<std::string> requestedUrls;
        for (int i = 1; i < argc; i++) {
            char *url = argv[i];
            auto it = std::find(requestedUrls.begin(), requestedUrls.end(), url);
            if (it == requestedUrls.end()) {
                std::cout << "arg[" << i << "] = " << argv[i] << std::endl;
                //TODO send request
                requestedUrls.push_back(std::string(argv[i]));
            }
        }
    }
    return 0;
}