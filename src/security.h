//
// Created by iol on 5/19/17.
//

#ifndef SECURITY_H_
#define SECURITY_H_

#include <string.h>

void *ptpd_EVP_sha256();
unsigned char *ptpd_HMAC(void *evp_md, const void *key,
                       int key_len, const unsigned char *d, int n,
                       unsigned char *md, unsigned int *md_len);
int GMAC(const unsigned char *key,
            unsigned char *iv, int iv_len,
            unsigned char *data, int data_len,
            unsigned char *icv, int icv_len);
void generate_chain(unsigned char **base, int key_len, int n);
void generate_icv_key(unsigned char *result, unsigned char *key, int key_len);
int verify_key(unsigned char **verified_keys, unsigned char *new_key, int new_interval, int latest_interval,
               int chain_len, int key_len);
size_t ptpd_EVP_MAX_MD_SIZE();

#endif /* SECURITY_H_ */
