#include "encrypt.h"


bool encrypt_init(unsigned char *key) {
	char iv[EVP_MAX_IV_LENGTH];

	cipher = EVP_aes_128_cbc();
	if (!cipher) {
		fprintf(stderr, "[ERROR] Unable to initialize AES cipher\n");

		return false;
	}

	EVP_EncryptInit(&ctx, cipher, key, iv);
	return true;
}

char *encrypt_data(const void *data, int size) {
	char *ret = NULL;
	int block_size = 100;
	int alloc_size = size + EVP_CIPHER_CTX_block_size(&ctx);
	int tmp = 0;
	int ol = 0;

	ret = (char *) malloc(alloc_size);

	if (!ret) {
		fprintf(stderr, "[ERROR] Not enough memory to allocate for cipher\n");

		return NULL;
	}

	for (int i = 0; i < size / block_size; i++) {
		EVP_EncryptUpdate(&ctx, &ret[ol], &tmp, &data[ol], block_size);
		ol += tmp;
	}

	if (size % block_size) {
		EVP_EncryptUpdate(&ctx, &ret[ol], &tmp, &data[ol], size % block_size);
		ol += tmp;
	}

	EVP_EncryptFinal(&ctx, &ret[ol], &tmp);
	return ret;
}

