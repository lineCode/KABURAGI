#ifndef _INCLUDED_TEXT_LAYER_H_
#define _INCLUDED_TEXT_LAYER_H_

#include <gtk/gtk.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _eTEXT_STYLE
{
	TEXT_STYLE_NORMAL,
	TEXT_STYLE_ITALIC,
	TEXT_STYLE_OBLIQUE
} eTEXT_STYLE;

/*********************************
* eTEXT_LAYER_FLAGS�񋓑�        *
* �e�L�X�g���C���[�̕\���p�t���O *
*********************************/
typedef enum _eTEXT_LAYER_FLAGS
{
	TEXT_LAYER_VERTICAL = 0x01,	// �c����
	TEXT_LAYER_BOLD = 0x02,		// ����
	TEXT_LAYER_ITALIC = 0x04,	// �C�^���b�N��
	TEXT_LAYER_OBLIQUE = 0x08	// �Α�
} eTEXT_LAYER_FLAGS;

/*******************************
* TEXT_LAYER�\����             *
* �e�L�X�g���C���[�̏����i�[ *
*******************************/
typedef struct _TEXT_LAYER
{
	GtkWidget* text_field;	// �e�L�X�g�r���[�E�B�W�F�b�g
	GtkTextBuffer* buffer;	// �`��e�L�X�g�o�b�t�@
	char* text;				// �o�b�t�@������o�����e�L�X�g���
	gdouble x, y;			// �e�L�X�g�\���̈捶��̍��W
	gdouble width, height;	// �e�L�X�g�\���̈�̕��A����
	// �h���b�O�̊J�n���W
	gdouble drag_start_x, drag_start_y;
	int base_size;			// �t�H���g�T�C�Y�̔{��
	gdouble font_size;		// �\���e�L�X�g�̃T�C�Y
	int32 font_id;			// �t�H���g�t�@�C������ID
	uint8 color[3];			// �\�������F
	uint32 flags;			// �c�����A�������̃t���O
} TEXT_LAYER;

/*****************************************
* TEXT_LAYER_BASE_DATA�\����             *
* �e�L�X�g���C���[�̊�{���(�����o���p) *
*****************************************/
typedef struct _TEXT_LAYER_BASE_DATA
{
	gdouble x, y;			// �e�L�X�g�\���̈捶��̍��W
	gdouble width, height;	// �e�L�X�g�\���̈�̕��A����
	gdouble font_size;		// �\���e�L�X�g�̃T�C�Y
	uint8 base_size;		// �����T�C�Y�̔{��
	uint8 color[3];			// �\�������F
	guint32 flags;			// �c�����A�������̃t���O
} TEXT_LAYER_BASE_DATA;

/*********************************************
* CreateTextLayer�֐�                        *
* �e�L�X�g���C���[�̃f�[�^�������m�ۂƏ����� *
* ����                                       *
* x			: ���C���[��X���W                *
* y			: ���C���[��Y���W                *
* width		: ���C���[�̕�                   *
* height	: ���C���[�̍���                 *
* base_size	: �����T�C�Y�̔{��               *
* font_size	: �����T�C�Y                     *
* color		: �����F                         *
* flags		: �e�L�X�g�\���̃t���O           *
* �Ԃ�l                                     *
*	���������ꂽ�\���̂̃A�h���X             *
*********************************************/
extern TEXT_LAYER* CreateTextLayer(
	gdouble x,
	gdouble y,
	gdouble width,
	gdouble height,
	int base_size,
	gdouble font_size,
	int32 font_id,
	uint8 color[3],
	uint32 flags
);

/*****************************************************
* DeleteTextLayer�֐�                                *
* �e�L�X�g���C���[�̃f�[�^���폜����                 *
* ����                                               *
* layer	: �e�L�X�g���C���[�̃f�[�^�|�C���^�̃A�h���X *
*****************************************************/
extern void DeleteTextLayer(TEXT_LAYER** layer);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TEXT_LAYER_H_
