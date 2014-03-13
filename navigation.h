#ifndef _INCLUDED_NAVIGATION_H_
#define _INCLUDED_NAVIGATION_H_

#include <gtk/gtk.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************
* NAVIGATION�\����               *
* �i�r�Q�[�V�����E�B���h�E�̏�� *
*********************************/
typedef struct _NAVIGATION_WINDOW
{
	GtkWidget *window;				// �i�r�Q�[�V�����E�B���h�E
	GtkWidget *draw_area;			// �摜�̕\���̈�
	GtkWidget *vbox;				// �i�r�Q�[�V�����̃E�B�W�F�b�g������{�b�N�X
	int window_x, window_y;			// �E�B���h�E�̍��W
	int window_width;				// �E�B���h�E�̕�
	int window_height;				// �E�B���h�E�̍���
	cairo_pattern_t *pattern;		// �\���p�p�^�[��
	uint8* pixels;					// �\������摜�̃s�N�Z���f�[�^]
	uint8* reverse_buff;			// ���E���]�p�̃o�b�t�@
	int width;						// �\���摜�̕�
	int height;						// �\���摜�̍���
	int draw_width;					// �`��̈�̕�
	int draw_height;				// �`��̈�̍���
	// �i�r�Q�[�V�����̍��W�֌W
	struct
	{
		gdouble x, y;				// �i�r�Q�[�V�����̎w�肵�Ă�����W
		gdouble ratio_x;			// �\��X���W�␳�p
		gdouble ratio_y;			// �\��Y���W�␳�p
		gdouble width, height;		// �i�r�Q�[�V�����̎w�肵�Ă��镝�E����
		gdouble angle;				// �i�r�Q�[�V�����̉�]�p�x
		uint16 zoom;				// �i�r�Q�[�V�����̊g��k����
	} point;

	GtkAdjustment *zoom_slider;		// �g��k�����𑀍삷��X���C�_�̃A�W���X�^
	GtkAdjustment *rotate_slider;	// ��]�p�x�𑀍삷��X���C�_�̃A�W���X�^

	// �E�B���h�E�폜���̖������E�B�W�F�b�g�w��p
	uint16 zoom_index, rotate_index;
} NAVIGATION_WINDOW;

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_NAVIGATION_H_
