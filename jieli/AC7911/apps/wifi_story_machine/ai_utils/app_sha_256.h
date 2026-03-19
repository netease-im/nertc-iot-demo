#ifndef __APP_SHA_256_H__
#define __APP_SHA_256_H__

#define AES_KEY_LENGTH 32
void app_sha256(const unsigned char *input, size_t length, unsigned char *output);
void generate_aes_key(char *pk, char *aes_key);

void app_md5(unsigned char *initial_msg, size_t initial_len, unsigned char *digest);

#endif
