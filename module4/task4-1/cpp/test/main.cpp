extern "C" {
#include <stdio.h>
#include <sys/stat.h>
}

int main() {
    struct stat st{};
    stat("main.cpp", &st);
    printf("%o", st.st_mode);
}