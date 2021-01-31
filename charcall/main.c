#include "test.h"
#include "charcall.h"

#include <stdio.h>
#include <string.h>

static bool test_init(void);
static bool test_simple_get_set_use(void);
static bool test_str_functionality(void);

static ftest tests[] = {
	test_init,
	test_simple_get_set_use,
	test_str_functionality,
};
//------------------------------------------------------------------------------

static bool pred(cc_byte ch)
{
	return (('h' == ch) || ('e' == ch) || ('l' == ch) || ('o' == ch)
		|| ('\0' == ch));
}

typedef struct foo {
	cc_byte * str;
	size_t pos;
} foo;

static void hello(cc_byte ch, void * arg)
{	
	cc_byte new_ch = ch;
	switch (ch)
	{
		case 'h': new_ch = 'H'; break;
		case 'e': new_ch = 'E'; break;
		case 'l': new_ch = 'L'; break;
		case 'o': new_ch = 'O'; break;
		case '\0': new_ch = '\0'; break;
		default: break;
	}
	
	foo * pf = (foo *)arg;
	pf->str[pf->pos++] = new_ch;
}

static void generic(cc_byte ch, void * arg)
{
	*((int *)arg) = ch;
}

static bool test_str_functionality(void)
{
	{
		// nothing happens
		cc_state cc_, * cc = &cc_;
		cc_init(cc);
		
		cc_call(cc, 'a');
		cc_call(cc, 'h');
		cc_call(cc, 'z');
	}

	{
		// single characters
		cc_state cc_, * cc = &cc_;
		cc_init(cc);

		int target_ = 0, * target = &target_;
		cc_fpair fp_ = {generic, target}, * fp = &fp_;
		cc_set_ch(cc, 'a', fp);
		cc_set_ch(cc, 'h', fp);
		cc_set_ch(cc, 'z', fp);
		
		check(0 == *target);
		cc_call(cc, 'a');
		check('a' == *target);
		*target = 0;
		check(0 == *target);
		cc_call(cc, 'h');
		check('h' == *target);
		*target = 0;
		check(0 == *target);
		cc_call(cc, 'z');
		check('z' == *target);
	}
	
	{
		// char string
		cc_state cc_, * cc = &cc_;
		cc_init(cc);
		
		const char str[CC_BYTE_TBL_SZ] = "hello";
		char str_out[CC_BYTE_TBL_SZ] = "foo";
		
		foo foo_str = {(cc_byte *)str_out, 0};
		
		cc_fpair fp_ = {hello, &foo_str}, * fp = &fp_;
		cc_set_ch_pred(cc, pred, fp);
		cc_set_ch_str(cc, str);
		
		check(strcmp(str_out, "HELLO") != 0);
		cc_scan(cc);
		check(6 == foo_str.pos);
		check(strcmp(str_out, "HELLO") == 0);
	}
	
	{
		// byte string
		cc_state cc_, * cc = &cc_;
		cc_init(cc);
		
		const cc_byte str[CC_BYTE_TBL_SZ] = {0xAA, 0xBB, 0xCC, 0xDD};
		cc_byte str_out[CC_BYTE_TBL_SZ] = {0, 0, 0, 0};
		
		foo foo_str = {(cc_byte *)str_out, 0};
		
		cc_fpair fp_ = {hello, &foo_str}, * fp = &fp_;
		cc_set_ch(cc, 0xAA, fp);
		cc_set_ch(cc, 0xBB, fp);
		cc_set_ch(cc, 0xCC, fp);
		cc_set_str(cc, str, 3);
		
		check(0 == foo_str.pos);
		check(0 == str_out[0]);
		check(0 == str_out[1]);
		check(0 == str_out[2]);
		check(0 == str_out[3]);
		cc_scan(cc);
		check(3 == foo_str.pos);
		check(0xAA == str_out[0]);
		check(0xBB == str_out[1]);
		check(0xCC == str_out[2]);
		check(0x00 == str_out[3]);
	}
	
	return true;
}
//------------------------------------------------------------------------------

static bool test_simple_get_set_use(void)
{
	cc_state cc_, * cc = &cc_;
	cc_init(cc);
	check(NULL == cc->str);
	check(0 == cc->len);
	check(0 == cc->pos);
	
	static const char str_[] = "hello";
	cc_byte * str = (cc_byte *)str_;
	
	cc_set_str(cc, str, strlen(str_));
	check(str == cc->str);
	check(cc_get_str(cc) == str);
	check(5 == cc->len);
	check(cc_get_str_len(cc) == cc->len);
	check(0 == cc->pos);
	check(cc_get_pos(cc) == cc->pos);
	
	check(cc_has_more_ch_n(cc, 4));
	check(!cc_has_more_ch_n(cc, 5));
	
	check('h' == cc_peek(cc));
	check(cc_get_pos(cc) == 0);
	
	cc_next(cc);
	check(cc_get_pos(cc) == 1);
	check('e' == cc_peek(cc));
	check(cc_get_pos(cc) == 1);
	
	cc_next(cc);
	check(cc_get_pos(cc) == 2);
	check('l' == cc_peek(cc));
	check(cc_get_pos(cc) == 2);
	
	cc_back(cc);
	check(cc_get_pos(cc) == 1);
	check('e' == cc_peek(cc));
	check(cc_get_pos(cc) == 1);
	check(cc_has_more_ch(cc));

	check(4 == cc_set_pos(cc, 4));
	check('o' == cc_peek(cc));
	check(!cc_has_more_ch(cc));
	
	cc_next(cc);
	check('\0' == cc_peek(cc));
	check(!cc_has_more_ch(cc));
	cc_rewind(cc);
	check('h' == cc_peek(cc));
	check(0 == cc_get_pos(cc));
	check(cc_has_more_ch_n(cc, 4));
	check(!cc_has_more_ch_n(cc, 5));
	
	return true;
}
//------------------------------------------------------------------------------

static void my_empty(cc_byte ch, void * arg) {return;}
static bool test_init(void)
{
	{
		cc_state cc_, * cc = &cc_;

		cc->str = (void*)1;
		cc->len = 2;
		cc->pos = 3;

		cc_init(cc);
		check(NULL == cc->str);
		check(0 == cc->len);
		check(0 == cc->pos);
		for (int i = 0; i < CC_BYTE_TBL_SZ; ++i)
		{
			check(cc_empty == cc->etbl[i].func);
			check(my_empty != cc->etbl[i].func);
			check(NULL == cc->etbl[i].arg);
		}
	}
	{
		cc_state cc_, * cc = &cc_;

		cc->str = (void*)1;
		cc->len = 2;
		cc->pos = 3;
		
		cc_fpair fp = {my_empty, NULL};
		cc_init_cbk(cc, &fp);
		
		check(NULL == cc->str);
		check(0 == cc->len);
		check(0 == cc->pos);
		for (int i = 0; i < CC_BYTE_TBL_SZ; ++i)
		{
			check(cc_empty != cc->etbl[i].func);
			check(my_empty == cc->etbl[i].func);
			check(NULL == cc->etbl[i].arg);
		}
	}
	
	return true;
}
//------------------------------------------------------------------------------

int run_tests(void)
{
    int i, end = sizeof(tests)/sizeof(*tests);

    int passed = 0;
    for (i = 0; i < end; ++i)
        if (tests[i]())
            ++passed;

    if (passed != end)
        putchar('\n');

    int failed = end - passed;
    report(passed, failed);
    return failed;
}
//------------------------------------------------------------------------------

int main(void)
{
	return run_tests();
}
//------------------------------------------------------------------------------
