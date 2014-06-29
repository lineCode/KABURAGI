// Visual Studio 2005�ȍ~�ł͌Â��Ƃ����֐����g�p����̂�
	// �x�����o�Ȃ��悤�ɂ���
#if defined _MSC_VER && _MSC_VER >= 1400
# define _CRT_SECURE_NO_DEPRECATE
# define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <ctype.h>
#include <png.h>
#include <setjmp.h>
#include <zlib.h>
#include "libtiff/tiffio.h"
#include "libjpeg/jpeglib.h"
#include "types.h"
#include "memory.h"
#include "memory_stream.h"
#include "bit_stream.h"
#include "image_read_write.h"
#include "utils.h"
#include "tlg.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************
* DecodeImageData�֐�                                            *
* ���������̉摜�f�[�^���f�R�[�h����                             *
* ����                                                           *
* data				: �摜�f�[�^                                 *
* data_size			: �摜�f�[�^�̃o�C�g��                       *
* file_type			: �摜�̃^�C�v��\��������(��:PNG�Ȃ�".png") *
* width				: �摜�̕��̊i�[��                           *
* height			: �摜�̍����̊i�[��                         *
* channel			: �摜�`�����l�����̊i�[��                   *
* resolution		: �摜�𑜓x�̊i�[��                         *
* icc_profile_name	: ICC�v���t�@�C���̖��O�̊i�[��              *
* icc_profile_data	: ICC�v���t�@�C���̃f�[�^�̊i�[��            *
* icc_profile_size	: ICC�v���t�@�C���̃f�[�^�T�C�Y�̊i�[��      *
* �Ԃ�l                                                         *
*	�f�R�[�h�����s�N�Z���f�[�^                                   *
*****************************************************************/
uint8* DecodeImageData(
	uint8* data,
	size_t data_size,
	const char* file_type,
	int* width,
	int* height,
	int* channel,
	int* resolution,
	char** icc_profile_name,
	uint8** icc_profile_data,
	uint32* icc_profile_size
)
{
	MEMORY_STREAM stream = {data, 0, data_size, 1};
	char *type = MEM_STRDUP_FUNC(file_type);
	uint8 *result = NULL;
	int local_width = 0, local_height = 0;
	int local_channel = 0;
	int local_resolution = 0;

	{
		char *p = type;
		while(*p != '\0')
		{
			*p = tolower(*p);
			p++;
		}
	}

	if(strcmp(type, ".png") == 0)
	{
		result = ReadPNGDetailData((void*)&stream, (stream_func_t)MemRead,
			&local_width, &local_height, &local_channel, &local_resolution,
			icc_profile_name, icc_profile_data, icc_profile_size
		);
		if(local_width > 0)
		{
			local_channel = local_channel / local_width;
		}
	}
	else if(strcmp(type, ".jpg") == 0 || strcmp(type, ".jpeg") == 0)
	{
		result = ReadJpegStream((void*)&stream, (stream_func_t)MemRead,
			data_size, &local_width, &local_height, &local_channel, &local_resolution,
			icc_profile_data, icc_profile_size
		);
	}
	else if(strcmp(type, ".tga") == 0)
	{
		result = ReadTgaStream((void*)&stream, (stream_func_t)MemRead, data_size,
			&local_width, &local_height, &local_channel);
	}
	else if(strcmp(type, ".tlg") == 0)
	{
		result = ReadTlgStream((void*)&stream, (stream_func_t)MemRead, (seek_func_t)MemSeek,
			(long (*)(void*))MemTell, &local_width, &local_height, &local_channel);
	}
	else if(strcmp(type, ".bmp") == 0 || strcmp(type, ".spa") == 0 || strcmp(type, ".sph") == 0)
	{
		result = ReadBmpStream((void*)&stream, (stream_func_t)MemRead, (seek_func_t)MemSeek,
			data_size, &local_width, &local_height, &local_channel, &local_resolution);
	}

	MEM_FREE_FUNC(type);

	if(width != NULL)
	{
		*width = local_width;
	}
	if(height != NULL)
	{
		*height = local_height;
	}
	if(channel != NULL)
	{
		*channel = local_channel;
	}
	if(resolution != NULL)
	{
		*resolution = local_resolution;
	}

	return result;
}

/***********************************************************
* PNG_IO�\����                                             *
* pnglib�ɃX�g���[���̃A�h���X�Ɠǂݏ����֐��|�C���^�n���p *
***********************************************************/
typedef struct _PNG_IO
{
	void* data;
	stream_func_t rw_func;
	void (*flush_func)(void* stream);
} PNG_IO;

/***************************************************
* PngReadWrite�֐�                                 *
* ����                                             *
* png_p		: pnglib�̈��k�E�W�J�Ǘ��\���̃A�h���X *
* data		: �ǂݏ�����̃A�h���X                 *
* length	: �ǂݏ�������o�C�g��                 *
***************************************************/
static void PngReadWrite(
	png_structp png_p,
	png_bytep data,
	png_size_t length
)
{
	PNG_IO *io = (PNG_IO*)png_get_io_ptr(png_p);
	(void)io->rw_func(data, 1, length, io->data);
}

/***************************************************
* PngFlush�֐�                                     *
* �X�g���[���Ɏc�����f�[�^���N���A                 *
* ����                                             *
* png_p		: libpng�̈��k�E�W�J�Ǘ��\���̃A�h���X *
***************************************************/
static void PngFlush(png_structp png_p)
{
	PNG_IO* io = (PNG_IO*)png_get_io_ptr(png_p);
	if(io->flush_func != NULL)
	{
		io->flush_func(io->data);
	}
}

/***********************************************
* PngReadChunkCallback�֐�                     *
* �`�����N�ǂݍ��ݗp�R�[���o�b�N�֐�           *
* png_p	: libpng�̈��k�E�W�J�Ǘ��\���̃A�h���X *
* chunk	: �`�����N�f�[�^                       *
* �Ԃ�l                                       *
*	����I��:���̒l�A�s��:0�A�G���[:���̒l     *
***********************************************/
int PngReadChunkCallback(png_structp ptr, png_unknown_chunkp chunk)
{
	return 0;
}

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
uint8* ReadPNGStream(
	void* stream,
	stream_func_t read_func,
	gint32* width,
	gint32* height,
	gint32* stride
)
{
	PNG_IO io;
	png_structp png_p;
	png_infop info_p;
	uint32 local_width, local_height, local_stride;
	int32 bit_depth, color_type, interlace_type;
	uint8* pixels;
	uint8** image;
	uint32 i;

	// �X�g���[���̃A�h���X�Ɗ֐��|�C���^���Z�b�g
	io.data = stream;
	io.rw_func = read_func;

	// PNG�W�J�p�̃f�[�^�𐶐�
	png_p = png_create_read_struct(
		PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL
	);

	// �摜�����i�[����f�[�^�̈�쐬
	info_p = png_create_info_struct(png_p);

	if(setjmp(png_jmpbuf(png_p)) != 0)
	{
		png_destroy_read_struct(&png_p, &info_p, NULL);

		return NULL;
	}

	// �ǂݍ��ݗp�̃X�g���[���A�֐��|�C���^���Z�b�g
	png_set_read_fn(png_p, (void*)&io, PngReadWrite);

	// �摜���̓ǂݍ���
	png_read_info(png_p, info_p);
	png_get_IHDR(png_p, info_p, &local_width, &local_height,
		&bit_depth, &color_type, &interlace_type, NULL, NULL
	);
	local_stride = (uint32)png_get_rowbytes(png_p, info_p);

	// �s�N�Z���������̊m��
	pixels = (uint8*)MEM_ALLOC_FUNC(local_stride*local_height);
	// pnglib�ł�2�����z��ɂ���K�v������̂�
		// �������̃|�C���^�z����쐬
	image = (uint8**)MEM_ALLOC_FUNC(sizeof(*image)*local_height);

	// 2�����z��ɂȂ�悤�A�h���X���Z�b�g
	for(i=0; i<local_height; i++)
	{
		image[i] = &pixels[local_stride*i];
	}
	// �摜�f�[�^�̓ǂݍ���
	png_read_image(png_p, image);

	// �������̊J��
	png_destroy_read_struct(&png_p, &info_p, NULL);
	MEM_FREE_FUNC(image);

	// �摜�f�[�^���w��A�h���X�ɃZ�b�g
	*width = (int32)local_width;
	*height = (int32)local_height;
	*stride = (int32)local_stride;

	return pixels;
}

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
void ReadPNGHeader(
	void* stream,
	stream_func_t read_func,
	gint32* width,
	gint32* height,
	gint32* stride,
	int32* dpi,
	char** icc_profile_name,
	uint8** icc_profile_data,
	uint32* icc_profile_size
)
{
	PNG_IO io;
	png_structp png_p;
	png_infop info_p;
	uint32 local_width, local_height, local_stride;
	int32 bit_depth, color_type, interlace_type;

	// �X�g���[���̃A�h���X�Ɗ֐��|�C���^���Z�b�g
	io.data = stream;
	io.rw_func = read_func;

	// PNG�W�J�p�̃f�[�^�𐶐�
	png_p = png_create_read_struct(
		PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL
	);

	// �摜�����i�[����f�[�^�̈�쐬
	info_p = png_create_info_struct(png_p);

	if(setjmp(png_jmpbuf(png_p)) != 0)
	{
		png_destroy_read_struct(&png_p, &info_p, NULL);

		return;
	}

	// �ǂݍ��ݗp�̃X�g���[���A�֐��|�C���^���Z�b�g
	png_set_read_fn(png_p, (void*)&io, PngReadWrite);

	// �摜���̓ǂݍ���
	png_read_info(png_p, info_p);
	png_get_IHDR(png_p, info_p, &local_width, &local_height,
		&bit_depth, &color_type, &interlace_type, NULL, NULL
	);
	local_stride = (uint32)png_get_rowbytes(png_p, info_p);

	// �𑜓x�̎擾
	if(dpi != NULL)
	{
		png_uint_32 res_x, res_y;
		int unit_type;

		if(png_get_pHYs(png_p, info_p, &res_x, &res_y, &unit_type) == PNG_INFO_pHYs)
		{
			if(unit_type == PNG_RESOLUTION_METER)
			{
				if(res_x > res_y)
				{
					*dpi = (int32)(res_x * 0.0254 + 0.5);
				}
				else
				{
					*dpi = (int32)(res_y * 0.0254 + 0.5);
				}
			}
		}
	}

	// ICC�v���t�@�C���̎擾
	if(icc_profile_data != NULL && icc_profile_size != NULL)
	{
		png_charp name;
		png_charp profile;
		png_uint_32 profile_size;
		int compression_type;

		*icc_profile_data = NULL;
		*icc_profile_size = 0;

		if(png_get_iCCP(png_p, info_p, &name, &compression_type,
			&profile, &profile_size) == PNG_INFO_iCCP)
		{
			if(name != NULL && icc_profile_name != NULL)
			{
				*icc_profile_name = MEM_STRDUP_FUNC(name);
			}

			*icc_profile_data = (uint8*)MEM_ALLOC_FUNC(profile_size);
			(void)memcpy(*icc_profile_data, profile, profile_size);
			*icc_profile_size = profile_size;
		}
	}

	// �������̊J��
	png_destroy_read_struct(&png_p, &info_p, NULL);

	// �摜�f�[�^���w��A�h���X�ɃZ�b�g
	if(width != NULL)
	{
		*width = (int32)local_width;
	}
	if(height != NULL)
	{
		*height = (int32)local_height;
	}
	if(stride != NULL)
	{
		*stride = (int32)local_stride;
	}
}

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
uint8* ReadPNGDetailData(
	void* stream,
	stream_func_t read_func,
	gint32* width,
	gint32* height,
	gint32* stride,
	int32* dpi,
	char** icc_profile_name,
	uint8** icc_profile_data,
	uint32* icc_profile_size
)
{
	PNG_IO io;
	png_structp png_p;
	png_infop info_p;
	uint32 local_width, local_height, local_stride;
	int32 bit_depth, color_type, interlace_type;
	uint8* pixels;
	uint8** image;
	uint32 i;

	// �X�g���[���̃A�h���X�Ɗ֐��|�C���^���Z�b�g
	io.data = stream;
	io.rw_func = read_func;

	// PNG�W�J�p�̃f�[�^�𐶐�
	png_p = png_create_read_struct(
		PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL
	);

	// �摜�����i�[����f�[�^�̈�쐬
	info_p = png_create_info_struct(png_p);

	if(setjmp(png_jmpbuf(png_p)) != 0)
	{
		png_destroy_read_struct(&png_p, &info_p, NULL);

		return NULL;
	}

	// �ǂݍ��ݗp�̃X�g���[���A�֐��|�C���^���Z�b�g
	png_set_read_fn(png_p, (void*)&io, PngReadWrite);

	// �`�����N�ǂݍ��ݗp�̊֐��|�C���^���Z�b�g
	png_set_read_user_chunk_fn(png_p, NULL,
		(png_user_chunk_ptr)PngReadChunkCallback);

	// �摜���̓ǂݍ���
	png_read_info(png_p, info_p);
	png_get_IHDR(png_p, info_p, &local_width, &local_height,
		&bit_depth, &color_type, &interlace_type, NULL, NULL
	);
	local_stride = (uint32)png_get_rowbytes(png_p, info_p);

	// �𑜓x�̎擾
	if(dpi != NULL)
	{
		png_uint_32 res_x, res_y;
		int unit_type;

		if(png_get_pHYs(png_p, info_p, &res_x, &res_y, &unit_type) == PNG_INFO_pHYs)
		{
			if(unit_type == PNG_RESOLUTION_METER)
			{
				if(res_x > res_y)
				{
					*dpi = (int32)(res_x * 0.0254 + 0.5);
				}
				else
				{
					*dpi = (int32)(res_y * 0.0254 + 0.5);
				}
			}
		}
	}

	// ICC�v���t�@�C���̎擾
	if(icc_profile_data != NULL && icc_profile_size != NULL)
	{
		png_charp name;
		png_charp profile;
		png_uint_32 profile_size;
		int compression_type;

		if(png_get_iCCP(png_p, info_p, &name, &compression_type,
			&profile, &profile_size) == PNG_INFO_iCCP)
		{
			if(name != NULL)
			{
				*icc_profile_name = MEM_STRDUP_FUNC(name);
			}

			*icc_profile_data = (uint8*)MEM_ALLOC_FUNC(profile_size);
			(void)memcpy(*icc_profile_data, profile, profile_size);
			*icc_profile_size = profile_size;
		}
	}

	// �s�N�Z���������̊m��
	pixels = (uint8*)MEM_ALLOC_FUNC(local_stride*local_height);
	// pnglib�ł�2�����z��ɂ���K�v������̂�
		// �������̃|�C���^�z����쐬
	image = (uint8**)MEM_ALLOC_FUNC(sizeof(*image)*local_height);

	// 2�����z��ɂȂ�悤�A�h���X���Z�b�g
	for(i=0; i<local_height; i++)
	{
		image[i] = &pixels[local_stride*i];
	}
	// �摜�f�[�^�̓ǂݍ���
	png_read_image(png_p, image);

	// �������̊J��
	png_destroy_read_struct(&png_p, &info_p, NULL);
	MEM_FREE_FUNC(image);

	// �摜�f�[�^���w��A�h���X�ɃZ�b�g
	*width = (int32)local_width;
	*height = (int32)local_height;
	*stride = (int32)local_stride;

	return pixels;
}

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
void WritePNGStream(
	void* stream,
	stream_func_t write_func,
	void (*flush_func)(void*),
	uint8* pixels,
	int32 width,
	int32 height,
	int32 stride,
	uint8 channel,
	int32 interlace,
	int32 compression
)
{
	PNG_IO io = {stream, write_func, flush_func};
	png_structp png_p;
	png_infop info_p;
	uint8** image;
	int color_type;
	int i;

	// �`�����l�����ɍ��킹�ăJ���[�^�C�v��ݒ�
	switch(channel)
	{
	case 1:
		color_type = PNG_COLOR_TYPE_GRAY;
		break;
	case 2:
		color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
		break;
	case 3:
		color_type = PNG_COLOR_TYPE_RGB;
		break;
	case 4:
		color_type = PNG_COLOR_TYPE_RGBA;
		break;
	default:
		return;
	}

	// PNG�������ݗp�̃f�[�^�𐶐�
	png_p = png_create_write_struct(
		PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL
	);

	// �摜���i�[�p�̃��������m��
	info_p = png_create_info_struct(png_p);

	// �������ݗp�̃X�g���[���Ɗ֐��|�C���^���Z�b�g
	png_set_write_fn(png_p, &io, PngReadWrite, PngFlush);
	// ���k�ɂ͑S�Ẵt�B���^���g�p
	png_set_filter(png_p, 0, PNG_ALL_FILTERS);
	// ���k���x����ݒ�
	png_set_compression_level(png_p, compression);

	// PNG�̏����Z�b�g
	png_set_IHDR(png_p, info_p, width, height, 8, color_type,
		interlace, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// pnglib�p��2�����z����쐬
	image = (uint8**)MEM_ALLOC_FUNC(height*sizeof(*image));
	for(i=0; i<height; i++)
	{
		image[i] = &pixels[stride*i];
	}

	// �摜�f�[�^�̏�������
	png_write_info(png_p, info_p);
	png_write_image(png_p, image);
	png_write_end(png_p, info_p);

	// �������̊J��
	png_destroy_write_struct(&png_p, &info_p);

	MEM_FREE_FUNC(image);
}

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
void WritePNGDetailData(
	void* stream,
	stream_func_t write_func,
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
)
{
	PNG_IO io = {stream, write_func, flush_func};
	png_structp png_p;
	png_infop info_p;
	uint8** image;
	int color_type;
	int i;

	// �`�����l�����ɍ��킹�ăJ���[�^�C�v��ݒ�
	switch(channel)
	{
	case 1:
		color_type = PNG_COLOR_TYPE_GRAY;
		break;
	case 2:
		color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
		break;
	case 3:
		color_type = PNG_COLOR_TYPE_RGB;
		break;
	case 4:
		color_type = PNG_COLOR_TYPE_RGBA;
		break;
	default:
		return;
	}

	// PNG�������ݗp�̃f�[�^�𐶐�
	png_p = png_create_write_struct(
		PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL
	);

	// �摜���i�[�p�̃��������m��
	info_p = png_create_info_struct(png_p);

	// �������ݗp�̃X�g���[���Ɗ֐��|�C���^���Z�b�g
	png_set_write_fn(png_p, &io, PngReadWrite, PngFlush);
	// ���k�ɂ͑S�Ẵt�B���^���g�p
	png_set_filter(png_p, 0, PNG_ALL_FILTERS);
	// ���k���x����ݒ�
	png_set_compression_level(png_p, compression);

	// PNG�̏����Z�b�g
	png_set_IHDR(png_p, info_p, width, height, 8, color_type,
		interlace, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	// pnglib�p��2�����z����쐬
	image = (uint8**)MEM_ALLOC_FUNC(height*sizeof(*image));
	for(i=0; i<height; i++)
	{
		image[i] = &pixels[stride*i];
	}

	// �𑜓x�̏����Z�b�g
	if(dpi > 0)
	{
		png_uint_32 dpm = (png_uint_32)((dpi * 10000 + 127) / 254);
		png_set_pHYs(png_p, info_p, dpm, dpm, PNG_RESOLUTION_METER);
	}

	// ICC�v���t�@�C���̏����Z�b�g
	if(icc_profile_data != NULL && icc_profile_size > 0)
	{
		png_set_iCCP(png_p, info_p, icc_profile_name, PNG_COMPRESSION_TYPE_BASE,
			icc_profile_data, icc_profile_size);
	}

	// �摜�f�[�^�̏�������
	png_write_info(png_p, info_p);
	png_write_image(png_p, image);
	png_write_end(png_p, info_p);

	// �������̊J��
	png_destroy_write_struct(&png_p, &info_p);

	MEM_FREE_FUNC(image);
}

typedef struct _JPEG_ERROR_MANAGER
{
	struct jpeg_error_mgr jerr;
	jmp_buf buf;
} JPEG_ERROR_MANAGER;

static void JpegErrorHandler(j_common_ptr cinfo)
{
	JPEG_ERROR_MANAGER *manager = (JPEG_ERROR_MANAGER*)(cinfo->err);

	longjmp(manager->buf, 1);
}

#define ICC_MARKER  (JPEG_APP0 + 2)			// ICC�v���t�@�C���̃}�[�N�̈ʒu
#define ICC_OVERHEAD_LEN 14					// ICC�v���t�@�C���f�[�^�̃I�[�o�[�w�b�h�̃o�C�g��
#define MAX_BYTES_IN_MARKER 65533			// ICC�v���t�@�C���̃}�[�N�̍ő�o�C�g��
#define MAX_DATA_BYTES_IN_MARKER (MAX_BYTES_IN_MARKER - ICC_OVERHEAD_LEN)

static int JpegMarkerIsICC (jpeg_saved_marker_ptr marker)
{
  return
    marker->marker == ICC_MARKER &&
    marker->data_length >= ICC_OVERHEAD_LEN &&
    /* verify the identifying string */
    GETJOCTET(marker->data[0]) == 0x49 &&
    GETJOCTET(marker->data[1]) == 0x43 &&
    GETJOCTET(marker->data[2]) == 0x43 &&
    GETJOCTET(marker->data[3]) == 0x5F &&
    GETJOCTET(marker->data[4]) == 0x50 &&
    GETJOCTET(marker->data[5]) == 0x52 &&
    GETJOCTET(marker->data[6]) == 0x4F &&
    GETJOCTET(marker->data[7]) == 0x46 &&
    GETJOCTET(marker->data[8]) == 0x49 &&
    GETJOCTET(marker->data[9]) == 0x4C &&
    GETJOCTET(marker->data[10]) == 0x45 &&
    GETJOCTET(marker->data[11]) == 0x0;
}

static int JpegReadICCProfile(
	j_decompress_ptr dinfo,
	uint8** icc_profile_data,
	int32* icc_profile_size
)
{
	jpeg_saved_marker_ptr marker;
	int num_markers = 0;
	int seq_no;
	int total_length = 0;
#define MAX_SEQ_NO 255
	char marker_present[MAX_SEQ_NO+1] = {0};
	int data_length[MAX_SEQ_NO+1];
	int data_offset[MAX_SEQ_NO+1];

	*icc_profile_data = NULL;
	*icc_profile_size = 0;

	for(marker = dinfo->marker_list; marker != NULL; marker = marker->next)
	{
		if(JpegMarkerIsICC(marker))
		{
			if(num_markers == 0)
			{
				num_markers = GETJOCTET(marker->data[13]);
			}
			else if(num_markers != GETJOCTET(marker->data[13]))
			{
				return -1;
			}

			seq_no = GETJOCTET(marker->data[12]);
			if(seq_no <= 0 || seq_no > num_markers)
			{
				return -2;
			}

			if(marker_present[seq_no] != 0)
			{
				return -3;
			}

			marker_present[seq_no] = 1;
			data_length[seq_no] = marker->data_length - ICC_OVERHEAD_LEN;
		}
	}

	for(seq_no = 1; seq_no <= num_markers; seq_no++)
	{
		if(marker_present[seq_no] == 0)
		{
			return -4;
		}

		data_offset[seq_no] = total_length;
		total_length += data_length[seq_no];
	}

	if(total_length <= 0)
	{
		return -5;
	}

	*icc_profile_data = (uint8*)MEM_ALLOC_FUNC(total_length);
	*icc_profile_size = (int32)total_length;

	for(marker = dinfo->marker_list; marker != NULL; marker = marker->next)
	{
		if(JpegMarkerIsICC(marker))
		{
			uint8 *src_ptr;
			uint8 *dst_ptr;

			seq_no = GETJOCTET(marker->data[12]);
			dst_ptr = *icc_profile_data + data_offset[seq_no];
			src_ptr = marker->data + ICC_OVERHEAD_LEN;
			(void)memcpy(dst_ptr, src_ptr, data_length[seq_no]);
		}
	}

	return 0;
}

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
void ReadJpegHeader(
	const char* system_file_path,
	int* width,
	int* height,
	int* resolution,
	uint8** icc_profile_data,
	int32* icc_profile_size
)
{
	struct jpeg_decompress_struct dinfo;
	FILE *fp = fopen(system_file_path, "rb");
	JPEG_ERROR_MANAGER error_manager;

	if(fp == NULL)
	{
		return;
	}
	
	error_manager.jerr.error_exit = JpegErrorHandler;
	dinfo.err = jpeg_std_error(&error_manager.jerr);

	jpeg_create_decompress(&dinfo);

	if(setjmp(error_manager.buf) != 0)
	{
		return;
	}

	jpeg_create_decompress(&dinfo);
	jpeg_stdio_src(&dinfo, fp);

	if(icc_profile_data != NULL && icc_profile_size != NULL)
	{
		jpeg_save_markers(&dinfo, ICC_MARKER, 0xffff);
	}

	jpeg_read_header(&dinfo, FALSE);

	(void)JpegReadICCProfile(&dinfo, icc_profile_data, icc_profile_size);

	if(width != NULL)
	{
		*width = dinfo.image_width;
	}
	if(height != NULL)
	{
		*height = dinfo.image_height;
	}
	if(resolution != NULL)
	{
		*resolution = dinfo.X_density;
	}

	jpeg_destroy_decompress(&dinfo);
	(void)fclose(fp);
}

/********************************************************************
* ReadJpegStream�֐�                                                *
* JPEG�̃f�[�^��ǂݍ��݁A�f�R�[�h����                              *
* ����                                                              *
* stream			: �ǂݍ��݌�                                    *
* read_func			: �ǂݍ��݂Ɏg���֐��|�C���^                    *
* data_size			: JPEG�̃f�[�^�T�C�Y                            *
* width				: �摜�̕��f�[�^�̓ǂݍ��ݐ�                    *
* height			: �摜�̍����f�[�^�̓ǂݍ��ݐ�                  *
* channel			: �摜�f�[�^�̃`�����l����                      *
* resolution		: �摜�̉𑜓x�f�[�^�̓ǂݍ��ݐ�                *
* icc_profile_data	: ICC�v���t�@�C���̃f�[�^���󂯂�|�C���^       *
* icc_profile_size	: ICC�v���t�@�C���̃f�[�^�̃o�C�g���̓ǂݍ��ݐ� *
* �Ԃ�l                                                            *
*	�f�R�[�h�����s�N�Z���f�[�^(���s������NULL)                      *
********************************************************************/
uint8* ReadJpegStream(
	void* stream,
	stream_func_t read_func,
	size_t data_size,
	int* width,
	int* height,
	int* channel,
	int* resolution,
	uint8** icc_profile_data,
	int32* icc_profile_size
)
{
	// JPEG�f�R�[�h�p�̃f�[�^
	struct jpeg_decompress_struct decode;
	// �f�R�[�h�����s�N�Z���f�[�^
	uint8 *pixels = NULL;
	// �f�[�^����x�������ɑS�ēǂݍ���
	uint8 *jpeg_data = (uint8*)MEM_ALLOC_FUNC(data_size);
	// �s�N�Z���f�[�^�[���I��2�����z��ɂ���
	uint8 **pixel_datas;
	// �摜�̕��E����(���[�J��)
	int local_width, local_height;
	// �摜�̃`�����l����(���[�J��)
	int local_channel;
	// 1�s���̃o�C�g��
	int stride;
	// �f�R�[�h���̃G���[����
	JPEG_ERROR_MANAGER error;
	// for���p�̃J�E���^
	int i;

	// �f�[�^�̓ǂݍ���
	(void)read_func(jpeg_data, 1, data_size, stream);

	// �G���[�����̐ݒ�
	error.jerr.error_exit = (noreturn_t (*)(j_common_ptr))JpegErrorHandler;
	decode.err = jpeg_std_error(&error.jerr);
	// �G���[���̃W�����v���ݒ�
	if(setjmp(error.buf) != 0)
	{	// �G���[�Ŗ߂��Ă���
		MEM_FREE_FUNC(jpeg_data);
		MEM_FREE_FUNC(pixel_datas);
		MEM_FREE_FUNC(pixels);
		return NULL;
	}

	// �f�R�[�h�f�[�^�̏�����
	jpeg_create_decompress(&decode);
	jpeg_mem_src(&decode, jpeg_data, (unsigned long)data_size);

	// ICC�v���t�@�C���ǂݍ��ݏ���
	if(icc_profile_data != NULL && icc_profile_size != NULL)
	{
		jpeg_save_markers(&decode, ICC_MARKER, 0xffff);
	}

	// �w�b�_�̓ǂݍ���
	jpeg_read_header(&decode, FALSE);

	// ICC�v���t�@�C���̓ǂݍ���
	(void)JpegReadICCProfile(&decode, icc_profile_data, icc_profile_size);

	// �摜�̕��A�����A�`�����l�����A�𑜓x���擾
	local_width = decode.image_width;
	local_height = decode.image_height;
	local_channel = decode.num_components;
	stride = local_channel * local_width;

	if(width != NULL)
	{
		*width = local_width;
	}
	if(height != NULL)
	{
		*height = local_height;
	}
	if(channel != NULL)
	{
		*channel = local_channel;
	}
	if(resolution != NULL)
	{
		*resolution = decode.X_density;
	}

	// �s�N�Z���f�[�^�̃��������m��
	pixels = (uint8*)MEM_ALLOC_FUNC(local_height * stride);
	pixel_datas = (uint8**)MEM_ALLOC_FUNC(sizeof(*pixel_datas) * local_height);
	for(i=0; i<local_height; i++)
	{
		pixel_datas[i] = &pixels[i*stride];
	}

	// �f�R�[�h�J�n
	(void)jpeg_start_decompress(&decode);
	while(decode.output_scanline < decode.image_height)
	{
		jpeg_read_scanlines(&decode, pixel_datas + decode.output_scanline,
			decode.image_height - decode.output_scanline);
	}

	// �������J��
	jpeg_finish_decompress(&decode);
	MEM_FREE_FUNC(pixel_datas);
	MEM_FREE_FUNC(pixels);
	jpeg_destroy_decompress(&decode);
	MEM_FREE_FUNC(jpeg_data);

	return pixels;
}

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
void WriteJpegFile(
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
)
{
#define MARKER_LENGTH 65533
#define ICC_MARKER_HEADER_LENGTH 14
#define ICC_MARKER_DATA_LENGTH (MARKER_LENGTH - ICC_MARKER_HEADER_LENGTH)

	struct jpeg_compress_struct cinfo;
	JPEG_ERROR_MANAGER error_manager;
	FILE *fp;
	uint8 *write_array;
	int stride = width * channel;
	int i;

	if((fp = fopen(system_file_path, "wb")) == NULL)
	{
		return;
	}

	error_manager.jerr.error_exit = JpegErrorHandler;
	cinfo.err = jpeg_std_error(&error_manager.jerr);

	jpeg_create_compress(&cinfo);

	if(setjmp(error_manager.buf) != 0)
	{
		return;
	}

	jpeg_stdio_dest(&cinfo, fp);

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = channel;

	if(data_type == IMAGE_DATA_CMYK)
	{
		cinfo.in_color_space = JCS_CMYK;
		cinfo.jpeg_color_space = JCS_YCCK;
		jpeg_set_defaults(&cinfo);
		jpeg_set_colorspace(&cinfo, JCS_YCCK);
	}
	else
	{
		cinfo.in_color_space = JCS_RGB;
		jpeg_set_defaults(&cinfo);
	}

	if(resolution > 0)
	{
		cinfo.X_density = resolution;
		cinfo.Y_density = resolution;
		cinfo.density_unit = 1;
	}
	cinfo.optimize_coding = (boolean)optimize_coding;
	cinfo.dct_method = JDCT_FLOAT;
	cinfo.write_JFIF_header = TRUE;

	jpeg_set_quality(&cinfo, quality, TRUE);

	jpeg_start_compress(&cinfo, TRUE);

	if(icc_profile_data != NULL)
	{
		int n_markers, current_marker = 1;
		uint8 *icc_profile_byte_data = (uint8*)icc_profile_data;
		int data_point = 0;

		n_markers = icc_profile_size / ICC_MARKER_DATA_LENGTH;

		if(icc_profile_size % ICC_MARKER_DATA_LENGTH != 0)
		{
			n_markers++;
		}

		while(icc_profile_size > 0)
		{
			int length;

			length = MIN(icc_profile_size, ICC_MARKER_DATA_LENGTH);
			icc_profile_size -= length;

			jpeg_write_m_header(&cinfo, JPEG_APP0 + 2, length + ICC_MARKER_HEADER_LENGTH);
			jpeg_write_m_byte (&cinfo, 'I');
			jpeg_write_m_byte (&cinfo, 'C');
			jpeg_write_m_byte (&cinfo, 'C');
			jpeg_write_m_byte (&cinfo, '_');
			jpeg_write_m_byte (&cinfo, 'P');
			jpeg_write_m_byte (&cinfo, 'R');
			jpeg_write_m_byte (&cinfo, 'O');
			jpeg_write_m_byte (&cinfo, 'F');
			jpeg_write_m_byte (&cinfo, 'I');
			jpeg_write_m_byte (&cinfo, 'L');
			jpeg_write_m_byte (&cinfo, 'E');
			jpeg_write_m_byte (&cinfo, '\0');

			jpeg_write_m_byte(&cinfo, current_marker);
			jpeg_write_m_byte(&cinfo, n_markers);

			while(length -- > 0)
			{
				jpeg_write_m_byte(&cinfo, icc_profile_byte_data[data_point]);
				data_point++;
			}

			current_marker++;
		}
	}	// if(icc_profile_data != NULL)

	for(i=0; i<height; i++)
	{
		write_array = &pixels[i*stride];
		jpeg_write_scanlines(&cinfo,
			(JSAMPARRAY)&write_array, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	(void)fclose(fp);
}

/***************************************
* DecodeTgaRle�֐�                     *
* TGA�摜��RLE���k���f�R�[�h����       *
* ����                                 *
* src		: ���k���ꂽ�f�[�^         *
* dst		: �f�R�[�h��̃f�[�^�ۑ��� *
* width		: �摜�̕�                 *
* height	: �摜�̍���               *
* channel	: �摜�̃`�����l����       *
***************************************/
static void DecodeTgaRle(
	uint8* src,
	uint8* dst,
	int width,
	int height,
	int channel
)
{
	// RLE���k�W�J��̃f�[�^
	uint8 data[4];
	// BGR��RGB�p
	uint8 *swap;
	uint8 b;
	// �J��Ԃ���
	int num;
	// ���k����Ă��邩�ۂ�
	int compressed;
	// for���p�̃J�E���^
	int i, j, k;

	for(i=0; i<width*height; i++)
	{
		// �擪��1�o�C�g���爳�k/�����k�𔻒�
		compressed = ((*src & (1 << 7)) == 0) ? FALSE : TRUE;
		num = *src;
		src++;

		if(compressed != FALSE)
		{	// ���k�f�[�^
			num = (num & 127) + 1;
			for(j=0; j<channel; j++)
			{
				data[j] = *src;
				src++;
			}

			// BGR��RGB���Ȃ���f�R�[�h
			for(j=0; j<num; j++)
			{
				swap = dst;
				for(k=0; k<channel; k++)
				{
					*dst = data[k];
					dst++;
				}
				b = swap[0];
				swap[0] = swap[2];
				swap[2] = b;
			}
		}
		else
		{	// �����k�f�[�^
			num++;
			for(j=0; j<num; j++)
			{
				swap = dst;
				for(k=0; k<channel; k++)
				{
					*dst = *src;
					src++,	dst++;
				}
				// BGR��RGB
				b = swap[0];
				swap[0] = swap[2];
				swap[2] = b;
			}
		}
	}
}

/**********************************************
* ReadTgaStream�֐�                           *
* TGA�t�@�C���̓ǂݍ���(�K�v�ɉ����ăf�R�[�h) *
* ����                                        *
* stream	: �ǂݍ��݌�                      *
* read_func	: �ǂݍ��݂Ɏg���֐��|�C���^      *
* data_size	: �摜�f�[�^�̃o�C�g��            *
* width		: �摜�̕�                        *
* height	: �摜�̍���                      *
* channel	: �摜�̃`�����l����              *
* �Ԃ�l                                      *
*	�s�N�Z���f�[�^(���s����NULL)              *
**********************************************/
uint8* ReadTgaStream(
	void* stream,
	stream_func_t read_func,
	size_t data_size,
	int* width,
	int* height,
	int* channel
)
{
#define TGA_HEADER_SIZE 18
	// �s�N�Z���f�[�^
	uint8 *pixels = NULL;
	// �w�b�_�f�[�^
	uint8 header_data[TGA_HEADER_SIZE] = {0};
	// �摜�̕��E����(���[�J��)
	int local_width, local_height;
	// �摜�̃`�����l����(���[�J��)
	int local_channel = 0;
	// �r�b�g�[��
	int depth;
	// 1�s���̃o�C�g��
	int stride;
	// ���k�̗L��
	int compressed = FALSE;
	// �C���f�b�N�X���ۂ�
	int indexed = FALSE;
	// for���p�̃J�E���^
	int i;

	// �w�b�_���̓ǂݍ���
	(void)read_func(header_data, 1, TGA_HEADER_SIZE, stream);

	// ���ƍ������v�Z
	local_width = (header_data[13] << 8) + header_data[12];
	local_height = (header_data[15] << 8) + header_data[14];

	// �C���f�b�N�X�E���k�̏���ǂݍ���
	switch(header_data[2])
	{
	case 0x00:
		return NULL;
	case 0x01:
		indexed = TRUE;
		local_channel = 3;
		break;
	case 0x02:
		local_channel = 3;
		break;
	case 0x03:
		local_channel = 1;
		break;
	case 0x09:
		compressed = TRUE;
		indexed = TRUE;
		local_channel = 3;
		break;
	case 0x0A:
		compressed = TRUE;
		local_channel = 3;
		break;
	case 0x0B:
		compressed = TRUE;
		local_channel = 1;
		break;
	}

	depth = header_data[16];

	if(depth == 32)
	{
		local_channel = 4;
	}

	if(width != NULL)
	{
		*width = local_width;
	}
	if(height != NULL)
	{
		*height = local_height;
	}
	if(channel != NULL)
	{
		*channel = local_channel;
	}
	stride = local_channel * local_width;

	// �s�N�Z���f�[�^�̃������m��
	pixels = (uint8*)MEM_ALLOC_FUNC(stride * local_height);

	// �s�N�Z���f�[�^�̓ǂݍ���
	if(compressed == FALSE)
	{	// �����k
		(void)read_func(pixels, 1, stride * local_height, stream);
	}
	else
	{	// ���k
		uint8 *tga_data;	// ���k���ꂽ�f�[�^
		tga_data = (uint8*)MEM_ALLOC_FUNC(data_size - TGA_HEADER_SIZE);
		(void)read_func(tga_data, 1, data_size - TGA_HEADER_SIZE, stream);
		DecodeTgaRle(tga_data, pixels, local_width, local_height, local_channel);
		MEM_FREE_FUNC(tga_data);
	}

	// BBR��RGB
	if(local_channel >= 3)
	{
		uint8 b;
		for(i=0; i<local_width*local_height; i++)
		{
			b = pixels[i*local_channel];
			pixels[i*local_channel] = pixels[i*local_channel+2];
			pixels[i*local_channel+2] = b;
		}
	}

	return pixels;
}

typedef enum _eBMP_TYPE
{
	BMP_TYPE_UNKNOWN,
	BMP_TYPE_OS2,
	BMP_TYPE_MS
} eBMP_TYPE;

typedef enum _eBMP_COMPRESS_TYPE
{
	BMP_NO_COMPRESS,
	BMP_COMPRESS_RLE8,
	BMP_COMPRESS_RLE4,
	BMP_COMPRESS_BIT_FIELDS
} eBMP_COMPRESS_TYPE;

/*****************************************
* DecodeBmpRLE8�֐�                      *
* 8�r�b�g��RLE���k�f�[�^���f�R�[�h����   *
* ����                                   *
* src		: ���k�f�[�^                 *
* dst		: �f�R�[�h��̃f�[�^�̊i�[�� *
* width		: �摜�f�[�^�̕�             *
* height	: �摜�f�[�^�̍���           *
* stride	: 1�s���̃o�C�g��            *
* �Ԃ�l                                 *
*	�f�R�[�h��̃o�C�g��                 *
*****************************************/
static unsigned int DecodeBmpRLE8(
	void* src,
	void* dst,
	int width,
	int height,
	int stride
)
{
	uint8 *current_src = src;
	// for���p�̃J�E���^
	int x, y;
	int i;

	if(dst == NULL || src == NULL || width <= 0 || height <= 0)
	{
		return 0;
	}

	(void)memset(dst, 0, stride * height);

	for(y=0; y<height; y++)
	{
		uint8 *current_dst = (uint8*)dst + stride * y;
		// EOB�AEOL�o���t���O
		int eob = FALSE, eol;
		uint8 code[2];

		// EOB�ɓ��B����܂Ń��[�v
		for(x=0; ; )
		{
			// 2�o�C�g�擾
			code[0] = *current_src++;
			code[1] = *current_src++;

			if(code[0] == 0)
			{
				eol = FALSE;

				switch(code[1])
				{
				case 0x00:	// EOL
					eol = TRUE;
					break;
				case 0x01:	// EOB
					eob = TRUE;
					break;
				case 0x02:	// �ʒu�ړ����
					code[0] = *current_src++;
					code[1] = *current_src++;
					current_dst += code[0] + stride * code[1];
					break;
				default:	// ��΃��[�h�f�[�^
					x += code[1];
					for(i=0; i<code[1]; i++)
					{
						*current_dst++ = *current_src++;
					}

					// �p�f�B���O
					if((code[1] & 1) != 0)
					{
						current_src++;
					}
				}

				if(eol != FALSE || eob != FALSE)
				{
					break;
				}
			}
			else if(x < width)
			{
				// �G���R�[�h�f�[�^
				x += code[0];
				for(i=0; i<code[0]; i++)
				{
					*current_dst++ = code[1];
				}
			}
			else
			{
				return 0;
			}
		}

		if(eob != FALSE)
		{
			break;
		}
	}

	return stride * height;
}

/*****************************************
* DecodeBmpRLE4�֐�                      *
* 4�r�b�g��RLE���k�f�[�^���f�R�[�h����   *
* ����                                   *
* src		: ���k�f�[�^                 *
* dst		: �f�R�[�h��̃f�[�^�̊i�[�� *
* width		: �摜�f�[�^�̕�             *
* height	: �摜�f�[�^�̍���           *
* stride	: 1�s���̃o�C�g��            *
* �Ԃ�l                                 *
*	�f�R�[�h��̃o�C�g��                 *
*****************************************/
static unsigned int DecodeBmpRLE4(
	void* src,
	void* dst,
	int width,
	int height,
	int stride
)
{
	uint8 *current_src = src;
	// for���p�̃J�E���^
	int x, y;
	int i;

	if(dst == NULL || src == NULL || width <= 0 || height <= 0)
	{
		return 0;
	}

	(void)memset(dst, 0, stride * height);

	for(y=0; y<height; y++)
	{
		uint8 *current_dst = (uint8*)dst + stride * y;
		// ���̏����o���ʒu����p
		int is_hi = TRUE;
		// EOB�AEOL�o���t���O
		int eob = FALSE, eol;
		uint8 code[2];

		// EOL�ɓ��B����܂Ń��[�v
		for(x=0; ; )
		{
			code[0] = *current_src++;
			code[1] = *current_src++;

			// 1�o�C�g�ڂ�0�Ȃ�񈳏k
			if(code[0] == 0)
			{
				eol = FALSE;

				// ���ʃR�[�h�ŏ�����؂�ւ�
				switch(code[1])
				{
				case 0x00:	// EOL
					eol = TRUE;
					break;
				case 0x01:	// EOB
					eob = TRUE;
					break;
				case 0x02:	// �ʒu�ړ����
					code[0] = *current_src++;
					code[1] = *current_src++;
					x += code[0];
					y += code[1];
					current_dst += (int)code[0] / 2 + stride * code[1];

					// ��s�N�Z���Ȃ甼�o�C�g�ʒu��i�߂�
					if((code[0] & 1) != 0)
					{
						is_hi = !is_hi;
						if(is_hi != FALSE)
						{
							current_dst++;
						}
					}
					break;
				default:	// ��΃��[�h
					{
						int dst_byte = ((int)code[1] + 1) / 2;

						if(is_hi != FALSE)
						{
							for(i=0; i<dst_byte; i++)
							{
								*current_dst++ = *current_src++;
							}

							// ��s�N�Z���Ȃ甼�o�C�g�߂�
							if((code[1] & 1) != 0)
							{
								*(--current_dst) &= 0xf0;
								is_hi = FALSE;
							}
						}
						else
						{
							for(i=0; i<dst_byte; i++)
							{
								*current_dst++ |= (*current_src >> 4) & 0x0f;
								*current_dst |= (*current_src++ << 4) & 0xf0;
							}

							// ��s�N�Z���Ȃ甼�o�C�g�߂�
							if((code[1] & 1) != 0)
							{
								*current_dst = 0x00;
								is_hi = TRUE;
							}
						}

						// �p�f�B���O
						if((dst_byte & 1) != 0)
						{
							current_src++;
						}
					}
					break;
				}

				if(eol != FALSE || eob != FALSE)
				{
					break;
				}
			}
			else if(x < width)
			{
				int dst_byte;

				// ���k�f�[�^
				x += code[0];

				// ���̏����o�������4�r�b�g�łȂ��ꍇ
				if(is_hi == FALSE)
				{
					*current_dst++ = (code[1] >> 4) & 0x0f;
					code[1] = ((code[1] >> 4) & 0x0f) | ((code[1] << 4) & 0xf0);
					code[0]--;
					is_hi = TRUE;
				}

				// �����o��
				dst_byte = ((int)code[0] + 1) / 2;
				for(i=0; i<dst_byte; i++)
				{
					*current_dst++ = code[1];
				}

				// ��s�N�Z���Ȃ甼�o�C�g�߂�
				if((code[0] & 1) != 0)
				{
					*(--current_dst) &= 0xf0;
					is_hi = FALSE;
				}
			}
			else
			{
				return 0;
			}
		}

		if(eob != FALSE)
		{
			break;
		}
	}

	return stride * height;
}

/*******************************************************
* ReadBmpStream�֐�                                    *
* BMP�t�@�C���̃s�N�Z���f�[�^��ǂݍ���                *
* ����                                                 *
* stream		: �摜�f�[�^�̃X�g���[��               *
* read_func		: �X�g���[���ǂݍ��ݗp�̊֐��|�C���^   *
* seek_func		: �X�g���[���̃V�[�N�Ɏg���֐��|�C���^ *
* data_size		: �摜�f�[�^�̃o�C�g��                 *
* width			: �摜�̕��̊i�[��                     *
* height		: �摜�̍����̊i�[��                   *
* channel		: �摜�̃`�����l�����̊i�[��           *
* resolution	: �摜�̉𑜓x(dpi)�̊i�[��            *
* �Ԃ�l                                               *
*	�s�N�Z���f�[�^(���s����NULL)                       *
*******************************************************/
uint8* ReadBmpStream(
	void* stream,
	stream_func_t read_func,
	seek_func_t seek_func,
	size_t data_size,
	int* width,
	int* height,
	int* channel,
	int* resolution
)
{
#define BMP_FILE_HEADER_SIZE 14
#define OS2_BMP_INFO_HEADER_SIZE 12
#define MS_BMP_INFO_HEADER_SIZE 40
	uint8 *pixels = NULL;
	int local_width, local_height;
	int local_channel;
	int stride;
	int file_size;
	int data_offset;
	int info_header_size;
	int reverse_y = FALSE;
	int bit_per_pixel = 0;
	eBMP_TYPE bmp_type = BMP_TYPE_UNKNOWN;
	eBMP_COMPRESS_TYPE compress_type = BMP_NO_COMPRESS;
	uint8 header[64] = {0};
	int i;

	// �t�@�C���w�b�_�̓ǂݍ���
	(void)read_func(header, 1, BMP_FILE_HEADER_SIZE, stream);
	// �t�@�C���^�C�v�̃`�F�b�N
	if(header[0] != 'B' || header[1] != 'M')
	{
		return NULL;
	}
	// �t�@�C���T�C�Y�̎擾
	file_size = (header[5] << 24) + (header[4] << 16)
		+ (header[3] << 8) + header[2];
	// �s�N�Z���f�[�^�܂ł̃I�t�Z�b�g
	data_offset = (header[13] << 24) + (header[12] << 16)
		+ (header[11] << 8) + header[10];

	// �C���t�H�w�b�_�̃o�C�g�����擾
	(void)read_func(header, 1, 4, stream);
	info_header_size = (header[3] << 24) + (header[2] << 16)
		+ (header[1] << 8) + header[0];
	// �w�b�_�̃o�C�g����OS/2 or Windows�𔻕�
	if(info_header_size == OS2_BMP_INFO_HEADER_SIZE)
	{
		bmp_type = BMP_TYPE_OS2;
		(void)read_func(header, 1, OS2_BMP_INFO_HEADER_SIZE - 4, stream);
		local_width = (header[1] << 8) + header[0];
		local_height = (header[3] << 8) + header[2];
		if(header[4] != 1)
		{
			return NULL;
		}
		bit_per_pixel = (header[7] << 8) + header[6];
	}
	else if(info_header_size == MS_BMP_INFO_HEADER_SIZE)
	{
		bmp_type = BMP_TYPE_MS;
		(void)read_func(header, 1, MS_BMP_INFO_HEADER_SIZE - 4, stream);
		local_width = (header[3] << 24) + (header[2] << 16)
			+ (header[1] << 8) + header[0];
		local_height = (header[7] << 24) + (header[6] << 16)
			+ (header[5] << 8) + header[4];
		if(local_height < 0)
		{
			reverse_y = TRUE;
			local_height = - local_height;
		}

		if(header[8] != 1)
		{
			return NULL;
		}

		bit_per_pixel = (header[11] << 8) + header[10];
		compress_type = (eBMP_COMPRESS_TYPE)((header[15] << 24) + (header[14] << 16)
			+ (header[13] << 8) + header[12]);

		if(resolution != NULL)
		{
			*resolution = (header[23] << 24) + (header[22] << 16)
				+ (header[21] << 8) + header[20];
			*resolution = (int)(*resolution * 0.0254 + 0.5);
		}
	}
	else
	{
		return NULL;
	}

	// �`�����l�����̐ݒ�
	switch(bit_per_pixel)
	{
	case 1:
		local_channel = 1;
		break;
	case 4:
	case 8:
	case 24:
		local_channel = 3;
		break;
	case 32:
		local_channel = 4;
		break;
	default:
		return NULL;
	}

	if(width != NULL)
	{
		*width = local_width;
	}
	if(height != NULL)
	{
		*height = local_height;
	}
	if(channel != NULL)
	{
		*channel = local_channel;
	}
	if(compress_type == BMP_NO_COMPRESS)
	{
		stride = local_channel * local_width;
	}
	else
	{
		stride = local_channel * ((local_width * 8 + 31) / 32 * 4);
	}

	(void)seek_func(stream, data_offset, SEEK_SET);

	// �s�N�Z���f�[�^�̃������m��
	pixels = (uint8*)MEM_ALLOC_FUNC(stride * local_height);

	// �s�N�Z���f�[�^�̓ǂݍ���
	switch(compress_type)
	{
	case BMP_NO_COMPRESS:
		if(bit_per_pixel == 24 || bit_per_pixel == 32)
		{
			(void)read_func(pixels, 1, stride * local_height, stream);
		}
		else if(bit_per_pixel == 1)
		{
			BIT_STREAM bit_stream = {0};
			uint8 *bmp_data = (uint8*)MEM_ALLOC_FUNC(data_size - data_offset);
			(void)read_func(bmp_data, 1, data_size - data_offset, stream);

			bit_stream.bytes = bmp_data;
			bit_stream.num_bytes = data_size - data_offset;

			(void)memset(pixels, 0, stride * local_height);
			for(i=0; i<local_width * local_height; i++)
			{
				if(BitsRead(&bit_stream, 1) != 0)
				{
					pixels[i] = 0xff;
				}
			}

			MEM_FREE_FUNC(bmp_data);
		}
		else
		{
			MEM_FREE_FUNC(pixels);
			return NULL;
		}
		break;
	case BMP_COMPRESS_RLE8:
		{
			uint8 *bmp_data = (uint8*)MEM_ALLOC_FUNC(data_size - data_offset);
			(void)read_func(bmp_data, 1, data_size - data_offset, stream);

			DecodeBmpRLE8(bmp_data, pixels, local_width, local_height, stride);
			MEM_FREE_FUNC(bmp_data);
		}
		break;
	case BMP_COMPRESS_RLE4:
		{
			uint8 *bmp_data = (uint8*)MEM_ALLOC_FUNC(data_size - data_offset);
			(void)read_func(bmp_data, 1, data_size - data_offset, stream);

			DecodeBmpRLE4(bmp_data, pixels, local_width, local_height, stride);
			MEM_FREE_FUNC(bmp_data);
		}
		break;
	}

	if(reverse_y != FALSE)
	{
		int copy_stride = local_width * local_channel;
		uint8 *copy_pixels = (uint8*)MEM_ALLOC_FUNC(copy_stride * local_height);

		for(i=0; i<local_height; i++)
		{
			(void)memcpy(&copy_pixels[i*copy_stride], &pixels[(local_height-i-1)*stride], copy_stride);
		}

		MEM_FREE_FUNC(pixels);
		pixels = copy_pixels;
	}
	else
	{
		int copy_stride = local_width * local_channel;
		uint8 *copy_pixels = (uint8*)MEM_ALLOC_FUNC(copy_stride * local_height);

		for(i=0; i<local_height; i++)
		{
			(void)memcpy(&copy_pixels[i*copy_stride], &pixels[i*stride], copy_stride);
		}

		MEM_FREE_FUNC(pixels);
		pixels = copy_pixels;
	}

	if(local_channel >= 3)
	{
		uint8 r;
		for(i=0; i<local_width*local_height; i++)
		{
			r = pixels[i*local_channel];
			pixels[i*local_channel] = pixels[i*local_channel+2];
			pixels[i*local_channel+2] = r;
		}
	}

	return pixels;
}

#ifdef __cplusplus
}
#endif
