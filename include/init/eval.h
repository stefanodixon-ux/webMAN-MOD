#define MAX(a, b)			((a) >= (b) ? (a) : (b))
#define MIN(a, b)			((a) <= (b) ? (a) : (b))
#define ABS(a)				(((a) < 0) ? -(a) : (a))
#define RANGE(a, b, c)		((a) <= (b) ? (b) : (a) >= (c) ? (c) : (a))
#define BETWEEN(a, b, c)	( ((a) <= (b)) && ((b) <= (c)) )
#define ISDIGIT(a)			( ('0' <= (a)) && ((a) <= '9') )
#define ISSPACE(a)			( ( 0  <  (a)) && ((a) <= ' ') )
#define ISHEX(a)			(ISDIGIT(a) || BETWEEN('a', LCASE(a), 'f'))
#define	INT32(a)			(*((u32*)(a)))

extern long int stdc_D14ECE90(const char *str, char **endptr, int base);              // strtol()
#define strtol stdc_D14ECE90

extern uint64_t stdc_36C067C1(const char *a, char **b, int x);                        // _Stoll
#define _Stoll stdc_36C067C1

static char h2a(const char hex) // hex byte to ascii char
{
	char c = (unsigned char)hex;
	if(BETWEEN(0, c, 9))
		c += '0'; //'0'-'9'
	else if(BETWEEN(0xA, c, 0xF))
		c += 55;  //'A'-'F'
	return c;
}

static u8 h2b(const char hex) // hex char to byte
{
	u8 c = LCASE(hex);
	if(BETWEEN('0', c, '9'))
		c -= '0'; // 0-9
	else if(BETWEEN('a', c, 'f'))
		c -= 'W'; // 10-15
	return c;
}

static u64 convertH(const char *val) // convert hex string to unsigned integer 64bit
{
	if(!val || (*val == 0)) return 0;

	if(islike(val, "0x")) val += 2;

	char *end;
	return _Stoll(val, &end, 16);
/*
	u64 ret = 0; u8 n = 0, c;

	if(islike(val, "0x")) n = 2;

	for(u8 i = n; i < 16 + n; i++)
	{
		if(val[i]==' ') {n++; continue;}

		c = h2b(val[i]);
		if(c > 0xF)
			return ret;

		ret = (ret << 4) | c;
	}

	return ret;
*/
}

#ifndef LITE_EDITION
static bool isHEX(const char *value)
{
	if(islike(value, "0x")) value += 2;
	for(; *value; ++value)
	{
		char c = *value;
		if(!(ISHEX(c) || (c == ' ') || (c == '*'))) return false;
	}
	return true;
}

static u16 Hex2Bin(const char *src, char *out)
{
	char *target = out;
	if(islike(src, "0x")) src += 2;
	while(*src && src[1])
	{
		if(*src <= ' ') {++src; continue;} // ignore spaces & line breaks
		if((*src == '*') || (*src == '?'))
			{*(target++) = '*'; src += 2; continue;} // convert mask ** / ?? to binary *

		*(target++) = (h2b(src[0])<<4) | h2b(src[1]);
		src += 2;
	}
	return (target - out);
}
#endif

static int oct(const char *c)
{
	if(!c) return 0;
	
	char *end;
	return strtol(c, &end, 8);
/*
	int value = 0;
	while(ISDIGIT(*c))
	{
		value <<= 3;
		value += (*c - '0'); c++;
	}
	return value;
*/
}

static s64 val(const char *c)
{
	if(!c) return 0;

	if(islike(c, "0x"))
	{
		return convertH(c);
	}
	
	char *end;
	return _Stoll(c, &end, 10);
/*
	s64 result = 0;
	s64 sign = 1;

	if(c && *c == '-')
	{
		sign = -1;
		c++;
	}

	while(*c)
	{
		if(!ISDIGIT(*c)) break;

		result *= 10;
		result += (*c & 0x0F);

		c++;
	}
	return(result * sign);
*/
}

static u16 get_value(char *value, const char *url, u16 max_size)
{
	if(!value || !url || !max_size) return 0;

	u16 n;
	for(n = 0; n < max_size; n++)
	{
		if(url[n] == '&' || url[n] == 0) break;
		value[n] = url[n];
	}
	value[n] = '\0';
	return n;
}

static u16 get_param(const char *name, char *value, const char *url, u16 max_size)
{
	if(!name || !value || !url || !max_size) return 0;

	u8 name_len = strlen(name);

	if(name_len)
	{
		char *pos = strstr(url, name);
		if(pos)
		{
			if(name[name_len - 1] != '=') name_len++;
			return get_value(value, pos + name_len, max_size);
		}
	}

	*value = '\0';
	return 0;
}

static s64 get_valuen64(const char *param, const char *label)
{
	if(!param || !label) return false;

	char value[12], *pos = strstr(param, label);
	if(pos)
	{
		get_value(value, pos + strlen(label), 11);
		return val(value);
	}
	return 0;
}

static u32 get_valuen32(const char *param, const char *label)
{
	return (u32)get_valuen64(param, label);
}

static u16 get_port(const char *param, const char *label, u16 default_port)
{
	u16 port = (u16)get_valuen32(param, label);
	if(port)
		port = RANGE(port, 1, 65535);
	else
		port = default_port;
	return port;
}

static u8 get_valuen(const char *param, const char *label, u8 min_value, u8 max_value)
{
	u8 value = (u8)get_valuen64(param, label);
	return RANGE(value, min_value, max_value);
}

static u8 get_flag(const char *param, const char *label)
{
	if(!param || !label) return false;

	char *flag = strstr(param, label);
	if(flag)
	{
		*flag = NULL; return true;
	}
	return false;
}

#ifndef LITE_EDITION
static void check_path_tags(char *param);

static u8 parse_tags(char *text)
{
	if(!text) return 0;

	check_path_tags(text);

	u8 op = 0;
	char *pos = strstr(text, "@info");
	if(pos)
	{
		op = val(pos + 5) + 10;
		get_sys_info(pos, op % 100, (op >= 100));
	}
	return op;
}
#endif

#ifndef LITE_EDITION
static u8 get_operator(char *equal_pos, bool nullfy)
{
	if(!equal_pos) return 0;
	if(*equal_pos != '=') equal_pos = strchr(equal_pos, '=');
	if(!equal_pos) return 0;

	char *prev = (equal_pos - 1);
	char *oper = strchr("^|&+ -@", *prev);
	if(oper) {*prev = nullfy ? '\0' : *prev; return *oper;}
	//if(*prev == '^') {*prev = c; return '^';} // field^=value (XOR)
	//if(*prev == '|') {*prev = c; return '|';} // field|=value (OR)
	//if(*prev == '&') {*prev = c; return '&';} // field&=value (AND)
	//if(*prev == '+') {*prev = c; return '+';} // field+=value (ADD)
	//if(*prev == '-') {*prev = c; return '-';} // field-=value (SUB)
	//if(*prev == '@') {*prev = c; return '@';} // field@=max (CYCLE) e.g. var@=5 // cycle value from 0 to 5
	return 0;
}

static u64 update_value(u64 old_valuen, u64 new_valuen, u8 oper)
{
	if(oper == '|')
		new_valuen |= old_valuen; // OR
	if(oper == '&')
		new_valuen &= old_valuen; // AND
	if(oper == '^')
		new_valuen ^= old_valuen; // XOR
	if(oper == '+' || oper == ' ')
		new_valuen += old_valuen; // ADD
	if(oper == '-')
		new_valuen -= old_valuen; // SUB
	if(oper == '@')
		new_valuen = (old_valuen < new_valuen) ? ++old_valuen : 0; // CYCLE
	return new_valuen;
}
#endif
