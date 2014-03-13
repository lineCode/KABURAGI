#ifndef _INCLUDED_UTILS_H_
#define _INCLUDED_UTILS_H_

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
extern int StringCompareIgnoreCase(const char* str1, const char* str2);

/**************************************
* memset32�֐�                        *
* 32bit�����Ȃ������Ńo�b�t�@�𖄂߂� *
* ����                                *
* buff	: ���߂�Ώۂ̃o�b�t�@        *
* value	: ���߂�l                    *
* size	: ���߂�o�C�g��              *
**************************************/
extern void memset32(void* buff, uint32 value, size_t size);

/**************************************
* memset64�֐�                        *
* 64bit�����Ȃ������Ńo�b�t�@�𖄂߂� *
* ����                                *
* buff	: ���߂�Ώۂ̃o�b�t�@        *
* value	: ���߂�l                    *
* size	: ���߂�o�C�g��              *
**************************************/
extern void memset64(void* buff, uint64 value, size_t size);

#ifdef _MSC_VER
extern int strncasecmp(const char* s1, const char* s2, size_t n);
#endif

/**************************************************************
* StringStringIgnoreCase�֐�                                  *
* �啶��/�������𖳎����Ĉ���1�̕����񂩂����2�̕������T�� *
* str		: �����Ώۂ̕�����                                *
* search	: �������镶����                                  *
* �Ԃ�l                                                      *
*	�����񔭌�:���������ʒu�̃|�C���^	������Ȃ�:NULL     *
**************************************************************/
extern char* StringStringIgnoreCase(const char* str, const char* search);

/***************************************
* StringReplace�֐�                    *
* �w�肵���������u������             *
* ����                                 *
* str			: ���������镶����     *
* replace_from	: �u���������镶���� *
* replace_to	: �u�������镶����     *
***************************************/
extern void StringRepalce(
	char* str,
	char* replace_from,
	char* replace_to
);

extern int ForFontFamilySearchCompare(const char* str, PangoFontFamily** font);

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
extern size_t FileRead(void* dst, size_t block_size, size_t read_num, GFileInputStream* stream);

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
extern size_t FileWrite(void* src, size_t block_size, size_t read_num, GFileOutputStream* stream);

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
extern int FileSeek(void* stream, long offset, int origin);

/************************************************
* FileSeekTell�֐�                              *
* Gtk�̊֐��𗘗p���ăt�@�C���̃V�[�N�ʒu��Ԃ� *
* ����                                          *
* stream	: �V�[�N�ʒu�𒲂ׂ�X�g���[��      *
* �Ԃ�l                                        *
*	�V�[�N�ʒu                                  *
************************************************/
extern long FileSeekTell(void* stream);

/***********************************************
* InvertMatrix�֐�                             *
* �t�s����v�Z����                             *
* ����                                         *
* a	: �v�Z�Ώۂ̍s��(�t�s��f�[�^�͂����ɓ���) *
* n	: �s��̎���                               *
***********************************************/
extern void InvertMatrix(FLOAT_T **a, int n);

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

extern void AdjustmentChangeValueCallBackInt8(GtkAdjustment* adjustment, int8* store);
extern void AdjustmentChangeValueCallBackUint8(GtkAdjustment* adjustment, uint8* store);
extern void AdjustmentChangeValueCallBackInt16(GtkAdjustment* adjustment, int16* store);
extern void AdjustmentChangeValueCallBackUint16(GtkAdjustment* adjustment, uint16* store);
extern void AdjustmentChangeValueCallBackInt32(GtkAdjustment* adjustment, int32* store);
extern void AdjustmentChangeValueCallBackUint32(GtkAdjustment* adjustment, uint32* store);
extern void AdjustmentChangeVaueCallBackDouble(GtkAdjustment* adjustment, gdouble* value);

extern void CheckButtonSetFlagsCallBack(GtkWidget* button, guint32* flags, guint32 flag_value);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_UTILS_H_
