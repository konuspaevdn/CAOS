#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int PrintResponseContent(int sock)
{
    char buffer = '~';
    while (' ' != buffer) { // reading "HTTP/1.1"
        recv(sock, &buffer, sizeof(buffer), 0);
    }
    char status_code_ch[4] = {};
    for (size_t i = 0; i < 3; ++i) {
        recv(sock, &status_code_ch[i], sizeof(status_code_ch[i]), 0);
    }
    int status_code = (int)strtol(status_code_ch, NULL, 10);
    buffer = '~';
    size_t line_length = 0;
    while (1) // reading status code remnants and headers till an empty line
    {
        recv(sock, &buffer, sizeof(buffer), 0);
        if (buffer == '\n') {
            if (0 == line_length) {
                break;
            } else {
                line_length = 0;
                continue;
            }
        }
        if (buffer != '\r') {
            ++line_length;
        } else {
            recv(sock, &buffer, sizeof(buffer), 0);
            if (buffer == '\n') {
                if (0 == line_length) {
                    break;
                } else {
                    line_length = 0;
                    continue;
                }
            }
        }
    }

    // the actual content of the requested file
    char response[65536];
    ssize_t bytes_read = 0;
    while (0 < (bytes_read = recv(sock, &response, sizeof(response), 0))) {
        write(STDOUT_FILENO, response, bytes_read);
    }
    return status_code;
}

int main(int argc, char* argv[])
{
    const char* host = argv[1];
    const char* resource = argv[2];
    // socket initialization and connection
    int sock;
    if (-1 == (sock = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("Can't create a socket");
        exit(1);
    }
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    struct addrinfo* result = NULL;
    if (0 != getaddrinfo(host, "http", &hints, &result)) {
        perror("Can't obtain host address");
        exit(1);
    }
    if (-1 == connect(sock, result->ai_addr, result->ai_addrlen)) {
        perror("Can't connect to the server");
        exit(1);
    }

    // send request
    char query[4096] = {};
    snprintf(
        query,
        sizeof(query),
        "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
        resource,
        host);
    send(sock, query, strnlen(query, sizeof(query)), 0);

    // get response
    int status_code = PrintResponseContent(sock);
    if (200 != status_code) {
        printf(
            "Some error (may) have occurred, status_code: %d\n",
            status_code);
    }
    close(sock);
    exit(0);
}