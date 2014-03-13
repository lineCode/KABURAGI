#ifndef _INCLUDED_DISPLAY_FILTER_H_
#define _INCLUDED_DISPLAY_FILTER_H_

typedef enum _eDISPLAY_FUNC_TYPE
{
	DISPLAY_FUNC_TYPE_NO_CONVERT,
	DISPLAY_FUNC_TYPE_GRAY_SCALE,
	DISPLAY_FUNC_TYPE_GRAY_SCALE_YIQ,
	DISPLAY_FUNC_TYPE_ICC_PROFILE,
	NUM_DISPLAY_FUNC_TYPE
} eDISPLAY_FUNC_TYPE;

extern void (*g_display_filter_funcs[])(uint8* source, uint8* destination, int num_pixel, void* filter_data);

/***********************************
* DISPLAY_FILTER�\����             *
* �\�����ɓK�p����t�B���^�[�̏�� *
***********************************/
typedef struct _DISPLAY_FILTER
{
	void (*filter_func)(
		uint8* source, uint8* destination, int size, void* filter_data);
	void *filter_data;
} DISPLAY_FILTER;

// �֐��̃v���g�^�C�v�錾
/*********************************************
* RGB2GrayScaleFilter�֐�                    *
* RGB����O���[�X�P�[���֕ϊ�����            *
* ����                                       *
* source		:                            *
* destination	: �ϊ���̃f�[�^�i�[��       *
* num_pixel		: �s�N�Z����                 *
* filter_data	: �t�B���^�[�Ŏg�p����f�[�^ *
*********************************************/
extern void RGB2GrayScaleFilter(uint8* source, uint8* destination, int num_pixel, void* filter_data);

/*********************************************
* RGBA2GrayScaleFilter�֐�                   *
* RGBA����O���[�X�P�[���֕ϊ�����           *
* ����                                       *
* source		:                            *
* destination	: �ϊ���̃f�[�^�i�[��       *
* num_pixel		: �s�N�Z����                 *
* filter_data	: �t�B���^�[�Ŏg�p����f�[�^ *
*********************************************/
extern void RGBA2GrayScaleFilter(uint8* source, uint8* destination, int num_pixel, void* filter_data);

#endif	// #ifndef _INCLUDED_DISPLAY_FILTER_H_
