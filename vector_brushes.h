#ifndef _INCLUDED_VECTOR_BRUSHES_H_
#define _INCLUDED_VECTOR_BRUSHES_H_

#include <gtk/gtk.h>
#include "vector_brush_core.h"
#include "ini_file.h"
#include "types.h"
#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _ePOLY_LINE_FLAGS
{
	POLY_LINE_START = 0x01,
	POLY_LINE_SIZE_WITH_PRESSURE = 0x02,
	POLY_LINE_ANTI_ALIAS = 0x04
} ePOLY_LINE_FLAGS;

typedef struct _POLY_LINE_BRUSH
{
	int base_scale;
	gdouble r;
	gdouble blur;
	gdouble outline_hardness;
	gdouble before_pressure;
	uint8 flow;
	uint8 line_type;
	uint8 first_pressure;
	uint8 last_pressure;
	uint16 flags;
	GTimer *timer;
} POLY_LINE_BRUSH;

typedef enum _eBEZIER_LINE_FLAGS
{
	BEZIER_LINE_START = 0x01,
	BEZIER_LINE_SIZE_WITH_PRESSURE = 0x02,
	BEZIER_LINE_ANTI_ALIAS = 0x04
} eBEZIER_LINE_FLAGS;

typedef struct _BEZIER_LINE_BRUSH
{
	int base_scale;
	gdouble r, blur;
	gdouble outline_hardness;
	gdouble before_pressure;
	uint8 flow;
	uint8 first_pressure;
	uint8 last_pressure;
	uint8 line_type;
	uint16 flags;
	GTimer *timer;
} BEZIER_LINE_BRUSH;

#define FREE_HAND_MINIMUM_DISTANCE 3

typedef enum _eFREE_HAND_FLAGS
{
	FREE_HAND_SIZE_WITH_PRESSURE = 0x01,
	FREE_HAND_ANTI_ALIAS = 0x02,
	FREE_HAND_PRIOR_ARG = 0x04,
	FREE_HAND_STARTED = 0x08
} eFREE_HAND_FLAGS;

typedef struct _FREE_HAND_BRUSH
{
	GdkInputSource source;
	int base_scale;
	gdouble r, blur, outline_hardness;
	gdouble before_x, before_y;
	gdouble draw_before_x, draw_before_y;
	gdouble before_arg;
	gdouble min_degree, min_arg;
	gdouble min_distance;
	uint8 flow;
	uint8 line_type;
	uint16 flags;
} FREE_HAND_BRUSH;

typedef enum _eCONTROL_POINT_MODE
{
	CONTROL_POINT_SELECT,
	CONTROL_POINT_MOVE,
	CONTROL_POINT_CHANGE_PRESSURE,
	CONTROL_POINT_DELETE,
	CONTROL_STROKE_MOVE,
	CONTROL_STROKE_COPY_MOVE,
	CONTROL_STROKE_JOINT,
	NUM_CONTROL_POINT_MODE
} eCONTROL_POINT_MODE;

typedef enum _eCONTROL_POINT_TOOL_FLAGS
{
	CONTROL_POINT_TOOL_HAS_POINT = 0x01,
	CONTROL_POINT_TOOL_HAS_STROKE = 0x02,
	CONTROL_POINT_TOOL_HAS_LOCK = 0x04
} eCONTROL_POINT_TOOL_FLAGS;

typedef struct _CONTROL_POINT_TOOL
{
	FLOAT_T lock_x, lock_y;
	uint8 mode;
	uint32 flags;
	gdouble before_x, before_y;
	VECTOR_POINT change_point_data;
	int add_flag;
} CONTROL_POINT_TOOL;

typedef enum _eCHANGE_LINE_COLOR_MODE
{
	CHANGE_LINE_COLOR_MODE,
	CHANGE_POINT_COLOR_MODE
} eCHANGE_LINE_COLOR_MODE;

typedef struct _CHANGE_LINE_COLOR_TOOL
{
	FLOAT_T lock_x, lock_y;
	uint8 flags;
	uint8 flow;
	uint8 mode;
} CHANGE_LINE_COLOR_TOOL;

typedef struct _CHANGE_LINE_SIZE_TOOL
{
	FLOAT_T lock_x, lock_y;
	gdouble r, blur;
	gdouble outline_hardness;
	uint8 flow;
	uint32 flags;
} CHANGE_LINE_SIZE_TOOL;

typedef enum _eVECTOR_ERASER_MODE
{
	VECTOR_ERASER_STROKE_DIVIDE,
	VECTOR_ERASER_STROKE_DELETE,
	VECTOR_ERASER_MODE_NUM
} eVECTOR_ERASER_MODE;

typedef enum _eVECTOR_ERASER_FLAGS
{
	VECTOR_ERASER_PRESSURE_SIZE = 0x01
} eVECTOR_ERASER_FLAGS;

/*************************
* VECTOR_ERASER�\����    *
* �x�N�g�������S���̏�� *
*************************/
typedef struct _VECTOR_ERASER
{
	gdouble r;		// �����S���T�C�Y
	// �ȑO�̍��W
	gdouble before_x, before_y;
	// �������͈�
	gdouble min_x, min_y, max_x, max_y;
	uint8 mode;		// �X�g���[�N�����A�����̃��[�h
	uint8 flags;	// �M���g�p�Ȃǂ̃t���O
} VECTOR_ERASER;

/***********************************
* VECTOR_SHAPE_BRUSH�\����         *
* �l�p�`, �ȉ~�쐬�p�̃u���V�̏�� *
***********************************/
typedef struct _VECTOR_SHAPE_BRUSH
{
	uint8 mode;						// �ǉ��E�ό`�E�폜
	uint8 shape_type;				// �`��(�l�p�`or�ȉ~)
	uint8 control_point;			// �ړ����̒��_
	uint8 add_vector;				// �x�N�g�����C���[��ǉ����邩
	uint8 line_joint;				// ���̌q����
	uint8 changing_shape;			// �`��̕ύX���̃t���O
	uint8 change_line_width;		// �ό`���̐��̑����ύX�L��
	uint8 manually_set;				// ����͂ł̌`��f�[�^�ݒ�
	gdouble line_width;				// ���̑���
	uint8 line_color[4];			// ���̐F
	uint8 fill_color[4];			// �h��ׂ��̐F
	cairo_line_join_t join_type;	// ���̌q���ڂ̌`
	FLOAT_T shape_points[9][2];		// �g��k���E��]�E�ړ��p�̍��W�L���o�b�t�@
	VECTOR_DATA before_shape;		// �`��ύX�O�̃f�[�^
	GtkWidget *detail_widget;		// �e���[�h�̐ݒ�E�B�W�F�b�g
	GtkWidget *detail_frame;		// �e���[�h�̐ݒ�E�B�W�F�b�g�̃R���e�i
	const char *brush_name;			// �u���V�̖��O
	char *before_script_directory;	// �O��X�N���v�g�ǂݍ��ݎ��̃t�H���_
	int text_field_size[2];			// �X�N���v�g�̕ҏW�E�B�W�F�b�g�̃T�C�Y
} VECTOR_SHAPE_BRUSH;

typedef enum _eVECTOR_SHAPE_BRUSH_MODE
{
	VECTOR_SHAPE_BRUSH_MODE_ADD,
	VECTOR_SHAPE_BRUSH_MODE_TRANSFORM,
	VECTOR_SHAPE_BRUSH_MODE_CHANGE_COLOR,
	VECTOR_SHAPE_BRUSH_MODE_DELETE
} eVECTOR_SHAPE_BRUSH_MODE;

typedef enum _eVECTOR_BRUSH_TYPE
{
	TYPE_POLY_LINE_BRUSH,
	TYPE_BEZIER_LINE_BRUSH,
	TYPE_FREE_HAND_BRUSH,
	TYPE_CONTROL_POINT,
	TYPE_CHANGE_LINE_COLOR,
	TYPE_CHANGE_LINE_SIZE,
	TYPE_VECTOR_ERASER,
	TYPE_VECTOR_SHAPE_BRUSH
} eVECTOR_BRUSH_TYPE;

// �֐��̃v���g�^�C�v�錾
extern void LoadVectorBrushDetailData(
	VECTOR_BRUSH_CORE* core,
	INI_FILE_PTR file,
	const char* section_name,
	const char* brush_type
);

extern void SetControlPointTool(
	VECTOR_BRUSH_CORE* core,
	CONTROL_POINT_TOOL* control
);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_VECTOR_BRUSHES_H_
