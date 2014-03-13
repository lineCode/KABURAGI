#ifndef _INCLUDED_VECTOR_H_
#define _INCLUDED_VECTOR_H_

#include <gtk/gtk.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VECTOR_LINE_BUFFER_SIZE 32

typedef enum _eVECTOR_LINE_TYPE
{
	VECTOR_LINE_STRAIGHT,
	VECTOR_LINE_BEZIER_OPEN,
	VECTOR_LINE_STRAIGHT_CLOSE,
	VECTOR_LINE_BEZIER_CLOSE
} eVECTOR_LINE_TYPE;

typedef enum _eVECTOR_POINT_FLAGS
{
	VECTOR_POINT_SELECTED = 0x01,
} eVECTOR_POINT_FLAGS;

typedef struct _VECTOR_POINT
{
	uint8 vector_type;
	uint8 flags;
	uint8 color[4];
	gdouble pressure, size;
	gdouble x, y;
} VECTOR_POINT;

typedef enum _eVECTOR_LINE_FLAGS
{
	VECTOR_LINE_ANTI_ALIAS = 0x01,
	VECTOR_LINE_FIX = 0x02,
	VECTOR_LINE_MARKED = 0x04
} eVECTOR_LINE_FLAGS;

typedef struct _VECTOR_LINE_LAYER
{
	int32 x, y, width, height, stride;
	uint8* pixels;
	cairo_t* cairo_p;
	cairo_surface_t* surface_p;
} VECTOR_LINE_LAYER;

/*************************************
* VECTOR_LINE�\����                  *
* �x�N�g�����C���[�̐���{���̃f�[�^ *
*************************************/
typedef struct _VECTOR_LINE
{
	uint8 vector_type;			// ���̃^�C�v
	uint8 flags;				// �A���`�G�C���A�X���̃t���O
	uint16 num_points;			// ����_�̐�
	uint32 buff_size;			// ����_�o�b�t�@�̃T�C�Y
	gdouble blur;				// �{�P���̊J�n�I�t�Z�b�g
	gdouble outline_hardness;	// �֊s�̍d��
	VECTOR_POINT* points;		// ����_���W�z��
	VECTOR_LINE_LAYER* layer;	// ���X�^���C�Y��̃f�[�^
								// ���X�^���C�Y�����y���p
	// �O��̐��ւ̃|�C���^
	struct _VECTOR_LINE *prev, *next;
} VECTOR_LINE;

/*********************************************************
* VECTOR_LINE_BASE_DATA�\����                            *
* �x�N�g�����C���[�̐���{���̊�{���(�f�[�^�����o���p) *
*********************************************************/
typedef struct _VECTOR_LINE_BASE_DATA
{
	uint8 vector_type;			// ���̃^�C�v
	uint8 flags;				// ���ɐݒ肳��Ă���t���O
	uint16 num_points;			// ����_�̐�
	gdouble blur;				// �{�P���̊J�n�I�t�Z�b�g
	gdouble outline_hardness;	// �֊s�̍d��
} VECTOR_LINE_BASE_DATA;

typedef struct _VECTOR_LAYER_RECTANGLE
{
	gdouble min_x, min_y, max_x, max_y;
} VECTOR_LAYER_RECTANGLE;

typedef struct _DIVIDE_LINE_CHANGE_DATA
{
	size_t data_size;
	int8 added;
	int8 line_type;
	uint16 before_num_points;
	uint16 after_num_points;

	int index;
	VECTOR_POINT *before;
	VECTOR_POINT *after;
} DIVIDE_LINE_CHANGE_DATA;

typedef struct _DIVIDE_LINE_ADD_DATA
{
	size_t data_size;
	int8 line_type;
	uint16 num_points;

	int index;
	VECTOR_POINT *after;
} DIVIDE_LINE_ADD_DATA;

typedef struct _DIVIDE_LINE_DATA
{
	int num_change;
	int num_add;
	int change_size;
	int layer_name_length;

	char *layer_name;
	DIVIDE_LINE_CHANGE_DATA *change;
	DIVIDE_LINE_ADD_DATA *add;
} DIVIDE_LINE_DATA;

extern VECTOR_LINE* CreateVectorLine(VECTOR_LINE* prev, VECTOR_LINE* next);

extern void DeleteVectorLine(VECTOR_LINE** line);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_VECTOR_H_
