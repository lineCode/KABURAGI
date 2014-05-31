#ifndef _INCLUDED_COLOR_H_
#define _INCLUDED_COLOR_H_

#include "lcms/lcms2.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PALLETE_BOX_SIZE 10
#define MAX_COLOR_HISTORY_NUM 10

typedef struct _HSV
{
	int16 h;
	uint8 s;
	uint8 v;
} HSV;

typedef struct _CMYK
{
	uint8 c;
	uint8 m;
	uint8 y;
	uint8 k;
} CMYK;

typedef struct _CMYK16
{
	uint16 c;
	uint16 m;
	uint16 y;
	uint16 k;
} CMYK16;

typedef enum _eCOLOR_CHOOSER_FLAGS
{
	COLOR_CHOOSER_MOVING_CIRCLE = 0x01,
	COLOR_CHOOSER_MOVING_SQUARE = 0x02,
	COLOR_CHOOSER_SHOW_CIRCLE = 0x04,
	COLOR_CHOOSER_SHOW_PALLETE = 0x08
} eCOLOR_CHOOSER_FLAGS;

typedef struct _COLOR_CHOOSER
{
	GtkWidget* widget, *choose_area, *color_box, *pallete;
	GtkWidget *chooser_box;
	GtkWidget *pallete_widget;
	GtkWidget *circle_button, *pallete_button;
	cairo_surface_t *color_box_surface;
	uint8 *color_box_pixel_data;
	cairo_surface_t *circle_surface;
	uint8 *color_circle_data;
	uint8 *chooser_pixel_data;
	cairo_surface_t *chooser_surface;
	cairo_t *chooser_cairo;
	uint8 *pallete_pixels;
	cairo_surface_t *pallete_surface;
	void (*filter_func)(
		uint8* source, uint8* destination, int size, void* filter_data);
	void *filter_data;
	int32 widget_width;
	int32 widget_height;
	HSV hsv, back_hsv;
	uint8 rgb[3], back_rgb[3];
	uint8 color_history[MAX_COLOR_HISTORY_NUM][3];
	uint8 (*pallete_data)[3];
	uint8 *pallete_flags;
	uint8 line_width;
	uint8 flags;
	uint8 num_color_history;
	uint16 selected_pallete;
	FLOAT_T sv_x, sv_y;
	void (*color_change_callback)(
		GtkWidget* widget, const uint8 color[3], void* data);
	void* data;
} COLOR_CHOOSER;

extern void RGB2HSV_Pixel(uint8 rgb[3], HSV* hsv);

extern HSV* RGB2HSV(
	uint8 *pixels,
	int32 width,
	int32 height,
	int32 channel
);

extern void HSV2RGB_Pixel(HSV* hsv, uint8 rgb[3]);

extern void HSV2RGB(
	HSV* hsv,
	uint8 *pixels,
	int32 width,
	int32 height,
	int32 channel
);

extern void DrawColorCircle(
	COLOR_CHOOSER* chooser,
	FLOAT_T cx,
	FLOAT_T cy,
	FLOAT_T r
);

extern COLOR_CHOOSER *CreateColorChooser(
	int32 width,
	int32 height,
	uint8 line_width,
	int32 start_rad,
	uint8 (*pallete)[3],
	uint8 *pallete_use,
	const gchar* base_path,
	APPLICATION_LABELS* labels
);

extern void SetColorChooserPoint(COLOR_CHOOSER* chooser, HSV* set_hsv, gboolean add_history);

extern void UpdateColorBox(COLOR_CHOOSER* chooser);

extern void DestroyColorChooser(COLOR_CHOOSER* chooser);

/*********************************************************
* SetColorChangeCallBack�֐�                             *
* �F���ύX���ꂽ�Ƃ��ɌĂяo�����R�[���o�b�N�֐���ݒ� *
* ����                                                   *
* chooser	: �F�I��p�̃f�[�^                           *
* function	: �R�[���o�b�N�֐�                           *
* data		: �R�[���o�b�N�֐��Ŏg�p����f�[�^           *
*********************************************************/
extern void SetColorChangeCallBack(
	COLOR_CHOOSER* chooser,
	void (*function)(GtkWidget* widget, const uint8 color[3], void* data),
	void *data
);

/*************************************************
* ReadACO�֐�                                    *
* Adobe Color File��ǂݍ���                     *
* ����                                           *
* src			: �f�[�^�ւ̃|�C���^             *
* read_func		: �ǂݍ��݂Ɏg�p����֐��|�C���^ *
* rgb			: �ǂݍ���RGB�f�[�^�̊i�[��    *
* buffer_size	: �ő�ǂݍ��ݐ�                 *
* �Ԃ�l                                         *
*	�ǂݍ��݂ɐ��������f�[�^�̐�                 *
*************************************************/
extern int ReadACO(
	void* src,
	stream_func_t read_func,
	uint8 (*rgb)[3],
	int buffer_size
);

/***********************************************
* WriteACO�t�@�C��                             *
* Adobe Color File�ɏ����o��                   *
* ����                                         *
* dst			: �����o����ւ̃|�C���^       *
* write_func	: �����o���p�̊֐��ւ̃|�C���^ *
* rgb			: ��������RGB�f�[�^�z��        *
* write_num		: ���������f�[�^�̐�           *
***********************************************/
extern void WriteACO(
	void* dst,
	stream_func_t write_func,
	uint8 (*rgb)[3],
	int write_num
);

/***************************************
* LoadPallete�֐�                      *
* �p���b�g�f�[�^��ǂݍ���             *
* ����                                 *
* file_path	: �f�[�^�t�@�C���̃p�X     *
* rgb		: �ǂݍ��񂾃f�[�^�̊i�[�� *
* max_read	: �ǂݍ��݂��s���ő吔     *
* �Ԃ�l                               *
*	�ǂݍ��񂾃f�[�^�̐�               *
***************************************/
extern int LoadPallete(
	const char* file_path,
	uint8 (*rgb)[3],
	int max_read
);

/***********************************
* LoadPalleteAdd�֐�               *
* �p���b�g�f�[�^��ǉ��ǂݍ���     *
* ����                             *
* chooser	: �F�I���̏��         *
* file_path	: �f�[�^�t�@�C���̃p�X *
* �Ԃ�l                           *
*	�ǂݍ��񂾃f�[�^�̐�           *
***********************************/
extern int LoadPalleteAdd(
	COLOR_CHOOSER* chooser,
	const gchar* file_path
);

/*************************************
* LoadPalleteFile�֐�                *
* �p���b�g�̏���ǂݍ���           *
* chooser	: �F�I��p�̏��         *
* file_path	: �ǂݍ��ރt�@�C���̃p�X *
* �Ԃ�l                             *
*	����I��:0	���s:���̒l          *
*************************************/
extern int LoadPalleteFile(COLOR_CHOOSER* chooser, const gchar* file_path);

/*************************************
* WritePalleteFile�֐�               *
* �p���b�g�̏�����������           *
* chooser	: �F�I��p�̏��         *
* file_path	: �������ރt�@�C���̃p�X *
* �Ԃ�l                             *
*	����I��:0	���s:���̒l          *
*************************************/
extern int WritePalleteFile(COLOR_CHOOSER* chooser, const gchar* file_path);

/***************************************
* RegisterColorPallete�֐�             *
* �p���b�g�ɐF��ǉ�����               *
* ����                                 *
* chooser	: �F�I���E�B�W�F�b�g�̏�� *
* color		: �ǉ�����F               *
***************************************/
extern void RegisterColorPallete(COLOR_CHOOSER* chooser, const uint8 color[3]);

extern cmsHPROFILE* CreateDefaultSrgbProfile(void);
extern cmsHPROFILE* GetPrimaryMonitorProfile(void);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_COLOR_H_
