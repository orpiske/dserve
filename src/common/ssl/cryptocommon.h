/**
 Copyright 2012 Otavio Rodolfo Piske
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

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
