#ifndef _INCLUDED_SCRIPT_H_
#define _INCLUDED_SCRIPT_H_

#include <gtk/gtk.h>
#include "application.h"
#include "lua/lua.h"
#include "memory_stream.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _eSCRIPT_BRUSH_FLAGS
{
	SCRIPT_BRUSH_STARTED = 0x01
} eSCRIPT_BRUSH_FLAGS;

typedef struct _SCRIPT_BRUSH
{
	BRUSH_CORE *core;
	unsigned int flags;
	cairo_surface_t *brush_surface;
	cairo_surface_t *alpha_surface;
	char *image_path;
	uint8 *pixels;
	uint8 *alpha;
	uint16 blend_mode;
	uint8 channel;
	uint8 colorize_mode;
	FLOAT_T r;
	int width, height;
	int stride;
	FLOAT_T before_x, before_y;
	FLOAT_T zoom;
	FLOAT_T distance;
	FLOAT_T arg;
	cairo_surface_t *cursor_surface;
	uint8 *cursor_pixels;
} SCRIPT_BRUSH;

typedef struct _SCRIPT
{
	APPLICATION *app;
	SCRIPT_BRUSH *brush_data;
	lua_State *state;
	MEMORY_STREAM_PTR before_data;
	GtkWidget *main_window;
	GtkWidget *debug_window;
	GtkWidget *debug_view;
	GtkTextBuffer *debug_buffer;
	LAYER *work[128];
	char *file_name;
	int num_work;
} SCRIPT;

// �֐��̃v���g�^�C�v�錾
/*********************************************************
* CreateScript�֐�                                       *
* Lua�X�N���v�g���s�p�̃f�[�^�쐬                        *
* ����                                                   *
* app		: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
* file_path	: �X�N���v�g�t�@�C���̃p�X                   *
* �Ԃ�l                                                 *
*	���������ꂽ�\���̂̃A�h���X                         *
*********************************************************/
extern SCRIPT* CreateScript(APPLICATION* app, const char* file_path);

/***********************************************
* DeleteScript�֐�                             *
* Lua�X�N���v�g���s�p�̃f�[�^���폜            *
* ����                                         *
* script	: �폜����f�[�^�|�C���^�̃A�h���X *
***********************************************/
extern void DeleteScript(SCRIPT** script);

/*************************************************
* RunScript�֐�                                  *
* �X�N���v�g�����s����                           *
* ����                                           *
* script	: �X�N���v�g���s�p�̍\���̂̃A�h���X *
*************************************************/
extern void RunScript(SCRIPT* script);

/*************************************************
* ScriptBrushPatternNew�֐�                      *
* �X�N���v�g�u���V�̃p�^�[�����쐬����           *
* ����                                           *
* script	: �X�N���v�g���s�p�̍\���̂̃A�h���X *
*************************************************/
extern void ScriptBrushPatternNew(SCRIPT* script);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SCRIPT_H_
