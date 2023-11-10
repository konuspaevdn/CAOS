#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

typedef double (*func_t)(double);

int main(int argc, char* argv[])
{
    void* lib = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
    if (!lib) {
        fprintf(stderr, "Can't load library %s\n", argv[1]);
        exit(1);
    }
    func_t func = dlsym(lib, argv[2]);
    if (!func) {
        fprintf(stderr, "Can't load sym %s from lib %s\n", argv[2], argv[1]);
        exit(1);
    }
    double argument;
    while (EOF != scanf("%lf", &argument)) {
        printf("%.3f\n", func(argument));
    }
    dlclose(lib);
    exit(0);
}