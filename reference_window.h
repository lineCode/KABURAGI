#ifndef _INCLUDED_REFERENCE_WINDOW_H_
#define _INCLUDED_REFERENCE_WINDOW_H_

#include <gtk/gtk.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _REFERENCE_IMAGE
{
	uint8 *pixels;
	int width, height, stride;
	uint8 channel;
	cairo_surface_t *surface_p;
	gdouble zoom;
	gdouble rev_zoom;
	GtkWidget *draw_area;
	GtkWidget *scroll;
	struct _APPLICATION *app;
} REFERENCE_IMAGE;

typedef struct _REFERENCE_WINDOW_DATA
{
	GtkWidget *window;
	GtkWidget *note_book;
	GtkWidget *scale;
	GtkAdjustment *scale_adjust;
	int16 num_image;
	int16 active_image;
	REFERENCE_IMAGE *images[32];
} REFERENCE_WINDOW_DATA;

typedef struct _REFERENCE_WINDOW
{
	gint window_x, window_y;		// �E�B���h�E�̍��W
	gint window_width;				// �E�B���h�E�̕�
	gint window_height;				// �E�B���h�E�̍���
	GtkWidget *menu_item;			// �\��/��\����؂�ւ��郁�j���[�A�C�e��
	REFERENCE_WINDOW_DATA *data;	// �\���摜�̃f�[�^
	struct _APPLICATION *app;		// �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X
} REFERENCE_WINDOW;

// �֐��̃v���g�^�C�v�錾
/*********************************************************
* InitializeReferenceWindow�֐�                          *
* �Q�l�p�摜�\���E�B���h�E�̏�����                       *
* ����                                                   *
* reference	: �Q�l�p�摜�\���E�B���h�E�̏��             *
* app		: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*********************************************************/
extern void InitializeReferenceWindow(REFERENCE_WINDOW* reference, struct _APPLICATION* app);

/***************************************************************
* DisplayReferenceWindowMenuActivate�֐�                       *
* �Q�l�p�摜�\���E�B���h�E��\�����郁�j���[�̃R�[���o�b�N�֐� *
* ����                                                         *
* menu_item	: ���j���[�A�C�e���E�B�W�F�b�g                     *
* app		: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X       *
***************************************************************/
extern void DisplayReferenceWindowMenuActivate(GtkWidget* menu_item, struct _APPLICATION* app);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_REFERENCE_WINDOW_H_
