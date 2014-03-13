#ifndef _INCLUDED_MENU_H_
#define _INCLUDED_MENU_H_

#include <gtk/gtk.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern GtkWidget* GetMainMenu(
	APPLICATION* app,
	GtkWidget* window,
	const char* path
);

/*************************************************
* ParentItemSelected�֐�                         *
* �e���j���[�A�C�e�����I�����ꂽ���ɌĂяo����� *
* ����                                           *
* widget	: �e���j���[�A�C�e��                 *
* data		: �_�~�[                             *
*************************************************/
extern void ParentItemSelected(GtkWidget* widget, gpointer* data);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_MENU_H_
