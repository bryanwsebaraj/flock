
#include "aes.h"

gcc_inline v2di v2di_cast(uint128_t x)
{
	v2di y;
	*(uint64_t *) &y = x.v[0];
	*((uint64_t *) &y + 1) = x.v[1];

	return y;
}

gcc_inline uint128_t uint128_cast(v2di x)
{
	uint128_t y;
	y.v[0] = *(uint64_t *) &x;
	y.v[1] = *((uint64_t *) &x + 1);

	return y;
}

uint128_t aesenc128 (uint128_t a, uint128_t round_key)
{
	v2di _a = v2di_cast(a);
	v2di _round_key = v2di_cast(round_key);

	v2di _ret = __builtin_ia32_aesenc128(_a, _round_key);

	uint128_t ret = uint128_cast(_ret);
	return ret;
}
