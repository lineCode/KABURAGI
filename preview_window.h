#ifndef _INCLUDED_PREVIEW_WINDOW_H_
#define _INCLUDED_PREVIEW_WINDOW_H_

#include <gtk/gtk.h>
#include "draw_window.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _PREVIEW_WINDOW
{
	GtkWidget *window;				// �E�B���h�E
	GtkWidget *menu_item;			// �\��/��\����؂�ւ��郁�j���[�E�B�W�F�b�g
	GtkWidget *image;				// �v���r���[�ɕ\������C���[�W
	int window_x, window_y;			// �E�B���h�E�̍��W
	int window_width;				// �E�B���h�E�̕�
	int window_height;				// �E�B���h�E�̍���
	cairo_t *cairo_p;				// �\���pCAIRO���
	cairo_surface_t *surface_p;		// �\���pCAIRO�T�[�t�F�[�X
	int width, height;				// �E�B���h�E�̕��ƍ���
	int draw_width, draw_height;	// �\������C���[�W�̕��ƍ���
	uint8* pixels;					// �\������C���[�W�̃s�N�Z���f�[�^
	uint8* reverse_buff;			// ���E���]�p�̃o�b�t�@
	gdouble zoom, rev_zoom;			// �v���r���[����C���[�W�̊g�嗦
	DRAW_WINDOW* draw;				// �v���r���[����`��̈�
} PREVIEW_WINDOW;

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_PREVIEW_WINDOW_H_
