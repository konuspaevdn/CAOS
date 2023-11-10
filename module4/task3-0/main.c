#include <openssl/evp.h>
#include <stdlib.h>
#include <unistd.h>

const int FAIL = 0;

int main() {
    EVP_MD_CTX* ctx;
    if (NULL == (ctx = EVP_MD_CTX_new())) {
        perror("Context creation failed");
        exit(1);
    }
    if (FAIL == EVP_DigestInit(ctx, EVP_sha512())) {
        perror("Init failed");
        exit(1);
    }
    char buffer;
    while (0 < read(STDIN_FILENO, &buffer, sizeof(buffer))) {
        if (FAIL == EVP_DigestUpdate(ctx, &buffer, sizeof(buffer))) {
            perror("Update failed");
            exit(1);
        }
    }
    unsigned char value[64] = {};
    unsigned int len;
    if (FAIL == EVP_DigestFinal(ctx, value, &len)) {
        perror("Final failed");
        exit(1);
    }
    EVP_MD_CTX_free(ctx);
    printf("0x");
    for (size_t i = 0; i < len; ++i)
        printf("%02x", value[i]);
    printf("\n");
    exit(0);
}