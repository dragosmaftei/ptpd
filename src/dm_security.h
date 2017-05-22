//
// Created by iol on 5/19/17.
//

//#ifndef PTPD_DM_SECURITY_H
//#define PTPD_DM_SECURITY_H

void *dm_EVP_sha256();
unsigned char *dm_HMAC(void *evp_md, const void *key,
                       int key_len, const unsigned char *d, int n,
                       unsigned char *md, unsigned int *md_len);

//#endif //PTPD_DM_SECURITY_H
