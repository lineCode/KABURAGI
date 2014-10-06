#ifndef _INCLUDED_UTILS_H_
#define _INCLUDED_UTILS_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SWAP_LE_BE(val)	((uint32) ( \
	(((uint32) (val) & (uint32) 0x000000ffU) << 24) | \
	(((uint32) (val) & (uint32) 0x0000ff00U) <<  8) | \
	(((uint32) (val) & (uint32) 0x00ff0000U) >>  8) | \
	(((uint32) (val) & (uint32) 0xff000000U) >> 24)))

#define UINT32_FROM_BE(value) SWAP_LE_BE(value)

typedef struct _BYTE_ARRAY
{
	uint8 *buffer;
	size_t num_data;
	size_t buffer_size;
	size_t block_size;
} BYTE_ARRAY;

typedef struct _WORD_ARRAY
{
	uint16 *buffer;
	size_t num_data;
	size_t buffer_size;
	size_t block_size;
} WORD_ARRAY;

typedef struct _UINT32_ARRAY
{
	uint32 *buffer;
	size_t num_data;
	size_t buffer_size;
	size_t block_size;
} UINT32_ARRAY;

typedef struct _POINTER_ARRAY
{
	void **buffer;
	size_t num_data;
	size_t buffer_size;
	size_t block_size;
} POINTER_ARRAY;

typedef struct _STRUCT_ARRAY
{
	uint8 *buffer;
	size_t num_data;
	size_t buffer_size;
	size_t block_size;
	size_t data_size;
} STRUCT_ARRAY;

/**************************************
* StringCompareIgnoreCase�֐�         *
* �啶��/�������𖳎����ĕ�������r *
* ����                                *
* str1	: ��r������1                 *
* str2	: ��r������2                 *
* �Ԃ�l                              *
*	������̍�(��������Ȃ�0)         *
**************************************/
EXTERN int StringCompareIgnoreCase(const char* str1, const char* str2);

/**************************************
* memset32�֐�                        *
* 32bit�����Ȃ������Ńo�b�t�@�𖄂߂� *
* ����                                *
* buff	: ���߂�Ώۂ̃o�b�t�@        *
* value	: ���߂�l                    *
* size	: ���߂�o�C�g��              *
**************************************/
EXTERN void memset32(void* buff, uint32 value, size_t size);

/**************************************
* memset64�֐�                        *
* 64bit�����Ȃ������Ńo�b�t�@�𖄂߂� *
* ����                                *
* buff	: ���߂�Ώۂ̃o�b�t�@        *
* value	: ���߂�l                    *
* size	: ���߂�o�C�g��              *
**************************************/
EXTERN void memset64(void* buff, uint64 value, size_t size);

EXTERN BYTE_ARRAY* ByteArrayNew(size_t block_size);
EXTERN void ByteArrayAppend(BYTE_ARRAY* barray, uint8 data);
EXTERN void ByteArrayDesteroy(BYTE_ARRAY** barray);
EXTERN void ByteArrayResize(BYTE_ARRAY* barray, size_t new_size);

EXTERN WORD_ARRAY* WordArrayNew(size_t block_size);
EXTERN void WordArrayAppend(WORD_ARRAY* warray, uint16 data);
EXTERN void WordArrayDestroy(WORD_ARRAY** warray);
EXTERN void WordArrayResize(WORD_ARRAY* warray, size_t new_size);

EXTERN UINT32_ARRAY* Uint32ArrayNew(size_t block_size);
EXTERN void Uint32ArrayAppend(UINT32_ARRAY* uarray, uint32 data);
EXTERN void Uint32ArrayDestroy(UINT32_ARRAY** uarray);
EXTERN void Uint32ArrayResize(UINT32_ARRAY* uint32_array, size_t new_size);

EXTERN POINTER_ARRAY* PointerArrayNew(size_t block_size);
EXTERN void PointerArrayRelease(
	POINTER_ARRAY* pointer_array,
	void (*destroy_func)(void*)
);
EXTERN void PointerArrayDestroy(
	POINTER_ARRAY** pointer_array,
	void (*destroy_func)(void*)
);
void PointerArrayAppend(POINTER_ARRAY* pointer_array, void* data);
EXTERN void PointerArrayRemoveByIndex(
	POINTER_ARRAY* pointer_array,
	size_t index,
	void (*destroy_func)(void*)
);
EXTERN void PointerArrayRemoveByData(
	POINTER_ARRAY* pointer_array,
	void* data,
	void (*destroy_func)(void*)
);
EXTERN int PointerArrayDoesCointainData(POINTER_ARRAY* pointer_array, void* data);

EXTERN STRUCT_ARRAY* StructArrayNew(size_t data_size, size_t block_size);
EXTERN void StructArrayDestroy(
	STRUCT_ARRAY** struct_array,
	void (*destroy_func)(void*)
);
EXTERN void StructArrayAppend(STRUCT_ARRAY* struct_array, void* data);
EXTERN void* StructArrayReserve(STRUCT_ARRAY* struct_array);
EXTERN void StructArrayResize(STRUCT_ARRAY* struct_array, size_t new_size);
EXTERN void StructArrayRemoveByIndex(
	STRUCT_ARRAY* struct_array,
	size_t index,
	void (*destroy_func)(void*)
);

#ifdef _MSC_VER
EXTERN int strncasecmp(const char* s1, const char* s2, size_t n);
#endif

/**************************************************************
* StringStringIgnoreCase�֐�                                  *
* �啶��/�������𖳎����Ĉ���1�̕����񂩂����2�̕������T�� *
* str		: �����Ώۂ̕�����                                *
* search	: �������镶����                                  *
* �Ԃ�l                                                      *
*	�����񔭌�:���������ʒu�̃|�C���^	������Ȃ�:NULL     *
**************************************************************/
EXTERN char* StringStringIgnoreCase(const char* str, const char* search);

/*********************************
* GetFileExtention�֐�           *
* �t�@�C��������g���q���擾���� *
* ����                           *
* file_name	: �t�@�C����         *
* �Ԃ�l                         *
*	�g���q�̕�����               *
*********************************/
EXTERN char* GetFileExtention(char* file_name);

/***************************************
* StringReplace�֐�                    *
* �w�肵���������u������             *
* ����                                 *
* str			: ���������镶����     *
* replace_from	: �u���������镶���� *
* replace_to	: �u�������镶����     *
***************************************/
EXTERN void StringRepalce(
	char* str,
	char* replace_from,
	char* replace_to
);

EXTERN int ForFontFamilySearchCompare(const char* str, PangoFontFamily** font);

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
EXTERN size_t FileRead(void* dst, size_t block_size, size_t read_num, GFileInputStream* stream);

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
EXTERN size_t FileWrite(void* src, size_t block_size, size_t read_num, GFileOutputStream* stream);

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
EXTERN int FileSeek(void* stream, long offset, int origin);

/************************************************
* FileSeekTell�֐�                              *
* Gtk�̊֐��𗘗p���ăt�@�C���̃V�[�N�ʒu��Ԃ� *
* ����                                          *
* stream	: �V�[�N�ʒu�𒲂ׂ�X�g���[��      *
* �Ԃ�l                                        *
*	�V�[�N�ʒu                                  *
************************************************/
EXTERN long FileSeekTell(void* stream);

/***********************************************
* InvertMatrix�֐�                             *
* �t�s����v�Z����                             *
* ����                                         *
* a	: �v�Z�Ώۂ̍s��(�t�s��f�[�^�͂����ɓ���) *
* n	: �s��̎���                               *
***********************************************/
EXTERN void InvertMatrix(FLOAT_T **a, int n);

/*********************************
* FLAG_CHECK�}�N���֐�           *
* �t���O��ON/OFF�𔻒肷��       *
* ����                           *
* FLAGS	: �t���O�z��             *
* ID	: �`�F�b�N����t���O��ID *
* �Ԃ�l                         *
*	ON:0�ȊO OFF:0               *
*********************************/
#define FLAG_CHECK(FLAGS, ID) (FLAGS[((ID)/8)] & (1 <<((ID)%8)))

/*****************************
* FLAG_ON�}�N���֐�          *
* �t���O��ON�ɂ���           *
* ����                       *
* FLAGS	: �t���O�z��         *
* ID	: ON�ɂ���t���O��ID *
*****************************/
#define FLAG_ON(FLAGS, ID) FLAGS[((ID)/8)] |= (1 << ((ID)%8))

/******************************
* FLAG_OFF�}�N���֐�          *
* �t���O��OFF�ɂ���           *
* ����                        *
* FLAGS	: �t���O�z��          *
* ID	: OFF�ɂ���t���O��ID *
*****************************/
#define FLAG_OFF(FLAGS, ID) FLAGS[((ID)/8)] &= ~(1 << ((ID)%8))

EXTERN void AdjustmentChangeValueCallBackInt(GtkAdjustment* adjustment, int* store);
EXTERN void AdjustmentChangeValueCallBackUint(GtkAdjustment* adjustment, unsigned int* store);
EXTERN void AdjustmentChangeValueCallBackInt8(GtkAdjustment* adjustment, int8* store);
EXTERN void AdjustmentChangeValueCallBackUint8(GtkAdjustment* adjustment, uint8* store);
EXTERN void AdjustmentChangeValueCallBackInt16(GtkAdjustment* adjustment, int16* store);
EXTERN void AdjustmentChangeValueCallBackUint16(GtkAdjustment* adjustment, uint16* store);
EXTERN void AdjustmentChangeValueCallBackInt32(GtkAdjustment* adjustment, int32* store);
EXTERN void AdjustmentChangeValueCallBackUint32(GtkAdjustment* adjustment, uint32* store);
EXTERN void AdjustmentChangeValueCallBackDouble(GtkAdjustment* adjustment, gdouble* value);

EXTERN void CheckButtonSetFlagsCallBack(GtkWidget* button, guint32* flags, guint32 flag_value);

/*******************************************************
* InflateData�֐�                                      *
* ZIP���k���ꂽ�f�[�^���f�R�[�h����                    *
* ����                                                 *
* data				: ���̓f�[�^                       *
* out_buffer		: �o�͐�̃o�b�t�@                 *
* in_size			: ���̓f�[�^�̃o�C�g��             *
* out_buffer_size	: �o�͐�̃o�b�t�@�̃T�C�Y         *
* out_size			: �o�͂����o�C�g���̊i�[��(NULL��) *
* �Ԃ�l                                               *
*	����I��:0�A���s:0�ȊO                             *
*******************************************************/
EXTERN int InflateData(
	uint8* data,
	uint8* out_buffer,
	size_t in_size,
	size_t out_buffer_size,
	size_t* out_size
);

/***************************************************
* DeflateData�֐�                                  *
* ZIP���k���s��                                    *
* ����                                             *
* data					: ���̓f�[�^               *
* out_buffer			: �o�͐�̃o�b�t�@         *
* target_data_size		: ���̓f�[�^�̃o�C�g��     *
* out_buffer_size		: �o�͐�̃o�b�t�@�̃T�C�Y *
* compressed_data_size	: ���k��̃o�C�g���i�[��   *
* compress_level		: ���k���x��(0�`9)         *
* �Ԃ�l                                           *
*	����I��:0�A���s:0�ȊO                         *
***************************************************/
EXTERN int DeflateData(
	uint8* data,
	uint8* out_buffer,
	size_t target_data_size,
	size_t out_buffer_size,
	size_t* compressed_data_size,
	int compress_level
);

EXTERN void UpdateWidget(GtkWidget* widget);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_UTILS_H_
