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

/*
 * this code is adopted from Jeffrey Walton https://pastebin.com/v3J93Cj4
 * return
 */
int dm_GMAC(const unsigned char *key,
            unsigned char *iv, int iv_len,
            unsigned char *data, int data_len,
            unsigned char *icv, int icv_len) {

    /* return code used for functions */
    int rc = 0, unused;

    /* initialize the context */
    EVP_CIPHER_CTX *ctx = NULL;
    ctx = EVP_CIPHER_CTX_new();
    if(ctx == NULL) {
        return 0;
    }

    /* specify the cipher, which determines key size */
    rc = EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    if(rc != 1) {
        return 0;
    }

    /* specify size of iv. default is 12, so this isn't necessary atm, but just to be explicit */
    rc = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL);
    if(rc != 1) {
        return 0;
    }

    /* give the key and the IV */
    rc = EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);
    if(rc != 1) {
        return 0;
    }

    /* provide data to be authenticated */
    rc = EVP_EncryptUpdate(ctx, NULL, &unused, data, data_len);
    if(rc != 1) {
        return 0;
    }

    /* finalize */
    rc = EVP_EncryptFinal_ex(ctx, NULL, &unused);
    if(rc != 1) {
        return 0;
    }

    /* give the tag/ICV location */
    rc = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, icv_len, icv);
    if(rc != 1) {
        return 0;
    }

    if(ctx) {
        EVP_CIPHER_CTX_free(ctx);
    }

    return 1;
}

