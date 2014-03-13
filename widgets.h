#ifndef _INCLUDED_WIDGETS_H_
#define _INCLUDED_WIDGETS_H_

#include <gtk/gtk.h>
#include "configure.h"
#include "types.h"
#include "spin_scale.h"
#include "cell_renderer_widget.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _IMAGE_CHECK_BUTTON
{
	GtkWidget* image;
	GtkWidget* widget;
	GdkPixbuf* pixbuf;
	void* data;
	gint (*func)(struct _IMAGE_CHECK_BUTTON *button, void* data);
	int8 state;
} IMAGE_CHECK_BUTTON;

// �֐��̃v���g�^�C�v�錾
extern IMAGE_CHECK_BUTTON* CreateImageCheckButton(
	GdkPixbuf* pixbuf,
	gint (*func)(IMAGE_CHECK_BUTTON* button, void* data),
	void *data
);

extern void ImageCheckButtonSetState(IMAGE_CHECK_BUTTON* button, int8 state);

extern GtkWidget* CreateNotebookLabel(
	const gchar* text,
	GtkWidget* note_book,
	gint page,
	gboolean (*destroy_func)(void* data, gint page),
	gpointer data
);

extern GtkWidget *CreateImageButton(
	const char* image_file_path,
	const gchar* label,
	const char* font_file
);

typedef enum _ICC_PROFILE_USAGE
{
	PROFILE_USAGE_GRAYSCALE = 1,
	PROFILE_USAGE_RGB = 2,
	PROFILE_USAGE_CMYK = 4,
	PROFILE_USAGE_ANY = 7,
	PROFILE_USAGE_DOCUMENT = 8,
	PROFILE_USAGE_GRAYSCALE_DOCUMENT = 9,
	PROFILE_USAGE_RGB_DOCUMENT = 10,
	PROFILE_USAGE_CMYK_DOCUMENT = 12,
	PROFILE_USAGE_PROOF_TARGET = 16,
	PROFILE_USAGE_GRAYSCALE_TARGET = 17,
	PROFILE_USAGE_RGB_TARGET = 18,
	PROFILE_USAGE_CMYK_TARGET = 20,
} ICC_PROFILE_USAGE;

/******************************************************************************
* IccProfileChooserDialogNew�֐�                                              *
* ICC�v���t�@�C����I������_�C�A���O�E�B�W�F�b�g���쐬                       *
* ����                                                                        *
* usage				: �ݒ�ł���ICC�v���t�@�C���̎�� ICC_PROFILE_USAGE���Q�� *
* set_profile_path	: �_�C�A���O�̏����l�Ƃ��Đݒ肷��ICC�v���t�@�C���̃p�X   *
* �Ԃ�l                                                                      *
*	�_�C�A���O�����p�̃E�B�W�F�b�g                                            *
******************************************************************************/
extern GtkWidget* IccProfileChooser(char** icc_path, ICC_PROFILE_USAGE usage);

/**************************************************************************
* IccProfileChooserDialogNew�֐�                                          *
* ICC�v���t�@�C����I������_�C�A���O�E�B�W�F�b�g���쐬                   *
* ����                                                                    *
* usage			: �ݒ�ł���ICC�v���t�@�C���̎�� ICC_PROFILE_USAGE���Q�� *
* profile_path	: �����l�y�ѐݒ肳�ꂽICC�v���t�@�C���̃p�X               *
* �Ԃ�l                                                                  *
*	�_�C�A���O�����p�̃E�B�W�F�b�g                                        *
**************************************************************************/
extern GtkWidget* IccProfileChooserDialogNew(int usage, char** profile_path);

extern void IccProfileChooserDialogRun(GtkWidget* dialog);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_WIDGETS_H_
