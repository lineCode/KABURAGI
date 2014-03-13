#ifndef _INCLDUED_SAVE_H_
#define _INCLDUED_SAVE_H_

#include "draw_window.h"

#ifdef __cplusplus
extern "C" {
#endif

// �֐��̃v���g�^�C�v�錾
/***************************************************
* IsSupportFileType�֐�                            *
* �����o�����T�|�[�g���Ă���t�@�C���^�C�v���𔻒� *
* ����                                             *
* extention	: �g���q�̕�����                       *
* �Ԃ�l                                           *
*	�T�|�[�g���Ă���:1	�T�|�[�g���Ă��Ȃ�:0       *
***************************************************/
extern int IsSupportFileType(const char* extention);

extern void SaveAsPng(struct _APPLICATION* app, DRAW_WINDOW* window, const gchar* file_name);
extern void SaveAsJpeg(struct _APPLICATION* app, DRAW_WINDOW* window, const gchar* file_name);

/*****************************************
* SaveAsOriginalFormat�֐�               *
* �Ǝ��`���Ńf�[�^�������o��             *
* ����                                   *
* app		: �A�v���P�[�V�����S�̂̏�� *
* window	: �`��̈�̏��             *
* file_name	: ���������t�@�C���p�X       *
*****************************************/
extern void SaveAsOriginalFormat(APPLICATION* app, DRAW_WINDOW* window, const gchar* file_name);

/*****************************************
* SaveAsPhotoShopDocument�֐�            *
* PSD�`���Ńf�[�^�������o��              *
* ����                                   *
* app		: �A�v���P�[�V�����S�̂̏�� *
* window	: �`��̈�̏��             *
* file_name	: ���������t�@�C���p�X       *
*****************************************/
extern void SaveAsPhotoShopDocument(APPLICATION* app, DRAW_WINDOW* window, const gchar* file_name);

/*********************************************************
* Save�֐�                                               *
* �����o�����s                                           *
* ����                                                   *
* app		: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
* window	: �`��̈�̏��                             *
* file_name	: �ۑ�����t�@�C���p�X                       *
* file_type	: �ۑ�����t�@�C���`���̊g���q               *
* �Ԃ�l                                                 *
*	�����o�����t�@�C���̃p�X                             *
*********************************************************/
extern const char* Save(
	APPLICATION* app,
	DRAW_WINDOW* window,
	const gchar* file_name,
	const char* file_type
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLDUED_SAVE_H_
