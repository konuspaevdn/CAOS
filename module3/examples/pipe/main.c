#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    int channel[2];
    pipe(channel);
    char string[6] = "Hello";
    write(channel[1], string, sizeof(string) - 1);
    //close(channel[1]);
    char buffer[100];
    int count;
    while(read(channel[0], buffer, sizeof(buffer) - 1) >= 0){
    	++count;
    }
    printf("%s\n", buffer);
    printf("%d\n", count);
    return 0;
}
