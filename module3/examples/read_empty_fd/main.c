#include <fcntl.h>
#include <unistd.h>

int main() {
    int channel[2];
    pipe(channel);
    char buffer[10];
    read(channel[0], buffer, sizeof(buffer));
    return 0;
}
