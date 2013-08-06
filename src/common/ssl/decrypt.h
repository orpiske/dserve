#ifndef ENCRYPT_H_
#define ENCRYPT_H_

#include <openssl/evp.h>

#include "cryptocommon.h"

bool decrypt_init(unsigned char *key);
char *decrypt_data(const void *data, int size);

// ENCRYPT_H_
#endif
