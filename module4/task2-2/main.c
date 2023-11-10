#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void callback_function(char* ptr, size_t chunk_size, size_t nmemb)
{
    size_t total_size = chunk_size * nmemb;
    char* title_start = strstr(ptr, "<title>") + sizeof("<title>") - 1;
    char* title_end = strstr(ptr, "</title>");
    write(STDOUT_FILENO, title_start, title_end - title_start);
}

int main(int argc, char* argv[])
{
    const char* url = argv[1];
    CURL* curl = curl_easy_init();
    if (!curl) {
        printf("Can't initialize curl\n");
        exit(-1);
    }
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_function);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    exit(res);
}