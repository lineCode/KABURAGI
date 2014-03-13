#ifndef _INCLUDED_SMOOTHER_H_
#define _INCLUDED_SMOOTHER_H_

#include <gtk/gtk.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// ��u���␳�Ɏg�p����f�[�^�_�̍ő吔
#define SMOOTHER_POINT_BUFFER_SIZE 30
// ��u���␳�̓K�p�����̍ő�l
#define SMOOTHER_RATE_MAX 100
// ��u���␳�̕���
typedef enum _eSMOOTH_METHOD
{
	SMOOTH_GAUSSIAN,
	SMOOTH_AVERAGE
} eSMOOTH_METHOD;

/***********************
* SMOOTHER_POINT�\���� *
* ��u���␳�p�̍��W   *
***********************/
typedef struct _SMOOTHER_POINT
{
	FLOAT_T x, y;
} SMOOTHER_POINT;

/*******************
* SMOOTHER�\����   *
* ��u���␳�̏�� *
*******************/
typedef struct _SMOOTHER
{
	// ��u���␳�Ɏg�p������W�f�[�^
	SMOOTHER_POINT buffer[SMOOTHER_POINT_BUFFER_SIZE];
	// �O���u���␳�����Ƃ��̍��W�f�[�^
	SMOOTHER_POINT before_point;
	// �Ō�ɕ`�悵�����W
	SMOOTHER_POINT last_draw_point;
	// ���ϒl�v�Z�p�̍��v�l
	SMOOTHER_POINT sum;
	// �����f�[�^
	FLOAT_T velocity[SMOOTHER_POINT_BUFFER_SIZE];
	// �␳�������銄��
	FLOAT_T rate;
	// �z��̃C���f�b�N�X
	int index;
	// �f�[�^�_��
	int num_data;
	// �g�p����f�[�^�_��
	int num_use;
	// �O���u���␳�����Ƃ��̎���
	guint last_time;
	// ��u���␳�̕���
	int mode;
} SMOOTHER;

// �֐��̃v���g�^�C�v�錾
#define INIT_SMOOTHER(SMOOTHER) (SMOOTHER).index = 0, (SMOOTHER).num_data=0

/********************************************
* Smooth�֐�                                *
* ��u���␳���s��                          *
* ����                                      *
* smoother	: ��u���␳�̃f�[�^            *
* x			: ���݂�x���W�B�␳��̒l������ *
* y			: ���݂�y���W�B�␳��̒l������ *
* this_time	: ��u���␳�����J�n����        *
* zoom_rate	: �`��̈�̊g��k����          *
********************************************/
extern void Smooth(
	SMOOTHER* smoother,
	FLOAT_T *x,
	FLOAT_T *y,
	guint this_time,
	FLOAT_T zoom_rate
);

/*********************************************
* AddAverageSmoothPoint�֐�                  *
* ���W���ω��ɂ���u���␳�Ƀf�[�^�_��ǉ� *
* ����                                       *
* smoother	: ��u���␳�̃f�[�^             *
* x			: �}�E�X��X���W                  *
* y			: �}�E�X��Y���W                  *
* pressure	: �M��                           *
* zoom_rate	: �`��̈�̊g��k����           *
* �Ԃ�l                                     *
*	�u���V�̕`����s��:TRUE	�s��Ȃ�:FALSE   *
*********************************************/
extern gboolean AddAverageSmoothPoint(
	SMOOTHER* smoother,
	FLOAT_T* x,
	FLOAT_T* y,
	FLOAT_T* pressure,
	FLOAT_T zoom_rate
);

/****************************************************
* AverageSmoothFlush�֐�                            *
* ���ω��ɂ���u���␳�̎c��o�b�t�@��1���o�� *
* ����                                              *
* smoother	: ��u���␳�̏��                      *
* x			: �}�E�X��X���W                         *
* y			: �}�E�X��Y���W                         *
* pressure	: �M��                                  *
* �Ԃ�l                                            *
*	�o�b�t�@�̎c�薳��:TRUE	�c��L��:FALSE          *
****************************************************/
extern gboolean AverageSmoothFlush(
	SMOOTHER* smoother,
	FLOAT_T* x,
	FLOAT_T* y,
	FLOAT_T* pressure
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SMOOTHER_H_
