//
// Created by iol on 5/19/17.
//

#include <openssl/hmac.h> /* includes HMAC and EVP_sha256() */
//#include <openssl/evp.h> /* this has EVP_sha256() only */

/*
 * just call the actual openssl version
 */
void *dm_EVP_sha256() {
    return (void *)EVP_sha256();
}

/*
 * call the openssl version, and now that we have the openssl structs w/ no
 * conflicts, can cast the evp_md from void* to EVP_MD *
 */
unsigned char *dm_HMAC(void *evp_md, const void *key,
                       int key_len, const unsigned char *d, int n,
                       unsigned char *md, unsigned int *md_len) {
    return HMAC((EVP_MD *)evp_md, key, key_len, d, n, md, md_len);
}

void dm_GMAC() {

    EVP_CIPHER_CTX *ctx = NULL;
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
}