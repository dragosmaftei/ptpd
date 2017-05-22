//
// Created by iol on 5/19/17.
//

#include <openssl/hmac.h> // includes HMAC and EVP_sha256()
//#include <openssl/evp.h> // this has EVP_sha256() only

void *dm_EVP_sha256() {
    return (void *)EVP_sha256();
}

unsigned char *dm_HMAC(void *evp_md, const void *key,
                       int key_len, const unsigned char *d, int n,
                       unsigned char *md, unsigned int *md_len) {
    return HMAC((EVP_MD *)evp_md, key, key_len, d, n, md, md_len);
}