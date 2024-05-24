#ifndef _KERN_DEV_AES_H_
#define _KERN_DEV_AES_H_

#include <lib/types.h>
#include <lib/debug.h>
#include <lib/gcc.h>

typedef long long int __attribute__((vector_size(16), aligned(16))) v2di;

typedef struct
{
	uint64_t v[2];
} __attribute__((aligned(16))) uint128_t;


uint128_t aesenc128 (uint128_t a, uint128_t round_key);


#endif /* !_KERN_DEV_AES_H_ */