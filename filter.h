#ifndef _INCLUDED_FILTER_H_
#define _INCLUDED_FILTER_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*filter_func)(struct _DRAW_WINDOW* window, struct _LAYER** layers,
							uint16 num_layer, void* data);
typedef void (*selection_filter_func)(struct _DRAW_WINDOW* window, void* data);

// �t�B���^�[�֐��z��̃C���f�b�N�X
typedef enum _eFILTER_FUNC_ID
{
	FILTER_FUNC_BLUR,
	FILTER_FUNC_MOTION_BLUR,
	FILTER_FUNC_BRIGHTNESS_CONTRAST,
	FILTER_FUNC_HUE_SATURATION,
	FILTER_FUNC_LUMINOSITY2OPACITY,
	FILTER_FUNC_COLOR2ALPHA,
	FILTER_FUNC_COLORIZE_WITH_UNDER,
	FILTER_FUNC_GRADATION_MAP,
	FILTER_FUNC_FILL_WITH_VECTOR,
	FILTER_FUNC_PERLIN_NOISE,
	FILTER_FUNC_FRACTAL,
	NUM_FILTER_FUNC
} eFILTER_FUNC_ID;

// �t�B���^�[�֐��|�C���^�z��
extern filter_func g_filter_funcs[NUM_FILTER_FUNC];
extern selection_filter_func g_selection_filter_funcs[NUM_FILTER_FUNC];

/*****************************************************
* ExecuteBlurFilter�֐�                              *
* �ڂ����t�B���^�����s                               *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
EXTERN void ExecuteBlurFilter(APPLICATION* app);

/*********************************************************
* ApplyBlurFilter�֐�                                    *
* �ڂ����t�B���^�[��K�p����                             *
* ����                                                   *
* target	: �ڂ����t�B���^�[��K�p���郌�C���[         *
* size		: �ڂ����t�B���^�[�ŕ��ϐF���v�Z�����`�͈� *
*********************************************************/
EXTERN void ApplyBlurFilter(LAYER* target, int size);

/*****************************************************
* ExecuteMotionBlurFilter�֐�                        *
* ���[�V�����ڂ����t�B���^�����s                     *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
EXTERN void ExecuteMotionBlurFilter(APPLICATION* app);

/*****************************************************
* ExecuteChangeBrightContrastFilter�֐�              *
* ���邳�R���g���X�g�t�B���^�����s                   *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
EXTERN void ExecuteChangeBrightContrastFilter(APPLICATION* app);

/*****************************************************
* ExecuteChangeHueSaturationFilter�֐�               *
* �F���E�ʓx�t�B���^�����s                           *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
EXTERN void ExecuteChangeHueSaturationFilter(APPLICATION* app);

/*****************************************************
* ExecuteLuminosity2OpacityFilter�֐�                *
* �P�x�𓧖��x�֕ϊ������s                           *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
EXTERN void ExecuteLuminosity2OpacityFilter(APPLICATION* app);

/*****************************************************
* ExecuteColor2AlphaFilter�֐�                       *
* �w��F�𓧖��x�֕ϊ������s                         *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
EXTERN void ExecuteColor2AlphaFilter(APPLICATION* app);

/*****************************************************
* ExecuteColorizeWithUnderFilter�֐�                 *
* ���̃��C���[�Œ��F�����s                           *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
EXTERN void ExecuteColorizeWithUnderFilter(APPLICATION* app);

/*****************************************************
* ExecuteGradationMapFilter�֐�                      *
* �O���f�[�V�����}�b�v�����s                         *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
EXTERN void ExecuteGradationMapFilter(APPLICATION* app);

/*****************************************************
* ExecuteFillWithVectorLineFilter�֐�                *
* �x�N�g�����C���[�𗘗p���ĉ��̃��C���[��h��ׂ�   *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
EXTERN void ExecuteFillWithVectorLineFilter(APPLICATION* app);

/*****************************************************
* ExecutePerlinNoiseFilter�֐�                       *
* �p�[�����m�C�Y�ŉ��h������s                       *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
EXTERN void ExecutePerlinNoiseFilter(APPLICATION* app);

/*****************************************************
* ExecuteFractal�֐�                                 *
* �t���N�^���}�`�ŉ��h����s                         *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
EXTERN void ExecuteFractal(APPLICATION* app);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_FILTER_H_
