/* charcall.h -- a character based language parser
   v1.0

   charcall allows you to associate a character, or a byte value, with a
   callback function. Then you can associate a string with the resulting
   callback table. This, in turn, allows you to execute the callbacks while
   iterating over the string. Effectively, this turns out as execution of custom
   actions specified by the string - a character based language.
   
   Author: Vladimir Dinev
   vld.dinev@gmail.com
   2021-01-31
*/

#ifndef CHARCALL_H
#define CHARCALL_H

#include <string.h>
#include <stdbool.h>

typedef unsigned char cc_byte;
typedef void(*cc_callback)(cc_byte ch, void * arg);
typedef bool(*cc_pred)(cc_byte ch);

typedef struct cc_fpair {
	cc_callback func;
	void * arg;
} cc_fpair;

#define CC_BYTE_TBL_SZ (0xFF+1)

typedef struct cc_state {
	cc_fpair etbl[CC_BYTE_TBL_SZ];
	const cc_byte * str;
	size_t len;
	size_t pos;
} cc_state;

static inline void cc_set_ch_pred(cc_state * st, cc_pred pred, cc_fpair * cbk);

static void cc_empty(cc_byte ch, void * arg) {return;}
static bool cc_true(cc_byte ch) {return true;}
static inline void cc_init_cbk(cc_state * st, cc_fpair * cbk)
{
	memset(st, 0, sizeof(*st));
	cc_fpair fp_ = {cc_empty, NULL};
	cc_fpair * fp = (cbk) ? cbk : &fp_;
	cc_set_ch_pred(st, cc_true, fp);
}
#define cc_init(pst) cc_init_cbk((pst), NULL)

static inline void cc_set_ch(cc_state * st, cc_byte ch, cc_fpair * cbk)
{
	st->etbl[ch] = *cbk;
}
static inline void cc_set_ch_pred(cc_state * st, cc_pred pred, cc_fpair * cbk)
{
	for (int i = 0; i < CC_BYTE_TBL_SZ; ++i)
	{
		if (pred(i))
			cc_set_ch(st, i, cbk);
	}
}
static inline void cc_set_str(cc_state * st, const cc_byte * str, size_t len)
{
	st->str = str;
	st->len = len;
}
static inline void cc_set_ch_str(cc_state * st, const char * str)
{
	cc_set_str(st, (cc_byte *)str, strlen(str)+1);
}
static inline const cc_byte * cc_get_str(cc_state * st)
{
	return st->str;
}
static inline size_t cc_get_str_len(cc_state * st)
{
	return st->len;
}
static inline size_t cc_set_pos(cc_state * st, size_t pos)
{
	return st->pos = pos;
}
static inline size_t cc_get_pos(cc_state * st)
{
	return st->pos;
}
static inline void cc_call(cc_state * st, cc_byte ch)
{
	cc_fpair * fp = &(st->etbl[ch]);
	fp->func(ch, fp->arg);
}
static inline cc_byte cc_peak(cc_state * st)
{
	return st->str[st->pos];
}
static inline void cc_next(cc_state * st)
{
	st->pos++;
}
static inline void cc_back(cc_state * st)
{
	st->pos--;
}
static inline void cc_rewind(cc_state * st)
{
	st->pos = 0;
}
static inline bool cc_has_more_ch_n(cc_state * st, size_t n)
{
	return ((st->pos + n) < st->len);
}
#define cc_has_more_ch(pst) cc_has_more_ch_n((pst), 1)

static inline void cc_scan(cc_state * st)
{
	while (st->pos < st->len)
	{
		cc_call(st, cc_peak(st));
		cc_next(st);
	}
}

#endif
