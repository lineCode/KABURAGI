#ifndef _INCLUDED_PREFERENCE_H_
#define _INCLUDED_PREFERENCE_H_

#include "ini_file.h"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************
* PREFERENCE�\���� *
* ���ݒ���i�[   *
*******************/
typedef struct _PREFERENCE
{
	// ���ݐݒ蒆�̍���
	int16 current_setting;
	// ZIP���k��
	int8 compress;
	// �����ۑ��̗L��
	int8 auto_save;
	// �e�[�}�t�@�C����
	char *theme;
	// �����ۑ��̊Ԋu
	int32 auto_save_time;
	// �L�����o�X�̔w�i�F
	uint8 canvas_back_ground[3];
} PREFERENCE;

// �֐��̃v���g�^�C�v�錾
/*************************************************************
* ReadPreference�֐�                                         *
* ���ݒ��ǂݍ���                                         *
* ����                                                       *
* file			: �������t�@�C���ǂݍ��ݗp�̍\���̂̃A�h���X *
* preference	: ���ݒ���Ǘ�����\���̂̃A�h���X         *
*************************************************************/
extern void ReadPreference(INI_FILE_PTR file, PREFERENCE* preference);

/*************************************************************
* WritePreference�֐�                                        *
* ���ݒ����������                                         *
* ����                                                       *
* file			: �������t�@�C���������ݗp�̍\���̂̃A�h���X *
* preference	: ���ݒ���Ǘ�����\���̂̃A�h���X         *
*************************************************************/
extern void WritePreference(INI_FILE_PTR file, PREFERENCE* preference);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_PREFERENCE_H_
