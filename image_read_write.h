#ifndef _INCLDUED_IMAGE_READ_WRITE_H_
#define _INCLDUED_IMAGE_READ_WRITE_H_

#include <stdio.h>
#include <gtk/gtk.h>
#include "application.h"
#include "memory_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _eIMAGE_RW_FLAGS
{
	IMAGE_COMPRESS_PNG = 0x01	// PNG���k
} eIMAGE_RW_FRAGS;

typedef enum _eIMAGE_DATA_TYPE
{
	IMAGE_DATA_RGB,
	IMAGE_DATA_RGBA,
	IMAGE_DATA_CMYK
} eIMAGE_DATA_TYPE;

// �֐��̃v���g�^�C�v�錾
/***********************************************************
* ReadPNGStream�֐�                                        *
* PNG�C���[�W�f�[�^��ǂݍ���                              *
* ����                                                     *
* stream	: �f�[�^�X�g���[��                             *
* read_func	: �ǂݍ��ݗp�̊֐��|�C���^                     *
* width		: �C���[�W�̕����i�[����A�h���X               *
* height	: �C���[�W�̍������i�[����A�h���X             *
* stride	: �C���[�W�̈�s���̃o�C�g�����i�[����A�h���X *
* �Ԃ�l                                                   *
*	�s�N�Z���f�[�^                                         *
***********************************************************/
extern uint8* ReadPNGStream(
	void* stream,
	stream_func read_func,
	gint32* width,
	gint32* height,
	gint32* stride
);

/**************************************************************************
* ReadPNGHeader�֐�                                                       *
* PNG�C���[�W�̃w�b�_����ǂݍ���                                       *
* ����                                                                    *
* stream	: �f�[�^�X�g���[��                                            *
* read_func	: �ǂݍ��ݗp�̊֐��|�C���^                                    *
* width		: �C���[�W�̕����i�[����A�h���X                              *
* height	: �C���[�W�̍������i�[����A�h���X                            *
* stride	: �C���[�W�̈�s���̃o�C�g�����i�[����A�h���X                *
* dpi		: �C���[�W�̉𑜓x(DPI)���i�[����A�h���X                     *
* icc_profile_name	: ICC�v���t�@�C���̖��O���󂯂�|�C���^               *
* icc_profile_data	: ICC�v���t�@�C���̃f�[�^���󂯂�|�C���^             *
* icc_profile_size	: ICC�v���t�@�C���̃f�[�^�̃o�C�g�����i�[����A�h���X *
**************************************************************************/
extern void ReadPNGHeader(
	void* stream,
	stream_func read_func,
	gint32* width,
	gint32* height,
	gint32* stride,
	int32* dpi,
	char** icc_profile_name,
	uint8** icc_profile_data,
	uint32* icc_profile_size
);

/**************************************************************************
* ReadPNGDetailData�֐�                                                   *
* PNG�C���[�W�f�[�^���ڍ׏��t���œǂݍ���                               *
* ����                                                                    *
* stream	: �f�[�^�X�g���[��                                            *
* read_func	: �ǂݍ��ݗp�̊֐��|�C���^                                    *
* width		: �C���[�W�̕����i�[����A�h���X                              *
* height	: �C���[�W�̍������i�[����A�h���X                            *
* stride	: �C���[�W�̈�s���̃o�C�g�����i�[����A�h���X                *
* dpi		: �C���[�W�̉𑜓x(DPI)���i�[����A�h���X                     *
* icc_profile_name	: ICC�v���t�@�C���̖��O���󂯂�|�C���^               *
* icc_profile_data	: ICC�v���t�@�C���̃f�[�^���󂯂�|�C���^             *
* icc_profile_size	: ICC�v���t�@�C���̃f�[�^�̃o�C�g�����i�[����A�h���X *
* �Ԃ�l                                                                  *
*	�s�N�Z���f�[�^                                                        *
**************************************************************************/
extern uint8* ReadPNGDetailData(
	void* stream,
	stream_func read_func,
	gint32* width,
	gint32* height,
	gint32* stride,
	int32* dpi,
	char** icc_profile_name,
	uint8** icc_profile_data,
	uint32* icc_profile_size
);

/*********************************************
* WritePNGStream�֐�                         *
* PNG�̃f�[�^���X�g���[���ɏ�������          *
* ����                                       *
* stream		: �f�[�^�X�g���[��           *
* write_func	: �������ݗp�̊֐��|�C���^   *
* pixels		: �s�N�Z���f�[�^             *
* width			: �摜�̕�                   *
* height		: �摜�̍���                 *
* stride		: �摜�f�[�^��s���̃o�C�g�� *
* channel		: �摜�̃`�����l����         *
* interlace		: �C���^�[���[�X�̗L��       *
* compression	: ���k���x��                 *
*********************************************/
extern void WritePNGStream(
	void* stream,
	stream_func write_func,
	void (*flush_func)(void*),
	uint8* pixels,
	int32 width,
	int32 height,
	int32 stride,
	uint8 channel,
	int32 interlace,
	int32 compression
);

/********************************************************
* WritePNGDetailData�֐�                                *
* PNG�̃f�[�^���ڍ׏��t���ŃX�g���[���ɏ�������       *
* ����                                                  *
* stream			: �f�[�^�X�g���[��                  *
* write_func		: �������ݗp�̊֐��|�C���^          *
* pixels			: �s�N�Z���f�[�^                    *
* width				: �摜�̕�                          *
* height			: �摜�̍���                        *
* stride			: �摜�f�[�^��s���̃o�C�g��        *
* channel			: �摜�̃`�����l����                *
* interlace			: �C���^�[���[�X�̗L��              *
* compression		: ���k���x��                        *
* dpi				: �𑜓x(DPI)                       *
* icc_profile_name	: ICC�v���t�@�C���̖��O             *
* icc_profile_data	: ICC�v���t�@�C���̃f�[�^           *
* icc_profile_data	: ICC�v���t�@�C���̃f�[�^�̃o�C�g�� *
********************************************************/
extern void WritePNGDetailData(
	void* stream,
	stream_func write_func,
	void (*flush_func)(void*),
	uint8* pixels,
	int32 width,
	int32 height,
	int32 stride,
	uint8 channel,
	int32 interlace,
	int32 compression,
	int32 dpi,
	char* icc_profile_name,
	uint8* icc_profile_data,
	uint32 icc_profile_size
);

/********************************************************************
* ReadJpegHeader�֐�                                                *
* JPEG�̃w�b�_����ǂݍ���                                        *
* ����                                                              *
* system_file_path	: OS�̃t�@�C���V�X�e���ɑ������t�@�C���p�X      *
* width				: �摜�̕��f�[�^�̓ǂݍ��ݐ�                    *
* height			: �摜�̍����f�[�^�̓ǂݍ��ݐ�                  *
* resolution		: �摜�̉𑜓x�f�[�^�̓ǂݍ��ݐ�                *
* icc_profile_data	: ICC�v���t�@�C���̃f�[�^���󂯂�|�C���^       *
* icc_profile_size	: ICC�v���t�@�C���̃f�[�^�̃o�C�g���̓ǂݍ��ݐ� *
********************************************************************/
extern void ReadJpegHeader(
	const char* system_file_path,
	int* width,
	int* height,
	int* resolution,
	uint8** icc_profile_data,
	int32* icc_profile_size
);

/****************************************************************
* WriteJpegFile�֐�                                             *
* JPEG�`���ŉ摜�f�[�^�������o��                                *
* ����                                                          *
* system_file_path	: OS�̃t�@�C���V�X�e���ɑ������t�@�C���p�X  *
* pixels			: �s�N�Z���f�[�^                            *
* width				: �摜�f�[�^�̕�                            *
* height			: �摜�f�[�^�̍���                          *
* channel			: �摜�̃`�����l����                        *
* data_type			: �s�N�Z���f�[�^�̌`��(RGB or CMYK)         *
* quality			: JPEG�摜�̉掿                            *
* optimize_coding	: �œK���G���R�[�h�̗L��                    *
* icc_profile_data	: ���ߍ���ICC�v���t�@�C���̃f�[�^           *
*						(���ߍ��܂Ȃ�����NULL)                  *
* icc_profile_size	: ���ߍ���ICC�v���t�@�C���̃f�[�^�̃o�C�g�� *
* resolution		: �𑜓x(dpi)                               *
****************************************************************/
extern void WriteJpegFile(
	const char* system_file_path,
	uint8* pixels,
	int width,
	int height,
	int channel,
	eIMAGE_DATA_TYPE data_type,
	int quality,
	int optimize_coding,
	void* icc_profile_data,
	int icc_profile_size,
	int resolution
);

/***********************************************************
* ReadOriginalFormat�֐�                                   *
* �Ǝ��`���̃f�[�^��ǂݍ���                               *
* ����                                                     *
* stram			: �ǂݍ��݌��̃A�h���X                     *
* read_func		: �ǂݍ��ݗp�̊֐��|�C���^                 *
* stream_size	: �f�[�^�̑��o�C�g��                       *
* app			: �A�v���P�[�V�������Ǘ�����\���̃A�h���X *
* data_name		: �t�@�C����                               *
* �Ԃ�l                                                   *
*	�`��̈�̃f�[�^                                       *
***********************************************************/
extern DRAW_WINDOW* ReadOriginalFormat(
	void* stream,
	stream_func read_func,
	size_t stream_size,
	APPLICATION* app,
	const char* data_name
);

/***********************************************************
* ReadOriginalFormatMixedData�֐�                          *
* �Ǝ��`���̍����f�[�^���쐬���ĕԂ�                       *
* ����                                                     *
* stram			: �ǂݍ��݌��̃A�h���X                     *
* read_func		: �ǂݍ��ݗp�̊֐��|�C���^                 *
* stream_size	: �f�[�^�̑��o�C�g��                       *
* app			: �A�v���P�[�V�������Ǘ�����\���̃A�h���X *
* data_name		: �t�@�C����                               *
* �Ԃ�l                                                   *
*	�����������C���[                                       *
***********************************************************/
extern LAYER* ReadOriginalFormatMixedData(
	void* stream,
	stream_func read_func,
	size_t stream_size,
	APPLICATION* app,
	const char* data_name
);

/*******************************************************************
* ReadOriginalFormatMemoryStream�֐�                               *
* �������X�g���[���̓Ǝ��t�H�[�}�b�g�̃f�[�^��ǂݍ���(�A���h�D�p) *
* ����                                                             *
* window	: �f�[�^�̓ǂݍ��ݐ�                                   *
* stream	: �������X�g���[��                                     *
*******************************************************************/
extern void ReadOriginalFormatMemoryStream(
	DRAW_WINDOW* window,
	MEMORY_STREAM_PTR stream
);

/*******************************************
* WriteOriginalFormat�֐�                  *
* �Ǝ��`���̃f�[�^�𐶐�����               *
* ����                                     *
* stream		: �������ݐ�̃X�g���[��   *
* write_func	: �������ݗp�̊֐��|�C���^ *
* window		: �`��̈�̏��           *
* add_thumbnail	: �T���l�C���̗L��         *
* compress		: ���k��                   *
*******************************************/
extern void WriteOriginalFormat(
	void* stream,
	stream_func write_func,
	DRAW_WINDOW* window,
	int add_thumbnail,
	int compress
);

/*************************************************
* WritePhotoShopDocument�֐�                     *
* PSD�`���ŏ�������                              *
* ����                                           *
* stream		: �������ݐ�̃X�g���[��         *
* write_func	: �������ݗp�̊֐��|�C���^       *
* seek_func		: �V�[�N�p�̊֐��|�C���^         *
* tell_func		: �V�[�N�ʒu�擾�p�̊֐��|�C���^ *
* window		: �`��̈�̏��                 *
*************************************************/
extern void WritePhotoShopDocument(
	void* stream,
	stream_func write_func,
	seek_func seek_func,
	long (*tell_func)(void*),
	DRAW_WINDOW* window
);

/**************************************************************
* ReadTiffTagData�֐�                                         *
* TIFF�t�@�C���̃^�O����ǂݍ���                            *
* ����                                                        *
* system_file_path	: ���sOS�ł̃t�@�C���p�X                  *
* resolution		: �𑜓x(DPI)���i�[����A�h���X           *
* icc_profile_data	: ICC�v���t�@�C���̃f�[�^���󂯂�|�C���^ *
* icc_profile_size	: ICC�v���t�@�C���̃f�[�^�̃o�C�g��       *
**************************************************************/
extern void ReadTiffTagData(
	const char* system_file_path,
	int* resolution,
	uint8** icc_profile_data,
	int32* icc_profile_size
);

/*******************************************************
* WriteTiff�֐�                                        *
* TIFF�`���ŉ摜�������o��                             *
* ����                                                 *
* system_file_path	: OS�̃t�@�C���V�X�e���ɑ������p�X *
* window			: �摜�������o���`��̈�̏��     *
* write_aplha		: �������������o�����ۂ�         *
* compress			: ���k���s�����ۂ�                 *
* profile_data		: ICC�v���t�@�C���̃f�[�^          *
*					  (�����o���Ȃ��ꍇ��NULL)         *
* profile_data_size	: ICC�v���t�@�C���̃f�[�^�o�C�g��  *
*******************************************************/
extern void WriteTiff(
	const char* system_file_path,
	DRAW_WINDOW* window,
	gboolean write_alpha,
	gboolean compress,
	void* profile_data,
	guint32 profile_data_size
);

/*******************************************************
* SetFileChooserPreview�֐�                            *
* �t�@�C���I���_�C�A���O�Ƀv���r���[�E�B�W�F�b�g��ݒ� *
* ����                                                 *
* file_chooser	: �t�@�C���I���_�C�A���O�E�B�W�F�b�g   *
*******************************************************/
extern void SetFileChooserPreview(GtkWidget *file_chooser);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLDUED_IMAGE_READ_WRITE_H_
