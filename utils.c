// Visual Studio 2005�ȍ~�ł͌Â��Ƃ����֐����g�p����̂�
	// �x�����o�Ȃ��悤�ɂ���
#if defined _MSC_VER && _MSC_VER >= 1400
# define _CRT_SECURE_NO_DEPRECATE
# define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <ctype.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include "memory.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef _MSC_VER
int strncasecmp(const char* s1, const char* s2, size_t n)
{
	if(n == 0)
	{
		return 0;
	}

	while(n-- != 0 && tolower(*s1) == tolower(*s2))
	{
		if(n == 0 || *s1 == '\0' || *s2 == '\0')
		{
			break;
		}

		s1++,	s2++;
	}

	return toupper((int)(*(unsigned char*)s1)) - tolower(*s2);
}
#endif

/**************************************************************
* StringStringIgnoreCase�֐�                                  *
* �啶��/�������𖳎����Ĉ���1�̕����񂩂����2�̕������T�� *
* str		: �����Ώۂ̕�����                                *
* search	: �������镶����                                  *
* �Ԃ�l                                                      *
*	�����񔭌�:���������ʒu�̃|�C���^	������Ȃ�:NULL     *
**************************************************************/
char* StringStringIgnoreCase(const char* str, const char* search)
{
	const char *ret = NULL;
	char *upper_str = MEM_STRDUP_FUNC(str);
	char *upper_search = MEM_STRDUP_FUNC(search);

	if(str != NULL && search != NULL)
	{
		char *c;
		char *upper_ret;

		c = upper_str;
		while(*c != '\0')
		{
			*c = toupper(*c);
			c++;
		}

		c = upper_search;
		while(*c != '\0')
		{
			*c = toupper(*c);
			c++;
		}

		upper_ret = strstr(upper_str, upper_search);
		if(upper_ret != NULL)
		{
			ret = &str[upper_ret - upper_str];
		}

		MEM_FREE_FUNC(upper_str);
		MEM_FREE_FUNC(upper_search);
	}

	return (char*)ret;
}

/**************************************
* memset32�֐�                        *
* 32bit�����Ȃ������Ńo�b�t�@�𖄂߂� *
* ����                                *
* buff	: ���߂�Ώۂ̃o�b�t�@        *
* value	: ���߂�l                    *
* size	: ���߂�o�C�g��              *
**************************************/
void memset32(void* buff, uint32 value, size_t size)
{
	size_t i;
	for(i=0; i<(size & (~(sizeof(value) - 1))); i+=sizeof(value))
	{
		(void)memcpy(((char*)buff)+i, &value, sizeof(value));
	}
	for( ; i<size; i++)
	{
		((char*)buff)[i] = ((char*)&value)[i&(sizeof(value)-1)];
	}
}

/**************************************
* memset64�֐�                        *
* 64bit�����Ȃ������Ńo�b�t�@�𖄂߂� *
* ����                                *
* buff	: ���߂�Ώۂ̃o�b�t�@        *
* value	: ���߂�l                    *
* size	: ���߂�o�C�g��              *
**************************************/
void memset64(void* buff, uint64 value, size_t size)
{
	size_t i;
	for(i=0; i<(size & (~(sizeof(value) - 1))); i+=sizeof(value))
	{
		(void)memcpy(((char*)buff)+i, &value, sizeof(value));
	}
	for( ; i<size; i++)
	{
		((char*)buff)[i] = ((char*)&value)[i&(sizeof(value)-1)];
	}
}

/***************************************
* StringReplace�֐�                    *
* �w�肵���������u������             *
* ����                                 *
* str			: ���������镶����     *
* replace_from	: �u���������镶���� *
* replace_to	: �u�������镶����     *
***************************************/
void StringRepalce(
	char* str,
	char* replace_from,
	char* replace_to
)
{
	char work[1024];
	char* p = str;

	while((p = strstr(str, replace_from)) != NULL)
	{
		*p = '\0';
		p += strlen(replace_from);
		(void)strcpy(work, p);
		(void)strcat(str, replace_to);
		(void)strcat(str, work);
	}
}

int ForFontFamilySearchCompare(const char* str, PangoFontFamily** font)
{
	return StringCompareIgnoreCase(str, pango_font_family_get_name(*font));
}

/*******************************************
* FileRead�֐�                             *
* Gtk�̊֐��𗘗p���ăt�@�C���ǂݍ���      *
* ����                                     *
* dst			: �ǂݍ��ݐ�̃A�h���X     *
* block_size	: �ǂݍ��ރu���b�N�̃T�C�Y *
* read_num		: �ǂݍ��ރu���b�N�̐�     *
* stream		: �ǂݍ��݌��̃X�g���[��   *
* �Ԃ�l                                   *
*	�ǂݍ��񂾃u���b�N��                   *
*******************************************/
size_t FileRead(void* dst, size_t block_size, size_t read_num, GFileInputStream* stream)
{
	return g_input_stream_read(
		G_INPUT_STREAM(stream), dst, block_size*read_num, NULL, NULL) / block_size;
}

/*******************************************
* FileWrite�֐�                            *
* Gtk�̊֐��𗘗p���ăt�@�C����������      *
* ����                                     *
* src			: �������݌��̃A�h���X     *
* block_size	: �������ރu���b�N�̃T�C�Y *
* read_num		: �������ރu���b�N�̐�     *
* stream		: �������ݐ�̃X�g���[��   *
* �Ԃ�l                                   *
*	�������񂾃u���b�N��                   *
*******************************************/
size_t FileWrite(void* src, size_t block_size, size_t read_num, GFileOutputStream* stream)
{
	return g_output_stream_write(
		G_OUTPUT_STREAM(stream), src, block_size*read_num, NULL, NULL) / block_size;
}

/********************************************
* FileSeek�֐�                              *
* Gtk�̊֐��𗘗p���ăt�@�C���V�[�N         *
* ����                                      *
* stream	: �V�[�N���s���X�g���[��        *
* offset	: �ړ��o�C�g��                  *
* origin	: �ړ��J�n�ʒu(fseek�֐��Ɠ���) *
* �Ԃ�l                                    *
*	����I��(0), �ُ�I��(0�ȊO)            *
********************************************/
int FileSeek(void* stream, long offset, int origin)
{
	GSeekType seek;

	switch(origin)
	{
	case SEEK_SET:
		seek = G_SEEK_SET;
		break;
	case SEEK_CUR:
		seek = G_SEEK_CUR;
		break;
	case SEEK_END:
		seek = G_SEEK_END;
		break;
	default:
		return -1;
	}

	return !(g_seekable_seek(G_SEEKABLE(stream), offset, seek, NULL, NULL));
}

/************************************************
* FileSeekTell�֐�                              *
* Gtk�̊֐��𗘗p���ăt�@�C���̃V�[�N�ʒu��Ԃ� *
* ����                                          *
* stream	: �V�[�N�ʒu�𒲂ׂ�X�g���[��      *
* �Ԃ�l                                        *
*	�V�[�N�ʒu                                  *
************************************************/
long FileSeekTell(void* stream)
{
	return (long)g_seekable_tell(G_SEEKABLE(stream));
}

/***********************************************
* InvertMatrix�֐�                             *
* �t�s����v�Z����                             *
* ����                                         *
* a	: �v�Z�Ώۂ̍s��(�t�s��f�[�^�͂����ɓ���) *
* n	: �s��̎���                               *
***********************************************/
void InvertMatrix(FLOAT_T **a, int n)
{
	FLOAT_T **inv_a = (FLOAT_T**)MEM_ALLOC_FUNC(
		sizeof(*inv_a)*n);
	FLOAT_T buf;
	int maximum;
	int i, j, k;

	inv_a[0] = (FLOAT_T*)MEM_ALLOC_FUNC(
		sizeof(**inv_a)*(n*n));
	for(i=1; i<n; i++)
	{
		inv_a[i] = inv_a[i-1] + n;
	}
	(void)memset(inv_a[0], 0, sizeof(**inv_a)*(n*n));

	for(i=0; i<n; i++)
	{
		inv_a[i][i] = 1;
	}

	for(i=0; i<n; i++)
	{
		maximum = i;
		for(j=i+1; j<n; j++)
		{
			if(fabs(a[j][i]) > fabs(a[maximum][i]))
			{
				maximum = j;
			}
		}

		if(maximum != i)
		{
			for(k=0; k<n; k++)
			{
				buf = a[maximum][k];
				a[maximum][k] = a[i][k];
				a[i][k] = buf;

				buf = inv_a[maximum][k];
				inv_a[maximum][k] = inv_a[i][k];
				inv_a[i][k] = buf;
			}
		}

		buf = a[i][i];
		for(k=0; k<n; k++)
		{
			a[i][k] /= buf;
			inv_a[i][k] /= buf;
		}

		for(j=0; j<n; j++)
		{
			if(j != i)
			{
				buf = a[j][i] / a[i][i];
				for(k=0; k<n; k++)
				{
					a[j][k] = a[j][k] - a[i][k] * buf;
					inv_a[j][k] = inv_a[j][k] - inv_a[i][k] * buf;
				}
			}
		}
	}

	for(i=0; i<n; i++)
	{
		for(j=0; j<n; j++)
		{
			a[i][j] = inv_a[i][j];
		}
	}

	MEM_FREE_FUNC(inv_a[0]);
	MEM_FREE_FUNC(inv_a);
}

#ifndef _WIN32
#include <stdlib.h>

 char* utoa(unsigned val, char *buf, int radix){
     char *p = NULL;
     char *s = "0123456789abcdefghijklmnopqrstuvwxyz";
     if(radix == 0) {
         radix = 10;
     }
     if(buf == NULL) {
         return NULL;
     }
     if(val < (unsigned)radix) {
         buf[0] = s[val];
         buf[1] = '\0';
     } else {
         for(p = utoa(val / ((unsigned)radix), buf, radix); *p; p++);
         utoa(val % ((unsigned)radix), p, radix);
     }
     return buf;
 }

 char* itoa(int val, char *buf, int radix) {
     char *p;
     unsigned u;

     p = buf;
     if(radix == 0) {
         radix = 10;
     }
     if(buf == NULL) {
         return NULL;
     }
     if(val < 0) {
         *p++ = '-';
         u = -val;
     } else {
         u = val;
     }
     utoa(u, p, radix);

     return buf;
 }
#endif

void AdjustmentChangeValueCallBackInt8(GtkAdjustment* adjustment, int8* store)
{
	void (*func)(void*) = g_object_get_data(G_OBJECT(adjustment), "changed_callback");
	void *func_data = g_object_get_data(G_OBJECT(adjustment), "callback_data");

	*store = (int8)gtk_adjustment_get_value(adjustment);

	if(func != NULL)
	{
		func(func_data);
	}
}

void AdjustmentChangeValueCallBackUint8(GtkAdjustment* adjustment, uint8* store)
{
	void (*func)(void*) = g_object_get_data(G_OBJECT(adjustment), "changed_callback");
	void *func_data = g_object_get_data(G_OBJECT(adjustment), "callback_data");

	*store = (uint8)gtk_adjustment_get_value(adjustment);

	if(func != NULL)
	{
		func(func_data);
	}
}

void AdjustmentChangeValueCallBackInt16(GtkAdjustment* adjustment, int16* store)
{
	void (*func)(void*) = g_object_get_data(G_OBJECT(adjustment), "changed_callback");
	void *func_data = g_object_get_data(G_OBJECT(adjustment), "callback_data");

	*store = (int16)gtk_adjustment_get_value(adjustment);

	if(func != NULL)
	{
		func(func_data);
	}
}

void AdjustmentChangeValueCallBackUint16(GtkAdjustment* adjustment, uint16* store)
{
	void (*func)(void*) = g_object_get_data(G_OBJECT(adjustment), "changed_callback");
	void *func_data = g_object_get_data(G_OBJECT(adjustment), "callback_data");

	*store = (uint16)gtk_adjustment_get_value(adjustment);

	if(func != NULL)
	{
		func(func_data);
	}
}

void AdjustmentChangeValueCallBackInt32(GtkAdjustment* adjustment, int32* store)
{
	void (*func)(void*) = g_object_get_data(G_OBJECT(adjustment), "changed_callback");
	void *func_data = g_object_get_data(G_OBJECT(adjustment), "callback_data");

	*store = (int32)gtk_adjustment_get_value(adjustment);

	if(func != NULL)
	{
		func(func_data);
	}
}

void AdjustmentChangeValueCallBackUint32(GtkAdjustment* adjustment, uint32* store)
{
	void (*func)(void*) = g_object_get_data(G_OBJECT(adjustment), "changed_callback");
	void *func_data = g_object_get_data(G_OBJECT(adjustment), "callback_data");

	*store = (uint32)gtk_adjustment_get_value(adjustment);

	if(func != NULL)
	{
		func(func_data);
	}
}

void AdjustmentChangeVaueCallBackDouble(GtkAdjustment* adjustment, gdouble* value)
{
	void (*func)(void*) = g_object_get_data(G_OBJECT(adjustment), "changed_callback");
	void *func_data = g_object_get_data(G_OBJECT(adjustment), "callback_data");

	*value = gtk_adjustment_get_value(adjustment);

	if(func != NULL)
	{
		func(func_data);
	}
}

static void CheckButtonChangeFlags(GtkWidget* button, guint32* flags)
{
	guint32 flag_value = (guint32)GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(button), "flag-value"));
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)) == FALSE)
	{
		*flags &= ~flag_value;
	}
	else
	{
		*flags |= flag_value;
	}
}

void CheckButtonSetFlagsCallBack(GtkWidget* button, guint32* flags, guint32 flag_value)
{
	g_object_set_data(G_OBJECT(button), "flag-value", GUINT_TO_POINTER(flag_value));
	g_signal_connect(G_OBJECT(button), "toggled",
		G_CALLBACK(CheckButtonChangeFlags), flags);
}

#ifdef __cplusplus
}
#endif
