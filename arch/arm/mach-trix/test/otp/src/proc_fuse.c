#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>	/*uintxx_t*/
#include <ctype.h>	/*isxdigit*/

#define PROC_OTP "/proc/otp"

#define fuse_handle_error(fmt...) do {			\
	printf("%s:%d:error:", __FILE__, __LINE__);	\
	printf(fmt);					\
	return -1;					\
}while(0)

/* proc present data in network byte order (BE) */
#define STR2HEX str2hexbe

/*
 * big number
 */
typedef struct {
	int total;	/*number of bytes data*/
	uint8_t *data;
} number_t, *number_ptr_t;

static unsigned char ch2hex(char ch, int base)
{
	int val = -1;
	if (16 == base) {
		if (ch >= '0' && ch <= '9')
			val = ch - '0';
		else if (tolower(ch) >= 'a' && tolower(ch) <= 'f')
			val = tolower(ch) - 'a' + 10;
	}
	return val;
}

/*
 * str is in little-ending form: nnNN
 * convert the input string to hex (0xNNnn) at base16 (or the first total*2
 * xdigits in case strlen(str) exceeds that length).
 * the result number is stored in host ending mode in memory.
 * return number of bytes data.
 */
static int str2hexle(const char* str, number_ptr_t pv)
{
	int i = 0, nx = 0;
	uint8_t *p = pv->data;
	const char *s = str;

	while(isxdigit(*s++))
		nx++;

	memset(p, 0, pv->total);
#ifdef PURE_BIG_ENDIAN
	s -= 2;
	for (i = 0; i < nx && (p - pv->data) < pv->total; i += 2) {
		if ((nx - i) > 1) {
			*p++ = ch2hex(*s, 16)  + (ch2hex(*(s-1), 16) << 4);
			s -= 2;
		} else {
			*p++ = ch2hex(*s, 16);
			s--;
		}
	}
#else
	s = str;
	if (nx && (nx & 1) && pv->total > nx/2)
		*p++ = ch2hex(*s++, 16);

	for (i = 0; i < nx/2 && (p - pv->data) < pv->total; i++) {
		*p++ = (ch2hex(*s, 16) << 4)  + ch2hex(*(s+1), 16);
		s += 2;
	}
#endif
	return (p - pv->data);
}

/*
 * str is in big-ending form: NNnn
 * convert the input string to hex (0xNNnn) at base16 (or the first total*2
 * xdigits in case strlen(str) exceeds that length).
 * the result number is stored in host ending mode in memory.
 * return number of bytes data
 */
static int str2hexbe(const char* str, number_ptr_t pv)
{
	int i = 0, nx = 0;
	uint8_t *p = pv->data;
	const char *s = str;

	while(isxdigit(*s++))
		nx++;

	memset(p, 0, pv->total);
#ifdef PURE_BIG_ENDIAN
	s = str;
	if (nx && (nx & 1) && pv->total > nx/2)
		*p++ = ch2hex(*s++, 16);

	for (i = 0; i < nx/2 && (p - pv->data) < pv->total; i++) {
		*p++ = (ch2hex(*s, 16) << 4)  + ch2hex(*(s+1), 16);
		s += 2;
	}
#else
	s -= 2;
	for (i = 0; i < nx && (p - pv->data) < pv->total; i += 2) {
		if ((nx - i) > 1) {
			*p++ = ch2hex(*s, 16)  + (ch2hex(*(s-1), 16) << 4);
			s -= 2;
		} else {
			*p++ = ch2hex(*s, 16);
			s--;
		}
	}
#endif
	return (p - pv->data);
}

int proc_fuse_read_generic(const char* name, const char* field, void *buf, const int len)
{
	int ret = -1;
	FILE *fp = NULL;
	char tmp[1024];

	if (snprintf(tmp, sizeof(tmp), "%s/%s", PROC_OTP, name) >= sizeof(tmp)) {
		fuse_handle_error("small buffer!\n");
	}

	if ((fp = fopen(tmp, "r")) == NULL) {
		fuse_handle_error("fopen(%s): %s\n", tmp, strerror(errno));
	}

	if (NULL == field) {
		/*
		 * fuse data
		 */
		if (fgets(tmp, sizeof(tmp), fp) != NULL) {
			number_t num = {
				.total = len,
				.data = buf,
			};
			ret = STR2HEX(tmp, &num);
		}
	} else {
		/*
		 * fuse field
		 *
		 * field_name: hex_val
		 */
		char *ch;
		while (fgets(tmp, sizeof(tmp), fp) != NULL) {
			if ((ch = strstr(tmp, field)) != NULL)
			{
				ch += strlen(field);
				if (*ch == ':') {
					uint32_t val;
					ch++;
					sscanf(ch, "%x", &val);
					memcpy(buf, &val, sizeof(uint32_t));
					ret = 4;
					break;
				}
			}
		}
	}

	fclose(fp);
	return ret;
}
