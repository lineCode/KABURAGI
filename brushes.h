#ifndef _INCLUDED_BRUSHES_H_
#define _INCLUDED_BRUSHES_H_

#include <gtk/gtk.h>
#include "types.h"
#include "brush_core.h"
#include "ini_file.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _eBRUSH_FLAGS
{
	BRUSH_FLAG_SIZE = 0x01,
	BRUSH_FLAG_FLOW = 0x02,
	BRUSH_FLAG_ENTER = 0x04,
	BRUSH_FLAG_OUT = 0x08,
	BRUSH_FLAG_ROTATE = 0x10,
	BRUSH_FLAG_ANTI_ALIAS = 0x20
} eBRUSH_FLAGS;

typedef struct _PENCIL
{
	BRUSH_CORE *core;
	int base_scale;
	gdouble r;
	gdouble alpha;
	gdouble outline_hardness;
	gdouble minimum_pressure;
	gdouble before_x, before_y;

	int8 channel;
	uint8 shape;
	uint8 brush_mode;
	uint8 flags;

	uint16 blend_mode;
} PENCIL;

#define BRUSH_POINT_BUFFER_SIZE 256

typedef struct _HARD_PEN
{
	BRUSH_CORE *core;
	int base_scale;
	gdouble r;
	gdouble alpha;
	gdouble minimum_pressure;
	gdouble before_x, before_y;
	gdouble before_alpha;
	gdouble before_r;

	unsigned int flags;

	int num_points;

	uint16 blend_mode;
} HARD_PEN;

typedef struct _AIR_BRUSH
{
	BRUSH_CORE *core;
	int base_scale;
	gdouble r, opacity;
	gdouble outline_hardness;
	gdouble before_x, before_y;
	gdouble blur;
	FLOAT_T points[BRUSH_POINT_BUFFER_SIZE][4];
	FLOAT_T sum_distance, travel;
	FLOAT_T finish_length;
	FLOAT_T draw_start;
	FLOAT_T enter_length;
	FLOAT_T enter_size;
	uint16 blend_mode;
	int num_point;
	int draw_finished;
	int ref_point;
	gdouble enter, out;
	int flags;
} AIR_BRUSH;

typedef enum _eBLEND_BRUSH_TARGET
{
	BLEND_BRUSH_TARGET_UNDER_LAYER,
	BLEND_BRUSH_TARGET_CANVAS
} eBLEND_BRUSH_TARGET;

typedef struct _BLEND_BRUSH
{
	BRUSH_CORE *core;
	int base_scale;
	gdouble r, opacity;
	gdouble outline_hardness;
	gdouble before_x, before_y;
	gdouble blur;
	FLOAT_T points[BRUSH_POINT_BUFFER_SIZE][4];
	FLOAT_T sum_distance, travel;
	FLOAT_T finish_length;
	FLOAT_T draw_start;
	FLOAT_T enter_length;
	FLOAT_T enter_size;
	uint16 blend_mode;
	uint8 target;
	int num_point;
	int draw_finished;
	int ref_point;
	gdouble enter, out;
	int flags;
} BLEND_BRUSH;

typedef struct _OLD_AIR_BRUSH
{
	gdouble r, opacity;
	gdouble outline_hardness;
	gdouble before_x, before_y;
	gdouble blur;
	int flags;
} OLD_AIR_BRUSH;

typedef struct _WATER_COLOR_BRUSH
{
	BRUSH_CORE *core;
	int base_scale;
	FLOAT_T r, alpha;
	FLOAT_T outline_hardness;
	FLOAT_T blur;
	FLOAT_T extend;
	FLOAT_T mix;
	FLOAT_T enter, out;
	uint8 before_color[4];
	FLOAT_T points[BRUSH_POINT_BUFFER_SIZE][4];
	FLOAT_T before_x, before_y;
	FLOAT_T last_draw_x, last_draw_y;
	FLOAT_T sum_distance, travel;
	FLOAT_T finish_length;
	FLOAT_T draw_start;
	FLOAT_T enter_length;
	FLOAT_T enter_size;
	int num_point;
	int draw_finished;
	int ref_point;
	int flags;
} WATER_COLOR_BRUSH;

typedef enum _eSTAMP_FLAGS
{
	STAMP_PRESSURE_SIZE = 0x01,
	STAMP_PRESSURE_FLOW = 0x02,
	STAMP_FLIP_HORIZONAL = 0x04,
	STAMP_FLIP_VERTICAL = 0x08,
	STAMP_RANDOM_SIZE = 0x20,
	STAMP_RANDOM_ROTATE = 0x40
} eSTAMP_FLAGS;

typedef enum _eSTAMP_MODE
{
	STAMP_MODE_SATURATION,
	STAMP_MODE_BRIGHTNESS
} eSTAMP_MODE;

/*****************************
* STAMP_CORE�\����           *
* �X�^���v�n�c�[���̊�{��� *
*****************************/
typedef struct _STAMP_CORE
{
	// �u���V�̊�{���
	BRUSH_CORE *brush_core;
	// �X�^���v�̊g��k�����A�Z�x
	gdouble scale, flow;
	// �g��k�����̋t��(�s��ݒ肪���]���Ă�?)
	gdouble inv_scale;
	// �X�^���v�̔��a
	gdouble r;
	// �X�^���v�̕��A�����̔���
	gdouble half_width, half_height;
	// �O�ɕ`�悵�����W
	gdouble before_x, before_y;
	// �X�^���v�Ԃ̋���
	gdouble d, stamp_distance;
	// �X�^���v�̊p�x
	gdouble rotate;
	// �X�^���v�̉�]�J�n�p�x
	gdouble rotate_start;
	// �X�^���v�̉�]���x
	gdouble rotate_speed;
	// �X�^���v�̉�]����
	int rotate_direction;

	// �X�^���v�`��p�̃T�[�t�F�[�X
	cairo_surface_t* brush_surface;
	// �J�[�\���`��p�̃T�[�t�F�[�X
	cairo_surface_t* cursor_surface;

	// �g�p�p�^�[����ID
	int32 stamp_id;

	// ���E���]�A�㉺���]�̃t���O
	uint8 pattern_flags;
	// �M���ݒ�Ȃǂ̃t���O
	uint8 flags;

	// ���F���[�h�ϐ��̃A�h���X
	uint8* mode;
	// ���F���[�h�I���E�B�W�F�b�g
	GtkWidget* mode_select[3];

	// �p�^�[���I��p�̃{�^��
	GtkWidget** buttons;
	// �{�^���̐�
	int32 num_button;

	// �A�v���P�[�V�������Ǘ�����\���̂ւ̃|�C���^
	struct _APPLICATION* app;
} STAMP_CORE;

/*****************************
* STAMP�\����                *
* �X�^���v�c�[���̏ڍ׃f�[�^ *
*****************************/
typedef struct _STAMP
{
	STAMP_CORE core;
	FLOAT_T points[BRUSH_POINT_BUFFER_SIZE][4];
	uint8 mode;
} STAMP;

typedef struct _IMAGE_BRUSH
{
	STAMP_CORE core;
	FLOAT_T points[BRUSH_POINT_BUFFER_SIZE][4];
	FLOAT_T sum_distance, travel;
	FLOAT_T finish_length;
	FLOAT_T draw_start;
	FLOAT_T enter_length;
	FLOAT_T enter_size;
	FLOAT_T stamp_distance;
	FLOAT_T remain_distance;
	FLOAT_T size_range;
	FLOAT_T rotate_range;
	uint16 blend_mode;
	int16 random_rotate_range;
	int num_point;
	int draw_finished;
	int ref_point;
	gdouble enter, out;
	uint8 mode;
} IMAGE_BRUSH;

typedef struct _BLEND_IMAGE_BRUSH
{
	STAMP_CORE core;
	int base_scale;
	FLOAT_T points[BRUSH_POINT_BUFFER_SIZE][4];
	FLOAT_T sum_distance, travel;
	FLOAT_T finish_length;
	FLOAT_T draw_start;
	FLOAT_T enter_length;
	FLOAT_T enter_size;
	FLOAT_T stamp_distance;
	FLOAT_T remain_distance;
	FLOAT_T size_range;
	FLOAT_T rotate_range;
	uint16 blend_mode;
	int16 random_rotate_range;
	uint8 target;
	uint8 mode;
	int num_point;
	int draw_finished;
	int ref_point;
	gdouble enter, out;
	int flags;
} BLEND_IMAGE_BRUSH;

typedef enum _ePICKER_FLAGS
{
	PICKER_FLAG_PRESSURE_SIZE = 0x01,
	PICKER_FLAG_PRESSURE_FLOW = 0x02,
	PICKER_FLAG_CHANGE_HSV = 0x04,
	PICKER_FLAG_ANTI_ALIAS = 0x08
} ePICKER_FLAGS;

typedef enum _ePICKER_MODE
{
	PICKER_MODE_SINGLE_PIXEL,
	PICKER_MODE_AVERAGE
} ePICKER_MODE;

typedef struct _PICKER_BRUSH
{
	BRUSH_CORE *core;

	int base_scale;
	FLOAT_T r;
	FLOAT_T blur;
	FLOAT_T outline_hardness;
	FLOAT_T alpha;
	FLOAT_T before_x, before_y;

	int add_h, add_s, add_v;

	uint16 blend_mode;
	uint8 picker_mode;
	uint8 picker_source;
	unsigned int flags;
} PICKER_BRUSH;

typedef struct _PICKER_IMAGE_BRUSH
{
	STAMP_CORE core;
	FLOAT_T points[BRUSH_POINT_BUFFER_SIZE][4];
	FLOAT_T sum_distance, travel;
	FLOAT_T finish_length;
	FLOAT_T draw_start;
	FLOAT_T enter_length;
	FLOAT_T enter_size;
	FLOAT_T stamp_distance;
	FLOAT_T remain_distance;
	FLOAT_T size_range;
	FLOAT_T rotate_range;
	uint16 blend_mode;
	int16 random_rotate_range;
	uint8 picker_mode;
	uint8 picker_source;
	int num_point;
	int draw_finished;
	int ref_point;
	int add_h, add_s, add_v;
	gdouble enter, out;
	uint8 flags;
} PICKER_IMAGE_BRUSH;

typedef struct _ERASER
{
	BRUSH_CORE *core;
	int base_scale;
	gdouble r;
	gdouble alpha;
	gdouble outline_hardness;
	gdouble blur;
	gdouble before_x, before_y;

	int8 channel;
	uint8 shape;
	uint8 brush_mode;
	uint8 flags;
} ERASER;

typedef enum _eBUCKET_SELECT_MODE
{
	BUCKET_RGB,
	BUCKET_RGBA,
	BUCKET_ALPHA
} eBUCKET_SELECT_MODE;

typedef enum _eBUCKET_TARGET
{
	BUCKET_TARGET_ACTIVE_LAYER,
	BUCKET_TARGET_CANVAS
} eBUCKET_TARGET;

typedef enum _eBUCKET_FLAGS
{
	BUCKET_FLAG_ANTI_ALIAS = 0x01
} eBUCKET_FLAGS;

typedef struct _BUCKET
{
	uint16 threshold;
	int16 extend;
	uint8 mode;
	uint8 target;
	uint8 select_direction;
	uint8 flags;
} BUCKET;

typedef enum _ePATTERN_FILL_SELECT_MODE
{
	PATTERN_FILL_RGB,
	PATTERN_FILL_RGBA,
	PATTERN_FILL_ALPHA
} ePATTERN_FILL_SELECT_MODE;

typedef enum _ePATTERN_FILL_TARGET
{
	PATTERN_FILL_TARGET_ACTIVE_LAYER,
	PATTERN_FILL_TARGET_CANVAS
} ePATTERN_FILL_TARGET;

typedef enum _ePATTERN_FILL_FLAGS
{
	PATTERN_FILL_FLAG_ANTI_ALIAS = 0x01
} ePATTERN_FILL_FLAGS;

/*******************************
* PATTERN_FILL�\����           *
* �p�^�[���h��ׂ��̏ڍ׃f�[�^ *
*******************************/
typedef struct _PATTERN_FILL
{
	// �A�v���P�[�V�����̏��
	struct _APPLICATION* app;
	gdouble scale;					// �p�^�[���̊g�嗦
	gdouble flow;					// �h��ׂ��Z�x
	GtkWidget** buttons;			// �p�^�[���I��p�̃{�^���z��
	GtkWidget *mode_select[2];		// �`�����l����2�̂Ƃ��̍������[�h�I���E�B�W�F�b�g
	int32 num_button;				// �p�^�[���I��p�̃{�^���̐�
	int32 pattern_id;				// �g�p���̃p�^�[��ID
	uint16 threshold;				// �h��ׂ��͈͌���p��臒l
	uint8 mode;						// �h��ׂ��͈͌��胂�[�h
	uint8 target;					// �h��ׂ��͈͌���^�[�Q�b�g
	uint8 pattern_flags;			// �p�^�[���̍��E���]�A�㉺���]�̃t���O
	uint8 pattern_mode;				// �p�^�[���̃`�����l������2�̎��̃��[�h
	uint8 area_detect_direction;	// �h��ׂ��͈͂̌��o����
	uint8 flags;					// �A���`�G�C���A�X���̃t���O
	int16 extend;					// �h��Ԃ��͈͂̊g��k��
} PATTERN_FILL;

typedef enum _eGRADATION_MODE
{
	GRADATION_DRAW_RGB_TO_BACK_RGB,
	GRADATION_DRAW_RGB_TO_TRANSPARENT,
	GRADATION_DRAW_RGB_TO_BACK_RGB_BILINEAR,
	GRADATION_DRAW_RGB_TO_TRANSPARENT_BILINEAR,
	GRADATION_DRAW_RGB_TO_BACK_RGB_REPEAT,
	GRADATION_DRAW_RGB_TO_TRANSPARENT_REPEAT,
	GRADATION_DRAW_RGB_TO_BACK_RGB_RADIALLY,
	GRADATION_DRAW_RGB_TO_TRANSPARENT_RADIALLY,
	GRADATION_DRAW_RGB_TO_BACK_RGB_RADIALLY_BILINEAR,
	GRADATION_DRAW_RGB_TO_TRANSPARENT_RADIALLY_BILINEAR,
	GRADATION_DRAW_RGB_TO_BACK_RGB_RADIALLY_REPEAT,
	GRADATION_DRAW_RGB_TO_TRANSPARENT_RADIALLY_REPEAT,
	GRADATION_DRAW_IRIDESCENE_LINEAR,
	GRADATION_DRAW_IRIDESCENE_RADIALLY,
	GRADATION_MODE_NUM
} eDRADATION_MODE;

typedef enum _eGRADATION_FLAGS
{
	GRADATION_STARTED = 0x01,
	GRADATION_COLOR_REVERSE = 0x02
} eGRADATION_FLAGS;

#define GRADATION_PATTERN_TABLE_WIDTH 4
#define GRADATION_PATTERN_TABLE_HEIGHT 4

typedef struct _GRADATION
{
	GtkWidget* button_table[GRADATION_PATTERN_TABLE_HEIGHT][
		GRADATION_PATTERN_TABLE_WIDTH];
	struct _APPLICATION *app;
	gdouble start_x, start_y;
	gdouble end_x, end_y;
	uint16 flags;
	uint16 mode;
} GRADATION;

typedef struct _BLUR_TOOL
{
	BRUSH_CORE *core;
	int base_scale;
	gdouble r;
	gdouble alpha;
	gdouble outline_hardness;
	gdouble opacity;
	gdouble blur;
	gdouble color_extend;
	gdouble before_x, before_y;

	int8 channel;
	uint8 shape;
	uint8 brush_mode;
	uint8 flags;
} BLUR_TOOL;

typedef enum _eSMUDGE_FLAGS
{
	SMUDGE_PRESSURE_SIZE = 0x01,
	SMUDGE_PRESSURE_EXTENTION = 0x02,
	SMUDGE_INITIALIZED = 0x04,
	SMUDGE_DRAW_STARTED = 0x08
} eSMUDGE_FLAGS;

typedef struct _SMUDGE
{
	BRUSH_CORE *core;
	int base_scale;
	gdouble r, before_r;
	gdouble before_x, before_y;
	gdouble blur, opacity;
	gdouble outline_hardness;
	gdouble extention;
	uint32 flags;
} SMUDGE;

typedef struct _MIX_BRUSH
{
	int base_scale;				// �u���V�T�C�Y�̔{��
	gdouble r;					// �u���V�T�C�Y
	gdouble alpha;				// �u���V�Z�x
	gdouble blur;				// �{�P��
	gdouble outline_hardness;	// �֊s�̍d��
	gdouble before_x, before_y;	// 1�X�e�b�v�O�̍��W
	int flags;					// �M���ݒ蓙�̃t���O
} MIX_BRUSH;

typedef enum _eTEXT_TOOL_FLAGS
{
	TEXT_TOOL_STARTED = 0x01,
	TEXT_TOOL_VERTICAL = 0x02,
	TEXT_TOOL_BOLD = 0x04,
	TEXT_TOOL_ITALIC = 0x08,
	TEXT_TOOL_OBLIQUE = 0x10
} eTEXT_TOOL_FLAGS;

typedef struct _TEXT_TOOL
{
	gdouble start_x, start_y;
	gdouble end_x, end_y;
	int base_size;
	gdouble font_size;
	uint16 flags;
	int32 font_id;
} TEXT_TOOL;

typedef struct _PLUG_IN_BRUSH
{
	char *plug_in_name;
	struct _APPLICATION *app;
	void *detail_data;
	GtkWidget* (*setting_widget_new)(struct _APPLICATION* app, struct _BRUSH_CORE* core);
} PLUG_IN_BRUSH;

extern void LoadBrushDetailData(
	BRUSH_CORE* core,
	INI_FILE_PTR file,
	const char* section_name,
	const char* brush_type,
	struct _APPLICATION* app
);

extern void LoadBrushDefaultData(
	BRUSH_CORE* core,
	eBRUSH_TYPE brush_type,
	struct _APPLICATION* app
);

/*********************************************************
* WriteBrushDetailData�֐�                               *
* �u���V�̏ڍאݒ�������o��                             *
* ����                                                   *
* window	: �c�[���{�b�N�X�E�B���h�E                   *
* file_path	: �����o���t�@�C���̃p�X                     *
* app		: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
* �Ԃ�l                                                 *
*	����I��:0	���s:���̒l                              *
*********************************************************/
extern int WriteBrushDetailData(
	struct _TOOL_WINDOW* window,
	const char* file_path,
	struct _APPLICATION *app
);

/*****************************************************
* SetBrushCallBack�֐�                               *
* �u���V�̃}�E�X�֘A�̃R�[���o�b�N�֐���ݒ肷��     *
* ����                                               *
* core	: �R�[���o�b�N�֐���ݒ肷��u���V�̊�{��� *
*****************************************************/
extern void SetBrushCallBack(BRUSH_CORE* core);

/*************************************************************
* SetEditSelectionCallBack�֐�                               *
* �u���V�̃}�E�X�֘A�̃R�[���o�b�N�֐���I��͈͕ҏW�p�ɂ��� *
* ����                                                       *
* core	: �R�[���o�b�N�֐���ݒ肷��u���V�̊�{���         *
*************************************************************/
extern void SetEditSelectionCallBack(BRUSH_CORE* core);

extern gboolean LoadPlugInBrushCallbacks(GModule* module, BRUSH_CORE* core, PLUG_IN_BRUSH* brush);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_BRUSHES_H_
