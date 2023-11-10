#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

volatile sig_atomic_t if_int_term = 0;

void handler_int_term(int signum)
{
    if_int_term = 1;
}

void format_path(const char* old_path, char* new_path)
{
    size_t length = strnlen(old_path, PATH_MAX);
    for (size_t idx = 0; idx < length; ++idx) {
        new_path[idx] = old_path[idx];
    }
    if (new_path[length - 1] != '/') {
        new_path[length] = '/';
    }
}

void get_filename(int client, char* filename)
{
    char symbol = '~';
    while (symbol != ' ') // reading "GET"
    {
        recv(client, &symbol, sizeof(symbol), 0);
    }
    symbol = '~';
    size_t idx = 0;
    while (1) // reading a file's name
    {
        recv(client, &symbol, sizeof(symbol), 0);
        if (symbol == ' ') {
            break;
        }
        filename[idx] = symbol;
        ++idx;
    }
    symbol = '~';
    size_t line_length = 0;
    while (1) // reading "HTTP/1.1" and other lines till an empty line
    {
        recv(client, &symbol, sizeof(symbol), 0);
        if (symbol != '\r') {
            ++line_length;
        } else {
            recv(client, &symbol, sizeof(symbol), 0);
            if (symbol == '\n') {
                if (line_length == 0) {
                    break;
                } else {
                    line_length = 0;
                    continue;
                }
            }
        }
    }
}

int check_file(const char* path_to_file)
{
    if (-1 == access(path_to_file, F_OK)) {
        return 404; // doesn't exist
    } else {
        if (-1 == access(path_to_file, R_OK)) {
            return 403; // not readable
        }
    }
    return 200; // readable
}

void get_content(const char* path_to_file, char* content)
{
    int fd = open(path_to_file, O_RDONLY);
    if (-1 == fd) {
        perror("Can't open a file for reading");
        exit(1);
    }
    char symbol;
    size_t idx = 0;
    while (read(fd, &symbol, sizeof(symbol)) > 0) {
        content[idx] = symbol;
        ++idx;
    }
    content[idx] = '\r';
    content[++idx] = '\n';
    close(fd);
}

int main(int argc, char* argv[])
{
    struct sigaction sigact_int_term;
    memset(&sigact_int_term, 0, sizeof(sigact_int_term));
    sigact_int_term.sa_handler = handler_int_term;
    sigact_int_term.sa_flags = SA_RESTART;
    if (-1 == sigaction(SIGINT, &sigact_int_term, NULL)) {
        perror("Sigaction failed");
        exit(1);
    }
    if (-1 == sigaction(SIGTERM, &sigact_int_term, NULL)) {
        perror("Sigaction failed");
        exit(1);
    }

    char path[PATH_MAX] = {};
    format_path(argv[2], path);

    int sock;
    if (-1 == (sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))) {
        perror("Can't create a socket");
        exit(1);
    }
    in_port_t port = htons((short)strtol(argv[1], NULL, 10));
    struct in_addr ip;
    ip.s_addr = inet_addr("127.0.0.1");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr = ip;
    if (-1 == bind(sock, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("Bind failed");
        exit(1);
    }
    if (-1 == listen(sock, SOMAXCONN)) {
        perror("Listen failed");
        exit(1);
    }
    while (!if_int_term) {
        int client;
        if (-1 == (client = accept(sock, NULL, NULL))) {
            continue;
        }
        char filename[FILENAME_MAX] = {};
        get_filename(client, filename);
        char path_to_file[PATH_MAX] = {};
        snprintf(path_to_file, sizeof(path_to_file), "%s%s", path, filename);
        int flag = check_file(path_to_file);

        char answer[40] = {};
        switch (flag) {
        case 200:
            snprintf(answer, sizeof(answer), "HTTP/1.1 %d OK\r\n", flag);
            break;
        case 404:
            snprintf(answer, sizeof(answer), "HTTP/1.1 %d Not Found\r\n", flag);
            break;
        case 403:
            snprintf(answer, sizeof(answer), "HTTP/1.1 %d Forbidden\r\n", flag);
            break;
        default:
            break;
        }
        send(client, answer, strnlen(answer, sizeof(answer)), 0);

        if (flag == 200 && 0 == access(path_to_file, X_OK)) {
            pid_t pid;
            if (-1 == (pid = fork())) {
                perror("Fork failed");
                if (-1 == shutdown(client, SHUT_RDWR)) {
                    perror("Shutdown failed");
                }
                close(client);
                continue;
            }
            if (pid == 0) {
                if (-1 == dup2(client, STDOUT_FILENO)) {
                    perror("Dup failed");
                    exit(1);
                }
                execl(path_to_file, path_to_file, NULL);
                perror("Execution failed");
                exit(1);
            } else {
                int wstatus;
                waitpid(pid, &wstatus, 0);
                if (-1 == shutdown(client, SHUT_RDWR)) {
                    perror("Shutdown failed");
                }
                close(client);
                continue;
            }
        }

        char length[40] = {};
        int content_length = 0;
        if (flag == 200) {
            struct stat stat_buff;
            stat(path_to_file, &stat_buff);
            content_length = (int)stat_buff.st_size;
        }
        snprintf(
            length, sizeof(length), "Content-Length: %d\r\n", content_length);
        send(client, length, strnlen(length, sizeof(length)), 0);

        char empty_line[3] = "\r\n";
        send(client, empty_line, strnlen(empty_line, sizeof(empty_line)), 0);

        if (flag == 200) {
            char content[4096] = {};
            get_content(path_to_file, content);
            send(client, content, strnlen(content, sizeof(content)), 0);
        }
        if (-1 == shutdown(client, SHUT_RDWR)) {
            perror("Shutdown failed");
        }
        close(client);
    }
    shutdown(sock, SHUT_RDWR);
    close(sock);
    exit(0);
}
