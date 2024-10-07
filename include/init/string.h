#define LCASE(a)	(a | 0x20)

extern char *strcasestr(const char *s1, const char *s2);

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

static void memcpy64(void *dst, const void *src, int n)
{
	if(!dst || !src || !n) return;
	u8 p = n & 7; // remaining bytes (same as n % 8)

	n >>= 3; // same as n /= 8;
	u64 *d = (u64 *) dst;
	const u64 *s = (u64 *) src;
	while (n--) *d++ = *s++; // 64bit memcpy

	if(p)
		memcpy(d, s, p);
}

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
/*
static char *replace(char *text, const char *rep, const char *with, int count)
{
	if (!text || !rep || !with || !rep[0])
		return text;

	const int len_with = strlen(with);
	const int gap = len_with - strlen(rep);

	char *pos = strstr(text, rep);
	while (pos)
	{
		if(--count == -1) return text; // replace all if count < 0

		if(gap > 0)
			memmove(pos + gap, pos, strlen(pos) + 1);
		if(gap >= 0)
			memcpy(pos, with, len_with);
		else
			sprintf(pos, "%s%s", with, pos + len_with - gap);

		pos = strstr(pos + len_with, rep);
	}
	return text;
}
*/
static void replace_char(char *text, char c, char r)
{
	if(!text) return;

	for( ; *text; text++) if(*text == c) *text = r;
}

static void replace_invalid_chars(char *text)
{
	if(!text) return;

	for(unsigned char *c = (unsigned char*)text; *c; c++) if((*c < ' ') || (*c == '"')) *c = ' ';
}

static char *switch_case(char *text, char a, char z)
{
	char *new_text = text;
	for( ; *text; text++) if(*text >= a && *text <= z) *text ^= 0x20;
	return new_text;
}

static char *to_upper(char *text)
{
	return switch_case(text, 'a', 'z');
}

#ifdef COBRA_ONLY
static char *to_lower(char *text)
{
	return switch_case(text, 'A', 'Z');
}
#endif

static char *remove_brackets(char *title)
{
	if(*title == '[')
	{
		char *pos = strchr(title, ']');
		if(pos && !pos[1])
		{
			*pos = '\0'; // remove last bracket ]
			memcpy(title, title + 1, strlen(title)); // remove first bracket [
		}
	}
	return title;
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
