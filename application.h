#ifndef _INCLUDED_APPLICATION_H_
#define _INCLUDED_APPLICATION_H_

#define MAJOR_VERSION 1

#if MAJOR_VERSION == 1
# define MINOR_VERSION 3
# define RELEASE_VERSION 1
# define BUILD_VERSION 0
#elif MAJOR_VERSION == 2
# define MINOR_VERSION 0
# define RELEASE_VERSION 1
# define BUILD_VERSION 1
#endif

#define FILE_VERSION 4

#include "draw_window.h"
// �`��̈�̍ő吔
#define MAX_DRAW_WINDOW 128
// �^�b�`�C�x���g�������o�̍ő吔
#define MAX_TOUCH 10
// 3D���f�����쎞�̃X�N���[���o�[���̃}�[�W��
#define SCROLLED_WINDOW_MARGIN 32

#include "lcms/lcms2.h"
#include "labels.h"
#include "tool_box.h"
#include "layer_window.h"
#include "navigation.h"
#include "preview_window.h"
#include "reference_window.h"
#include "brush_core.h"
#include "common_tools.h"
#include "vector_brush_core.h"
#include "vector_brushes.h"
#include "color.h"
#include "pattern.h"
#include "texture.h"
#include "smoother.h"
#include "preference.h"
#include "display_filter.h"

#if defined(USE_3D_LAYER) && USE_3D_LAYER != 0
# include "MikuMikuGtk+/mikumikugtk.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// �������t�@�C���̃t�@�C���p�X
#define INITIALIZE_FILE_PATH "application.ini"
// �p���b�g�t�@�C���̃p�X
#define PALLETE_FILE_PATH "pallete.kpl"
// �f�t�H���g�̉𑜓x
#define DEFALUT_RESOLUTION 96

typedef enum _eUTILITY_PLACE
{
	UTILITY_PLACE_WINDOW,
	UTILITY_PLACE_LEFT,
	UTILITY_PLACE_RIGHT
} eUTILITY_PLACE;

typedef enum _eTOOL_WINDOW_FLAGS
{
	TOOL_USING_BRUSH = 0x01,
	TOOL_DOCKED = 0x02,
	TOOL_PLACE_RIGHT = 0x04,
	TOOL_SHOW_COLOR_CIRCLE = 0x08,
	TOOL_SHOW_COLOR_PALLETE = 0x10
} eTOOL_WINDOW_FLAGS;

/*************************************************
* eAPPLICATION_FLAGS�񋓑�                       *
* �A�v���P�[�V�����̃t�@�C�����쒆���ŗ��t���O *
*************************************************/
typedef enum _eAPPLICATION_FLAGS
{
	APPLICATION_IN_OPEN_OPERATION = 0x01,			// �t�@�C���I�[�v��
	APPLICATION_IN_MAKE_NEW_DRAW_AREA = 0x02,		// �`��̈�̐V�K�쐬
	APPLICATION_IN_REVERSE_OPERATION = 0x04,		// ���E���]������
	APPLICATION_IN_EDIT_SELECTION = 0x08,			// �I��͈͕ҏW�؂�ւ���
	APPLICATION_INITIALIZED = 0x10,					// �������ς�
	APPLICATION_IN_DELETE_EVENT = 0x20,				// �폜�C�x���g��
	APPLICATION_FULL_SCREEN = 0x40,					// �t���X�N���[�����[�h
	APPLICATION_WINDOW_MAXIMIZE = 0x80,				// �E�B���h�E�̍ő剻
	APPLICATION_DISPLAY_GRAY_SCALE = 0x100,			// �O���[�X�P�[���ł̕\��
	APPLICATION_DISPLAY_SOFT_PROOF = 0x200,			// ICC�v���t�@�C���Ń\�t�g�v���[�t�\��
	APPLICATION_REMOVE_ACCELARATOR = 0x400,			// �V���[�g�J�b�g�L�[�𖳌���
	APPLICATION_DRAW_WITH_TOUCH = 0x800,			// �^�b�`�C�x���g�ŕ`�悷��
	APPLICATION_SET_BACK_GROUND_COLOR = 0x1000,		// �L�����o�X�̔w�i��ݒ肷��
	APPLICATION_SHOW_PREVIEW_ON_TASK_BAR = 0x2000,	// �v���r���[�E�B���h�E���^�X�N�o�[�ɕ\������
	APPLICATION_IN_SWITCH_DRAW_WINDOW = 0x4000		// �`��̈�̐ؑ֒�
} eAPPLICATION_FLAGS;

#define DND_THRESHOLD 20

#define BRUSH_TABLE_WIDTH (4)
#define BRUSH_TABLE_HEIGHT (16)

#define MAX_BRUSH_SIZE 500
#define MAX_BRUSH_STRIDE ((MAX_BRUSH_SIZE) * 4)

#define VECTOR_BRUSH_TABLE_WIDTH (4)
#define VECTOR_BRUSH_TABLE_HEIGHT (8)

#define COMMON_TOOL_TABLE_WIDTH 5
#define COMMON_TOOL_TABLE_HEIGHT 2

#define THUMBNAIL_SIZE 128

#define PALLETE_WIDTH 16
#define PALLETE_HEIGHT 16

typedef enum _eINPUT_DEVICE
{
	INPUT_PEN,
	INPUT_ERASER
} eINPUT_DEVICE;

/*************************
* TOOL_WINDOW�\����      *
* �c�[���{�b�N�X�̃f�[�^ *
*************************/
typedef struct _TOOL_WINDOW
{
	// �E�B���h�E�A�u���V�e�[�u���A�u���V�e�[�u���̃X�N���[��
	GtkWidget *window, *common_tool_table, *brush_table, *brush_scroll;
	// ���j���[�E�B�W�F�b�g
	GtkWidget *menu_item;
	// �u���V�e�[�u���Əڍאݒ����؂�y�[��
	GtkWidget *pane;
	// �E�B�W�F�b�g�̔z�u
	int place;
	// �E�B���h�E�̈ʒu�A�T�C�Y
	int window_x, window_y, window_width, window_height;
	// �u���V�e�[�u���Əڍאݒ����؂�y�[���̈ʒu
	gint pane_position;
	// �u���V�{�^���Ɏg�p����t�H���g��
	char* font_file;
	// �F�I��p�̃f�[�^
	COLOR_CHOOSER *color_chooser;
	// �u���V�̃R�A�e�[�u��
	BRUSH_CORE brushes[BRUSH_TABLE_HEIGHT][BRUSH_TABLE_WIDTH];
#if MAJOR_VERSION > 1
	// �^�b�`�C�x���g�p�̃f�[�^
	BRUSH_CORE touch[MAX_TOUCH];
	// �w����p
	GdkEventSequence *fingers[MAX_TOUCH];
	// ���݂̃^�b�`�C�x���g���̎w�̐�
	int num_touch;
#endif
	// ���ݎg�p���Ă���u���V�ւ̃|�C���^
	BRUSH_CORE *active_brush[2];
	// �u���V�摜�̃s�N�Z���f�[�^
	uint8 *brush_pattern_buff, *temp_pattern_buff;
	// �R�s�[����u���V�ւ̃|�C���^
	void* copy_brush;
	// �u���V�t�@�C���̕����R�[�h
	char* brush_code;
	// ���ʃc�[���̃u���V�e�[�u��
	COMMON_TOOL_CORE common_tools[COMMON_TOOL_TABLE_HEIGHT][COMMON_TOOL_TABLE_WIDTH];
	// ���ݎg�p���Ă��鋤�ʃc�[���ւ̃|�C���^
	COMMON_TOOL_CORE* active_common_tool;
	// ���ʃc�[���t�@�C���̕����R�[�h
	char* common_tool_code;
	// �x�N�g���u���V�̃R�A�e�[�u��
	VECTOR_BRUSH_CORE vector_brushes[VECTOR_BRUSH_TABLE_HEIGHT][VECTOR_BRUSH_TABLE_WIDTH];
	// ���ݎg�p���Ă���x�N�g���u���V�ւ̃|�C���^
	VECTOR_BRUSH_CORE *active_vector_brush[2];
	// �x�N�g���u���V�t�@�C���̕����R�[�h
	char* vector_brush_code;
	// �ʏ탌�C���[��Control�L�[��������Ă���Ƃ��̓X�|�C�g�c�[���ɕ΍������邽��
		// �X�|�C�g�c�[���̃f�[�^��ێ����Ă���
	COMMON_TOOL_CORE color_picker_core;
	COLOR_PICKER color_picker;
	// �x�N�g�����C���[��Control�AShift�L�[��������Ă���Ƃ��ɂ͐���_�c�[���ɕω������邽��
		// ����_�c�[���̃f�[�^��ێ����Ă���
	VECTOR_BRUSH_CORE vector_control_core;
	CONTROL_POINT_TOOL vector_control;
	// �u���V��UI�E�B�W�F�b�g
	GtkWidget *ui, *detail_ui, *detail_ui_scroll;
	// ��u���␳�̏��
	SMOOTHER smoother;
	// �^�b�`�C�x���g�p�̎�u���␳
#if MAJOR_VERSION > 1
	SMOOTHER touch_smoother[MAX_TOUCH];
#endif
	// �p���b�g�̏��
	uint8 pallete[(PALLETE_WIDTH*PALLETE_HEIGHT)][3];
	uint8 pallete_use[((PALLETE_WIDTH*PALLETE_HEIGHT)+7)/8];
	// �c�[���{�b�N�X�S�̂ň����t���O
	unsigned int flags;
} TOOL_WINDOW;

/*************************************************************************
* APPLICATION_MENUS�\����                                                *
* �A�v���P�[�V�����ŕ`��̈�̗L�����ŗL���A�������ω����郁�j���[���L�� *
*************************************************************************/
typedef struct _APPLICATION_MENUS
{	// �`��̈悪�����Ƃ��ɖ���
	GtkWidget *disable_if_no_open[80];
	// �I��͈͂������Ƃ��ɖ���
	GtkWidget *disable_if_no_select[32];
	// ���C���[���ꖇ�����Ȃ��Ƃ��ɖ���
	GtkWidget *disable_if_single_layer[8];
	// �ʏ�̃��C���[�̂Ƃ��ɖ���
	GtkWidget *disable_if_normal_layer[8];
	uint8 num_disable_if_no_open;
	uint8 num_disable_if_no_select;
	uint8 num_disable_if_single_layer;
	uint8 num_disable_if_normal_layer;
	// ���̃��C���[�ƌ������j���[
	GtkWidget *merge_down_menu;
	// �w�i�F�؂�ւ����j���[
	GtkWidget *change_back_ground_menu;
	// �\���t�B���^�[�؂�ւ����j���[
	GtkWidget *display_filter_menus[NUM_DISPLAY_FUNC_TYPE];
} APPLICATION_MENUS;

/***********************
* MENU_DATA�\����      *
* ���j���[�ň����f�[�^ *
***********************/
typedef struct _MENU_DATA
{
	struct
	{
		int32 width, height;
		int16 resolution;
		uint8 second_bg_color[3];
	} make_new;

	GtkWidget *reverse_horizontally;
	GtkWidget *edit_selection;
} MENU_DATA;

/*****************************************
* SCRIPTS�\����                          *
* �X�N���v�g�f�B���N�g���̃X�N���v�g��� *
*****************************************/
typedef struct _SCRIPTS
{
	gchar **file_paths;
	gchar **file_names;
	int num_script;
} SCRIPTS;

/*************************************
* APPLICATION�\����                  *
* �A�v���P�[�V�����S�̂̃f�[�^���Ǘ� *
*************************************/
typedef struct _APPLICATION
{
	// �E�B���h�E�A���j���[�A�^�u������p�b�L���O�{�b�N�X�A�^�u
	GtkWidget *window, *vbox, *note_book;
	// �X�e�[�^�X�o�[
	GtkWidget *status_bar;
	// �ۑ��A�t�B���^�[���̐i���󋵂�\���v���O���X�o�[
	GtkWidget *progress;
	// �V���O���E�B���h�E�p�Ƀc�[�������鍶�E�̃y�[��
	GtkWidget *left_pane, *right_pane;
	// �i�r�Q�[�V�����ƃ��C���[�r���[���h�b�L���O���邽�߂̃{�b�N�X
	GtkWidget *navi_layer_pane;
	// �E�B���h�E�̈ʒu�A�T�C�Y
	int window_x, window_y, window_width, window_height;
	// ���E�y�[���̈ʒu
	gint left_pane_position, right_pane_position;
	// �V���[�g�J�b�g�L�[
	GtkAccelGroup *hot_key;
	// ���̓f�o�C�X
	eINPUT_DEVICE input;
	// �`��̈�
	DRAW_WINDOW* draw_window[MAX_DRAW_WINDOW];
	// �c�[���E�B���h�E
	TOOL_WINDOW tool_window;
	// ���C���[�E�B���h�E
	LAYER_WINDOW layer_window;
	// �i�r�Q�[�V�����E�B���h�E
	NAVIGATION_WINDOW navigation_window;
	// �v���r���[�E�B���h�E
	PREVIEW_WINDOW preview_window;
	// �Q�l�p�摜�\���E�B���h�E
	REFERENCE_WINDOW reference_window;
	// ���ݒ�
	PREFERENCE preference;
	// �`��̈�̏�Ԃɍ��킹�ėL���A�������؂�ւ�郁�j���[
	APPLICATION_MENUS menus;
	// �\�����ɓK�p����t�B���^�[�̃f�[�^
	DISPLAY_FILTER display_filter;
	// �`��̈�̐�
	int window_num;
	// ���݃A�N�e�B�u�ȕ`��̈�
	int active_window;

	// �g�p�\�ȃt�H���g���X�g
	PangoFontFamily** font_list;
	// �g�p�\�ȃt�H���g�̐�
	int num_font;

	// �u���V�e�N�X�`���p
	TEXTURES textures;
	// �g�p�e�N�X�`���̃��x��
	GtkWidget *texture_label;

	// UI�ɕ\�����镶����
	APPLICATION_LABELS *labels;

	// �V�X�e���̃R�[�h
	char* system_code;
	// ���s�t�@�C���̃p�X
	char* current_path;
	// ����t�@�C���̃p�X
	char* language_file_path;
	// �u���V�t�@�C���̃p�X
	char* brush_file_path;
	// �x�N�g���u���V�t�@�C���̃p�X
	char* vector_brush_file_path;
	// ���ʃc�[���t�@�C���̃p�X
	char* common_tool_file_path;
	// �o�b�N�A�b�v���쐬����f�B���N�g���̃p�X
	char *backup_directory_path;
	// �p�^�[���t�@�C���̂���f�B���N�g���ւ̃p�X
	char* pattern_path;
	// �X�^���v�t�@�C���̂���f�B���N�g���ւ̃p�X
	char* stamp_path;
	// �e�N�X�`���摜�̂���f�B���N�g���ւ̃p�X
	char* texture_path;
	// �X�N���v�g�t�@�C���̂���f�B���N�g���ւ̃p�X
	char* script_path;

	// �X�N���v�g�̃t�@�C�����
	SCRIPTS scripts;

	// �V�K�쐬�Ȃǂ̃��j���[�ň����f�[�^
	MENU_DATA menu_data;

	// ���E���]�̃{�^���ƃ��x��
	GtkWidget *reverse_button, *reverse_label;

	// �I��͈͕ҏW�̃{�^��
	GtkWidget *edit_selection;

	// �t�@�C���I�[�v�������̑��쒆�ɗ��t���O
	unsigned int flags;

	// ����p�̃f�[�^
	GtkPrintSettings* print_settings;
	// �������T�[�t�F�[�X
	cairo_surface_t* print_surface;
	// �������s�N�Z���f�[�^
	uint8* print_pixels;

	// �\�����̃c�[��
	uint16 current_tool;

	// ICC�v���t�@�C���ւ̃p�X
	char *input_icc_path;
	char *output_icc_path;

	// ICC�v���t�@�C���̓��e
	cmsHPROFILE input_icc;
	cmsHPROFILE output_icc;
	// ICC�v���t�@�C���ɂ��F�ϊ��p
	cmsHTRANSFORM icc_transform;

	// �p�^�[���h��ׂ��p
	PATTERNS patterns;
	// �X�^���v�p
	PATTERNS stamps;
	uint8* stamp_buff;
	uint8* stamp_shape;
	uint8* stamp_alpha;
	size_t stamp_buff_size;

#ifndef _WIN32
	// Ubuntu�ł̓c�[���̐؂�ւ��������Ȃ��Ȃ�̂�
		// �f�o�C�X�̐ݒ���L������K�v������?
	int num_device;
	GdkInputSource *input_sources;
	gboolean *set_input_modes;
#endif

#if defined(USE_3D_LAYER) && USE_3D_LAYER != 0
	// 3D���f�����O�p�f�[�^
	void *modeling;
#endif
} APPLICATION;

// �֐��̃v���g�^�C�v�錾
/*********************************************************************
* InitializeApplication�֐�                                          *
* �A�v���P�[�V�����̏�����                                           *
* ����                                                               *
* app				: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
* init_file_path	: �������t�@�C���̃p�X                           *
*********************************************************************/
extern void InitializeApplication(APPLICATION* app, char* init_file_path);

/*****************************************************
* RecoverBackUp�֐�                                  *
* �o�b�N�A�b�v�t�@�C�����������ꍇ�ɕ�������         *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void RecoverBackUp(APPLICATION* app);

extern GtkWidget *CreateToolBoxWindow(struct _APPLICATION* app, GtkWidget *parent);

extern void CreateBrushTable(
	APPLICATION* app,
	TOOL_WINDOW* window,
	BRUSH_CORE brush_data[BRUSH_TABLE_HEIGHT][BRUSH_TABLE_WIDTH]
);

extern void CreateVectorBrushTable(
	APPLICATION* app,
	TOOL_WINDOW* window,
	VECTOR_BRUSH_CORE brush_data[VECTOR_BRUSH_TABLE_HEIGHT][VECTOR_BRUSH_TABLE_WIDTH]
);

#if defined(USE_3D_LAYER) && USE_3D_LAYER != 0
extern void CreateChange3DLayerUI(
	APPLICATION* app,
	TOOL_WINDOW* window
);
#endif

/*********************************************************
* WriteCommonToolData�֐�                                *
* ���ʃc�[���̃f�[�^�������o��                           *
* ����                                                   *
* window	: �c�[���{�b�N�X                             *
* file_path	: �����o���t�@�C���̃p�X                     *
* app		: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void WriteCommonToolData(
	TOOL_WINDOW* window,
	const char* file_path,
	APPLICATION* app
);

/*********************************************************
* WriteVectorBrushData�֐�                               *
* �x�N�g�����C���[�p�̃u���V�e�[�u���̃f�[�^�������o��   *
* ����                                                   *
* window	: �c�[���{�b�N�X�E�B���h�E                   *
* file_path	: �����o���t�@�C���̃p�X                     *
* app		: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void WriteVectorBrushData(
	struct _TOOL_WINDOW* window,
	const char* file_path,
	struct _APPLICATION* app
);

/*******************************************************
* OnQuitApplication�֐�                                *
* �A�v���P�[�V�����I���O�ɌĂяo�����R�[���o�b�N�֐� *
* ����                                                 *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X   *
* �Ԃ�l                                               *
*	�I�����~:TRUE	�I�����s:FALSE                     *
*******************************************************/
extern gboolean OnQuitApplication(APPLICATION* app);

#ifndef INCLUDE_WIN_DEFAULT_API
/*********************************************************
* OpenFile�֐�                                           *
* �󂯂��Ƃ��p�X�̃t�@�C�����J��                         *
* ����                                                   *
* file_path	: �t�@�C���p�X                               *
* app		: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void OpenFile(char *file_path, APPLICATION* app);
#endif

/*********************************************************
* ExecuteOpenFile�֐�                                    *
* �t�@�C�����J��                                         *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void ExecuteOpenFile(APPLICATION* app);

/*********************************************************
* ExecuteOpenFileAsLayer�֐�                             *
* ���C���[�Ƃ��ăt�@�C�����J��                           *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void ExecuteOpenFileAsLayer(APPLICATION* app);

/*********************************************************
* ExecuteSave�֐�                                        *
* �㏑���ۑ�                                             *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void ExecuteSave(APPLICATION* app);

/*********************************************************
* ExecuteSaveAs�֐�                                      *
* ���O��t���ĕۑ�                                       *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void ExecuteSaveAs(APPLICATION* app);

/*********************************************************
* ExecuteClose�֐�                                       *
* �A�N�e�B�u�ȕ`��̈�����                           *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void ExecuteClose(APPLICATION* app);

/*****************************************************
* ExecuteMakeColorLayer�֐�                          *
* �ʏ탌�C���[�쐬�����s                             *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteMakeColorLayer(APPLICATION *app);

/*****************************************************
* ExecuteMakeVectorLayer�֐�                         *
* �V�K�x�N�g�����C���[�쐬�����s                     *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteMakeVectorLayer(APPLICATION *app);

/*****************************************************
* ExecuteMakeLayerSet�֐�                            *
* �V�K���C���[�Z�b�g�쐬�����s                       *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteMakeLayerSet(APPLICATION *app);

#if defined(USE_3D_LAYER) && USE_3D_LAYER != 0
/*****************************************************
* ExecuteMake3DLayer�֐�                             *
* 3D���f�����O���C���[�쐬�����s                     *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteMake3DLayer(APPLICATION* app);
#endif

/*********************************************************
* DeleteActiveLayer�֐�                                  *
* ���݂̃A�N�e�B�u���C���[���폜����                     *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void DeleteActiveLayer(APPLICATION* app);

/*****************************************************
* ExecuteUpLayer�֐�                                 *
* ���C���[�̏�����1��ɕύX����                    *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteUpLayer(APPLICATION* app);

/*****************************************************
* ExecuteDownLayer�֐�                               *
* ���C���[�̏�����1���ɕύX����                    *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteDownLayer(APPLICATION* app);

/*********************************************************
* FillForeGroundColor�֐�                                *
* �`��F�ŃA�N�e�B�u���C���[��h��Ԃ�                 *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void FillForeGroundColor(APPLICATION* app);

/*********************************************************
* FillPattern�֐�                                        *
* �A�N�e�B�u�ȃp�^�[���ŃA�N�e�B�u�ȃ��C���[��h��ׂ�   *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void FillPattern(APPLICATION* app);

/*********************************************************
* FlipImageHorizontally�֐�                              *
* �`��̈�����E���]����                                 *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void FlipImageHorizontally(APPLICATION* app);

/*********************************************************
* FlipImageVertically�֐�                                *
* �`��̈���㉺���]����                                 *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void FlipImageVertically(APPLICATION* app);

/*****************************************************
* SwitchSecondBackColor�֐�                          *
* �w�i�F��2�߂̂��̂Ɠ���ւ���                    *
* ����                                               *
* menu	: ���j���[�E�B�W�F�b�g                       *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void SwitchSecondBackColor(GtkWidget* menu, APPLICATION* app);

/*****************************************************
* Change2ndBackColor�֐�                             *
* 2�߂̔w�i�F��ύX����                            *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void Change2ndBackColor(APPLICATION* app);

/*********************************************************
* MergeDownActiveLayer�֐�                               *
* �A�N�e�B�u�ȃ��C���[�����̃��C���[�ƌ�������           *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void MergeDownActiveLayer(APPLICATION* app);

/*********************************************************
* FlattenImage�֐�                                       *
* �摜�̓��������s                                       *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void FlattenImage(APPLICATION* app);

/*********************************************************
* ActiveLayerAlpha2SelectionArea�֐�                     *
* �A�N�e�B�u���C���[�̕s����������I��͈͂ɂ���         *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void ActiveLayerAlpha2SelectionArea(APPLICATION* app);

/*********************************************************
* ActiveLayerAlphaAddSelectionArea�֐�                   *
* �A�N�e�B�u���C���[�̕s����������I��͈͂ɉ�����       *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void ActiveLayerAlphaAddSelectionArea(APPLICATION* app);

/*********************************************************
* ExecuteCopyLayer�֐�                                   *
* ���C���[�̕��������s����                               *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void ExecuteCopyLayer(APPLICATION* app);

/*********************************************************
* ExecuteVisible2Layer�֐�                               *
* ���������C���[�ɂ���                                 *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void ExecuteVisible2Layer(APPLICATION* app);

/*********************************************************
* RasterizeActiveLayer�֐�                               *
* �A�N�e�B�u���C���[�����X�^���C�Y����                   *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void RasterizeActiveLayer(APPLICATION* app);

/*********************************************************
* ExecuteSelectAll�֐�                                   *
* �S�đI�������s                                         *
* ����                                                   *
* app	: �A�v���P�[�V�����S�̂��Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void ExecuteSelectAll(APPLICATION* app);

/*****************************************************
* ExecuteZoomIn�֐�                                  *
* �g������s                                         *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteZoomIn(APPLICATION *app);

/*****************************************************
* ExecuteZoomReset�֐�                               *
* ���{�\�������s                                     *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteZoomReset(APPLICATION* app);

/*****************************************************
* ExecuteRotateClockwise�֐�                         *
* �\�������v���ɉ�]                               *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteRotateClockwise(APPLICATION* app);

/*****************************************************
* ExecuteRotateCounterClockwise�֐�                  *
* �\���𔽎��v���ɉ�]                             *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteRotateCounterClockwise(APPLICATION* app);

/*****************************************************
* ExecuteRotateReset�֐�                             *
* ��]�p�x�����Z�b�g����                             *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteRotateReset(APPLICATION* app);

/*****************************************************
* ExecuteChangeResolution�֐�                        *
* �L�����o�X�̉𑜓x�ύX�����s                       *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteChangeResolution(APPLICATION* app);

/*****************************************************
* ExecuteZoomOut�֐�                                 *
* �k�������s                                         *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteZoomOut(APPLICATION *app);

/*****************************************************
* ExecuteChangeCanvasSize�֐�                        *
* �L�����o�X�̃T�C�Y�ύX�����s                       *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteChangeCanvasSize(APPLICATION* app);

/*****************************************************
* ExecuteChangeCanvasIccProfile�֐�                  *
* ����                                               *
* menu	: ���j���[�E�B�W�F�b�g                       *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteChangeCanvasIccProfile(GtkWidget* menu, APPLICATION* app);

extern void ExecuteUndo(struct _APPLICATION* app);
extern void ExecuteRedo(struct _APPLICATION* app);

/*********************************************************
* FillLayerPattern�֐�                                   *
* ���C���[���p�^�[���œh��ׂ�                           *
* ����                                                   *
* target	: �h��ׂ����s�����C���[                     *
* patterns	: �p�^�[�����Ǘ�����\���̂̃A�h���X         *
* app		: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
* color		: �p�^�[�����O���[�X�P�[���̂Ƃ��Ɏg���F     *
*********************************************************/
extern void FillLayerPattern(
	struct _LAYER* target,
	struct _PATTERNS* patterns,
	struct _APPLICATION* app,
	uint8 color[3]
);

extern void AddLayerNameChangeHistory(
	APPLICATION* app,
	const gchar* before_name,
	const gchar* after_name
);

/*********************************************************************
* InitializeNavigation�֐�                                           *
* �i�r�Q�[�V�����E�B���h�E��������                                   *
* ����                                                               *
* navigation	: �i�r�Q�[�V�����E�B���h�E���Ǘ�����\���̂̃A�h���X *
* app			: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X         *
* box			: �h�b�L���O����ꍇ�̓{�b�N�X�E�B�W�F�b�g���w��     *
*********************************************************************/
extern void InitializeNavigation(
	NAVIGATION_WINDOW* navigation,
	APPLICATION *app,
	GtkWidget* box
);

extern void InitializePreviewWindow(PREVIEW_WINDOW* preview, APPLICATION* app);

extern void UnSetSelectionArea(APPLICATION* app);

extern void InvertSelectionArea(APPLICATION* app);

/*****************************************************
* ReductSelectionArea�֐�                            *
* �I��͈͂��k������                                 *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ReductSelectionArea(APPLICATION* app);

/*********************************************************
* ChangeEditSelectionMode�֐�                            *
* �I��͈͕ҏW���[�h��؂�ւ���                         *
* ����                                                   *
* menu_item	: ���j���[�A�C�e���E�B�W�F�b�g               *
* app		: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void ChangeEditSelectionMode(GtkWidget* menu_item, APPLICATION* app);

/*************************************************************
* Move2ActiveLayer                                           *
* ���C���[�r���[���A�N�e�B�u�ȃ��C���[�ɃX�N���[��           *
* widget		: �A�N�e�B�u���C���[�̃E�B�W�F�b�g           *
* allocation	: �E�B�W�F�b�g�Ɋ��蓖�Ă�ꂽ�T�C�Y         *
* app			: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*************************************************************/
extern void Move2ActiveLayer(GtkWidget* widget, GdkRectangle * allocation, APPLICATION* app);

extern void TextLayerButtonPressCallBack(
	DRAW_WINDOW* window,
	gdouble x,
	gdouble y,
	GdkEventButton* event
);

extern void TextLayerMotionCallBack(
	DRAW_WINDOW* window,
	gdouble x,
	gdouble y,
	GdkModifierType state
);

extern GtkWidget* CreateTextLayerDetailUI(APPLICATION* app, struct _LAYER* target, TEXT_LAYER* layer);

/*************************************************************
* TextFieldFocusIn�֐�                                       *
* �e�L�X�g���C���[�̕ҏW�E�B�W�F�b�g���t�H�[�J�X���ꂽ����   *
*   �Ăяo�����R�[���o�b�N�֐�                             *
* ����                                                       *
* text_field	: �e�L�X�g���C���[�̕ҏW�E�B�W�F�b�g         *
* focus			: �C�x���g�̏��                             *
* app			: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
* �Ԃ�l                                                     *
*	���FALSE                                                *
*************************************************************/
extern gboolean TextFieldFocusIn(GtkWidget* text_field, GdkEventFocus* focus, APPLICATION* app);

/*******************************************************************
* TextFieldFocusOut�֐�                                            *
* �e�L�X�g���C���[�̕ҏW�E�B�W�F�b�g����t�H�[�J�X���ړ����������� *
*   �Ăяo�����R�[���o�b�N�֐�                                   *
* ����                                                             *
* text_field	: �e�L�X�g���C���[�̕ҏW�E�B�W�F�b�g               *
* focus			: �C�x���g�̏��                                   *
* app			: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X       *
* �Ԃ�l                                                           *
*	���FALSE                                                      *
*******************************************************************/
extern gboolean TextFieldFocusOut(GtkWidget* text_field, GdkEventFocus* focus, APPLICATION* app);

/*************************************************************
* OnDestroyTextField�֐�                                     *
* �e�L�X�g���C���[�̕ҏW�E�B�W�F�b�g���폜����鎞��         *
*   �Ăяo�����R�[���o�b�N�֐�                             *
* ����                                                       *
* text_field	: �e�L�X�g���C���[�̕ҏW�E�B�W�F�b�g         *
* app			: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*************************************************************/
extern void OnDestroyTextField(GtkWidget* text_field, APPLICATION* app);

/*****************************************************
* ExtendSelectionArea�֐�                            *
* �I��͈͂��g�傷��                                 *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExtendSelectionArea(APPLICATION* app);

/*****************************************************
* ExecuteSetPreference�֐�                           *
* ���ݒ��ύX����                                 *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void ExecuteSetPreference(APPLICATION* app);

/*****************************************************
* DisplayVersion�֐�                                 *
* �o�[�W��������\������                           *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void DisplayVersion(APPLICATION* app);

/*************************************************
* ChangeNavigationRotate�֐�                     *
* �i�r�Q�[�V�����̉�]�p�x�ύX���ɌĂяo���֐�   *
* ����                                           *
* navigation	: �i�r�Q�[�V�����E�B���h�E�̏�� *
* draw_window	: �\������`��̈�               *
*************************************************/
extern void ChangeNavigationRotate(NAVIGATION_WINDOW* navigation, DRAW_WINDOW* window);

/*************************************************
* ChangeNavigationDrawWindow�֐�                 *
* �i�r�Q�[�V�����ŕ\������`��̈��ύX����     *
* ����                                           *
* navigation	: �i�r�Q�[�V�����E�B���h�E�̏�� *
* draw_window	: �\������`��̈�               *
*************************************************/
extern void ChangeNavigationDrawWindow(NAVIGATION_WINDOW* navigation, DRAW_WINDOW* window);

/*****************************************************
* NoDisplayFilter�֐�                                *
* �\�����̃t�B���^�[���I�t                           *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void NoDisplayFilter(APPLICATION* app);

/*****************************************************
* GrayScaleDisplayFilter�֐�                         *
* �\�����̃t�B���^�[���O���[�X�P�[���ϊ��̂��̂�     *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void GrayScaleDisplayFilter(APPLICATION* app);

/******************************************************************
* GrayScaleDisplayFilterYIQ�֐�                                   *
* �\�����̃t�B���^�[���O���[�X�P�[���ϊ�(YIQ�J���[���f��)�̂��̂� *
* ����                                                            *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X              *
******************************************************************/
extern void GrayScaleDisplayFilterYIQ(APPLICATION* app);

/*****************************************************
* IccProfileDisplayFilter�֐�                        *
* �\�����̃t�B���^�[��ICC�v���t�@�C���K�p�̂��̂�    *
* ����                                               *
* menu	: ���j���[�E�B�W�F�b�g                       *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
extern void IccProfileDisplayFilter(GtkWidget* menu, APPLICATION* app);

/*********************************************************
* CreateTextureChooser�֐�                               *
* �e�N�X�`����I������E�B�W�F�b�g���쐬����             *
* ����                                                   *
* texture	: �e�N�X�`�����Ǘ�����\���̂̃A�h���X       *
* app		: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
* �Ԃ�l                                                 *
*	�e�N�X�`����I������E�B���h�E�E�B�W�F�b�g           *
*********************************************************/
extern GtkWidget* CreateTextureChooser(TEXTURES* textures, struct _APPLICATION* app);

/***************************************************************
* InitializeScripts�֐�                                        *
* �X�N���v�g�f�B���N�g���ɂ���t�@�C���ꗗ���쐬               *
* ����                                                         *
* scripts		: �X�N���v�g�t�@�C�����Ǘ�����\���̂̃A�h���X *
* scripts_path	: �X�N���v�g�f�B���N�g���̃p�X                 *
***************************************************************/
extern void InitializeScripts(SCRIPTS* scripts, const char* scripts_path);

/*********************************************************
* ExecuteScript�֐�                                      *
* ���j���[����X�N���v�g���I�����ꂽ���̃R�[���o�b�N�֐� *
* ����                                                   *
* menu_item	: ���j���[�A�C�e���E�B�W�F�b�g               *
* app		: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void ExecuteScript(GtkWidget* menu_item, APPLICATION* app);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_APPLICATION_H_