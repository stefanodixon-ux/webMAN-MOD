static void _memset(void *m, size_t n)
{
	if(n & 7)
	{
		char *s = (char *) m;
		while (n--) *s++ = 0;
	}
	else
	{
		n >>= 3;
		uint64_t *s = (uint64_t *) m;
		while (n--) *s++ = 0;
	}
}

static void _memcpy(void *dst, void *src, size_t n)
{
	if(n & 7)
	{
		char *s = (char *) src;
		char *d = (char *) dst;
		while (n--) *d++ = *s++;
	}
	else
	{
		n >>= 3;
		uint64_t *s = (uint64_t *) src;
		uint64_t *d = (uint64_t *) dst;
		while (n--) *d++ = *s++;
	}
}
