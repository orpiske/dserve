#ifndef CRYPTOCOMMON_H_
#define CRYPTOCOMMON_H_

#include <openssl/evp.h>

static EVP_CIPHER *cipher = NULL;
static EVP_CIPHER_CTX ctx;

static void select_random_iv(char *iv, int size) {
	RAND_pseudo_bytes(iv, size);
}

// CRYPTOCOMMON_H_
#endif
