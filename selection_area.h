#ifndef _INCLUDED_SELECTION_AREA_H_
#define _INCLUDED_SELECTION_AREA_H_

#include <gtk/gtk.h>
#include "layer.h"
#include "types.h"
#include "configure.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _eSELECT_FUZZY_DIRECTION
{
	FUZZY_SELECT_DIRECTION_QUAD,
	FUZZY_SELECT_DIRECTION_OCT
} eSELECT_FUZZY_DIRECTION;

typedef struct _SELECTION_SEGMENT_POINT
{
	int32 x, y;
} SELECTION_SEGMENT_POINT;

typedef struct _SELECTION_SEGMENT
{
	SELECTION_SEGMENT_POINT* points;
	int32 num_points;
} SELECTION_SEGMENT;

typedef struct _SELECTION_AREA
{
	int32 min_x, min_y, max_x, max_y;
#ifdef OLD_SELECTION_AREA
	int32 num_area;
	int16 index;
	SELECTION_SEGMENT* area_data;
#else
	cairo_surface_t *surface_p;
	int stride;
	int index;
	uint8 *pixels;
#endif
} SELECTION_AREA;

// �I��͈͕ҏW�p�̊֐��|�C���^�z��
extern void (*g_blend_selection_funcs[])(LAYER* work, LAYER* selection);

typedef enum _eSELECTION_BLEND_MODE
{
	SELECTION_BLEND_NORMAL,
	SELECTION_BLEND_SOURCE,
	SELECTION_BLEND_COPY,
	SELECTION_BLEND_MINUS
} eSELECTION_BLEND_MODE;

// �֐��̃v���g�^�C�v�錾
extern gboolean UpdateSelectionArea(SELECTION_AREA* area, LAYER* selection, LAYER* temp);

/***************************************************************
* LaplacianFilter�֐�                                          *
* ���v���V�A���t�B���^�ŃO���[�X�P�[����񂩂�G�b�W�𒊏o���� *
* ����                                                         *
* pixels		: �O���[�X�P�[���̃C���[�W                     *
* width			: �摜�̕�                                     *
* height		: �摜�̍���                                   *
* stride		: 1�s���̃o�C�g��                              *
* write_buff	: �G�b�W�f�[�^�������o���o�b�t�@               *
***************************************************************/
extern void LaplacianFilter(
	uint8* pixels,
	int width,
	int height,
	int stride,
	uint8* write_buff
);

extern void DrawSelectionArea(
	SELECTION_AREA* area,
	struct _DRAW_WINDOW* window
);

extern void AddSelectionAreaChangeHistory(
	struct _DRAW_WINDOW* window,
	const gchar* tool_name,
	int32 min_x,
	int32 min_y,
	int32 max_x,
	int32 max_y
);

extern void DetectSameColorArea(
	LAYER* target,
	uint8* buff,
	uint8* temp_buff,
	int32 start_x,
	int32 start_y,
	uint8 *color,
	uint8 channel,
	int16 threshold,
	int32* min_x,
	int32* min_y,
	int32* max_x,
	int32* max_y,
	eSELECT_FUZZY_DIRECTION direction
);

/*****************************************
* AddSelectionAreaByColor�֐�            *
* �F��I�������s                         *
* ����                                   *
* target	: �F��r���s�����C���[       *
* buff		: �I��͈͂��L������o�b�t�@ *
* color		: �I������F                 *
* threshold	: �I�𔻒f��臒l             *
* min_x		: �I��͈͂̍ŏ���X���W      *
* min_y		: �I��͈͂̍ŏ���Y���W      *
* max_x		: �I��͈͂̍ő��X���W      *
* max_y		: �I��͈͂̍ő��Y���W      *
* �Ԃ�l                                 *
*	�I��͈͗L��:1 �I��͈͖���:0        *
*****************************************/
extern int AddSelectionAreaByColor(
	LAYER* target,
	uint8* buff,
	uint8* color,
	int channel,
	int32 threshold,
	int32* min_x,
	int32* min_y,
	int32* max_x,
	int32* max_y
);

/*****************************************
* ExtendSelectionAreaOneStep�֐�         *
* �I��͈͂�1�s�N�Z���g�傷��            *
* ����                                   *
* select	: �I��͈͂��Ǘ����郌�C���[ *
* temp		: �ꎞ�ۑ��p�̃��C���[       *
*****************************************/
extern void ExtendSelectionAreaOneStep(LAYER* select, LAYER* temp);

/*****************************************
* RecuctSelectionAreaOneStep�֐�         *
* �I��͈͂�1�s�N�Z���k������            *
* ����                                   *
* select	: �I��͈͂��Ǘ����郌�C���[ *
* temp		: �ꎞ�ۑ��p�̃��C���[       *
*****************************************/
extern void ReductSelectionAreaOneStep(LAYER* select, LAYER* temp);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SELECTION_AREA_H_
