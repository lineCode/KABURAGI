#ifndef _INCLUDED_MEMORY_STREAM_H_
#define _INCLUDED_MEMORY_STREAM_H_

#include <stdio.h>	// size_t�^���g������

// C++�ŃR���p�C������ۂ̃G���[���
#ifdef __cplusplus
extern "C" {
#endif

#ifdef EXTERN
# undef EXTERN
#endif

#ifdef _MSC_VER
# ifdef __cplusplus
#  define EXTERN extern "C" __declspec(dllexport)
# else
#  define EXTERN extern __declspec(dllexport)
# endif
#else
# define EXTERN extern
#endif

typedef struct _MEMORY_STREAM
{
	unsigned char* buff_ptr;	// �o�b�t�@
	size_t data_point;			// �f�[�^�̎Q�ƈʒu
	size_t data_size;			// �f�[�^�̗e��
	size_t block_size;			// �������ݎ��̊m�ۃu���b�N�e��
} MEMORY_STREAM, *MEMORY_STREAM_PTR;

// �֐��̃v���g�^�C�v�錾
/*********************************************************
* CreateMemoryStream�֐�                                 *
* �������̓ǂݏ������Ǘ�����\���̂̃������̊m�ۂƏ����� *
* ����                                                   *
* buff_size	: �m�ۂ���o�b�t�@�̃T�C�Y                   *
* �Ԃ�l                                                 *
*	���������ꂽ�\���̂̃A�h���X                         *
*********************************************************/
EXTERN MEMORY_STREAM_PTR CreateMemoryStream(
	size_t buff_size
);

/*****************************************************
* DeleteMemoryStream�֐�                             *
* �������̓ǂݏ������Ǘ�����\���̂̃��������J��     *
* ����                                               *
* mem	: ��������ǂݏ������Ǘ�����\���̂̃A�h���X *
* �Ԃ�l                                             *
*	���0                                            *
*****************************************************/
EXTERN int DeleteMemoryStream(MEMORY_STREAM_PTR mem);

/***************************************************************
* MemRead�֐�                                                  *
* ����������f�[�^��ǂݍ���                                   *
* ����                                                         *
* dst			: �f�[�^�̓ǂݍ��ݐ�                           *
* block_size	: �f�[�^��ǂݍ��ލۂ�1�̃u���b�N�̃T�C�Y    *
* block_num		: �ǂݍ��ރu���b�N�̌�                       *
* mem			: �ǂݍ��݌��̃��������Ǘ�����\���̂̃A�h���X *
* �Ԃ�l                                                       *
*	�ǂݍ��񂾃o�C�g��                                         *
***************************************************************/
EXTERN size_t MemRead(
	void* dst,
	size_t block_size,
	size_t block_num,
	MEMORY_STREAM_PTR mem
);

/***************************************************************
* MemWrite�֐�                                                 *
* �������փf�[�^����������                                     *
* ����                                                         *
* src                                                          *
* block_size	: �f�[�^���������ލۂ�1�̃u���b�N�̃T�C�Y    *
* block_num		: �������ރu���b�N�̐�                         *
* mem			: �������ݐ�̃��������Ǘ�����\���̂̃A�h���X *
* �Ԃ�l                                                       *
*	�������񂾃o�C�g��                                         *
***************************************************************/
EXTERN size_t MemWrite(
	const void* src,
	size_t block_size,
	size_t block_num,
	MEMORY_STREAM_PTR mem
);

/***************************************************************
* MemSeek�֐�                                                  *
* �V�[�N���s��                                                 *
* ����                                                         *
* mem		: �V�[�N���s�����������Ǘ����Ă���\���̂̃A�h���X *
* offset	: �ړ�����o�C�g��                                 *
* origin	: �ړ����J�n����ʒu(fseek�Ɠ���)                  *
* �Ԃ�l                                                       *
*	����I��(0)�A�ُ�I��(0�ȊO)                               *
***************************************************************/
EXTERN int MemSeek(MEMORY_STREAM_PTR mem, long offset, int origin);

/***************************************************************
* MemSeek64�֐�                                                *
* �V�[�N���s��(64�r�b�g���b�v��)                               *
* ����                                                         *
* mem		: �V�[�N���s�����������Ǘ����Ă���\���̂̃A�h���X *
* offset	: �ړ�����o�C�g��                                 *
* origin	: �ړ����J�n����ʒu(fseek�Ɠ���)                  *
* �Ԃ�l                                                       *
*	����I��(0)�A�ُ�I��(0�ȊO)                               *
***************************************************************/
EXTERN int MemSeek64(MEMORY_STREAM_PTR mem, long long int offset, int origin);

/*****************************************************
* MemTell�֐�                                        *
* �f�[�^�̎Q�ƈʒu��Ԃ�                             *
* mem	: ��������ǂݏ������Ǘ�����\���̂̃A�h���X *
* �Ԃ�l                                             *
*	�f�[�^�̎Q�ƈʒu                                 *
*****************************************************/
EXTERN long MemTell(MEMORY_STREAM_PTR mem);

/***************************************************************
* MemGets�֐�                                                  *
* 1�s�ǂݍ���                                                  *
* ����                                                         *
* string	: �ǂݍ��ޕ�������i�[����A�h���X                 *
* n			: �ǂݍ��ލő�̕�����                             *
* mem		: �V�[�N���s�����������Ǘ����Ă���\���̂̃A�h���X *
* �Ԃ�l                                                       *
*	����(string)�A���s(NULL)                                   *
***************************************************************/
EXTERN char* MemGets(char* string, int n, MEMORY_STREAM_PTR mem);

// C++�ŃR���p�C������ۂ̃G���[���
#ifdef __cplusplus
}
#endif

#endif	// _INCLUDED_MEMORY_STREAM_H_
