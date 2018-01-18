//
// Created by iol on 5/19/17.
//

#include "dm_security.h"
#include <openssl/hmac.h> /* includes HMAC and EVP_sha256() */
//#include <openssl/evp.h> /* this has EVP_sha256() only */

/*
 * verifies a newly passed in key, returns non zero if the key passes verification, and stores the newly verified key
 * (as well as any intermediate keys between the new one and the latest verified key) in the verified_keys chain (due to
 * the call to generate_chain); in the calling function, latestInterval will be updated to reflect the presence of the new keys
 * returns 0 if the key fails verification; the verified_keys chain will still have the bad keys stored, but they are
 * effectively garbage since the latestInterval variable will not be updated in the calling function
 */
int verify_key(unsigned char **verified_keys, unsigned char *new_key, int new_interval, int latest_interval,
                int chain_len, int key_len) {
    /* translate intervals into keyChain indices since keychains are laid out in reverse order */
    int new_index = chain_len - new_interval - 1;
    int latest_index = chain_len - latest_interval - 1;

    /* copy new key into proper slot in verified keys */
    memcpy(verified_keys[new_index], new_key, key_len);

    /* generateChain from new key as base, up to but not including the key at the latest interval (the lastest verified key) */
    generate_chain(&verified_keys[new_index], key_len, new_interval - latest_interval - 1);

    /* hash the newly generated 2nd from latest key; it should hash to the latest key (which is already verified) */
    unsigned char tmpbuf[EVP_MAX_MD_SIZE];

    unsigned char message = 0;
    int message_len = 1;
    HMAC(EVP_sha256(), (void *)verified_keys[latest_index - 1], key_len, &message, message_len, tmpbuf, 0);

    /* memcmp returns 0 if identical */
    return !memcmp(tmpbuf, verified_keys[latest_index], key_len);

}

/*
 * base points to a key upon which n more keys will be based
 */
void generate_chain(unsigned char **base, int key_len, int n) {
    if (n == 0)
        return;

    /* hash over 0 to generate key chain, RFC 4082 section 3.2 */
    unsigned char message = 0;
    int message_len = 1;

    /*
     * n more keys need to be generated, so generate the next one, using *base as the key;
     * store it in the next slot in the chain
     */
    HMAC(EVP_sha256(),
         (void *)base[0], key_len,
         &message, message_len,
         base[1], 0);

    /* next call uses newly generated key as a base */
    generate_chain(base + 1, key_len, n - 1);
}

/*
 * given a key (from a keychain), generate a corresponding ICV key by hashing over 1 (RFC 4082 section 3.4)
 */
void generate_icv_key(unsigned char *result, unsigned char *key, int key_len) {
    unsigned char message = 1;
    int message_len = 1;

    /*
     * hash over message 1 using provided key, storing the result (the corresponding ICV key) in result
     * last argument is int * where the length of the result is placed, unless it is null (don't need it)
     */
    HMAC(EVP_sha256(),
         (void *)key, key_len,
         &message, message_len,
         result, 0);
}

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

