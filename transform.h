#ifndef _INCLUDED_TRANSFORM_H_
#define _INCLUDED_TRANSFORM_H_

#include "application.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************
* TRANSFORM_DATA�\���� *
* �ό`�����̃f�[�^     *
***********************/
typedef struct _TRANSFORM_DATA
{
	int32 x, y;						// �ό`�̈�̍���̍��W
	int32 width, height;			// �ό`�̈�̕��ƍ���
	int stride;						// ���摜�f�[�^1�s���̃o�C�g��
	uint8 **source_pixels;			// �ό`�̈�̃s�N�Z���f�[�^
	uint8 **before_pixels;			// �ό`�O�̃s�N�Z���f�[�^
	LAYER **layers;					// �ό`�������s�����C���[
	uint16 num_layers;				// �ό`�������s�����C���[�̐�
	uint16 trans_point;				// �I�𒆂̃|�C���g
	uint8 selected_mode;			// �I������Ă��郂�[�h
	uint8 trans_mode;				// �ό`���[�h
	uint8 trans_type;				// �ό`���@
	uint8 flags;					// �㉺�E���E���]�̃t���O
	FLOAT_T move_x, move_y;			// �A�t�B���ϊ��ł̈ړ���̍��W
	FLOAT_T angle;					// �A�t�B���ϊ��ł̉�]��
	gdouble before_x, before_y;		// �}�E�X����̍��W���L��
	FLOAT_T before_angle;			// ��O�̏�Ԃł̊p�x
	FLOAT_T trans[8][2];			// �ό`�Ŏw�肳��Ă���4�_�̍��W
	// �ˉe�ϊ����s���͈�
	int min_x[4], max_x[4], min_y[4], max_y[4];
} TRANSFORM_DATA;

typedef enum _eTRANSFORM_FLAGS
{
	TRANSFORM_REVERSE_HORIZONTALLY = 0x01,
	TRANSFORM_REVERSE_VERTICALLY = 0x02
} eTRANSFORM_FLAGS;

typedef enum _eTRANSFORM_POINT
{
	TRANSFORM_LEFT_UP,
	TRANSFORM_LEFT,
	TRANSFORM_LEFT_DOWN,
	TRANSFORM_DOWN,
	TRANSFORM_RIGHT_DOWN,
	TRANSFORM_RIGHT,
	TRANSFORM_RIGHT_UP,
	TRANSFORM_UP,
	TRANSFORM_POINT_NONE
} eTRANSFORM_POINT;

typedef enum _eTRANSFORM_MODE
{
	TRANSFORM_FREE,
	TRANSFORM_SCALE,
	TRANSFORM_FREE_SHAPE,
	TRANSFORM_ROTATE,
	TRANSFORM_MOVE
} eTRANSFORM_MODE;

typedef enum _eTRANSFORM_TYPE
{
	TRANSFORM_PROJECTION,
	TRANSFORM_PATTERN
} eTRANSFORM_TYPE;

// �֐��̃v���g�^�C�v�錾
/*************************************
* CreateTransformData�֐�            *
* �ό`�����p�̃f�[�^���쐬����       *
* ����                               *
* window	: �ό`�������s���`��̈� *
*************************************/
extern TRANSFORM_DATA *CreateTransformData(DRAW_WINDOW* window);

/***************************************
* DeleteTransformData�֐�              *
* �ό`�����p�̃f�[�^���J��             *
* ����                                 *
* transform	: �J������f�[�^�̃A�h���X *
***************************************/
extern void DeleteTransformData(TRANSFORM_DATA** transform);

/***********************************************
* Transform�֐�                                *
* �ό`���������s                               *
* ����                                         *
* transform			: �ό`�����p�̃f�[�^       *
* projection_param	: �ˉe�ϊ��p�̃p�����[�^�[ *
***********************************************/
extern void Transform(TRANSFORM_DATA* transform);

/***********************************************
* ProjectionTransform�֐�                      *
* �ˉe�ό`���������s                           *
* ����                                         *
* transform			: �ό`�����p�̃f�[�^       *
* projection_param	: �ˉe�ϊ��p�̃p�����[�^�[ *
***********************************************/
extern void ProjectionTransform(TRANSFORM_DATA* transform, FLOAT_T projection_param[4][9]);

/*****************************************************
* ExecuteTransform�֐�                               *
* �A�v���P�[�V�������j���[����̕ό`�����̌Ăяo��   *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteTransform(APPLICATION* app);

/*******************************************************
* ExecuteProjection�֐�                                *
* �A�v���P�[�V�������j���[����̎ˉe�ϊ������̌Ăяo�� *
* ����                                                 *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X   *
*******************************************************/
extern void ExecuteProjection(APPLICATION* app);

extern void TransformButtonPress(TRANSFORM_DATA* transform, gdouble x, gdouble y);

extern void DisplayTransform(DRAW_WINDOW *window);

/***********************************************
* TransformMotionNotifyCallBack�֐�            *
* �ό`�������̃}�E�X�h���b�O�̃R�[���o�b�N�֐� *
* ����                                         *
* window	: �`��̈�̏��                   *
* transform	: �ό`�����̏��                   *
* x			: �}�E�X��X���W                    *
* y			: �}�E�X��Y���W                    *
* state		: �}�E�X�E�L�[�{�[�h�̏��         *
***********************************************/
extern void TransformMotionNotifyCallBack(
	DRAW_WINDOW* window,
	TRANSFORM_DATA* transform,
	gdouble x,
	gdouble y,
	void* state
);

/*****************************************************
* CreateTransformDetailUI�֐�                        *
* �ό`��������UI�E�B�W�F�b�g���쐬                   *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
* �Ԃ�l                                             *
*	�쐬����UI�E�B�W�F�b�g                           *
*****************************************************/
extern GtkWidget* CreateTransformDetailUI(APPLICATION* app);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TRANSFORM_H_
