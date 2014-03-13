#ifndef _INCLUDED_CLIP_BOARD_H_
#define _INCLUDED_CLIP_BOARD_H_

#include "application.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************
* ExecutePaste�֐�                                   *
* �\��t�������s����                                 *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecutePaste(APPLICATION* app);

/*****************************************************
* ExecuteCopy�֐�                                    *
* �R�s�[�����s                                       *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteCopy(APPLICATION* app);

/*****************************************************
* ExecuteCopyVisible�֐�                             *
* �����R�s�[�����s                                 *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteCopyVisible(APPLICATION* app);

/*****************************************************
* ExecuteCut�֐�                                     *
* �؂�������s                                     *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteCut(APPLICATION* app);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_CLIP_BOARD_H_
