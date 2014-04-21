#ifndef _INCLUDED_DRAW_WINDOW_H_
#define _INCLUDED_DRAW_WINDOW_H_

#include "configure.h"
#if defined(USE_3D_LAYER) && USE_3D_LAYER != 0
# include <GL/glew.h>
#endif
#include <gtk/gtk.h>
#include "lcms/lcms2.h"
#include "layer.h"
#include "history.h"
#include "selection_area.h"
#include "memory_stream.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _eDRAW_WINDOW_FLAGS
{
	DRAW_WINDOW_HAS_SELECTION_AREA = 0x01,
	DRAW_WINDOW_UPDATE_ACTIVE_UNDER = 0x02,
	DRAW_WINDOW_UPDATE_ACTIVE_OVER = 0x04,
	DRAW_WINDOW_DISPLAY_HORIZON_REVERSE = 0x08,
	DRAW_WINDOW_EDIT_SELECTION = 0x10,
	DRAW_WINDOW_UPDATE_PART = 0x20,
	DRAW_WINDOW_DRAWING_STRAIGHT = 0x40,
	DRAW_WINDOW_SECOND_BG = 0x80,
	DRAW_WINDOW_TOOL_CHANGING = 0x100,
	DRAW_WINDOW_EDITTING_3D_MODEL = 0x200,
	DRAW_WINDOW_IS_FOCAL_WINDOW = 0x400
} eDRAW_WINDOW_FLAGS;

typedef struct _UPDATE_RECTANGLE
{
	FLOAT_T x, y;
	FLOAT_T width, height;
	cairo_t *cairo_p;
	cairo_surface_t *surface_p;
} UPDATE_RECTANGLE;

typedef struct _CALLBACK_IDS
{
	unsigned int display;
	unsigned int mouse_button_press;
	unsigned int mouse_move;
	unsigned int mouse_button_release;
	unsigned int mouse_wheel;
	unsigned int configure;
	unsigned int enter;
	unsigned int leave;
} CALLBACK_IDS;

#if defined(USE_3D_LAYER) && USE_3D_LAYER != 0
typedef struct _GL_DRAW_POINT
{
	GLfloat texture_coord[2];
	GLfloat vertex[3];
} GL_DRAW_POINT;

typedef struct _GL_DRAW_DATA
{
	GLuint texture_name;
	GL_DRAW_POINT points[4];
	float length;
	float angle;
	struct
	{
		double x1, y1, x2, y2;
	} update_rectangle, before_rectangle;
} GL_DRAW_DATA;

#endif

typedef struct _DRAW_WINDOW
{
	uint8 channel;		// �`�����l����
	uint8 color_mode;	// �J���[���[�h(���݂�RGBA32�̂�)

	int16 degree;		// ��]�p(�x���@)

	gchar* file_name;	// �t�@�C����

	gchar* file_path;	// �t�@�C���p�X

	// ���Ԃōĕ`����ĂԊ֐���ID
	guint timer_id;
	guint auto_save_id;

	// �^�u�؂�ւ��O�̃��C���[�r���[�̈ʒu
	int layer_view_position;

	// �摜�̃I���W�i�����A����
	int original_width, original_height;
	// �`��̈�̕��A����(4�̔{��)
	int width, height;
	// ��s���̃o�C�g���A���C���[�ꖇ���̃o�C�g��
	int stride, pixel_buf_size;
	// ��]�p�Ƀ}�[�W�����Ƃ����`��̈�̍L���A��s���̃o�C�g��
	int disp_size, disp_stride;
	// ��]��̍��W�v�Z�p
	FLOAT_T half_size;
	FLOAT_T angle;		// ��]�p(�ʓx�@)
	// ��]�����p�̎O�p�֐��̒l
	FLOAT_T sin_value, cos_value;
	// ��]�����ł̈ړ���
	FLOAT_T trans_x, trans_y;
	// �J�[�\�����W
	FLOAT_T cursor_x, cursor_y;
	FLOAT_T before_cursor_x, before_cursor_y;
	// �O��`�F�b�N���̃J�[�\�����W
	FLOAT_T before_x, before_y;
	// �Ō�ɃN���b�N�܂��̓h���b�O���ꂽ���W
	FLOAT_T last_x, last_y;
	// �Ō�ɃN���b�N�܂��̓h���b�O���ꂽ���̕M��
	FLOAT_T last_pressure;
	// �J�[�\�����W�␳�p
	FLOAT_T add_cursor_x, add_cursor_y;
	// ���ϖ@��u���␳�ł̍��W�ϊ��p
	FLOAT_T rev_add_cursor_x, rev_add_cursor_y;
	// �\���p�̃p�^�[��
	cairo_pattern_t *rotate;
	// ��ʕ����X�V�p
	UPDATE_RECTANGLE update, temp_update;
	// �`��̈�X�N���[���̍��W
	int scroll_x, scroll_y;
	// ��ʍX�V���̃N���b�s���O�p
	int update_clip_area[4][2];

	// Windows��Linux�ŃC�x���g�̌Ă΂�鏇�����قȂ�悤�Ȃ̂ŉ��p
	GdkModifierType state;

	// �R�[���o�b�N�֐���ID���L��
	CALLBACK_IDS callbacks;

	uint8 *back_ground;		// �w�i�̃s�N�Z���f�[�^
	uint8 *brush_buffer;	// �u���V�p�̃o�b�t�@

	LAYER* layer;			// ��ԉ��̃��C���[
	// �\���p�A�G�t�F�N�g�p�A�u���V�J�[�\���\���p�̈ꎞ�ۑ�
	LAYER* disp_layer, *effect, *disp_temp, *scaled_mixed;
	// �A�N�e�B�u�ȃ��C���[&���C���[�Z�b�g�ւ̃|�C���^
		// �y�ѕ\�����C���[�������������C���[
	LAYER* active_layer, *active_layer_set, *mixed_layer;
	// ��Ɨp�A�ꎞ�ۑ��p�A�I��͈́A�A�N�e�B�u���C���[��艺�̃��C���[
	LAYER *work_layer, *temp_layer,
		*selection, *under_active;
	// �}�X�N�ƃ}�X�N�K�p�O�̈ꎞ�ۑ��p
	LAYER* mask, *mask_temp;
	// �e�N�X�`���p
	LAYER* texture;
	// �\���p�p�^�[��
	cairo_pattern_t *mixed_pattern;
	// ���`�����l���݂̂̃}�X�N�p�T�[�t�F�[�X
	cairo_surface_t *alpha_surface, *alpha_temp, *gray_mask_temp;
	// ���`�����l���݂̂̃C���[�W
	cairo_t *alpha_cairo, *alpha_temp_cairo, *gray_mask_cairo;

	uint16 num_layer;		// ���C���[�̐�
	uint16 zoom;			// �g��E�k����
	FLOAT_T zoom_rate;		// ���������_�^�̊g��E�k����
	FLOAT_T rev_zoom;		// �g��E�k�����̋t��

	// �𑜓x
	int16 resolution;

	// �I��͈̓A�j���[�V�����p�̃^�C�}�[
	GTimer *timer;

	// �����ۑ��p�̃^�C�}�[
	GTimer *auto_save_timer;

	// �I��͈͕\���p�f�[�^
	SELECTION_AREA selection_area;

	// �����f�[�^
	HISTORY history;

	// �`��̈�̃E�B�W�F�b�g
	GtkWidget* window;

	// �`��̈�̃X�N���[��
	GtkWidget* scroll;

	// �\���X�V���̃t���O
	unsigned int flags;

	// �ό`�����p�̃f�[�^
	struct _TRANSFORM_DATA* transform;

	// 2�߂̔w�i�F
	uint8 second_back_ground[3];

	// �\���t�B���^�[�̏��
	uint8 display_filter_mode;

	// ICC�v���t�@�C���̃f�[�^
	char *icc_profile_path;
	void *icc_profile_data;
	int32 icc_profile_size;
	// ICC�v���t�@�C���̓��e
	cmsHPROFILE input_icc;
	// ICC�v���t�@�C���ɂ��F�ϊ��p
	cmsHTRANSFORM icc_transform;

	// �Ǐ��L�����o�X
	struct _DRAW_WINDOW *focal_window;

	// �ǉ��f�[�^
	void *extra_data;

	// �A�v���P�[�V�����S�̊Ǘ��p�\���̂ւ̃|�C���^
	struct _APPLICATION* app;

#if defined(USE_3D_LAYER) && USE_3D_LAYER != 0
	// 3D���f�����O�p�f�[�^
	void *first_project;
	// ���_�E�e�N�X�`���f�[�^
	GL_DRAW_DATA gl_data;
#endif
} DRAW_WINDOW;

// �֐��̃v���g�^�C�v�錾
/***************************************************************
* CreateDrawWindow�֐�                                         *
* �`��̈���쐬����                                           *
* ����                                                         *
* width		: �L�����o�X�̕�                                   *
* height	: �L�����o�X�̍���                                 *
* channel	: �L�����o�X�̃`�����l����(RGB:3, RGBA:4)          *
* name		: �L�����o�X�̖��O                                 *
* note_book	: �`��̈�^�u�E�B�W�F�b�g                         *
* window_id	: �`��̈�z�񒆂�ID                               *
* app		: �A�v���P�[�V�����̏����Ǘ�����\���̂̃A�h���X *
* �Ԃ�l                                                       *
*	�`��̈�̏����Ǘ�����\���̂̃A�h���X                   *
***************************************************************/
extern DRAW_WINDOW* CreateDrawWindow(
	int32 width,
	int32 height,
	uint8 channel,
	const gchar* name,
	GtkWidget* note_book,
	uint16 window_id,
	struct _APPLICATION* app
);

/***************************************************************
* CreateTempDrawWindow�֐�                                     *
* �ꎞ�I�ȕ`��̈���쐬����                                   *
* ����                                                         *
* width		: �L�����o�X�̕�                                   *
* height	: �L�����o�X�̍���                                 *
* channel	: �L�����o�X�̃`�����l����(RGB:3, RGBA:4)          *
* name		: �L�����o�X�̖��O                                 *
* note_book	: �`��̈�^�u�E�B�W�F�b�g                         *
* window_id	: �`��̈�z�񒆂�ID                               *
* app		: �A�v���P�[�V�����̏����Ǘ�����\���̂̃A�h���X *
* �Ԃ�l                                                       *
*	�`��̈�̏����Ǘ�����\���̂̃A�h���X                   *
***************************************************************/
extern DRAW_WINDOW* CreateTempDrawWindow(
	int32 width,
	int32 height,
	uint8 channel,
	const gchar* name,
	GtkWidget* note_book,
	uint16 window_id,
	struct _APPLICATION* app
);

/***************************************
* DeleteDrawWindow�֐�                 *
* �`��̈�̏����폜                 *
* ����                                 *
* window	: �`��̈�̏��̃A�h���X *
***************************************/
extern void DeleteDrawWindow(DRAW_WINDOW** window);

/*****************************************************
* SwapDrawWindowFromMemoryStream�֐�                 *
* �������[��̕`��̈�f�[�^�Ɠ���ւ���             *
* ����                                               *
* window	: �`��̈�̏��                         *
* stream	: �������[��̕`��̈�̃f�[�^�X�g���[�� *
*****************************************************/
extern void SwapDrawWindowFromMemoryStream(DRAW_WINDOW* window, MEMORY_STREAM_PTR stream);

/***********************************************
* OnCloseDrawWindow�֐�                        *
* �^�u��������Ƃ��̃R�[���o�b�N�֐�       *
* ����                                         *
* data	: �`��̈�̃f�[�^                     *
* page	: ����^�u��ID                       *
* �Ԃ�l                                       *
*	���鑀��̒��~:TRUE ���鑀�쑱�s:FALSE *
***********************************************/
extern gboolean OnCloseDrawWindow(void* data, gint page);

/***********************************************************
* GetWindowID�֐�                                          *
* �`��̈��ID���擾����                                   *
* ����                                                     *
* window	: �`��̈�̏��                               *
* app		: 	�A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
* �Ԃ�l                                                   *
*	�`��̈��ID (�s���ȕ`��̈�Ȃ��-1)                  *
***********************************************************/
int GetWindowID(DRAW_WINDOW* window, struct _APPLICATION* app);

/*********************************
* DrawWindowChangeZoom�֐�       *
* �`��̈�̊g��k������ύX���� *
* ����                           *
* window	: �`��̈�̏��     *
* zoom		: �V�����g��k����   *
*********************************/
extern void DrawWindowChangeZoom(
	DRAW_WINDOW* window,
	int16 zoom
);

/*****************************************
* FlipDrawWindowHorizontally�֐�         *
* �`��̈�𐅕����]����                 *
* ����                                   *
* window	: �������]����`��̈�̏�� *
*****************************************/
extern void FlipDrawWindowHorizontally(DRAW_WINDOW* window);

/*****************************************
* FlipDrawWindowVertically�֐�           *
* �`��̈�𐂒����]����                 *
* ����                                   *
* window	: �������]����`��̈�̏�� *
*****************************************/
extern void FlipDrawWindowVertically(DRAW_WINDOW* window);

/***************************************
* LayerAlpha2SelectionArea�֐�         *
* ���C���[�̕s����������I��͈͂ɂ��� *
* ����                                 *
* window	: �`��̈�̏��           *
***************************************/
extern void LayerAlpha2SelectionArea(DRAW_WINDOW* window);

/*****************************************
* LayerAlphaAddSelectionArea�֐�         *
* ���C���[�̕s����������I��͈͂ɉ����� *
* ����                                   *
* window	: �`��̈�̏��             *
*****************************************/
extern void LayerAlphaAddSelectionArea(DRAW_WINDOW* window);

/*****************************
* MergeAllLayer�֐�          *
* �S�Ẵ��C���[����������   *
* ����                       *
* window	: �`��̈�̏�� *
*****************************/
extern void MergeAllLayer(DRAW_WINDOW* window);

/*******************************************
* ChangeDrawWindowResolution�֐�           *
* �𑜓x��ύX����                         *
* ����                                     *
* window		: �𑜓x��ύX����`��̈� *
* new_width		: �V������                 *
* new_height	: �V��������               *
*******************************************/
extern void ChangeDrawWindowResolution(DRAW_WINDOW* window, int32 new_width, int32 new_height);

/*************************************************
* AddChangeDrawWindowResolutionHistory�֐�       *
* �𑜓x�ύX�̗����f�[�^��ǉ�����               *
* ����                                           *
* window		: �𑜓x��ύX����`��̈�̏�� *
* new_width		: �V������                       *
* new_height	: �V��������                     *
*************************************************/
extern void AddChangeDrawWindowResolutionHistory(
	DRAW_WINDOW* window,
	int32 new_width,
	int32 new_height
);

/*******************************************
* ChangeDrawWindowSize�֐�                 *
* �`��̈�̃T�C�Y��ύX����               *
* ����                                     *
* window		: �𑜓x��ύX����`��̈� *
* new_width		: �V������                 *
* new_height	: �V��������               *
*******************************************/
extern void ChangeDrawWindowSize(DRAW_WINDOW* window, int32 new_width, int32 new_height);

/*****************************************
* UpdateDrawWindowClippingArea           *
* ��ʍX�V���ɃN���b�s���O����̈���X�V *
* ����                                   *
* window	: �`��̈�̏��             *
*****************************************/
extern void UpdateDrawWindowClippingArea(DRAW_WINDOW* window);

/*************************************************
* ClipUpdateArea�֐�                             *
* ��ʂ̃X�N���[���ɓ����Ă��镔���ŃN���b�s���O *
* ����                                           *
* window	: �`��̈�̏��                     *
* cairo_p	: Cairo���                          *
*************************************************/
extern void ClipUpdateArea(DRAW_WINDOW* window, cairo_t* cairo_p);

/*******************************************************************
* DrawWindowSetIccProfile�֐�                                      *
* �L�����o�X��ICC�v���t�@�C�������蓖�Ă�                          *
* ����                                                             *
* window	: �`��̈�̏��(icc_profile_data�Ƀf�[�^���蓖�čς�) *
* data_size	: ICC�v���t�@�C���̃f�[�^�̃o�C�g��                    *
* ask_set	: �\�t�g�v���[�t�\����K�p���邩��q�˂邩�ۂ�         *
*******************************************************************/
extern void DrawWindowSetIccProfile(DRAW_WINDOW* window, int32 data_size, gboolean ask_set);

/*************************************************
* AddChangeDrawWindowSizeHistory�֐�             *
* �L�����o�X�T�C�Y�ύX�̗����f�[�^��ǉ�����     *
* ����                                           *
* window		: �𑜓x��ύX����`��̈�̏�� *
* new_width		: �V������                       *
* new_height	: �V��������                     *
*************************************************/
extern void AddChangeDrawWindowSizeHistory(
	DRAW_WINDOW* window,
	int32 new_width,
	int32 new_height
);

extern void RasterizeVectorLine(
	DRAW_WINDOW* window,
	VECTOR_LAYER* layer,
	VECTOR_LINE* line,
	VECTOR_LAYER_RECTANGLE* rect
);

extern void RasterizeVectorLayer(
	DRAW_WINDOW* window,
	LAYER* target,
	VECTOR_LAYER* layer
);

/***********************************************
* IsVectorLineInSelectionArea�֐�              *
* �����I��͈͓����ۂ��𔻒肷��               *
* ����                                         *
* window			: �`��̈�̏��           *
* selection_pixels	: �I��͈͂̃s�N�Z���f�[�^ *
* line				: ���肷���               *
* �Ԃ�l                                       *
*	�I��͈͓�:1	�I��͈͊O:0               *
***********************************************/
extern int IsVectorLineInSelectionArea(
	DRAW_WINDOW* window,
	uint8* selection_pixels,
	VECTOR_LINE* line
);

/*******************************************
* AddControlPointHistory�֐�               *
* ����_�ǉ��̗������쐬����               *
* ����                                     *
* window	: �`��̈�̏��               *
* layer		: ����_��ǉ��������C���[     *
* line		: ����_��ǉ��������C��       *
* point		: �ǉ���������_�̃A�h���X     *
* tool_name	: ����_��ǉ������c�[���̖��O *
*******************************************/
extern void AddControlPointHistory(
	DRAW_WINDOW* window,
	LAYER* layer,
	VECTOR_LINE* line,
	VECTOR_POINT* point,
	const char* tool_name
);

/***************************************************
* AddTopLineControlPointHistory�֐�                *
* �܂���c�[�����Ȑ��c�[���ł̐���_�ǉ�������ǉ� *
* ����                                             *
* window		: �`��̈�̏��                   *
* layer			: ����_��ǉ��������C���[         *
* line			: ����_��ǉ��������C��           *
* point			: �ǉ���������_                   *
* tool_name		: ����_��ǉ������c�[���̖��O     *
* add_line_flag	: ���C���ǉ��������Ƃ��̃t���O     *
***************************************************/
extern void AddTopLineControlPointHistory(
	DRAW_WINDOW* window,
	LAYER* layer,
	VECTOR_LINE* line,
	VECTOR_POINT* point,
	const char* tool_name,
	uint8 add_line_flag
);

/***********************************************************
* AddDeleteLinesHistory�֐�                                *
* �����̐��𓯎��ɍ폜�����ۂ̗����f�[�^��ǉ�             *
* ����                                                     *
* window		: �`��̈�̏����Ǘ�����\���̂̃A�h���X *
* active_layer	: �A�N�e�B�u�ȃ��C���[                     *
* line_data		: �폜������̃f�[�^                       *
* line_indexes	: �폜������̈�ԉ����琔�����C���f�b�N�X *
* num_lines		: �폜������̐�                           *
* tool_name		: �폜�Ɏg�p�����c�[���̖��O               *
***********************************************************/
extern void AddDeleteLinesHistory(
	DRAW_WINDOW* window,
	LAYER* active_layer,
	VECTOR_LINE* line_data,
	uint32* line_indexes,
	uint32 num_lines,
	const char* tool_name
);

/*******************************************
* DeleteLayerSet�֐�                       *
* ���C���[�Z�b�g�̍폜���s��               *
* ����                                     *
* layer_set	: �폜���郌�C���[�Z�b�g       *
* window	: ���C���[�Z�b�g�����`��̈� *
*******************************************/
extern void DeleteLayerSet(LAYER* layer_set, DRAW_WINDOW* window);

/*************************************************
* MixLayerSet�֐�                                *
* ���C���[�Z�b�g��������                         *
* ����                                           *
* bottom	: ���C���[�Z�b�g�̈�ԉ��̃��C���[   *
* next		: ������Ɏ��ɍ������郌�C���[       *
* window	: �`��̈���Ǘ�����\���̂̃A�h���X *
*************************************************/
extern void MixLayerSet(LAYER* bottom, LAYER** next, DRAW_WINDOW* window);

/***************************************************************
* MixLayerSetActiveOver�֐�                                    *
* ���C���[�Z�b�g���̃A�N�e�B�u���C���[�ȏ�̃��C���[���������� *
* ����                                                         *
* start		: �A�N�e�B�u���C���[                               *
* next		: ������̎��ɍ������郌�C���[                     *
* window	: �`��̈���Ǘ�����\���̂̃A�h���X               *
***************************************************************/
extern void MixLayerSetActiveOver(LAYER* start, LAYER** next, DRAW_WINDOW* window);

extern void RenderTextLayer(DRAW_WINDOW* window, struct _LAYER* target, TEXT_LAYER* layer);

extern void DisplayTextLayerRange(DRAW_WINDOW* window, TEXT_LAYER* layer);

extern void DisplayEditSelection(DRAW_WINDOW* window);

/***************************************
* g_part_layer_blend_funcs�z��         *
* ���C���[���������֐��ւ̃|�C���^�z�� *
* ����                                 *
* src		: �������郌�C���[         *
* update	: �X�V�͈͂̏��           *
***************************************/
extern void (*g_part_layer_blend_funcs[])(LAYER* src, UPDATE_RECTANGLE* update);

/*****************************************
* AutoSave�֐�                           *
* �o�b�N�A�b�v�Ƃ��ăt�@�C����ۑ�����   *
* ����                                   *
* window	: �o�b�N�A�b�v�����`��̈� *
*****************************************/
extern void AutoSave(DRAW_WINDOW* window);

/***************************************************
* GetBlendedUnderLayer�֐�                         *
* �Ώۂ�艺�̃��C���[�������������C���[���擾���� *
* ����                                             *
* target			: �Ώۂ̃��C���[               *
* window			: �`��̈�̏��               *
* use_back_ground	: �w�i�F���g�p���邩�ǂ���     *
* �Ԃ�l                                           *
*	�����������C���[                               *
***************************************************/
extern LAYER* GetBlendedUnderLayer(LAYER* target, DRAW_WINDOW* window, int use_back_ground);

extern void DivideLinesUndo(DRAW_WINDOW* window, void* p);
extern void DivideLinesRedo(DRAW_WINDOW* window, void* p);

extern void ScrollSizeChangeEvent(GtkWidget* scroll, GdkRectangle* size, DRAW_WINDOW* window);

extern gboolean DrawWindowConfigurEvent(GtkWidget* widget, GdkEventConfigure* event_info, DRAW_WINDOW* window);

/*********************************
* Change2FocalMode�֐�           *
* �Ǐ��L�����o�X���[�h�Ɉڍs���� *
* ����                           *
* parent_window	: �e�L�����o�X   *
*********************************/
extern void Change2FocalMode(DRAW_WINDOW* parent_window);

/*********************************
* ReturnFromFocalMode�֐�        *
* �Ǐ��L�����o�X���[�h����߂�   *
* ����                           *
* parent_window	: �e�L�����o�X   *
*********************************/
extern void ReturnFromFocalMode(DRAW_WINDOW* parent_window);

#if defined(USE_3D_LAYER) && USE_3D_LAYER != 0
extern void InitializeGL(DRAW_WINDOW* window);
extern void ResizeGL(DRAW_WINDOW* window, int width, int height);
extern void RotateGL(DRAW_WINDOW* window, float angle);
#endif

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_DRAW_WINDOW_H_
