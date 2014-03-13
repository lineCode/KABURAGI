#ifndef _INCLUDED_PATTERN_H_
#define _INCLUDED_PATTERN_H_

#include <gtk/gtk.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// �p�^�[���̍��E���]�A�㉺���]�̃t���O
typedef enum _PATTERN_FLAGS
{
	PATTERN_FLIP_HORIZONTALLY = 0x01,
	PATTERN_FLIP_VERTICALLY = 0x02
} PATTERN_FLAGS;

// �p�^�[���̃`�����l������2�̂Ƃ��̃��[�h
typedef enum _PATTERN_MODE
{
	PATTERN_MODE_SATURATION,
	PATTERN_MODE_BRIGHTNESS,
	PATTERN_MODE_FORE_TO_BACK,
	PATTERN_MODE_NUM
} PATTERN_MODE;

#ifndef INCLUDE_WIN_DEFAULT_API
/****************************
* PATTERN�\����             *
* �p�^�[���f�[�^1���̏�� *
****************************/
typedef struct _PATTERN
{
	// �摜�f�[�^�̕��A�����A��s���̃o�C�g��
	gint32 width, height, stride;
	// �摜�f�[�^�̃`�����l����
	uint8 channel;
	// �摜�̃s�N�Z���f�[�^
	uint8* pixels;
} PATTERN;
#else
#include <wingdi.h>
#endif

/*************************
* PATTERNS�\����         *
* �g�p�\�ȃp�^�[����� *
*************************/
typedef struct _PATTERNS
{
	// �p�^�[�����ƃA�N�e�B�u�ȃp�^�[��
	PATTERN* patterns, *active_pattern;
	// �N���b�v�{�[�h�̃p�^�[��
	PATTERN clip_board;
	// �N���b�v�{�[�h�̃p�^�[�������鎞�̃t���O
	int has_clip_board_pattern;
	// �g�p�\�ȃp�^�[���̐�
	int32 num_pattern;
	// RGBA�ɕϊ������p�^�[���̃o�C�g�f�[�^
	uint8* pattern_pixels;
	// �`�����l����2�̂Ƃ��̃}�X�N�p
	uint8* pattern_mask;
	// RGBA�ϊ����̈ꎞ�ۑ�
	uint8* pattern_pixels_temp;
	// �p�^�[���摜�̍ő�o�C�g��
	size_t pattern_max_byte;
	// �p�^�[���̊g�嗦
	gdouble scale;
} PATTERNS;

// �֐��̃v���g�^�C�v�錾
/*************************************************************
* InitializePatterns�֐�                                     *
* �p�^�[����������                                           *
* ����                                                       *
* pattern			: �p�^�[�������Ǘ�����\���̂̃A�h���X *
* directory_path	: �p�^�[���t�@�C���̂���f�B���N�g���p�X *
* buffer_size		: ���݂̃o�b�t�@�T�C�Y                   *
*************************************************************/
extern void InitializePattern(PATTERNS* pattern, const char* directory_path, int* buffer_size);

/*********************************************************
* UpdateClipBoardPattern�֐�                             *
* �N���b�v�{�[�h����摜�f�[�^�����o���ăp�^�[���ɂ��� *
* ����                                                   *
* patterns	: �p�^�[�����Ǘ�����\���̂̃A�h���X         *
*********************************************************/
extern void UpdateClipBoardPattern(PATTERNS* patterns);

/*************************************************
* CreatePatternSurface�֐�                       *
* �p�^�[���h��ׂ����s�p��CAIRO�T�[�t�F�[�X�쐬  *
* ����                                           *
* patterns	: �p�^�[�����Ǘ�����\���̂̃A�h���X *
* scale		: �p�^�[���̊g�嗦                   *
* rgb		: ���݂̕`��F                       *
* back_rgb	: ���݂̔w�i�F                       *
* flags		: ���E���]�A�㉺���]�̃t���O         *
* mode		: �`�����l������2�̂Ƃ��̍쐬���[�h  *
* flow		: �p�^�[���̔Z�x                     *
* �Ԃ�l                                         *
*	�쐬�����T�[�t�F�[�X                         *
*************************************************/
extern cairo_surface_t* CreatePatternSurface(
	PATTERNS* patterns,
	uint8 rgb[3],
	uint8 back_rgb[3],
	int flags,
	int mode,
	gdouble flow
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_PATTERN_H_
