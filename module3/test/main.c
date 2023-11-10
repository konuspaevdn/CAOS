#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
 
static volatile sig_atomic_t exitting = false;
static volatile sig_atomic_t loop = false;
static volatile sig_atomic_t accepting = false;
 
const size_t MAX_LENGTH = 8192;
const size_t BODY_LINE_MAX_LENGTH = 65536;
 
int socket_fd = -1;
int client_fd = -1;
int directory_fd = -1;
 
void read_line(char* line) {
    fgets(line, sizeof(line), stdin);
}
 
void read_head(char* file_name) {
    char head_line[MAX_LENGTH];
    memset(head_line, '\0', sizeof(head_line));
    read_line(head_line);
    memcpy(file_name, head_line + 4, strlen(head_line) - 15);
}
 
void read_body() {
    while(true) {
        char body_line[BODY_LINE_MAX_LENGTH];
        memset(body_line, '\0', sizeof(body_line));
        read_line(body_line);
 
        if (body_line[0] == '\r' && body_line[1] == '\n') {
            break;
        }
    }
}
 
void process_client() {
    /* Receiving a get request: */
    char file_name[MAX_LENGTH];
    memset(file_name, '\0', sizeof(file_name));
 
    read_head(file_name);
    read_body();
 
    /* Sending a response: */
    int file_fd = openat(directory_fd, file_name, O_RDONLY);
 
    if (file_fd == -1 && errno != EACCES) {
        char response_header[MAX_LENGTH];
        memset(response_header, '\0', sizeof(response_header));
        strcat(response_header, "HTTP/1.1 404 Not Found\r\n\r\n");
        write(client_fd, response_header, strlen(response_header));
    } else if (file_fd == -1 && errno == EACCES) {
        char response_header[MAX_LENGTH];
        memset(response_header, '\0', sizeof(response_header));
        strcat(response_header, "HTTP/1.1 403 Forbidden\r\n\r\n");
        write(client_fd, response_header, strlen(response_header));
    } else {
        char response_header[MAX_LENGTH];
        memset(response_header, '\0', sizeof(response_header));
        struct stat file_stat;
        fstatat(directory_fd, file_name, &file_stat, AT_SYMLINK_NOFOLLOW);
        sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: text/plain\r\n\r\n", file_stat.st_size);
        write(client_fd, response_header, strlen(response_header));
        // sendfile(client_fd, file_fd, NULL, file_stat.st_size);
    }
}
 
void shutdown_server() {
    if (client_fd != -1) {
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
    }
 
    if (socket_fd != -1) {
        shutdown(socket_fd, SHUT_RDWR);
        close(socket_fd);
    }
 
    if (directory_fd != -1) {
        close(directory_fd);
    }
    exit(0);
}
 
void handler(int signum) {
    exitting = true;
    if (!loop || accepting) {
        shutdown_server();
    }
}
 
void attach_handlers_listeners() {
    signal(SIGPIPE, SIG_IGN);
 
    sigaction(
        SIGINT,
        &(struct sigaction){.sa_handler = handler, .sa_flags = SA_RESTART},
        NULL);
    sigaction(
        SIGTERM,
        &(struct sigaction){.sa_handler = handler, .sa_flags = SA_RESTART},
        NULL);
}
 
 
int main(int argc, char* argv[]) {
    attach_handlers_listeners();
 
    directory_fd = open(argv[2], O_RDONLY | __O_DIRECTORY);
    if (socket_fd == -1) {
        perror("Error in open");
    }
 
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);    // AF_INET - IPv4 Internet protocols
 
    if (socket_fd == -1) {
        perror("Error in socket");
    }
 
    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr("127.0.0.1"),  // localhost
        .sin_port = htons(atoi(argv[1]))
    };
 
    if (bind(socket_fd, (const struct sockaddr*)(&server_address), sizeof(server_address)) == -1) {
        perror("Error in bind");
        shutdown_server();
    }
 
    if (listen(socket_fd, SOMAXCONN) == -1) {
        perror("Error in listen");
        shutdown_server();
    }
 
    // Now lets connect the client
    struct sockaddr_in client_address;
    socklen_t client_address_length;
 
    loop = true;
    while (!exitting) {
        accepting = true;
        client_fd = accept(socket_fd, (struct sockaddr*)(&client_address), &client_address_length);
 
        if (client_fd == -1) {
            perror("Error in client accept\n");
            shutdown_server();
        }
 
        accepting = false;
 
        process_client();
 
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
    }
 
    close(socket_fd);
    return 0;
}
