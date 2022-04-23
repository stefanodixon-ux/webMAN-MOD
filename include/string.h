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
