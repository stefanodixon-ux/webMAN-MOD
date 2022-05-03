#define LCASE(a)	(a | 0x20)

char *strcasestr(const char *s1, const char *s2);

typedef struct {
	u32  size;
	char *str;
} t_string;

static void _alloc(t_string *item, char *mem)
{
	if(!item || !mem) return;
	*mem = NULL;
	item->str  = mem;
	item->size = 0;
}

static void _set(t_string *item, char *mem, u32 len)
{
	if(!item || !mem) return;
	item->str  = mem;
	item->size = len;
}

static char *_concat(t_string *dest, const char *src)
{
	if(!dest) return NULL;

	char *str = dest->str + dest->size;

	while (*str) {str++, dest->size++;} // find last null byte

	if(src) while ((*str++ = *src++)) dest->size++; // append src

	return dest->str;
}

static void _concat2(t_string *dest, const char *src1, const char *src2)
{
	_concat(dest, src1);
	_concat(dest, src2);
}

static void _concat3(t_string *dest, const char *src1, const char *src2, const char *src3)
{
	_concat(dest, src1);
	_concat(dest, src2);
	_concat(dest, src3);
}

static t_string fast_concat;

static size_t concat(char *dest, const char *src)
{
	if(!dest || !src) return 0;

	if(fast_concat.str != dest)
	{
		fast_concat.str = dest;
		fast_concat.size = 0;
	}

	return _concat(&fast_concat, src) - dest;
}

#if defined(USE_INTERNAL_NTFS_PLUGIN) || defined(NET_SUPPORT) || defined(USE_NTFS) || defined(DEBUG_MEM)
static void memcpy64(void *dst, const void *src, int n)
{
	if(!dst || !src || !n) return;
	u8 p = n & 7; // remaining bytes (same as n % 8)

	n >>= 3; // same as n /= 8;
	u64 *d = (u64 *) dst;
	u64 *s = (u64 *) src;
	while (n--) *d++ = *s++; // 64bit memcpy

	if(p)
		memcpy(d, s, p);
}
#endif

void _memset(void *m, size_t n);
void _memset(void *m, size_t n)
{
	if(!m || !n) return;
	u8 p = n & 7; // remaining bytes (same as n % 8)

	n >>= 3; // same as n /= 8;
	u64 *s = (u64 *) m;
	while (n--) *s++ = 0LL; // 64bit memset

	if(p)
		memset(s, 0, p);
}

static void replace_char(char *text, char c, char r)
{
	if(!text) return;

	do
	{
		text = strchr(text, c);
		if(text) *text = r;
	}
	while (text);
}

static char *to_upper(char *text)
{
	char *upper = text;
	for( ; *text; text++) if(*text >= 'a' && *text <= 'z') *text ^= 0x20;
	return upper;
}

#ifndef LITE_EDITION
static char *prepend(char *a, const char *b, int len)
{
	int n = strlen(b);
	if(len <= 0) len = n;
	memmove(a + len, a, strlen(a) + 1);
	memcpy64(a, b, n);
	return a;
}
#endif