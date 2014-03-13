#ifndef _INCLUDED_ANTI_ALIAS_H_
#define _INCLUDED_ANTI_ALIAS_H_

#include "types.h"
#include "layer.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************
* ANTI_ALIAS_RECTANGLE�\����   *
* �A���`�G�C���A�X�͈͎̔w��p *
*******************************/
typedef struct _ANTI_ALIAS_RECTANGLE
{
	int x, y;			// �A���`�G�C���A�X�������鍶��̍��W
	int width, height;	// �A���`�G�C���A�X��������͈͂̕��E����
} ANTI_ALIAS_RECTANGLE;

// �֐��̃v���g�^�C�v�錾
/********************************************
* AntiAlias�֐�                             *
* �A���`�G�C���A�V���O���������s            *
* ����                                      *
* in_buff	: ���̓f�[�^                    *
* out_buff	: �o�̓f�[�^                    *
* width		: ���o�̓f�[�^�̕�              *
* height	: ���o�̓f�[�^�̍���            *
* stride	: ���o�̓f�[�^��1�s���̃o�C�g�� *
* channel	: ���o�̓f�[�^�̃`�����l����    *
********************************************/
extern void AntiAlias(
	uint8* in_buff,
	uint8* out_buff,
	int width,
	int height,
	int stride,
	int channel
);

/*********************************************************
* AntiAliasLayer�֐�                                     *
* ���C���[�ɑ΂��Ĕ͈͂��w�肵�ăA���`�G�C���A�X�������� *
* layer	: �A���`�G�C���A�X�������郌�C���[               *
* rect	: �A���`�G�C���A�X��������͈�                   *
*********************************************************/
extern void AntiAliasLayer(LAYER *layer, LAYER* temp, ANTI_ALIAS_RECTANGLE *rect);

/*********************************************************************
* AntiAliasVectorLine�֐�                                            *
* �x�N�g�����C���[�̐��ɑ΂��Ĕ͈͂��w�肵�ăA���`�G�C���A�X�������� *
* layer	: �A���`�G�C���A�X�������郌�C���[                           *
* rect	: �A���`�G�C���A�X��������͈�                               *
*********************************************************************/
extern void AntiAliasVectorLine(LAYER *layer, LAYER* temp, ANTI_ALIAS_RECTANGLE *rect);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_ANTI_ALIAS_H_
