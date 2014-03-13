#ifndef _INCLUDED_BEZIER_H_
#define _INCLUDED_BEZIER_H_

#include "vector.h"
#include "draw_window.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************
* BEZIER_POINT�\����       *
* �x�W�F�Ȑ��ł̐���_���W *
***************************/
typedef struct _BEZIER_POINT
{
	FLOAT_T x, y;
} BEZIER_POINT;

// �֐��̃v���g�^�C�v�錾
/******************************************************
* MakeBezier3EdgeControlPoint�֐�                     *
* 3�_�̍��W����3���x�W�F�Ȑ��̒[�_�ł̍��W�����肷��  *
* ����                                                *
* src		: �x�W�F�Ȑ����ʂ���W                    *
* control	: 2���x�W�F�Ȑ��̐���_���L������A�h���X *
******************************************************/
extern void MakeBezier3EdgeControlPoint(BEZIER_POINT* src, BEZIER_POINT* control);

/*************************************************
* CalcBezier2�֐�                                *
* 2���x�W�F�Ȑ��̔}��ϐ�t�ɂ�������W���v�Z���� *
* ����                                           *
* points	: �x�W�F�Ȑ��̐���_                 *
* t			: �}��ϐ�t                          *
* dest		: ���W�f�[�^���L������A�h���X       *
*************************************************/
extern void CalcBezier2(BEZIER_POINT* points, FLOAT_T t, BEZIER_POINT* dest);

extern void MakeBezier3ControlPoints(BEZIER_POINT* src, BEZIER_POINT* controls);

extern void CalcBezier3(BEZIER_POINT* points, FLOAT_T t, BEZIER_POINT* dest);

extern void StrokeBezierLine(
	DRAW_WINDOW* window,
	VECTOR_LINE* line,
	VECTOR_LAYER_RECTANGLE* rect
);

extern void StrokeBezierLineClose(
	DRAW_WINDOW* window,
	VECTOR_LINE* line,
	VECTOR_LAYER_RECTANGLE* rect
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_BEZIER_H_
