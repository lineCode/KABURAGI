#ifndef _INCLUDED_SPIN_SCALE_H_
#define _INCLUDED_SPIN_SCALE_H_

#include <gtk/gtk.h>

#ifdef _MSC_VER
# ifdef __cplusplus
#  define EXTERN extern "C" __declspec(dllexport)
# else
#  define EXTERN extern __declspec(dllexport)
# endif
#else
# define EXTERN extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
* SPIN_SCALE�\����                                 *
* �X�s���{�^���ƃX�P�[���̍����E�B�W�F�b�g�̃f�[�^ *
***************************************************/
typedef struct _SPIN_SCALE
{
	GtkSpinButton parent_instance;
} SPIN_SCALE;

// �֐��̃v���g�^�C�v�錾
/*****************************************************
* SpinScaleNew�֐�                                   *
* �X�s���{�^���ƃX�P�[���o�[�̍����E�B�W�F�b�g�𐶐� *
* ����                                               *
* adjustment	: �X�P�[���Ɏg�p����A�W���X�^       *
* label			: �X�P�[���̃��x��                   *
* digits		: �\�����錅��                       *
* �Ԃ�l                                             *
*	���������E�B�W�F�b�g                             *
*****************************************************/
EXTERN GtkWidget* SpinScaleNew(
	GtkAdjustment *adjustment,
	const gchar *label,
	gint digits
);

/***************************************************
* SpinScaleSetScaleLimits�֐�                      *
* �X�P�[���̏㉺���l��ݒ肷��                     *
* ����                                             *
* scale	: �X�s���{�^���ƃX�P�[���̍����E�B�W�F�b�g *
* lower	: �����l                                   *
* upper	: ����l                                   *
***************************************************/
EXTERN void SpinScaleSetScaleLimits(
	SPIN_SCALE *scale,
	gdouble lower,
	gdouble upper
);

/*****************************************************
* SpinScaleGetScaleLimits�֐�                        *
* �㉺���l���擾����                                 *
* ����                                               *
* scale	: �X�s���{�^���ƃX�P�[���̍����E�B�W�F�b�g   *
* lower	: �����l���i�[����{���x���������_�̃A�h���X *
* upper	: ����l���i�[����{���x���������_�̃A�h���X *
* �Ԃ�l                                             *
*	�擾����:TRUE	�擾���s:FALSE                   *
*****************************************************/
EXTERN gboolean SpinScaleGetScaleLimits(
	SPIN_SCALE *scale,
	gdouble *lower,
	gdouble *upper
);

/***************************************************
* SpinScaleSetStep�֐�                             *
* ���L�[�ł̑����̒l��ݒ肷��                   *
* ����                                             *
* scale	: �X�s���{�^���ƃX�P�[���̍����E�B�W�F�b�g *
* step	: �����̒l                                 *
***************************************************/
EXTERN void SpinScaleSetStep(SPIN_SCALE *scale, gdouble step);

/***************************************************
* SpinScaleSetPase�֐�                             *
* Pase Up/Down�L�[�ł̑����̒l��ݒ肷��           *
* ����                                             *
* scale	: �X�s���{�^���ƃX�P�[���̍����E�B�W�F�b�g *
* page	: �����̒l                                 *
***************************************************/
EXTERN void SpinScaleSetPage(SPIN_SCALE *scale, gdouble page);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SPIN_SCALE_H_
