#ifndef ENCRYPT_H_
#define ENCRYPT_H_

#include <openssl/evp.h>

#include "cryptocommon.h"

bool encrypt_init(unsigned char *key);
char *encrypt_data(const void *data, int size);

// ENCRYPT_H_
#endif
