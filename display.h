#ifndef _INCLUDED_DISPLAY_H_
#define _INCLUDED_DISPLAY_H_

#include <gtk/gtk.h>
#include "layer.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************
* DisplayDrawWindow�֐�                  *
* �`��̈�̉�ʍX�V����                 *
* ����                                   *
* widget		: �`��̈�̃E�B�W�F�b�g *
* event_info	: �`��X�V�̏��         *
* window		: �`��̈�̏��         *
* �Ԃ�l                                 *
*	���FALSE                            *
*****************************************/
EXTERN gboolean DisplayDrawWindow(
	GtkWidget* widget,
	GdkEventExpose* event_info,
	struct _DRAW_WINDOW* window
);

/***************************************************
* DisplayBrushPreview�֐�                          *
* �u���V�v���r���[�̉�ʍX�V����                   *
* ����                                             *
* widget		: �u���V�`�挋�ʕ\���p�E�B�W�F�b�g *
* event_info	: �`��X�V�̏��                   *
* window		: �`��̈�̏��                   *
* �Ԃ�l                                           *
*	���FALSE                                      *
***************************************************/
EXTERN gboolean DisplayBrushPreview(
	GtkWidget* widget,
	GdkEventExpose* event_info,
	struct _DRAW_WINDOW* window
);

/*****************************
* UpdateDrawWindow�֐�       *
* �`��̈�̍X�V����         *
* ����                       *
* window	: �`��̈�̏�� *
*****************************/
EXTERN void UpdateDrawWindow(struct _DRAW_WINDOW* window);

/*******************************************************
* MixLayerForSave�֐�                                  *
* �ۑ����邽�߂ɔw�i�s�N�Z���f�[�^�����Ń��C���[������ *
* ����                                                 *
* window	: �`��̈�̏��                           *
* �Ԃ�l                                               *
*	�����������C���[�̃f�[�^                           *
*******************************************************/
EXTERN LAYER* MixLayerForSave(DRAW_WINDOW* window);

/*******************************************************
* MixLayerForSaveWithBackGround�֐�                    *
* �ۑ����邽�߂ɔw�i�s�N�Z���f�[�^�L��Ń��C���[������ *
* ����                                                 *
* window	: �`��̈�̏��                           *
* �Ԃ�l                                               *
*	�����������C���[�̃f�[�^                           *
*******************************************************/
EXTERN LAYER* MixLayerForSaveWithBackGround(DRAW_WINDOW* window);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_DISPLAY_H_
