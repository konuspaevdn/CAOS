#include <openssl/evp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const int FAIL = 0;
const size_t SALT_LENGTH = 8;
const size_t KEY_LENGTH = 32;
const size_t IV_LENGTH = 16;

void ExtractSalt(unsigned char* salt)
{
    unsigned char prefix[8] = {'S', 'a', 'l', 't', 'e', 'd', '_', '_'};
    unsigned char buffer;
    for (size_t i = 0; i < 8; ++i) {
        read(STDIN_FILENO, &buffer, sizeof(buffer));
        if (buffer != prefix[i]) {
            printf("Wrong prefix\n");
            exit(1);
        }
    }

    for (size_t i = 0; i < SALT_LENGTH; ++i) {
        read(STDIN_FILENO, &salt[i], sizeof(salt[i]));
    }
}

int main(int argc, char* argv[])
{
    EVP_CIPHER_CTX* ctx;
    if (NULL == (ctx = EVP_CIPHER_CTX_new())) {
        printf("Context creation failed\n");
        exit(1);
    }

    unsigned char salt[SALT_LENGTH];
    ExtractSalt(salt);

    char password[65536] = {};
    strncpy(password, argv[1], sizeof(password));

    unsigned char key[KEY_LENGTH];
    unsigned char iv[IV_LENGTH];
    memset(key, 0, sizeof(key));
    memset(iv, 0, sizeof(iv));

    EVP_BytesToKey(
        EVP_aes_256_cbc(),
        EVP_sha256(),
        salt,
        (unsigned char*)password,
        (int)strlen(password),
        1,
        key,
        iv);

    if (FAIL == EVP_DecryptInit(ctx, EVP_aes_256_cbc(), key, iv)) {
        printf("Init failed\n");
        exit(1);
    }

    unsigned char buffer[4096] = {};
    unsigned char out[4096] = {};
    int outl = 0;
    int bytes_read = 0;
    while (0 < (bytes_read = (int)read(STDIN_FILENO, buffer, sizeof(buffer)))) {
        if (FAIL == EVP_DecryptUpdate(ctx, out, &outl, buffer, bytes_read)) {
            printf("Update failed\n");
            exit(1);
        }
        write(STDOUT_FILENO, out, outl);
    }

    if (FAIL == EVP_DecryptFinal(ctx, out, &outl)) {
        printf("Final failed\n");
        exit(1);
    }
    write(STDOUT_FILENO, out, outl);

    EVP_CIPHER_CTX_free(ctx);

    exit(0);
}