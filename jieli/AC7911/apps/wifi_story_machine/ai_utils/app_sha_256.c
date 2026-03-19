#include <stdio.h>
#include <stdlib.h>
// #include <string.h>
#include "app_sha_256.h"

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define SHR(x, n) ((x) >> (n))
#define Ch(x, y, z) (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define Sigma0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define Sigma1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define sigma0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define sigma1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void app_sha256(const unsigned char *input, size_t length, unsigned char *output) {
    uint32_t h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    size_t new_len, offset;
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, hh, t1, t2;

    new_len = length + 1;
    while (new_len % (512 / 8) != 448 / 8)
        new_len++;

    unsigned char *msg = (unsigned char *)malloc(new_len + 8);
    memcpy(msg, input, length);
    msg[length] = 0x80;
    for (offset = length + 1; offset < new_len; offset++)
        msg[offset] = 0;

    uint64_t bits_len = (uint64_t)length * 8;
    for (offset = 0; offset < 8; offset++)
        msg[new_len + offset] = (bits_len >> ((7 - offset) * 8)) & 0xff;

    for (offset = 0; offset < new_len + 8; offset += (512 / 8)) {
        for (int i = 0; i < 16; i++) {
            w[i] = (msg[offset + i * 4] << 24) | (msg[offset + i * 4 + 1] << 16) |
                   (msg[offset + i * 4 + 2] << 8) | msg[offset + i * 4 + 3];
        }
        for (int i = 16; i < 64; i++) {
            w[i] = sigma1(w[i - 2]) + w[i - 7] + sigma0(w[i - 15]) + w[i - 16];
        }

        a = h[0];
        b = h[1];
        c = h[2];
        d = h[3];
        e = h[4];
        f = h[5];
        g = h[6];
        hh = h[7];

        for (int i = 0; i < 64; i++) {
            t1 = hh + Sigma1(e) + Ch(e, f, g) + k[i] + w[i];
            t2 = Sigma0(a) + Maj(a, b, c);
            hh = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
        h[5] += f;
        h[6] += g;
        h[7] += hh;
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            output[i * 4 + j] = (h[i] >> ((3 - j) * 8)) & 0xff;
        }
    }

    free(msg);
}



void generate_aes_key(char *pk, char *aes_key) {
    int pk_length = strlen(pk);
    // 用字符 '0' 填充 aes_key
    for (int i = 0; i < AES_KEY_LENGTH; i++) {
        aes_key[i] = '0';
    }
    aes_key[AES_KEY_LENGTH] = '\0';
    // 将 pk 复制到 aes_key 开头
    strncpy(aes_key, pk, pk_length);
}


