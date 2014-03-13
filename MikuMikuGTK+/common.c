#include <ctype.h>
#include <string.h>
#include "memory.h"

/**************************************
* StringCompareIgnoreCase�֐�         *
* �啶��/�������𖳎����ĕ�������r *
* ����                                *
* str1	: ��r������1                 *
* str2	: ��r������2                 *
* �Ԃ�l                              *
*	������̍�(��������Ȃ�0)         *
**************************************/
int StringCompareIgnoreCase(const char* str1, const char* str2)
{
	int ret;

	while((ret = toupper(*str1) - toupper(*str2)) == 0)
	{
		str1++, str2++;
		if(*str1 == '\0')
		{
			return 0;
		}
	}

	return ret;
}

const char* StringStringIgnoreCase(const char* str, const char* compare)
{
	char *upper = MEM_STRDUP_FUNC(str);
	char *find = MEM_STRDUP_FUNC(compare);
	char *ret = NULL;
	const char *str_point;
	char *p;
	int position;

	p = upper;
	while(*p != '\0')
	{
		*p = toupper(*p);
		p++;
	}
	p = find;
	while(*p != '\0')
	{
		*p = toupper(*p);
		p++;
	}

	str_point = strstr(upper, find);
	if(str_point != NULL)
	{
		position = (int)(upper - str_point);
		ret = &str[position];
	}
	MEM_FREE_FUNC(upper);
	MEM_FREE_FUNC(find);

	return ret;
}
