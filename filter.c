// Visual Studio 2005�ȍ~�ł͌Â��Ƃ����֐����g�p����̂�
	// �x�����o�Ȃ��悤�ɂ���
#if defined _MSC_VER && _MSC_VER >= 1400
# define _CRT_SECURE_NO_DEPRECATE
#endif

#include <math.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <zlib.h>
#include "configure.h"
#include "application.h"
#include "memory_stream.h"
#include "memory.h"
#include "image_read_write.h"
#include "filter.h"
#include "utils.h"
#include "color.h"
#include "bezier.h"
#include "fractal_editor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _FILTER_HISTORY_DATA
{
	uint16 num_layer;			// �t�B���^�[��K�p�������C���[�̐�
	uint16* name_length;		// ���C���[�̖��O�̒���
	size_t filter_data_size;	// �t�B���^�[�ň����f�[�^�̃o�C�g��
	void* filter_data;			// �t�B���^�[�̃f�[�^
	char** names;				// ���C���[�̖��O
	int32* data_length;			// �s�N�Z���f�[�^�̒���
	uint8** pixels;				// ���C���[�̃s�N�Z���f�[�^
} FILTER_HISTORY_DATA;

/*****************************
* FilterHistoryUndo�֐�      *
* �t�B���^�[�K�p�O�ɖ߂�     *
* ����                       *
* window	: �`��̈�̏�� *
* p			: �����f�[�^     *
*****************************/
static void FilterHistoryUndo(DRAW_WINDOW* window, void* p)
{
	// �f�[�^�ǂݍ��ݗp�̃X�g���[��
	MEMORY_STREAM stream;
	// �f�[�^��K�p���郌�C���[
	LAYER* layer;
	// �X�g���[���̃f�[�^�T�C�Y�擾�p
	size_t data_size;
	// ���C���[�̐�
	uint16 num_layer;
	// ���C���[���̒���
	uint16 layer_name_length;
	// ���C���[�̕��A�����A��s���̃o�C�g��
	int32 width, height, stride;
	// ���C���[��
	char layer_name[4096];
	// �s�N�Z���f�[�^
	uint8* pixels;
	// for���p�̃J�E���^
	unsigned int i;

	// �X�g���[���̏����ݒ�
	stream.buff_ptr = (uint8*)p;
	stream.data_size = sizeof(stream.data_size);
	stream.data_point = 0;

	// �X�g���[���̑��o�C�g����ǂݍ���
	(void)MemRead(&data_size, sizeof(data_size), 1, &stream);
	stream.data_size = data_size;

	// �t�B���^�[�֐��̃C���f�b�N�X��ǂݔ�΂�
	(void)MemSeek(&stream, sizeof(uint32), SEEK_CUR);

	// �t�B���^�[�̑���f�[�^��ǂݔ�΂�
	(void)MemRead(&data_size, sizeof(data_size), 1, &stream);
	(void)MemSeek(&stream, (long)data_size, SEEK_CUR);

	// PNG�f�[�^��ǂݍ���Ń��C���[�ɃR�s�[����
		// �K�p���郌�C���[�̐��ǂݍ���
	(void)MemRead(&num_layer, sizeof(num_layer), 1, &stream);
	for(i=0; i<num_layer; i++)
	{
		// ���̃��C���[�f�[�^�̈ʒu
		size_t next_data_pos;
		// ���C���[���̒����ǂݍ���
		(void)MemRead(&layer_name_length, sizeof(layer_name_length), 1, &stream);
		// ���C���[���ǂݍ���
		(void)MemRead(layer_name, 1, layer_name_length, &stream);
		// ���C���[��T��
		layer = SearchLayer(window->layer, layer_name);
		// PNG�̃o�C�g����ǂݍ���
		(void)MemRead(&next_data_pos, sizeof(next_data_pos), 1, &stream);
		next_data_pos += stream.data_point;
		// �s�N�Z���f�[�^�ǂݍ���
		pixels = ReadPNGStream((void*)&stream, (stream_func_t)MemRead,
			&width, &height, &stride);
		// �s�N�Z���f�[�^�R�s�[
		(void)memcpy(layer->pixels, pixels, stride*height);

		MEM_FREE_FUNC(pixels);

		// ���̃��C���[�f�[�^�Ɉړ�
		(void)MemSeek(&stream, (long)next_data_pos, SEEK_SET);
	}

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
}

/*****************************
* FilterHistoryRedo�֐�      *
* �t�B���^�[�̓K�p����蒼�� *
* ����                       *
* window	: �`��̈�̏�� *
* p			: �����f�[�^     *
*****************************/
static void FilterHistoryRedo(DRAW_WINDOW* window, void* p)
{
	// �f�[�^�ǂݍ��ݗp�̃X�g���[��
	MEMORY_STREAM stream;
	// �t�B���^�[��K�p���郌�C���[�z��
	LAYER** layers;
	// �f�[�^�̑��o�C�g��
	size_t data_size;
	// ���C���[�̐�
	uint16 num_layer;
	// �֐��|�C���^�̃C���f�b�N�X
	uint32 func_id;
	// �t�B���^�[����f�[�^�̈ʒu
	size_t filter_data_pos;
	// ���C���[��
	char layer_name[4096];
	// ���C���[���̒���
	uint16 layer_name_length;
	// for���p�̃J�E���^
	unsigned int i;

	// �X�g���[���̏����ݒ�
	stream.buff_ptr = (uint8*)p;
	stream.data_size = sizeof(stream.data_size);
	stream.data_point = 0;

	// �X�g���[���̑��o�C�g����ǂݍ���
	(void)MemRead(&data_size, sizeof(data_size), 1, &stream);
	stream.data_size = data_size;

	// �֐��|�C���^�̃C���f�b�N�X��ǂݍ���
	(void)MemRead(&func_id, sizeof(func_id), 1, &stream);

	// �t�B���^�[����̃f�[�^�ʒu���L��
	(void)MemRead(&data_size, sizeof(data_size), 1, &stream);
	filter_data_pos = stream.data_point;

	// ���C���[�̐���ǂݍ���
	(void)MemSeek(&stream, (long)data_size, SEEK_CUR);
	(void)MemRead(&num_layer, sizeof(num_layer), 1, &stream);

	// ���C���[�z��̃������m��
	layers = (LAYER**)MEM_ALLOC_FUNC(sizeof(*layers)*num_layer);

	for(i=0; i<num_layer; i++)
	{
		// ���C���[���̒����ǂݍ���
		(void)MemRead(&layer_name_length, sizeof(layer_name_length), 1, &stream);
		// ���C���[���ǂݍ���
		(void)MemRead(layer_name, 1, layer_name_length, &stream);
		// ���C���[��T��
		layers[i] = SearchLayer(window->layer, layer_name);
		// PNG�f�[�^��ǂݔ�΂�
		(void)MemRead(&data_size, sizeof(data_size), 1, &stream);
		(void)MemSeek(&stream, (long)data_size, SEEK_CUR);
	}

	// �t�B���^�[���ēK�p
	g_filter_funcs[func_id](window, layers, num_layer, (void*)&stream.buff_ptr[filter_data_pos]);

	MEM_FREE_FUNC(layers);

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
}

/*******************************************************************
* AddFilterHistory�֐�                                             *
* �t�B���^�[�K�p�O��̗������쐬                                   *
* ����                                                             *
* filter_name			: �t�B���^�[�̖��O                         *
* operation_data		: �t�B���^�[����f�[�^                     *
* operation_data_size	: �t�B���^�[����f�[�^�̃o�C�g��           *
* filter_func_id		: �t�B���^�[�֐��|�C���^�z��̃C���f�b�N�X *
* layers				: �t�B���^�[��K�p���郌�C���[�̔z��       *
* num_layer				: �t�B���^�[��K�p���郌�C���[�̐�         *
* window				: �`��̈�̏��                           *
*******************************************************************/
void AddFilterHistory(
	const char* filter_name,
	void* operation_data,
	size_t operation_data_size,
	uint32 filter_func_id,
	LAYER** layers,
	uint16 num_layer,
	DRAW_WINDOW* window
)
{
// �f�[�^�̈��k���x��
#define DATA_COMPRESSION_LEVEL Z_DEFAULT_COMPRESSION
	// �����f�[�^�쐬�p�̃X�g���[��
	MEMORY_STREAM_PTR stream =
		CreateMemoryStream(num_layer * (window->pixel_buf_size + 256)
			+ operation_data_size + sizeof(size_t));
	// �f�[�^�̑��o�C�g��
	size_t data_size;
	// ���C���[���̒���
	uint16 name_length;
	// PNG�f�[�^�̃o�C�g���v�Z�p
	size_t before_pos;
	// for���p�̃J�E���^
	unsigned int i;

	// �擪�ɑ��o�C�g�����������ނ���4�o�C�g������
	(void)MemSeek(stream, sizeof(size_t), SEEK_SET);

	// �t�B���^�[�֐��̃C���f�b�N�X�������o��
	(void)MemWrite(&filter_func_id, sizeof(filter_func_id), 1, stream);

	// ����f�[�^�̃o�C�g������������
	(void)MemWrite(&operation_data_size, sizeof(operation_data_size), 1, stream);

	// ����f�[�^����������
	(void)MemWrite(operation_data, 1, operation_data_size, stream);

	// ���C���[�̐�����������
	(void)MemWrite(&num_layer, sizeof(num_layer), 1, stream);

	// ���C���[�f�[�^����������
	for(i=0; i<num_layer; i++)
	{
		// �܂��͖��O�̒����������o��
		name_length = (uint16)strlen(layers[i]->name) + 1;
		(void)MemWrite(&name_length, sizeof(name_length), 1, stream);

		// ���O�����o��
		(void)MemWrite(layers[i]->name, 1, name_length, stream);

		// PNG�f�[�^�̃o�C�g�����L������X�y�[�X���J����
		before_pos = stream->data_point;
		(void)MemSeek(stream, sizeof(before_pos), SEEK_CUR);

		// PNG�f�[�^�Ńs�N�Z�������L��
		WritePNGStream(stream, (stream_func_t)MemWrite, NULL, layers[i]->pixels,
			layers[i]->width, layers[i]->height, layers[i]->stride, layers[i]->channel, 0,
			DATA_COMPRESSION_LEVEL
		);

		// PNG�̃o�C�g�����v�Z���ď�������
		data_size = stream->data_point - before_pos - sizeof(data_size);
		(void)MemSeek(stream, (long)before_pos, SEEK_SET);
		(void)MemWrite(&data_size, sizeof(data_size), 1, stream);

		// ���̈ʒu�ɖ߂�
		(void)MemSeek(stream, (long)data_size, SEEK_CUR);
	}

	// ���o�C�g������������
	data_size = stream->data_point;
	(void)MemSeek(stream, 0, SEEK_SET);
	(void)MemWrite(&data_size, sizeof(data_size), 1, stream);

	// ����z��ɒǉ�
	AddHistory(&window->history, filter_name, stream->buff_ptr, (uint32)data_size,
		FilterHistoryUndo, FilterHistoryRedo);

	DeleteMemoryStream(stream);
}

typedef struct _SELECTION_FILTER_HISTORY_DATA
{
	size_t filter_data_size;	// �t�B���^�[�ň����f�[�^�̃o�C�g��
	void* filter_data;			// �t�B���^�[�̃f�[�^
	int32 data_length;			// �s�N�Z���f�[�^�̒���
	uint8* pixels;				// ���C���[�̃s�N�Z���f�[�^
} SELECTION_FILTER_HISTORY_DATA;

/*************************************
* SelectonFilterHistoryUndo�֐�      *
* �I��͈͂ւ̃t�B���^�[�K�p�O�ɖ߂� *
* ����                               *
* window	: �`��̈�̏��         *
* p			: �����f�[�^             *
*************************************/
static void SelectionFilterHistoryUndo(DRAW_WINDOW* window, void* p)
{
	// �f�[�^�ǂݍ��ݗp�̃X�g���[��
	MEMORY_STREAM stream;
	// �X�g���[���̃f�[�^�T�C�Y�擾�p
	size_t data_size;
	// ���C���[�̕��A�����A��s���̃o�C�g��
	int32 width, height, stride;
	// �s�N�Z���f�[�^
	uint8* pixels;
	// ���̃f�[�^�̈ʒu
	size_t next_data_pos;

	// �X�g���[���̏����ݒ�
	stream.buff_ptr = (uint8*)p;
	stream.data_size = sizeof(stream.data_size);
	stream.data_point = 0;

	// �X�g���[���̑��o�C�g����ǂݍ���
	(void)MemRead(&data_size, sizeof(data_size), 1, &stream);
	stream.data_size = data_size;

	// �t�B���^�[�֐��̃C���f�b�N�X��ǂݔ�΂�
	(void)MemSeek(&stream, sizeof(uint32), SEEK_CUR);

	// �t�B���^�[�̑���f�[�^��ǂݔ�΂�
	(void)MemRead(&data_size, sizeof(data_size), 1, &stream);
	(void)MemSeek(&stream, (long)data_size, SEEK_CUR);

	// PNG�f�[�^��ǂݍ���őI��͈͂ɃR�s�[����
		// PNG�̃o�C�g����ǂݍ���
	(void)MemRead(&next_data_pos, sizeof(next_data_pos), 1, &stream);
	// �s�N�Z���f�[�^�ǂݍ���
	pixels = ReadPNGStream((void*)&stream, (stream_func_t)MemRead,
		&width, &height, &stride);
	// �s�N�Z���f�[�^�R�s�[
	(void)memcpy(window->selection->pixels, pixels, stride*height);

	if(UpdateSelectionArea(&window->selection_area, window->selection, window->temp_layer) == FALSE)
	{
		window->flags &= ~(DRAW_WINDOW_HAS_SELECTION_AREA);
	}
	else
	{
		window->flags |= DRAW_WINDOW_HAS_SELECTION_AREA;
	}

	MEM_FREE_FUNC(pixels);

	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_OVER;
}

/*****************************************
* SelectionFilterHistoryRedo�֐�         *
* �I��͈͂ւ̃t�B���^�[�̓K�p����蒼�� *
* ����                                   *
* window	: �`��̈�̏��             *
* p			: �����f�[�^                 *
*****************************************/
static void SelectionFilterHistoryRedo(DRAW_WINDOW* window, void* p)
{
	// �f�[�^�ǂݍ��ݗp�̃X�g���[��
	MEMORY_STREAM stream;
	// �f�[�^�̑��o�C�g��
	size_t data_size;
	// �֐��|�C���^�̃C���f�b�N�X
	uint32 func_id;
	// �t�B���^�[����f�[�^�̈ʒu
	size_t filter_data_pos;

	// �X�g���[���̏����ݒ�
	stream.buff_ptr = (uint8*)p;
	stream.data_size = sizeof(stream.data_size);
	stream.data_point = 0;

	// �X�g���[���̑��o�C�g����ǂݍ���
	(void)MemRead(&data_size, sizeof(data_size), 1, &stream);
	stream.data_size = data_size;

	// �֐��|�C���^�̃C���f�b�N�X��ǂݍ���
	(void)MemRead(&func_id, sizeof(func_id), 1, &stream);

	// �t�B���^�[����̃f�[�^�ʒu���L��
	(void)MemRead(&data_size, sizeof(data_size), 1, &stream);
	filter_data_pos = stream.data_point;

	// PNG�f�[�^��ǂݔ�΂�
	(void)MemRead(&data_size, sizeof(data_size), 1, &stream);
	(void)MemSeek(&stream, (long)data_size, SEEK_CUR);

	// �t�B���^�[���ēK�p
	g_selection_filter_funcs[func_id](window, (void*)&stream.buff_ptr[filter_data_pos]);

	if(UpdateSelectionArea(&window->selection_area, window->selection, window->temp_layer) == FALSE)
	{
		window->flags &= ~(DRAW_WINDOW_HAS_SELECTION_AREA);
	}
	else
	{
		window->flags |= DRAW_WINDOW_HAS_SELECTION_AREA;
	}

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_OVER;
}

/*******************************************************************
* AddSelectionFilterHistory�֐�                                    *
* �I��͈͂ւ̃t�B���^�[�K�p�O��̗������쐬                       *
* ����                                                             *
* filter_name			: �t�B���^�[�̖��O                         *
* operation_data		: �t�B���^�[����f�[�^                     *
* operation_data_size	: �t�B���^�[����f�[�^�̃o�C�g��           *
* filter_func_id		: �t�B���^�[�֐��|�C���^�z��̃C���f�b�N�X *
* window				: �`��̈�̏��                           *
*******************************************************************/
void AddSelectionFilterHistory(
	const char* filter_name,
	void* operation_data,
	size_t operation_data_size,
	uint32 filter_func_id,
	DRAW_WINDOW* window
)
{
// �f�[�^�̈��k���x��
#define DATA_COMPRESSION_LEVEL Z_DEFAULT_COMPRESSION
	// �����f�[�^�쐬�p�̃X�g���[��
	MEMORY_STREAM_PTR stream =
		CreateMemoryStream(window->pixel_buf_size + 256
			+ operation_data_size + sizeof(size_t));
	// �f�[�^�̑��o�C�g��
	size_t data_size;
	// PNG�f�[�^�̃o�C�g���v�Z�p
	size_t before_pos;

	// �擪�ɑ��o�C�g�����������ނ���4�o�C�g������
	(void)MemSeek(stream, sizeof(size_t), SEEK_SET);

	// �t�B���^�[�֐��̃C���f�b�N�X�������o��
	(void)MemWrite(&filter_func_id, sizeof(filter_func_id), 1, stream);

	// ����f�[�^�̃o�C�g������������
	(void)MemWrite(&operation_data_size, sizeof(operation_data_size), 1, stream);

	// ����f�[�^����������
	(void)MemWrite(operation_data, 1, operation_data_size, stream);

	// �I��͈͂̃f�[�^����������
		// PNG�f�[�^�̃o�C�g�����L������X�y�[�X���J����
	before_pos = stream->data_point;
	(void)MemSeek(stream, sizeof(before_pos), SEEK_CUR);

	// PNG�f�[�^�Ńs�N�Z�������L��
	WritePNGStream(stream, (stream_func_t)MemWrite, NULL, window->selection->pixels,
		window->selection->width, window->selection->height, window->selection->stride, window->selection->channel, 0,
		DATA_COMPRESSION_LEVEL
	);

	// PNG�̃o�C�g�����v�Z���ď�������
	data_size = stream->data_point - before_pos - sizeof(data_size);
	(void)MemSeek(stream, (long)before_pos, SEEK_SET);
	(void)MemWrite(&data_size, sizeof(data_size), 1, stream);

	// ���̈ʒu�ɖ߂�
	(void)MemSeek(stream, (long)data_size, SEEK_CUR);

	// ���o�C�g������������
	data_size = stream->data_point;
	(void)MemSeek(stream, 0, SEEK_SET);
	(void)MemWrite(&data_size, sizeof(data_size), 1, stream);

	// ����z��ɒǉ�
	AddHistory(&window->history, filter_name, stream->buff_ptr, (uint32)data_size,
		SelectionFilterHistoryUndo, SelectionFilterHistoryRedo);

	DeleteMemoryStream(stream);
}

/*************************************************
* BlurFilterOneStep�֐�                          *
* �ڂ�����1�X�e�b�v�����s                        *
* ����                                           *
* layer	: �ڂ�����K�p���郌�C���[               *
* buff	: �K�p��̃s�N�Z���f�[�^�����郌�C���[ *
* size	: �ڂ�����̐F�����肷��s�N�Z���T�C�Y   *
*************************************************/
void BlurFilterOneStep(LAYER* layer, LAYER* buff, int size)
{
	// for���p�̃J�E���^
	int y;

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for(y=0; y<layer->height; y++)
	{	// �s�N�Z�����ӂ̍��v�l
		int sum[4];
		// �s�N�Z���l�̍��v�v�Z�Ɏg�p�����s�N�Z����
		int num_pixels;
		// �s�N�Z���l�̍��v�v�Z�̊J�n�E�I��
		int start_x, end_x, start_y, end_y;
		// for���p�̃J�E���^
		int x, i, j;


		for(x=0; x<layer->width; x++)
		{
			start_x = x - size;
			if(start_x < 0)
			{
				start_x = 0;
			}
			end_x = x + size;
			if(end_x >= layer->width)
			{
				end_x = layer->width - 1;
			}

			start_y = y - size;
			if(start_y < 0)
			{
				start_y = 0;
			}
			end_y = y + size;
			if(end_y >= layer->height)
			{
				end_y = layer->height - 1;
			}

			num_pixels = 0;
			sum[0] = sum[1] = sum[2] = sum[3] = 0;
			for(i=start_y; i<=end_y; i++)
			{
				for(j=start_x; j<=end_x; j++)
				{
					sum[0] += layer->pixels[i*layer->stride+j*4];
					sum[1] += layer->pixels[i*layer->stride+j*4+1];
					sum[2] += layer->pixels[i*layer->stride+j*4+2];
					sum[3] += layer->pixels[i*layer->stride+j*4+3];
					num_pixels++;
				}
			}

			buff->pixels[y*layer->stride+x*4] = sum[0] / num_pixels;
			buff->pixels[y*layer->stride+x*4+1] = sum[1] / num_pixels;
			buff->pixels[y*layer->stride+x*4+2] = sum[2] / num_pixels;
			buff->pixels[y*layer->stride+x*4+3] = sum[3] / num_pixels;
		}
	}
}

/*********************************************************
* ApplyBlurFilter�֐�                                    *
* �ڂ����t�B���^�[��K�p����                             *
* ����                                                   *
* target	: �ڂ����t�B���^�[��K�p���郌�C���[         *
* size		: �ڂ����t�B���^�[�ŕ��ϐF���v�Z�����`�͈� *
*********************************************************/
void ApplyBlurFilter(LAYER* target, int size)
{
	(void)memcpy(target->window->mask_temp->pixels, target->pixels,
		target->stride * target->height);
	BlurFilterOneStep(target->window->mask_temp, target, size);
}

typedef struct _BLUR_FILTER_DATA
{
	uint16 loop;
	uint16 size;
} BLUR_FILTER_DATA;

/*************************************
* BlurFilter�֐�                     *
* �ڂ�������                         *
* ����                               *
* window	: �`��̈�̏��         *
* layers	: �������s�����C���[�z�� *
* num_layer	: �������s�����C���[�̐� *
* data		: �ڂ��������̏ڍ׃f�[�^ *
*************************************/
void BlurFilter(DRAW_WINDOW* window, LAYER** layers, uint16 num_layer, void* data)
{
	// �ڂ��������̏ڍ׃f�[�^
	BLUR_FILTER_DATA* blur = (BLUR_FILTER_DATA*)data;
	// for���p�̃J�E���^
	int i, j;

	// �e���C���[�ɑ΂�
	for(i=0; i<(int)num_layer; i++)
	{	// �ڂ����������s��
			// ���݂̃A�N�e�B�u���C���[�̃s�N�Z�����ꎞ�ۑ��ɃR�s�[
		if(layers[i]->layer_type == TYPE_NORMAL_LAYER)
		{
			(void)memcpy(window->temp_layer->pixels, layers[i]->pixels, window->pixel_buf_size);
			for(j=0; j<blur->loop; j++)
			{
				BlurFilterOneStep(window->temp_layer, window->mask_temp, blur->size);
				(void)memcpy(window->temp_layer->pixels, window->mask_temp->pixels, window->pixel_buf_size);
			}

			if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) == 0)
			{
				(void)memcpy(layers[i]->pixels, window->temp_layer->pixels, window->pixel_buf_size);
			}
			else
			{
				uint8 select_value;

#ifdef _OPENMP
#pragma omp parallel for
#endif
				for(j=0; j<(int)window->width*window->height; j++)
				{
					select_value = window->selection->pixels[j];
					layers[i]->pixels[j*4+0] = ((0xff-select_value)*layers[i]->pixels[j*4+0]
						+ window->temp_layer->pixels[j*4+0]*select_value) / 255;
					layers[i]->pixels[j*4+1] = ((0xff-select_value)*layers[i]->pixels[j*4+1]
						+ window->temp_layer->pixels[j*4+1]*select_value) / 255;
					layers[i]->pixels[j*4+2] = ((0xff-select_value)*layers[i]->pixels[j*4+2]
						+ window->temp_layer->pixels[j*4+2]*select_value) / 255;
					layers[i]->pixels[j*4+3] = ((0xff-select_value)*layers[i]->pixels[j*4+3]
						+ window->temp_layer->pixels[j*4+3]*select_value) / 255;
				}
			}
		}
	}

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	gtk_widget_queue_draw(window->window);
}

/*************************************
* SelectionBlurFilter�֐�            *
* �I��͈͂̂ڂ�������               *
* ����                               *
* window	: �`��̈�̏��         *
* layers	: �������s�����C���[�z�� *
* num_layer	: �������s�����C���[�̐� *
* data		: �ڂ��������̏ڍ׃f�[�^ *
*************************************/
void SelectionBlurFilter(DRAW_WINDOW* window, void* data)
{
	// �ڂ��������̏ڍ׃f�[�^
	BLUR_FILTER_DATA* blur = (BLUR_FILTER_DATA*)data;
	// for���p�̃J�E���^
	unsigned int i;

	(void)memcpy(window->temp_layer->pixels, window->selection->pixels, window->width * window->height);
	window->temp_layer->channel = 1;
	window->temp_layer->stride = window->width;
	for(i=0; i<blur->loop; i++)
	{
		BlurFilterOneStep(window->temp_layer, window->mask_temp, blur->size);
		(void)memcpy(window->temp_layer->pixels, window->mask_temp->pixels, window->width * window->height);
	}
	window->temp_layer->channel = 4;
	window->temp_layer->stride = window->temp_layer->channel * window->width;

	(void)memcpy(window->selection->pixels, window->temp_layer->pixels, window->width * window->height);

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_OVER;
	gtk_widget_queue_draw(window->window);
}

/*****************************************************
* ExecuteBlurFilter�֐�                              *
* �ڂ����t�B���^�����s                               *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
void ExecuteBlurFilter(APPLICATION* app)
{
// �s�N�Z������ő�͈�
#define MAX_SIZE 100
// �J��Ԃ��ő吔
#define MAX_REPEAT 100
	// �g�傷��s�N�Z�������w�肷��_�C�A���O
	GtkWidget* dialog = gtk_dialog_new_with_buttons(
		app->labels->menu.blur,
		GTK_WINDOW(app->window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL
	);
	// �s�N�Z�����w��p�̃E�B�W�F�b�g
	GtkWidget* label, *spin, *hbox, *size;
	// �s�N�Z�����w��X�s���{�^���̃A�W���X�^
	GtkAdjustment* adjust;
	// ���x���p�̃o�b�t�@
	char str[4096];
	// �_�C�A���O�̌���
	gint result;

	// �_�C�A���O�ɃE�B�W�F�b�g������
		// ����T�C�Y
	hbox = gtk_hbox_new(FALSE, 0);
	(void)sprintf(str, "%s :", app->labels->tool_box.scale);
	label = gtk_label_new(str);
	adjust = GTK_ADJUSTMENT(gtk_adjustment_new(1, 1, MAX_SIZE, 1, 1, 0));
	size = gtk_spin_button_new(adjust, 1, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), size, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox, FALSE, TRUE, 0);

	// �K�p��
	hbox = gtk_hbox_new(FALSE, 0);
	(void)sprintf(str, "%s :", app->labels->unit.repeat);
	label = gtk_label_new(str);
	adjust = GTK_ADJUSTMENT(gtk_adjustment_new(1, 1, MAX_REPEAT, 1, 1, 0));
	spin = gtk_spin_button_new(adjust, 1, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), hbox, FALSE, TRUE, 0);
	gtk_widget_show_all(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));

	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if((app->draw_window[app->active_window]->flags & DRAW_WINDOW_EDIT_SELECTION) == 0)
	{
		if(app->draw_window[app->active_window]->active_layer->layer_type == TYPE_NORMAL_LAYER)
		{
			if(result == GTK_RESPONSE_ACCEPT)
			{	// O.K.�{�^���������ꂽ
				DRAW_WINDOW* window =	// ��������`��̈�
					app->draw_window[app->active_window];
				// �J��Ԃ���
				BLUR_FILTER_DATA loop = {
					(uint16)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin)),
					(uint16)gtk_spin_button_get_value(GTK_SPIN_BUTTON(size))
				};
				// ���C���[�̐�
				uint16 num_layer;
				// ���������s���郌�C���[
				LAYER** layers = GetLayerChain(window, &num_layer);

				// ��ɗ����f�[�^���c��
				AddFilterHistory(app->labels->menu.blur, &loop, sizeof(loop),
					FILTER_FUNC_BLUR, layers, num_layer, window);

				// �ڂ����t�B���^�[���s
				BlurFilter(window, layers, num_layer, &loop);

				MEM_FREE_FUNC(layers);

				// �L�����o�X���X�V
				window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
			}
		}
	}
	else
	{
		if(result == GTK_RESPONSE_ACCEPT)
		{	// O.K.�{�^���������ꂽ
			DRAW_WINDOW* window =	// ��������`��̈�
				app->draw_window[app->active_window];
			// �J��Ԃ���
			BLUR_FILTER_DATA loop = {(uint16)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin))};

			// ��ɗ����f�[�^���c��
			AddSelectionFilterHistory(app->labels->menu.blur, &loop, sizeof(loop),
				FILTER_FUNC_BLUR, window);

			// �ڂ����t�B���^�[���s
			SelectionBlurFilter(window, (void*)&loop);
		}
	}

	// �_�C�A���O������
	gtk_widget_destroy(dialog);
}

typedef enum _eMOTION_BLUR_TYPE
{
	MOTION_BLUR_STRAGHT,
	MOTION_BLUR_STRAGHT_RANDOM,
	MOTION_BLUR_ROTATE,
	MOTION_BLUR_GROW
} eMOTION_BLUR_TYPE;

typedef enum _eMOTION_BLUR_ROTATE_MODE
{
	MOTION_BLUR_ROTATE_CLOCKWISE,
	MOTION_BLUR_ROTATE_COUNTER_CLOCKWISE,
	MOTION_BLUR_ROTATE_BOTH_DIRECTION
} eMOTION_BLUR_ROTATE_MODE;

typedef enum _eMOTION_BLUR_FLAGS
{
	MOTION_BLUR_BIDIRECTION = 0x01
} eMOTION_BLUR_FLAGS;

typedef struct _MOTION_BLUR
{
	uint8 type;
	uint8 rotate_mode;
	uint16 size;
	int16 angle;
	int32 center_x;
	int32 center_y;
	guint32 flags;
	GtkWidget *detail_ui;
	GtkWidget *detail_ui_box;
	GtkWidget *preview;
	uint8 **before_pixels;
} MOTION_BLUR;

/*************************************
* MotionBlurFilter�֐�               *
* ���[�V�����ڂ����t�B���^�[��K�p   *
* ����                               *
* window	: �`��̈�̏��         *
* layers	: �������s�����C���[�z�� *
* num_layer	: �������s�����C���[�̐� *
* data		: �ڂ��������̏ڍ׃f�[�^ *
*************************************/
void MotionBlurFilter(DRAW_WINDOW* window, LAYER** layers, uint16 num_layer, void* data)
{
	MOTION_BLUR *filter_data = (MOTION_BLUR*)data;
	unsigned int sum_color[4];
	int i, j;

	for(i=0; i<num_layer; i++)
	{
		switch(filter_data->type)
		{
		case MOTION_BLUR_STRAGHT:
			{
				FLOAT_T check_x, check_y;
				FLOAT_T dx, dy;
				FLOAT_T angle;
				int length;
				int y;

				if((filter_data->flags & MOTION_BLUR_BIDIRECTION) != 0)
				{
					length = filter_data->size * 2 + 1;
				}
				else
				{
					length = filter_data->size + 1;
				}

				angle = filter_data->angle * G_PI / 180.0;
				dx = cos(angle),	dy = sin(angle);

#ifdef _OPENMP
#pragma omp parallel for
#endif
				for(y=0; y<layers[i]->height; y++)
				{
					int before_x, before_y;
					int int_x, int_y;
					int x;
					int count;

					for(x=0; x<layers[i]->width; x++)
					{
						check_x = x - dx * filter_data->size;
						check_y = y - dy * filter_data->size;
						before_x = -1,	before_y = -1;
						sum_color[0] = sum_color[1] = sum_color[2] = sum_color[3] = 0;
						count = 0;
						for(j=0; j<length; j++)
						{
							int_x = (int)check_x,	int_y = (int)check_y;
							if(int_x >= 0 && int_x < layers[i]->width
								&& int_y >= 0 && int_y < layers[i]->height)
							{
								if(int_x != before_x || int_y != before_y)
								{
									sum_color[0] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4];
									sum_color[1] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+1];
									sum_color[2] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+2];
									sum_color[3] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+3];
									count++;
									before_x = int_x,	before_y = int_y;
								}
							}
							check_x += dx,	check_y += dy;
						}

						if(count > 0)
						{
							window->temp_layer->pixels[y*layers[i]->stride+x*4] = sum_color[0] / count;
							window->temp_layer->pixels[y*layers[i]->stride+x*4+1] = sum_color[1] / count;
							window->temp_layer->pixels[y*layers[i]->stride+x*4+2] = sum_color[2] / count;
							window->temp_layer->pixels[y*layers[i]->stride+x*4+3] = sum_color[3] / count;
						}
					}
				}
			}
			break;
		case MOTION_BLUR_STRAGHT_RANDOM:
			{
				FLOAT_T add_x = 0, add_y = 0;
				FLOAT_T angle;
				FLOAT_T dx, dy;
				int y;

				angle = filter_data->angle * G_PI / 180.0;
				add_x = dx = cos(angle),	add_y = dy = sin(angle);
				if(add_x > 0 || add_y > 0)
				{
					add_x *= 2,	add_y *= 2;
					if(add_x > 0)
					{
						if(add_x > 1)
						{
							add_x /= add_x;
						}
					}
					else
					{
						if(add_x <= -2)
						{
							add_x /= - add_x;
						}
					}
					if(add_y > 0)
					{
						if(add_y > 1)
						{
							add_y /= add_y;
						}
					}
					else
					{
						if(add_y <= -2)
						{
							add_y /= - add_y;
						}
					}
				}

				(void)memset(window->mask->pixels, 0xff, window->width * window->height);

				if(add_x >= 0 && add_y >= 0)
				{
#ifdef _OPENMP
#pragma omp parallel for
#endif
					for(y=0; y<layers[i]->height; y++)
					{
						FLOAT_T check_x, check_y;
						int before_x, before_y;
						int int_x, int_y;
						int length;
						int x;
						int count;

						for(x=0; x<layers[i]->width; x++)
						{
							int_x = (int)(x - add_x),	int_y = (int)(y - add_y);
							if(int_x >= 0 && int_x < window->width && int_y >= 0 && int_y < window->height)
							{
								if(window->mask->pixels[int_y*window->width+int_x] != 0xff)
								{
									window->mask->pixels[y*window->width+x] = length =
										window->mask->pixels[int_y*window->width+int_x];
								}
								else
								{
									window->mask->pixels[y*window->width+x] = length =
										rand() % filter_data->size;
								}
							}
							else
							{
								window->mask->pixels[y*window->width+x] = length =
									rand() % filter_data->size;
							}

							check_x = x - dx * filter_data->size;
							check_y = y - dy * filter_data->size;
							before_x = -1,	before_y = -1;
							sum_color[0] = sum_color[1] = sum_color[2] = sum_color[3] = 0;
							count = 0;
							for(j=0; j<length; j++)
							{
								int_x = (int)check_x,	int_y = (int)check_y;
								if(int_x >= 0 && int_x < layers[i]->width
									&& int_y >= 0 && int_y < layers[i]->height)
								{
									if(int_x != before_x || int_y != before_y)
									{
										sum_color[0] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4];
										sum_color[1] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+1];
										sum_color[2] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+2];
										sum_color[3] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+3];
										count++;
										before_x = int_x,	before_y = int_y;
									}
								}
								check_x += dx,	check_y += dy;
							}

							if(count > 0)
							{
								window->temp_layer->pixels[y*layers[i]->stride+x*4] = sum_color[0] / count;
								window->temp_layer->pixels[y*layers[i]->stride+x*4+1] = sum_color[1] / count;
								window->temp_layer->pixels[y*layers[i]->stride+x*4+2] = sum_color[2] / count;
								window->temp_layer->pixels[y*layers[i]->stride+x*4+3] = sum_color[3] / count;
							}
						}
					}
				}
				else if(add_x < 0 && add_y >= 0)
				{
#ifdef _OPENMP
#pragma omp parallel for
#endif
					for(y=0; y<layers[i]->height; y++)
					{
						FLOAT_T check_x, check_y;
						int before_x, before_y;
						int int_x, int_y;
						int length;
						int x;
						int count;

						for(x=layers[i]->width-1; x>=0; x--)
						{
							int_x = (int)(x - add_x),	int_y = (int)(y - add_y);
							if(int_x >= 0 && int_x < window->width && int_y >= 0 && int_y < window->height)
							{
								if(window->mask->pixels[int_y*window->width+int_x] != 0xff)
								{
									window->mask->pixels[y*window->width+x] = length =
										window->mask->pixels[int_y*window->width+int_x];
								}
								else
								{
									window->mask->pixels[y*window->width+x] = length =
										rand() % filter_data->size;
								}
							}
							else
							{
								window->mask->pixels[y*window->width+x] = length =
									rand() % filter_data->size;
							}

							check_x = x - dx * filter_data->size;
							check_y = y - dy * filter_data->size;
							before_x = -1,	before_y = -1;
							sum_color[0] = sum_color[1] = sum_color[2] = sum_color[3] = 0;
							count = 0;
							for(j=0; j<length; j++)
							{
								int_x = (int)check_x,	int_y = (int)check_y;
								if(int_x >= 0 && int_x < layers[i]->width
									&& int_y >= 0 && int_y < layers[i]->height)
								{
									if(int_x != before_x || int_y != before_y)
									{
										sum_color[0] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4];
										sum_color[1] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+1];
										sum_color[2] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+2];
										sum_color[3] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+3];
										count++;
										before_x = int_x,	before_y = int_y;
									}
								}
								check_x += dx,	check_y += dy;
							}

							if(count > 0)
							{
								window->temp_layer->pixels[y*layers[i]->stride+x*4] = sum_color[0] / count;
								window->temp_layer->pixels[y*layers[i]->stride+x*4+1] = sum_color[1] / count;
								window->temp_layer->pixels[y*layers[i]->stride+x*4+2] = sum_color[2] / count;
								window->temp_layer->pixels[y*layers[i]->stride+x*4+3] = sum_color[3] / count;
							}
						}
					}
				}
				else if(add_x >= 0 && add_y < 0)
				{
#ifdef _OPENMP
#pragma omp parallel for
#endif
					for(y=layers[i]->height-1; y>=0; y--)
					{
						FLOAT_T check_x, check_y;
						int before_x, before_y;
						int int_x, int_y;
						int length;
						int x;
						int count;

						for(x=0; x<layers[i]->width; x++)
						{
							int_x = (int)(x - add_x),	int_y = (int)(y - add_y);
							if(int_x >= 0 && int_x < window->width && int_y >= 0 && int_y < window->height)
							{
								if(window->mask->pixels[int_y*window->width+int_x] != 0xff)
								{
									window->mask->pixels[y*window->width+x] = length =
										window->mask->pixels[int_y*window->width+int_x];
								}
								else
								{
									window->mask->pixels[y*window->width+x] = length =
										rand() % filter_data->size;
								}
							}
							else
							{
								window->mask->pixels[y*window->width+x] = length =
									rand() % filter_data->size;
							}

							check_x = x - dx * filter_data->size;
							check_y = y - dy * filter_data->size;
							before_x = -1,	before_y = -1;
							sum_color[0] = sum_color[1] = sum_color[2] = sum_color[3] = 0;
							count = 0;
							for(j=0; j<length; j++)
							{
								int_x = (int)check_x,	int_y = (int)check_y;
								if(int_x >= 0 && int_x < layers[i]->width
									&& int_y >= 0 && int_y < layers[i]->height)
								{
									if(int_x != before_x || int_y != before_y)
									{
										sum_color[0] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4];
										sum_color[1] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+1];
										sum_color[2] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+2];
										sum_color[3] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+3];
										count++;
										before_x = int_x,	before_y = int_y;
									}
								}
								check_x += dx,	check_y += dy;
							}

							if(count > 0)
							{
								window->temp_layer->pixels[y*layers[i]->stride+x*4] = sum_color[0] / count;
								window->temp_layer->pixels[y*layers[i]->stride+x*4+1] = sum_color[1] / count;
								window->temp_layer->pixels[y*layers[i]->stride+x*4+2] = sum_color[2] / count;
								window->temp_layer->pixels[y*layers[i]->stride+x*4+3] = sum_color[3] / count;
							}
						}
					}
				}
				else
				{
#ifdef _OPENMP
#pragma omp parallel for
#endif
					for(y=layers[i]->height-1; y>=0; y--)
					{
						FLOAT_T check_x, check_y;
						int before_x, before_y;
						int int_x, int_y;
						int length;
						int x;
						int count;

						for(x=layers[i]->width-1; x>=0; x--)
						{
							int_x = (int)(x - add_x),	int_y = (int)(y - add_y);
							if(int_x >= 0 && int_x < window->width && int_y >= 0 && int_y < window->height)
							{
								if(window->mask->pixels[int_y*window->width+int_x] != 0xff)
								{
									window->mask->pixels[y*window->width+x] = length =
										window->mask->pixels[int_y*window->width+int_x];
								}
								else
								{
									window->mask->pixels[y*window->width+x] = length =
										rand() % filter_data->size;
								}
							}
							else
							{
								window->mask->pixels[y*window->width+x] = length =
									rand() % filter_data->size;
							}

							check_x = x - dx * filter_data->size;
							check_y = y - dy * filter_data->size;
							before_x = -1,	before_y = -1;
							sum_color[0] = sum_color[1] = sum_color[2] = sum_color[3] = 0;
							count = 0;
							for(j=0; j<length; j++)
							{
								int_x = (int)check_x,	int_y = (int)check_y;
								if(int_x >= 0 && int_x < layers[i]->width
									&& int_y >= 0 && int_y < layers[i]->height)
								{
									if(int_x != before_x || int_y != before_y)
									{
										sum_color[0] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4];
										sum_color[1] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+1];
										sum_color[2] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+2];
										sum_color[3] += layers[i]->pixels[int_y*layers[i]->stride+int_x*4+3];
										count++;
										before_x = int_x,	before_y = int_y;
									}
								}
								check_x += dx,	check_y += dy;
							}

							if(count > 0)
							{
								window->temp_layer->pixels[y*layers[i]->stride+x*4] = sum_color[0] / count;
								window->temp_layer->pixels[y*layers[i]->stride+x*4+1] = sum_color[1] / count;
								window->temp_layer->pixels[y*layers[i]->stride+x*4+2] = sum_color[2] / count;
								window->temp_layer->pixels[y*layers[i]->stride+x*4+3] = sum_color[3] / count;
							}
						}
					}
				}
			}
			break;
		case MOTION_BLUR_ROTATE:
			{
				cairo_pattern_t *pattern;
				cairo_surface_t *pattern_surface;
				cairo_matrix_t matrix;
				uint8 select_value;
				int rotate_direction;
				FLOAT_T angle;
				FLOAT_T sin_value, cos_value;
				FLOAT_T alpha, alpha_minus;
				int pattern_width, pattern_height, pattern_stride;
				FLOAT_T half_width, half_height;
				int x, y;
				int sx, sy;

				if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) != 0)
				{
					pattern_width = window->selection_area.max_x - window->selection_area.min_x;
					pattern_stride = pattern_width * 4;
					pattern_height = window->selection_area.max_y - window->selection_area.min_y;

					for(y=window->selection_area.min_y, sy=0; y<window->selection_area.max_y; y++, sy++)
					{
						for(x=window->selection_area.min_x, sx=0; x<window->selection_area.max_x; x++, sx++)
						{
							select_value = window->selection->pixels[y*window->selection->stride+x];
							window->mask_temp->pixels[sy*pattern_stride+sx*4] = (layers[i]->pixels[y*layers[i]->stride+x*4] * select_value) / 255;
							window->mask_temp->pixels[sy*pattern_stride+sx*4+1] = (layers[i]->pixels[y*layers[i]->stride+x*4+1] * select_value) / 255;
							window->mask_temp->pixels[sy*pattern_stride+sx*4+2] = (layers[i]->pixels[y*layers[i]->stride+x*4+2] * select_value) / 255;
							window->mask_temp->pixels[sy*pattern_stride+sx*4+3] = (layers[i]->pixels[y*layers[i]->stride+x*4+3] * select_value) / 255;
						}
					}
				}
				else
				{
					pattern_width = layers[i]->width;
					pattern_height = layers[i]->height;
					pattern_stride = layers[i]->stride;
					(void)memcpy(window->mask_temp->pixels, layers[i]->pixels, window->pixel_buf_size);
				}

				half_width = pattern_width * 0.5,	half_height = pattern_height * 0.5;

				pattern_surface = cairo_image_surface_create_for_data(window->mask_temp->pixels,
					CAIRO_FORMAT_ARGB32, pattern_width, pattern_height, pattern_stride);
				pattern = cairo_pattern_create_for_surface(pattern_surface);

				(void)memcpy(window->temp_layer->pixels, layers[i]->pixels, window->pixel_buf_size);
				cairo_set_operator(window->temp_layer->cairo_p, CAIRO_OPERATOR_OVER);

				rotate_direction = (filter_data->rotate_mode == MOTION_BLUR_ROTATE_CLOCKWISE) ? -1 : 1;
				alpha_minus = 2.0 / (filter_data->angle + 1);
				alpha = 1 - alpha_minus;
				for(j=0; j<filter_data->angle*2; j++)
				{
					angle = ((rotate_direction * (j+1) * G_PI) / 180.0)*0.5;
					sin_value = sin(angle),	cos_value = cos(angle);
					cairo_matrix_init_scale(&matrix, 1, 1);
					cairo_matrix_rotate(&matrix, angle);
					cairo_matrix_translate(&matrix,
						- (filter_data->center_x - (half_width * cos_value + half_height * sin_value)),
						- (filter_data->center_y + (half_width * sin_value - half_height * cos_value))
					);
					cairo_pattern_set_matrix(pattern, &matrix);

					(void)memset(window->mask->pixels, 0, window->pixel_buf_size);
					cairo_set_source(window->mask->cairo_p, pattern);
					cairo_paint_with_alpha(window->mask->cairo_p, alpha);

#ifdef _OPENMP
#pragma omp parallel for
#endif
					for(x=0; x<window->width * window->height; x++)
					{
						if(window->mask->pixels[x*4+3] > window->temp_layer->pixels[x*4+3])
						{
							window->temp_layer->pixels[x*4+0] = (uint8)(
								(uint32)((MAXIMUM((int)window->mask->pixels[x*4+0] - window->temp_layer->pixels[x+4+0], 0))
									* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+0]);
							window->temp_layer->pixels[x*4+1] = (uint8)(
								(uint32)((MAXIMUM((int)window->mask->pixels[x*4+1] - window->temp_layer->pixels[x+4+1], 0))
									* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+1]);
							window->temp_layer->pixels[x*4+2] = (uint8)(
								(uint32)((MAXIMUM((int)window->mask->pixels[x*4+2] - window->temp_layer->pixels[x+4+2], 0))
									* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+2]);
							window->temp_layer->pixels[x*4+3] = (uint8)(
								(uint32)((MAXIMUM((int)window->mask->pixels[x*4+3] - window->temp_layer->pixels[x+4+3], 0))
									* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+3]);
						}
					}
					alpha -= alpha_minus;
				}

				if(filter_data->rotate_mode == MOTION_BLUR_ROTATE_BOTH_DIRECTION)
				{
					alpha = 1 - alpha_minus;
					for(j=0; j<filter_data->angle*2; j++)
					{
						angle = ((-(j+1) * G_PI) / 180.0)*0.5;
						sin_value = sin(angle),	cos_value = cos(angle);
						cairo_matrix_init_scale(&matrix, 1, 1);
						cairo_matrix_rotate(&matrix, angle);
						cairo_matrix_translate(&matrix,
							- (filter_data->center_x - (half_width * cos_value + half_height * sin_value)),
							- (filter_data->center_y + (half_width * sin_value - half_height * cos_value))
						);
						cairo_pattern_set_matrix(pattern, &matrix);

						(void)memset(window->mask->pixels, 0, window->pixel_buf_size);
						cairo_set_source(window->mask->cairo_p, pattern);
						cairo_paint_with_alpha(window->mask->cairo_p, alpha);

#ifdef _OPENMP
#pragma omp parallel for
#endif
						for(x=0; x<window->width * window->height; x++)
						{
							if(window->mask->pixels[x*4+3] > window->temp_layer->pixels[x*4+3])
							{
								window->temp_layer->pixels[x*4+0] = (uint8)(
									(uint32)((MAXIMUM((int)window->mask->pixels[x*4+0] - window->temp_layer->pixels[x+4+0], 0))
										* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+0]);
								window->temp_layer->pixels[x*4+1] = (uint8)(
									(uint32)((MAXIMUM((int)window->mask->pixels[x*4+1] - window->temp_layer->pixels[x+4+1], 0))
										* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+1]);
								window->temp_layer->pixels[x*4+2] = (uint8)(
									(uint32)((MAXIMUM((int)window->mask->pixels[x*4+2] - window->temp_layer->pixels[x+4+2], 0))
										* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+2]);
								window->temp_layer->pixels[x*4+3] = (uint8)(
									(uint32)((MAXIMUM((int)window->mask->pixels[x*4+3] - window->temp_layer->pixels[x+4+3], 0))
										* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+3]);
							}
						}
						alpha -= alpha_minus;
					}
				}

				cairo_surface_destroy(pattern_surface);
				cairo_pattern_destroy(pattern);
			}
			break;
		case MOTION_BLUR_GROW:
			{
				cairo_pattern_t *pattern;
				cairo_surface_t *pattern_surface;
				cairo_matrix_t matrix;
				FLOAT_T zoom, rev_zoom;
				FLOAT_T alpha, alpha_minus;
				int pattern_width, pattern_height, pattern_stride;
				int pattern_size;
				FLOAT_T half_width, half_height;
				int x, y;
				int sy;

				if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) != 0)
				{
					pattern_width = window->selection_area.max_x - window->selection_area.min_x;
					pattern_stride = pattern_width * 4;
					pattern_height = window->selection_area.max_y - window->selection_area.min_y;

					sy = 0;
#ifdef _OPENMP
#pragma omp parallel for
#endif
					for(y=window->selection_area.min_y; y<window->selection_area.max_y; y++)
					{
						uint8 select_value;
						int sx;
						int x;
						for(x=window->selection_area.min_x, sx=0; x<window->selection_area.max_x; x++, sx++)
						{
							select_value = window->selection->pixels[y*window->selection->stride+x];
							window->mask_temp->pixels[sy*pattern_stride+sx*4] = (layers[i]->pixels[y*layers[i]->stride+x*4] * select_value) / 255;
							window->mask_temp->pixels[sy*pattern_stride+sx*4+1] = (layers[i]->pixels[y*layers[i]->stride+x*4+1] * select_value) / 255;
							window->mask_temp->pixels[sy*pattern_stride+sx*4+2] = (layers[i]->pixels[y*layers[i]->stride+x*4+2] * select_value) / 255;
							window->mask_temp->pixels[sy*pattern_stride+sx*4+3] = (layers[i]->pixels[y*layers[i]->stride+x*4+3] * select_value) / 255;
						}
						sy++;
					}
				}
				else
				{
					pattern_width = layers[i]->width;
					pattern_height = layers[i]->height;
					pattern_stride = layers[i]->stride;
					(void)memcpy(window->mask_temp->pixels, layers[i]->pixels, window->pixel_buf_size);
				}

				pattern_surface = cairo_image_surface_create_for_data(window->mask_temp->pixels,
					CAIRO_FORMAT_ARGB32, pattern_width, pattern_height, pattern_stride);
				pattern = cairo_pattern_create_for_surface(pattern_surface);
				pattern_size = MAXIMUM(pattern_width, pattern_height);
				(void)memcpy(window->temp_layer->pixels, layers[i]->pixels, window->pixel_buf_size);

				alpha_minus = 1.0 / (filter_data->size * 2 + 1);
				alpha = 1 - alpha_minus;
				for(j=0; j<filter_data->size*2; j++)
				{
					zoom = (pattern_size + j*0.5 + 1) / (FLOAT_T)pattern_size;
					rev_zoom = 1 / zoom;
					half_width = (pattern_width * zoom) * 0.5;
					half_height = (pattern_height * zoom) * 0.5;
					cairo_matrix_init_scale(&matrix, zoom, zoom);
					cairo_matrix_translate(&matrix, - (filter_data->center_x - half_width),
						- (filter_data->center_y - half_height));

					cairo_pattern_set_matrix(pattern, &matrix);

					(void)memset(window->mask->pixels, 0, window->pixel_buf_size);
					cairo_set_source(window->mask->cairo_p, pattern);
					cairo_paint_with_alpha(window->mask->cairo_p, alpha);

#ifdef _OPENMP
#pragma omp parallel for
#endif
					for(x=0; x<window->width * window->height; x++)
					{
						if(window->mask->pixels[x*4+3] > window->temp_layer->pixels[x*4+3])
						{
							window->temp_layer->pixels[x*4+0] = (uint8)(
								(uint32)((MAXIMUM((int)window->mask->pixels[x*4+0] - window->temp_layer->pixels[x+4+0], 0))
									* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+0]);
							window->temp_layer->pixels[x*4+1] = (uint8)(
								(uint32)((MAXIMUM((int)window->mask->pixels[x*4+1] - window->temp_layer->pixels[x+4+1], 0))
									* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+1]);
							window->temp_layer->pixels[x*4+2] = (uint8)(
								(uint32)((MAXIMUM((int)window->mask->pixels[x*4+2] - window->temp_layer->pixels[x+4+2], 0))
									* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+2]);
							window->temp_layer->pixels[x*4+3] = (uint8)(
								(uint32)((MAXIMUM((int)window->mask->pixels[x*4+3] - window->temp_layer->pixels[x+4+3], 0))
									* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+3]);
						}
					}
					alpha -= alpha_minus;
				}

				cairo_surface_destroy(pattern_surface);
				cairo_pattern_destroy(pattern);
			}
			break;
		}
		
		if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) != 0)
		{
#ifdef _OPENMP
#pragma omp parallel for
#endif
			for(j=0; j<window->width*window->height; j++)
			{
				uint8 select_value = window->selection->pixels[j];
				layers[i]->pixels[j*4+0] = ((0xff-select_value)*layers[i]->pixels[j*4+0]
					+ window->temp_layer->pixels[j*4+0]*select_value) / 255;
				layers[i]->pixels[j*4+1] = ((0xff-select_value)*layers[i]->pixels[j*4+1]
					+ window->temp_layer->pixels[j*4+1]*select_value) / 255;
				layers[i]->pixels[j*4+2] = ((0xff-select_value)*layers[i]->pixels[j*4+2]
					+ window->temp_layer->pixels[j*4+2]*select_value) / 255;
				layers[i]->pixels[j*4+3] = ((0xff-select_value)*layers[i]->pixels[j*4+3]
					+ window->temp_layer->pixels[j*4+3]*select_value) / 255;
			}
		}
		else
		{
			(void)memcpy(layers[i]->pixels, window->temp_layer->pixels, window->pixel_buf_size);
		}
	}

	if(layers[0] == window->active_layer)
	{
		window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_OVER;
	}
	else
	{
		window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	}
	gtk_widget_queue_draw(window->window);
}

/***********************************************
* SelectionMotionBlurFilter�֐�                *
* �I��͈͂ւ̃��[�V�����ڂ����t�B���^�[��K�p *
* ����                                         *
* window	: �`��̈�̏��                   *
* data		: �ڂ��������̏ڍ׃f�[�^           *
***********************************************/
void SelectionMotionBlurFilter(DRAW_WINDOW* window, void* data)
{
	MOTION_BLUR *filter_data = (MOTION_BLUR*)data;
	unsigned int sum_color;
	int i;

	switch(filter_data->type)
	{
	case MOTION_BLUR_STRAGHT:
		{
			FLOAT_T dx, dy;
			FLOAT_T angle;
			int length;
			int y;

			if((filter_data->flags & MOTION_BLUR_BIDIRECTION) != 0)
			{
				length = filter_data->size * 2 + 1;
			}
			else
			{
				length = filter_data->size + 1;
			}

			angle = filter_data->angle * G_PI / 180.0;
			dx = cos(angle),	dy = sin(angle);

#ifdef _OPENMP
#pragma omp parallel for
#endif
			for(y=0; y<window->height; y++)
			{
				FLOAT_T check_x, check_y;
				int before_x, before_y;
				int int_x, int_y;
				int x;
				int count;

				for(x=0; x<window->width; x++)
				{
					check_x = x - dx * filter_data->size;
					check_y = y - dy * filter_data->size;
					before_x = -1,	before_y = -1;
					sum_color = 0;
					count = 0;
					for(i=0; i<length; i++)
					{
						int_x = (int)check_x,	int_y = (int)check_y;
						if(int_x >= 0 && int_x < window->width
							&& int_y >= 0 && int_y < window->height)
						{
							if(int_x != before_x || int_y != before_y)
							{
								sum_color += window->selection->pixels[int_y*window->selection->stride+int_x];
								count++;
								before_x = int_x,	before_y = int_y;
							}
						}
						check_x += dx,	check_y += dy;
					}

					if(count > 0)
					{
						window->temp_layer->pixels[y*window->selection->stride+x] = sum_color / count;
					}
				}
			}
		}
		break;
	case MOTION_BLUR_STRAGHT_RANDOM:
		{
			FLOAT_T dx, dy;
			FLOAT_T add_x = 0, add_y = 0;
			FLOAT_T angle;
			int y;

			angle = filter_data->angle * G_PI / 180.0;
			add_x = dx = cos(angle),	add_y = dy = sin(angle);
			if(add_x > 0 || add_y > 0)
			{
				add_x *= 2,	add_y *= 2;
				if(add_x > 0)
				{
					if(add_x > 1)
					{
						add_x /= add_x;
					}
				}
				else
				{
					if(add_x <= -2)
					{
						add_x /= - add_x;
					}
				}
				if(add_y > 0)
				{
					if(add_y > 1)
					{
						add_y /= add_y;
					}
				}
				else
				{
					if(add_y <= -2)
					{
						add_y /= - add_y;
					}
				}
			}

			(void)memset(window->mask->pixels, 0xff, window->width * window->height);

			if(add_x >= 0 && add_y >= 0)
			{
#ifdef _OPENMP
#pragma omp parallel for
#endif
				for(y=0; y<window->selection->height; y++)
				{
					FLOAT_T check_x, check_y;
					int before_x, before_y;
					int int_x, int_y;
					int length;
					int x;
					int count;

					for(x=0; x<window->selection->width; x++)
					{
						int_x = (int)(x - add_x),	int_y = (int)(y - add_y);
						if(int_x >= 0 && int_x < window->width && int_y >= 0 && int_y < window->height)
						{
							if(window->mask->pixels[int_y*window->width+int_x] != 0xff)
							{
								window->mask->pixels[y*window->width+x] = length =
									window->mask->pixels[int_y*window->width+int_x];
							}
							else
							{
								window->mask->pixels[y*window->width+x] = length =
									rand() % filter_data->size;
							}
						}
						else
						{
							window->mask->pixels[y*window->width+x] = length =
								rand() % filter_data->size;
						}

						check_x = x - dx * filter_data->size;
						check_y = y - dy * filter_data->size;
						before_x = -1,	before_y = -1;
						sum_color = 0;
						count = 0;
						for(i=0; i<length; i++)
						{
							int_x = (int)check_x,	int_y = (int)check_y;
							if(int_x >= 0 && int_x < window->width
								&& int_y >= 0 && int_y < window->height)
							{
								if(int_x != before_x || int_y != before_y)
								{
									sum_color += window->selection->pixels[int_y*window->selection->stride+int_x];
									count++;
									before_x = int_x,	before_y = int_y;
								}
							}
							check_x += dx,	check_y += dy;
						}

						if(count > 0)
						{
							window->temp_layer->pixels[y*window->selection->stride+x] = sum_color / count;
						}
					}
				}
			}
			else if(add_x < 0 && add_y >= 0)
			{
#ifdef _OPENMP
#pragma omp parallel for
#endif
				for(y=0; y<window->selection->height; y++)
				{
					FLOAT_T check_x, check_y;
					int before_x, before_y;
					int int_x, int_y;
					int length;
					int x;
					int count;

					for(x=window->selection->width-1; x>=0; x--)
					{
						int_x = (int)(x - add_x),	int_y = (int)(y - add_y);
						if(int_x >= 0 && int_x < window->width && int_y >= 0 && int_y < window->height)
						{
							if(window->mask->pixels[int_y*window->width+int_x] != 0xff)
							{
								window->mask->pixels[y*window->width+x] = length =
									window->mask->pixels[int_y*window->width+int_x];
							}
							else
							{
								window->mask->pixels[y*window->width+x] = length =
									rand() % filter_data->size;
							}
						}
						else
						{
							window->mask->pixels[y*window->width+x] = length =
								rand() % filter_data->size;
						}

						check_x = x - dx * filter_data->size;
						check_y = y - dy * filter_data->size;
						before_x = -1,	before_y = -1;
						sum_color = 0;
						count = 0;
						for(i=0; i<length; i++)
						{
							int_x = (int)check_x,	int_y = (int)check_y;
							if(int_x >= 0 && int_x < window->width
								&& int_y >= 0 && int_y < window->height)
							{
								if(int_x != before_x || int_y != before_y)
								{
									sum_color += window->selection->pixels[int_y*window->selection->stride+int_x*4];
									count++;
									before_x = int_x,	before_y = int_y;
								}
							}
							check_x += dx,	check_y += dy;
						}

						if(count > 0)
						{
								window->temp_layer->pixels[y*window->selection->stride+x] = sum_color / count;
						}
					}
				}
			}
			else if(add_x >= 0 && add_y < 0)
			{
#ifdef _OPENMP
#pragma omp parallel for
#endif
				for(y=window->selection->height-1; y>=0; y--)
				{
					FLOAT_T check_x, check_y;
					int before_x, before_y;
					int int_x, int_y;
					int length;
					int x;
					int count;

					for(x=0; x<window->selection->width; x++)
					{
						int_x = (int)(x - add_x),	int_y = (int)(y - add_y);
						if(int_x >= 0 && int_x < window->width && int_y >= 0 && int_y < window->height)
						{
							if(window->mask->pixels[int_y*window->width+int_x] != 0xff)
							{
								window->mask->pixels[y*window->width+x] = length =
									window->mask->pixels[int_y*window->width+int_x];
							}
							else
							{
								window->mask->pixels[y*window->width+x] = length =
									rand() % filter_data->size;
							}
						}
						else
						{
							window->mask->pixels[y*window->width+x] = length =
								rand() % filter_data->size;
						}

						check_x = x - dx * filter_data->size;
						check_y = y - dy * filter_data->size;
						before_x = -1,	before_y = -1;
						sum_color = 0;
						count = 0;
						for(i=0; i<length; i++)
						{
							int_x = (int)check_x,	int_y = (int)check_y;
							if(int_x >= 0 && int_x < window->width
								&& int_y >= 0 && int_y < window->height)
							{
								if(int_x != before_x || int_y != before_y)
								{
									sum_color += window->selection->pixels[int_y*window->selection->stride+int_x];
									count++;
									before_x = int_x,	before_y = int_y;
								}
							}
							check_x += dx,	check_y += dy;
						}

						if(count > 0)
						{
							window->temp_layer->pixels[y*window->selection->stride+x] = sum_color / count;
						}
					}
				}
			}
			else
			{
#ifdef _OPENMP
#pragma omp parallel for
#endif
				for(y=window->selection->height-1; y>=0; y--)
				{
					FLOAT_T check_x, check_y;
					int before_x, before_y;
					int int_x, int_y;
					int length;
					int x;
					int count;

					for(x=window->selection->width-1; x>=0; x--)
					{
						int_x = (int)(x - add_x),	int_y = (int)(y - add_y);
						if(int_x >= 0 && int_x < window->width && int_y >= 0 && int_y < window->height)
						{
							if(window->mask->pixels[int_y*window->width+int_x] != 0xff)
							{
								window->mask->pixels[y*window->width+x] = length =
									window->mask->pixels[int_y*window->width+int_x];
							}
							else
							{
								window->mask->pixels[y*window->width+x] = length =
									rand() % filter_data->size;
							}
						}
						else
						{
							window->mask->pixels[y*window->width+x] = length =
								rand() % filter_data->size;
						}

						check_x = x - dx * filter_data->size;
						check_y = y - dy * filter_data->size;
						before_x = -1,	before_y = -1;
						sum_color = 0;
						count = 0;
						for(i=0; i<length; i++)
						{
							int_x = (int)check_x,	int_y = (int)check_y;
							if(int_x >= 0 && int_x < window->width
								&& int_y >= 0 && int_y < window->height)
							{
								if(int_x != before_x || int_y != before_y)
								{
									sum_color += window->selection->pixels[int_y*window->selection->stride+int_x];
									count++;
									before_x = int_x,	before_y = int_y;
								}
							}
							check_x += dx,	check_y += dy;
						}

						if(count > 0)
						{
							window->temp_layer->pixels[y*window->selection->stride+x] = sum_color / count;
						}
					}
				}
			}
		}
		break;
	case MOTION_BLUR_ROTATE:
		{
			cairo_pattern_t *pattern;
			cairo_surface_t *pattern_surface;
			cairo_matrix_t matrix;
			int rotate_direction;
			FLOAT_T angle;
			FLOAT_T sin_value, cos_value;
			FLOAT_T alpha, alpha_minus;
			int pattern_width, pattern_height, pattern_stride;
			FLOAT_T half_width, half_height;
			int x;

			pattern_width = window->selection->width;
			pattern_height = window->selection->height;
			pattern_stride = window->stride;
			(void)memset(window->mask_temp->pixels, 0, window->pixel_buf_size);
			for(i=0; i<window->width * window->height; i++)
			{
				window->mask_temp->pixels[i*4+3] = window->selection->pixels[i];
			}

			half_width = pattern_width * 0.5,	half_height = pattern_height * 0.5;

			pattern_surface = cairo_image_surface_create_for_data(window->mask_temp->pixels,
				CAIRO_FORMAT_ARGB32, pattern_width, pattern_height, pattern_stride);
			pattern = cairo_pattern_create_for_surface(pattern_surface);

			(void)memset(window->temp_layer->pixels, 0, window->pixel_buf_size);
			for(i=0; i<window->width * window->height; i++)
			{
				window->temp_layer->pixels[i*4+3] = window->selection->pixels[i];
			}
			cairo_set_operator(window->temp_layer->cairo_p, CAIRO_OPERATOR_OVER);

			rotate_direction = (filter_data->rotate_mode == MOTION_BLUR_ROTATE_CLOCKWISE) ? -1 : 1;
			alpha_minus = 2.0 / (filter_data->angle + 1);
			alpha = 1 - alpha_minus;
			for(i=0; i<filter_data->angle*2; i++)
			{
				angle = ((rotate_direction * (i+1) * G_PI) / 180.0)*0.5;
				sin_value = sin(angle),	cos_value = cos(angle);
				cairo_matrix_init_scale(&matrix, 1, 1);
				cairo_matrix_rotate(&matrix, angle);
				cairo_matrix_translate(&matrix,
					- (filter_data->center_x - (half_width * cos_value + half_height * sin_value)),
					- (filter_data->center_y + (half_width * sin_value - half_height * cos_value))
				);
				cairo_pattern_set_matrix(pattern, &matrix);

				(void)memset(window->mask->pixels, 0, window->pixel_buf_size);
				cairo_set_source(window->mask->cairo_p, pattern);
				cairo_paint_with_alpha(window->mask->cairo_p, alpha);
				for(x=0; x<window->width * window->height; x++)
				{
					if(window->mask->pixels[x*4+3] > window->temp_layer->pixels[x*4+3])
					{
						window->temp_layer->pixels[x*4+3] = (uint8)(
							(uint32)((MAXIMUM((int)window->mask->pixels[x*4+3] - window->temp_layer->pixels[x+4+3], 0))
								* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+3]);
					}
				}
				alpha -= alpha_minus;
			}

			if(filter_data->rotate_mode == MOTION_BLUR_ROTATE_BOTH_DIRECTION)
			{
				alpha = 1 - alpha_minus;
				for(i=0; i<filter_data->angle*2; i++)
				{
					angle = ((-(i+1) * G_PI) / 180.0)*0.5;
					sin_value = sin(angle),	cos_value = cos(angle);
					cairo_matrix_init_scale(&matrix, 1, 1);
					cairo_matrix_rotate(&matrix, angle);
					cairo_matrix_translate(&matrix,
						- (filter_data->center_x - (half_width * cos_value + half_height * sin_value)),
						- (filter_data->center_y + (half_width * sin_value - half_height * cos_value))
					);
					cairo_pattern_set_matrix(pattern, &matrix);

					(void)memset(window->mask->pixels, 0, window->pixel_buf_size);
					cairo_set_source(window->mask->cairo_p, pattern);
					cairo_paint_with_alpha(window->mask->cairo_p, alpha);
					for(x=0; x<window->width * window->height; x++)
					{
						if(window->mask->pixels[x*4+3] > window->temp_layer->pixels[x*4+3])
						{
							window->temp_layer->pixels[x*4+3] = (uint8)(
								(uint32)((MAXIMUM((int)window->mask->pixels[x*4+3] - window->temp_layer->pixels[x+4+3], 0))
									* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+3]);
						}
					}
					alpha -= alpha_minus;
				}
			}

			cairo_surface_destroy(pattern_surface);
			cairo_pattern_destroy(pattern);
		}
		break;
	case MOTION_BLUR_GROW:
		{
			cairo_pattern_t *pattern;
			cairo_surface_t *pattern_surface;
			cairo_matrix_t matrix;
			FLOAT_T zoom, rev_zoom;
			FLOAT_T alpha, alpha_minus;
			int pattern_width, pattern_height, pattern_stride;
			int pattern_size;
			FLOAT_T half_width, half_height;
			int x;

			pattern_width = window->width;
			pattern_height = window->height;
			pattern_stride = window->stride;
			(void)memset(window->mask_temp->pixels, 0, window->pixel_buf_size);
			for(i=0; i<window->width * window->height; i++)
			{
				window->mask_temp->pixels[i*4+3] = window->selection->pixels[i];
			}

			pattern_surface = cairo_image_surface_create_for_data(window->mask_temp->pixels,
				CAIRO_FORMAT_ARGB32, pattern_width, pattern_height, pattern_stride);
			pattern = cairo_pattern_create_for_surface(pattern_surface);
			pattern_size = MAXIMUM(pattern_width, pattern_height);
			(void)memset(window->temp_layer->pixels, 0, window->pixel_buf_size);
			for(i=0; i<window->width * window->height; i++)
			{
				window->temp_layer->pixels[i*4+3] = window->selection->pixels[i];
			}

			alpha_minus = 1.0 / (filter_data->size * 2 + 1);
			alpha = 1 - alpha_minus;
			for(i=0; i<filter_data->size*2; i++)
			{
				zoom = (pattern_size + i*0.5 + 1) / (FLOAT_T)pattern_size;
				rev_zoom = 1 / zoom;
				half_width = (pattern_width * zoom) * 0.5;
				half_height = (pattern_height * zoom) * 0.5;
				cairo_matrix_init_scale(&matrix, zoom, zoom);
				cairo_matrix_translate(&matrix, - (filter_data->center_x - half_width),
					- (filter_data->center_y - half_height));

				cairo_pattern_set_matrix(pattern, &matrix);

				(void)memset(window->mask->pixels, 0, window->pixel_buf_size);
				cairo_set_source(window->mask->cairo_p, pattern);
				cairo_paint_with_alpha(window->mask->cairo_p, alpha);
				for(x=0; x<window->width * window->height; x++)
				{
					if(window->mask->pixels[x*4+3] > window->temp_layer->pixels[x*4+3])
					{
						window->temp_layer->pixels[x*4+3] = (uint8)(
							(uint32)((MAXIMUM((int)window->mask->pixels[x*4+3] - window->temp_layer->pixels[x+4+3], 0))
								* window->mask->pixels[x*4+3] >> 8) + window->temp_layer->pixels[x*4+3]);
					}
				}
				alpha -= alpha_minus;
			}

			cairo_surface_destroy(pattern_surface);
			cairo_pattern_destroy(pattern);
		}
		break;
	}

	if(filter_data->type == MOTION_BLUR_STRAGHT
		|| filter_data->type == MOTION_BLUR_STRAGHT_RANDOM)
	{
		(void)memcpy(window->selection->pixels, window->temp_layer->pixels,
			window->width * window->height);
	}
	else
	{
		for(i=0; i<window->width * window->height; i++)
		{
			window->selection->pixels[i] = window->temp_layer->pixels[i*4+3];
		}
	}

	if(UpdateSelectionArea(&window->selection_area, window->selection, window->temp_layer) == FALSE)
	{
		window->flags &= ~(DRAW_WINDOW_HAS_SELECTION_AREA);
	}
	else
	{
		window->flags |= DRAW_WINDOW_HAS_SELECTION_AREA;
	}

	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_OVER;
	gtk_widget_queue_draw(window->window);
}

static void ChangeMotionBlurSize(GtkAdjustment* control, MOTION_BLUR* filter_data)
{
	filter_data->size = (uint16)gtk_adjustment_get_value(control);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(filter_data->preview)) != FALSE)
	{
		DRAW_WINDOW *window = (DRAW_WINDOW*)g_object_get_data(
			G_OBJECT(filter_data->preview), "draw-window");
		LAYER **layers = (LAYER**)g_object_get_data(
			G_OBJECT(filter_data->preview), "layers");
		int num_layers = GPOINTER_TO_INT(g_object_get_data(
			G_OBJECT(filter_data->preview), "num_layers"));
		int i;

		if((window->flags & DRAW_WINDOW_EDIT_SELECTION) == 0)
		{
			for(i=0; i<num_layers; i++)
			{
				(void)memcpy(layers[i]->pixels, filter_data->before_pixels[i], window->pixel_buf_size);
			}

			MotionBlurFilter(window, layers, (uint16)num_layers, filter_data);
		}
		else
		{
			(void)memcpy(window->selection->pixels, filter_data->before_pixels[0], window->width * window->height);
			SelectionMotionBlurFilter(window, filter_data);
		}
	}
}

static void ChangeMotionBlurAngle(GtkAdjustment* control, MOTION_BLUR* filter_data)
{
	filter_data->angle = (int16)gtk_adjustment_get_value(control);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(filter_data->preview)) != FALSE)
	{
		DRAW_WINDOW *window = (DRAW_WINDOW*)g_object_get_data(
			G_OBJECT(filter_data->preview), "draw-window");
		LAYER **layers = (LAYER**)g_object_get_data(
			G_OBJECT(filter_data->preview), "layers");
		int num_layers = GPOINTER_TO_INT(g_object_get_data(
			G_OBJECT(filter_data->preview), "num-layers"));
		int i;

		if((window->flags & DRAW_WINDOW_EDIT_SELECTION) == 0)
		{
			for(i=0; i<num_layers; i++)
			{
				(void)memcpy(layers[i]->pixels, filter_data->before_pixels[i], window->pixel_buf_size);
			}

			MotionBlurFilter(window, layers, (uint16)num_layers, filter_data);
		}
		else
		{
			(void)memcpy(window->selection->pixels, filter_data->before_pixels[0], window->width * window->height);
			SelectionMotionBlurFilter(window, filter_data);
		}
	}
}

static void ChangeMotionBlurCenterX(GtkAdjustment* control, MOTION_BLUR* filter_data)
{
	filter_data->center_x = (int32)gtk_adjustment_get_value(control);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(filter_data->preview)) != FALSE)
	{
		DRAW_WINDOW *window = (DRAW_WINDOW*)g_object_get_data(
			G_OBJECT(filter_data->preview), "draw-window");
		LAYER **layers = (LAYER**)g_object_get_data(
			G_OBJECT(filter_data->preview), "layers");
		int num_layers = GPOINTER_TO_INT(g_object_get_data(
			G_OBJECT(filter_data->preview), "num-layers"));
		int i;

		if((window->flags & DRAW_WINDOW_EDIT_SELECTION) == 0)
		{
			for(i=0; i<num_layers; i++)
			{
				(void)memcpy(layers[i]->pixels, filter_data->before_pixels[i], window->pixel_buf_size);
			}

			MotionBlurFilter(window, layers, (uint16)num_layers, filter_data);
		}
		else
		{
			(void)memcpy(window->selection->pixels, filter_data->before_pixels[0], window->width * window->height);
			SelectionMotionBlurFilter(window, filter_data);
		}
	}
}

static void ChangeMotionBlurCenterY(GtkAdjustment* control, MOTION_BLUR* filter_data)
{
	filter_data->center_y = (int32)gtk_adjustment_get_value(control);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(filter_data->preview)) != FALSE)
	{
		DRAW_WINDOW *window = (DRAW_WINDOW*)g_object_get_data(
			G_OBJECT(filter_data->preview), "draw-window");
		LAYER **layers = (LAYER**)g_object_get_data(
			G_OBJECT(filter_data->preview), "layers");
		int num_layers = GPOINTER_TO_INT(g_object_get_data(
			G_OBJECT(filter_data->preview), "num-layers"));
		int i;

		if((window->flags & DRAW_WINDOW_EDIT_SELECTION) == 0)
		{
			for(i=0; i<num_layers; i++)
			{
				(void)memcpy(layers[i]->pixels, filter_data->before_pixels[i], window->pixel_buf_size);
			}

			MotionBlurFilter(window, layers, (uint16)num_layers, filter_data);
		}
		else
		{
			(void)memcpy(window->selection->pixels, filter_data->before_pixels[0], window->width * window->height);
			SelectionMotionBlurFilter(window, filter_data);
		}
	}
}

static void MotionBlurSetRotateMode(GtkWidget* button, MOTION_BLUR* filter_data)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)) != FALSE)
	{
		filter_data->rotate_mode = (uint8)g_object_get_data(G_OBJECT(button), "rotate-mode");
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(filter_data->preview)) != FALSE)
		{
			DRAW_WINDOW *window = (DRAW_WINDOW*)g_object_get_data(
			G_OBJECT(filter_data->preview), "draw-window");
			LAYER **layers = (LAYER**)g_object_get_data(
				G_OBJECT(filter_data->preview), "layers");
			int num_layers = GPOINTER_TO_INT(g_object_get_data(
				G_OBJECT(filter_data->preview), "num-layers"));
			int i;

			if((window->flags & DRAW_WINDOW_EDIT_SELECTION) == 0)
			{
				for(i=0; i<num_layers; i++)
				{
					(void)memcpy(layers[i]->pixels, filter_data->before_pixels[i], window->pixel_buf_size);
				}

				MotionBlurFilter(window, layers, (uint16)num_layers, filter_data);
			}
			else
			{
				(void)memcpy(window->selection->pixels, filter_data->before_pixels[0], window->width * window->height);
				SelectionMotionBlurFilter(window, filter_data);
			}
		}
	}
}

GtkWidget* CreateMotionBlurDetailUI(MOTION_BLUR* filter_data, DRAW_WINDOW* window)
{
	APPLICATION *app = window->app;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *control;
	GtkWidget *hbox;
	GtkWidget *buttons[3];
	GtkAdjustment *adjust;

	vbox = gtk_vbox_new(FALSE, 0);

	switch(filter_data->type)
	{
	case MOTION_BLUR_STRAGHT:
		filter_data->size = 1;
		adjust = GTK_ADJUSTMENT(gtk_adjustment_new(1, 1, 250, 1, 1, 0));
		control = SpinScaleNew(adjust, app->labels->unit.length, 0);
		gtk_box_pack_start(GTK_BOX(vbox), control, TRUE, TRUE, 0);
		(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
			G_CALLBACK(ChangeMotionBlurSize), filter_data);
		filter_data->angle = 0;
		adjust = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 360, 1, 1, 0));
		control = SpinScaleNew(adjust, app->labels->unit.angle, 0);
		gtk_box_pack_start(GTK_BOX(vbox), control, TRUE, TRUE, 0);
		(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
			G_CALLBACK(ChangeMotionBlurAngle), filter_data);
		buttons[0] = gtk_check_button_new_with_label(app->labels->filter.bidirection);
		CheckButtonSetFlagsCallBack(buttons[0], &filter_data->flags, MOTION_BLUR_BIDIRECTION);
		gtk_box_pack_start(GTK_BOX(vbox), buttons[0], TRUE, TRUE, 0);
		break;
	case MOTION_BLUR_STRAGHT_RANDOM:
		filter_data->size = 1;
		adjust = GTK_ADJUSTMENT(gtk_adjustment_new(1, 1, 250, 1, 1, 0));
		control = SpinScaleNew(adjust, app->labels->unit.length, 0);
		gtk_box_pack_start(GTK_BOX(vbox), control, TRUE, TRUE, 0);
		(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
			G_CALLBACK(ChangeMotionBlurSize), filter_data);
		filter_data->angle = 0;
		adjust = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 360, 1, 1, 0));
		control = SpinScaleNew(adjust, app->labels->unit.angle, 0);
		gtk_box_pack_start(GTK_BOX(vbox), control, TRUE, TRUE, 0);
		(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
			G_CALLBACK(ChangeMotionBlurAngle), filter_data);
		break;
	case MOTION_BLUR_ROTATE:
		label = gtk_label_new(app->labels->unit.center);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) == 0)
		{
			filter_data->center_x = window->width/2;
			filter_data->center_y = window->height/2;
		}
		else
		{
			filter_data->center_x = (window->selection_area.max_x + window->selection_area.min_x) / 2;
			filter_data->center_y = (window->selection_area.max_y + window->selection_area.min_y) / 2;
		}
		adjust = GTK_ADJUSTMENT(gtk_adjustment_new(filter_data->center_x, 0, window->width, 1, 1, 0));
		control = SpinScaleNew(adjust, "X", 0);
		gtk_box_pack_start(GTK_BOX(vbox), control, FALSE, FALSE, 0);
		(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
			G_CALLBACK(ChangeMotionBlurCenterX), filter_data);
		filter_data->center_y = window->height/2;
		adjust = GTK_ADJUSTMENT(gtk_adjustment_new(filter_data->center_y, 0, window->height, 1, 1, 0));
		control = SpinScaleNew(adjust, "Y", 0);
		gtk_box_pack_start(GTK_BOX(vbox), control, FALSE, FALSE, 0);
		(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
			G_CALLBACK(ChangeMotionBlurCenterY), filter_data);
		gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 3);
		filter_data->angle = 1;
		adjust = GTK_ADJUSTMENT(gtk_adjustment_new(1, 1, 360, 1, 1, 0));
		(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
			G_CALLBACK(ChangeMotionBlurAngle), filter_data);
		control = SpinScaleNew(adjust, app->labels->unit.angle, 0);
		gtk_box_pack_start(GTK_BOX(vbox), control, FALSE, FALSE, 0);

		hbox = gtk_hbox_new(FALSE, 0);
		buttons[0] = gtk_radio_button_new_with_label(
			NULL, app->labels->tool_box.clockwise);
		g_object_set_data(G_OBJECT(buttons[0]), "rotate-mode", GINT_TO_POINTER(MOTION_BLUR_ROTATE_CLOCKWISE));
		(void)g_signal_connect(G_OBJECT(buttons[0]), "toggled",
			G_CALLBACK(MotionBlurSetRotateMode), filter_data);
		gtk_box_pack_start(GTK_BOX(hbox), buttons[0], FALSE, FALSE, 0);
		buttons[1] = gtk_radio_button_new_with_label(
			gtk_radio_button_get_group(GTK_RADIO_BUTTON(buttons[0])), app->labels->tool_box.counter_clockwise);
		g_object_set_data(G_OBJECT(buttons[1]), "rotate-mode", GINT_TO_POINTER(MOTION_BLUR_ROTATE_COUNTER_CLOCKWISE));
		(void)g_signal_connect(G_OBJECT(buttons[1]), "toggled",
			G_CALLBACK(MotionBlurSetRotateMode), filter_data);
		gtk_box_pack_start(GTK_BOX(hbox), buttons[1], FALSE, FALSE, 0);
		buttons[2] = gtk_radio_button_new_with_label(
			gtk_radio_button_get_group(GTK_RADIO_BUTTON(buttons[0])), app->labels->tool_box.both_direction);
		g_object_set_data(G_OBJECT(buttons[2]), "rotate-mode", GINT_TO_POINTER(MOTION_BLUR_ROTATE_BOTH_DIRECTION));
		(void)g_signal_connect(G_OBJECT(buttons[2]), "toggled",
			G_CALLBACK(MotionBlurSetRotateMode), filter_data);
		gtk_box_pack_start(GTK_BOX(hbox), buttons[2], FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
		break;
	case MOTION_BLUR_GROW:
		label = gtk_label_new(app->labels->unit.center);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
		if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) == 0)
		{
			filter_data->center_x = window->width/2;
			filter_data->center_y = window->height/2;
		}
		else
		{
			filter_data->center_x = (window->selection_area.max_x + window->selection_area.min_x) / 2;
			filter_data->center_y = (window->selection_area.max_y + window->selection_area.min_y) / 2;
		}
		adjust = GTK_ADJUSTMENT(gtk_adjustment_new(filter_data->center_x, 0, window->width, 1, 1, 0));
		control = SpinScaleNew(adjust, "X", 0);
		gtk_box_pack_start(GTK_BOX(vbox), control, FALSE, FALSE, 0);
		(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
			G_CALLBACK(ChangeMotionBlurCenterX), filter_data);
		filter_data->center_y = window->height/2;
		adjust = GTK_ADJUSTMENT(gtk_adjustment_new(filter_data->center_y, 0, window->height, 1, 1, 0));
		control = SpinScaleNew(adjust, "Y", 0);
		gtk_box_pack_start(GTK_BOX(vbox), control, FALSE, FALSE, 0);
		(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
			G_CALLBACK(ChangeMotionBlurCenterY), filter_data);
		gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 3);
		filter_data->size = 1;
		adjust = GTK_ADJUSTMENT(gtk_adjustment_new(1, 1, 100, 1, 1, 0));
		(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
			G_CALLBACK(ChangeMotionBlurSize), filter_data);
		control = SpinScaleNew(adjust, app->labels->unit.length, 0);
		gtk_box_pack_start(GTK_BOX(vbox), control, FALSE, FALSE, 0);
		break;
	}

	return vbox;
}

static void MotionBlurFilterSetModeButtonClicked(GtkWidget* button, MOTION_BLUR* filter_data)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)) != FALSE)
	{
		DRAW_WINDOW *window = (DRAW_WINDOW*)g_object_get_data(G_OBJECT(button), "draw-window");
		filter_data->type = (uint8)g_object_get_data(G_OBJECT(button), "filter-type");
		gtk_widget_destroy(filter_data->detail_ui);
		filter_data->detail_ui = CreateMotionBlurDetailUI(filter_data, window);
		gtk_box_pack_start(GTK_BOX(filter_data->detail_ui_box), filter_data->detail_ui,
			TRUE, TRUE, 0);
		gtk_widget_show_all(filter_data->detail_ui);
	}
}

static void MotionBlurPrevewButtonClicked(MOTION_BLUR* filter_data)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(filter_data->preview)) != FALSE)
	{
		DRAW_WINDOW *window = (DRAW_WINDOW*)g_object_get_data(
		G_OBJECT(filter_data->preview), "draw-window");
		LAYER **layers = (LAYER**)g_object_get_data(
			G_OBJECT(filter_data->preview), "layers");
		int num_layers = GPOINTER_TO_INT(g_object_get_data(
			G_OBJECT(filter_data->preview), "num-layers"));
		int i;

		if((window->flags & DRAW_WINDOW_EDIT_SELECTION) == 0)
		{
			for(i=0; i<num_layers; i++)
			{
				(void)memcpy(layers[i]->pixels, filter_data->before_pixels[i], window->pixel_buf_size);
			}

			MotionBlurFilter(window, layers, (uint16)num_layers, filter_data);
		}
		else
		{
			(void)memcpy(window->selection->pixels, filter_data->before_pixels[0], window->width * window->height);
			SelectionMotionBlurFilter(window, filter_data);
		}
	}
}

/*****************************************************
* ExecuteMotionBlurFilter�֐�                        *
* ���[�V�����ڂ����t�B���^�����s                     *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
void ExecuteMotionBlurFilter(APPLICATION* app)
{
	MOTION_BLUR filter_data = {0};
	DRAW_WINDOW *window = app->draw_window[app->active_window];
	GtkWidget *dialog;
	GtkWidget *radio_buttons[4];
	GtkWidget *vbox;
	GtkWidget *frame;
	LAYER **layers;
	uint16 num_layers;
	int set_data;
	int i;

	dialog = gtk_dialog_new_with_buttons(
		app->labels->menu.motion_blur,
		GTK_WINDOW(app->window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL
	);

	vbox = gtk_vbox_new(FALSE, 0);
	radio_buttons[0] = gtk_radio_button_new_with_label(
		NULL, app->labels->unit.straight);
	g_object_set_data(G_OBJECT(radio_buttons[0]), "filter-type", GINT_TO_POINTER(0));
	g_object_set_data(G_OBJECT(radio_buttons[0]), "draw-window", window);
	(void)g_signal_connect(G_OBJECT(radio_buttons[0]), "toggled",
		G_CALLBACK(MotionBlurFilterSetModeButtonClicked), &filter_data);
	gtk_box_pack_start(GTK_BOX(vbox), radio_buttons[0], FALSE, FALSE, 0);
	radio_buttons[1] = gtk_radio_button_new_with_label(
		gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_buttons[0])),
		app->labels->filter.straight_random);
	g_object_set_data(G_OBJECT(radio_buttons[1]), "filter-type", GINT_TO_POINTER(1));
	g_object_set_data(G_OBJECT(radio_buttons[1]), "draw-window", window);
	(void)g_signal_connect(G_OBJECT(radio_buttons[1]), "toggled",
		G_CALLBACK(MotionBlurFilterSetModeButtonClicked), &filter_data);
	gtk_box_pack_start(GTK_BOX(vbox), radio_buttons[1], FALSE, FALSE, 0);
	radio_buttons[2] = gtk_radio_button_new_with_label(
		gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_buttons[0])),
		app->labels->tool_box.transform_rotate
	);
	g_object_set_data(G_OBJECT(radio_buttons[2]), "filter-type", GINT_TO_POINTER(2));
	g_object_set_data(G_OBJECT(radio_buttons[2]), "draw-window", window);
	(void)g_signal_connect(G_OBJECT(radio_buttons[2]), "toggled",
		G_CALLBACK(MotionBlurFilterSetModeButtonClicked), &filter_data);
	gtk_box_pack_start(GTK_BOX(vbox), radio_buttons[2], FALSE, FALSE, 0);
	radio_buttons[3] = gtk_radio_button_new_with_label(
		gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_buttons[0])),
		app->labels->unit.extend
	);
	g_object_set_data(G_OBJECT(radio_buttons[3]), "filter-type", GINT_TO_POINTER(3));
	g_object_set_data(G_OBJECT(radio_buttons[3]), "draw-window", window);
	(void)g_signal_connect(G_OBJECT(radio_buttons[3]), "toggled",
		G_CALLBACK(MotionBlurFilterSetModeButtonClicked), &filter_data);
	gtk_box_pack_start(GTK_BOX(vbox), radio_buttons[3], FALSE, FALSE, 0);

	frame = gtk_frame_new(app->labels->unit.detail);
	filter_data.detail_ui_box = gtk_vbox_new(FALSE, 0);
	filter_data.detail_ui = CreateMotionBlurDetailUI(&filter_data, window);
	gtk_box_pack_start(GTK_BOX(filter_data.detail_ui_box), filter_data.detail_ui, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(frame), filter_data.detail_ui_box);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 3);

	layers = GetLayerChain(window, &num_layers);
	filter_data.before_pixels = (uint8**)MEM_ALLOC_FUNC(sizeof(*filter_data.before_pixels)*num_layers);
	(void)memset(filter_data.before_pixels, 0, sizeof(*filter_data.before_pixels)*num_layers);

	filter_data.preview = gtk_check_button_new_with_label(app->labels->unit.preview);
	g_object_set_data(G_OBJECT(filter_data.preview), "draw-window", window);
	set_data = num_layers;
	g_object_set_data(G_OBJECT(filter_data.preview), "num-layers", GINT_TO_POINTER(set_data));
	g_object_set_data(G_OBJECT(filter_data.preview), "layers", layers);
	(void)g_signal_connect_swapped(G_OBJECT(filter_data.preview), "toggled",
		G_CALLBACK(MotionBlurPrevewButtonClicked), &filter_data);
	gtk_box_pack_start(GTK_BOX(vbox), filter_data.preview, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		vbox, TRUE, TRUE, 0);

	gtk_widget_show_all(vbox);

	if((window->flags & DRAW_WINDOW_EDIT_SELECTION) == 0)
	{
		for(i=0; i<num_layers; i++)
		{
			filter_data.before_pixels[i] = (uint8*)MEM_ALLOC_FUNC(window->pixel_buf_size);
			(void)memcpy(filter_data.before_pixels[i], layers[i]->pixels, window->pixel_buf_size);
		}
	}
	else
	{
		filter_data.before_pixels[0] = (uint8*)MEM_ALLOC_FUNC(window->pixel_buf_size);
		(void)memcpy(filter_data.before_pixels[0], window->selection->pixels, window->width * window->height);
	}

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
	{	// O.K.�{�^���������ꂽ
		if((window->flags & DRAW_WINDOW_EDIT_SELECTION) == 0)
		{	// �v���r���[�����̏ꍇ�͂��̂܂ܗ������c���ăt�B���^�[�K�p
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(filter_data.preview)) == FALSE)
			{
				AddFilterHistory(app->labels->menu.motion_blur, &filter_data, sizeof(filter_data),
					FILTER_FUNC_MOTION_BLUR, layers, num_layers, window);
				MotionBlurFilter(window, layers, num_layers, (void*)&filter_data);
			}
			// �v���r���[�L��̏ꍇ�̓s�N�Z���f�[�^��߂��Ă���t�B���^�[�K�p��̃f�[�^�ɂ���
			else
			{
				for(i=0; i<num_layers; i++)
				{
					(void)memcpy(window->temp_layer->pixels, layers[i]->pixels,
						window->pixel_buf_size);
					(void)memcpy(layers[i]->pixels, filter_data.before_pixels[i], window->pixel_buf_size);
					(void)memcpy(filter_data.before_pixels[i], window->temp_layer->pixels, window->pixel_buf_size);
				}

				// ��ɗ����f�[�^���c��
				AddFilterHistory(app->labels->menu.motion_blur, &filter_data, sizeof(filter_data),
					FILTER_FUNC_MOTION_BLUR, layers, num_layers, window);

				// ���[�V�����ڂ������s��̃f�[�^�ɖ߂�
				for(i=0; i<num_layers; i++)
				{
					(void)memcpy(layers[i]->pixels, filter_data.before_pixels[i],
						layers[i]->width * layers[i]->height * layers[i]->channel);
				}
			}
		}
		else
		{
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(filter_data.preview)) == FALSE)
			{
				AddSelectionFilterHistory(app->labels->menu.motion_blur, &filter_data, sizeof(filter_data),
					FILTER_FUNC_MOTION_BLUR, window);
				SelectionMotionBlurFilter(window, &filter_data);
			}
			else
			{
				(void)memcpy(window->temp_layer->pixels, window->selection->pixels,
					window->width * window->height);
				(void)memcpy(window->selection->pixels, filter_data.before_pixels[0],
					window->width * window->height);
				AddSelectionFilterHistory(app->labels->menu.motion_blur, &filter_data, sizeof(filter_data),
					FILTER_FUNC_MOTION_BLUR, window);
				(void)memcpy(window->selection->pixels, window->temp_layer->pixels,
					window->width * window->height);
			}
		}
	}
	else
	{
		for(i=0; i<num_layers; i++)
		{
			(void)memcpy(layers[i]->pixels, filter_data.before_pixels[i],
				layers[i]->width * layers[i]->height * layers[i]->channel);
		}
	}

	gtk_widget_destroy(dialog);

	for(i=0; i<num_layers; i++)
	{
		MEM_FREE_FUNC(filter_data.before_pixels[i]);
	}
	MEM_FREE_FUNC(filter_data.before_pixels);
	MEM_FREE_FUNC(layers);
}

/*************************************
* CHANGE_BRIGHT_CONTRAST_DATA�\����  *
* ���邳�E�R���g���X�g�����p�̃f�[�^ *
*************************************/
typedef struct _CHANGE_BRIGHT_CONTRAST
{
	int8 bright;	// ���邳�̒����l
	int8 contrast;	// �R���g���X�g�̒����l
} CHANGE_BRIGHT_CONTRAST;

/*************************************
* ChangeBrightContrastFilter�֐�     *
* ���邳�E�R���g���X�g��������       *
* ����                               *
* window	: �`��̈�̏��         *
* layers	: �������s�����C���[�z�� *
* num_layer	: �������s�����C���[�̐� *
* data		: �ڂ��������̏ڍ׃f�[�^ *
*************************************/
void ChangeBrightContrastFilter(DRAW_WINDOW* window, LAYER** layers, uint16 num_layer, void* data)
{
	// ���邳�E�R���g���X�g�̒����l�ɃL���X�g
	CHANGE_BRIGHT_CONTRAST *change_value = (CHANGE_BRIGHT_CONTRAST*)data;
	// ���C���[�̕��ϐF
	RGBA_DATA average_color;
	// �R���g���X�g�����l�e�[�u��
	uint8 contrast_r[UCHAR_MAX+1], contrast_g[UCHAR_MAX+1], contrast_b[UCHAR_MAX+1];
	// �R���g���X�g�o�͒����̌X��
	double a = tan((((double)((int)change_value->contrast + 127) / 255.0) * 90.0) * G_PI / 180.0);
	int i, j;	// for���p�̃J�E���^

	// �e���C���[�ɑ΂����邳�E�R���g���X�g�̕ύX�����s
	for(i=0; i<num_layer; i++)
	{
		// �ʏ탌�C���[�Ȃ���s
		if(layers[i]->layer_type == TYPE_NORMAL_LAYER)
		{
			(void)memset(window->mask_temp->pixels, 0, window->pixel_buf_size);

#ifdef _OPENMP
#pragma omp parallel for
#endif
			// ���邳�𒲐�
			for(j=0; j<layers[i]->width*layers[i]->height; j++)
			{
				// ���邳�����p
				int r, g, b;
				// �t�B���^�̃f�[�^���o�C�g�P�ʂ�
				uint8 *byte_data = (uint8*)data;
				// �s�N�Z���f�[�^�̃C���f�b�N�X
				int index;

				index = j*layers[i]->channel;
				r = layers[i]->pixels[index] + change_value->bright * 2;
				g = layers[i]->pixels[index+1] + change_value->bright * 2;
				b = layers[i]->pixels[index+2] + change_value->bright * 2;

				if(r < 0)
				{
					r = 0;
				}
				else if(r > UCHAR_MAX)
				{
					r = UCHAR_MAX;
				}

				if(g < 0)
				{
					g = 0;
				}
				else if(g > UCHAR_MAX)
				{
					g = UCHAR_MAX;
				}

				if(b < 0)
				{
					b = 0;
				}
				else if(b > UCHAR_MAX)
				{
					b = UCHAR_MAX;
				}

				window->temp_layer->pixels[index] = r;
				window->temp_layer->pixels[index+1] = g;
				window->temp_layer->pixels[index+2] = b;
				window->temp_layer->pixels[index+3] = 0xff;
				window->mask_temp->pixels[index+3] = layers[i]->pixels[index+3];
			}

		/////////////////////////////////////////////////

			// �R���g���X�g����
				// ���C���[�̕��ϐF���擾
			average_color = CalcLayerAverageRGBAwithAlpha(window->temp_layer);
			//average_color = CalcLayerAverageRGBAwithAlpha(layers[i]);

			if(change_value->contrast < CHAR_MAX)
			{
				// �ؕЌv�Z�p
				double intercept[3] =
				{
					average_color.r * (1 - a),
					average_color.g * (1 - a),
					average_color.b * (1 - a)
				};

				// �R���g���X�g�����l�̃e�[�u�����쐬
#ifdef _OPENMP
#pragma omp parallel for
#endif
				for(j=0; j<=UCHAR_MAX; j++)
				{
					int r = (int)(a * j + intercept[0]);
					int g = (int)(a * j + intercept[1]);
					int b = (int)(a * j + intercept[2]);

					if(r < 0)
					{
						r = 0;
					}
					else if(r > UCHAR_MAX)
					{
						r = UCHAR_MAX;
					}
					if(g < 0)
					{
						g = 0;
					}
					else if(g > UCHAR_MAX)
					{
						g = UCHAR_MAX;
					}
					if(b < 0)
					{
						g = 0;
					}
					else if(b > UCHAR_MAX)
					{
						b = UCHAR_MAX;
					}

					contrast_r[j] = r;
					contrast_g[j] = g;
					contrast_b[j] = b;
				}

				for(j=0; j<layers[i]->width*layers[i]->height; j++)
				{
					window->temp_layer->pixels[j*4] = contrast_r[layers[i]->pixels[j*4]];
					window->temp_layer->pixels[j*4+1] = contrast_r[layers[i]->pixels[j*4+1]];
					window->temp_layer->pixels[j*4+2] = contrast_r[layers[i]->pixels[j*4+2]];
				}
			}
			else
			{
				(void)memset(contrast_r, 0, average_color.r+1);
				(void)memset(&contrast_r[average_color.r+1], 0xff, UCHAR_MAX-average_color.r);
				(void)memset(contrast_g, 0, average_color.g+1);
				(void)memset(&contrast_g[average_color.g+1], 0xff, UCHAR_MAX-average_color.g);
				(void)memset(contrast_b, 0, average_color.b+1);
				(void)memset(&contrast_b[average_color.b+1], 0xff, UCHAR_MAX-average_color.b);
			}

			if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) != 0)
			{
#ifdef _OPENMP
#pragma omp parallel for
#endif
				for(j=0; j<window->width*window->height; j++)
				{
					uint8 select_value = window->selection->pixels[j];
					window->temp_layer->pixels[j*4+0] = ((0xff-select_value)*layers[i]->pixels[j*4+0]
						+ window->temp_layer->pixels[j*4+0]*select_value) / 255;
					window->temp_layer->pixels[j*4+1] = ((0xff-select_value)*layers[i]->pixels[j*4+1]
						+ window->temp_layer->pixels[j*4+1]*select_value) / 255;
					window->temp_layer->pixels[j*4+2] = ((0xff-select_value)*layers[i]->pixels[j*4+2]
						+ window->temp_layer->pixels[j*4+2]*select_value) / 255;
					window->temp_layer->pixels[j*4+3] = ((0xff-select_value)*layers[i]->pixels[j*4+3]
						+ window->temp_layer->pixels[j*4+3]*select_value) / 255;
				}
			}

			cairo_set_operator(layers[i]->cairo_p, CAIRO_OPERATOR_SOURCE);
			cairo_set_source_surface(layers[i]->cairo_p, window->temp_layer->surface_p, 0, 0);
			cairo_mask_surface(layers[i]->cairo_p, window->mask_temp->surface_p, 0, 0);
			cairo_set_operator(layers[i]->cairo_p, CAIRO_OPERATOR_OVER);
		}	// �ʏ탌�C���[�Ȃ���s
			// if(layers[i]->layer_type == TYPE_NORMAL_LAYER)
	}	// �e���C���[�ɑ΂����邳�E�R���g���X�g�̕ύX�����s
		// for(i=0; i<num_layer; i++)

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	gtk_widget_queue_draw(window->window);
}

/***************************************
* ChangeBrightnessValueCallBack�֐�    *
* ���邳�ύX���̃R�[���o�b�N�֐�       *
* ����                                 *
* slider		: �X���C�_�̃A�W���X�^ *
* filter_data	: �t�B���^�[�p�̃f�[�^ *
***************************************/
static void ChangeBrightnessValueCallBack(
	GtkAdjustment* slider,
	void* filter_data
)
{
	// �`��̈�
	DRAW_WINDOW *window = (DRAW_WINDOW*)g_object_get_data(G_OBJECT(slider), "draw_window");
	// �K�p���郌�C���[
	LAYER **layers = (LAYER**)g_object_get_data(G_OBJECT(slider), "layers");
	// ���̃s�N�Z���f�[�^
	uint8 **pixel_data = (uint8**)g_object_get_data(G_OBJECT(slider), "pixel_data");
	// �K�p���郌�C���[�̐�
	uint16 num_layer = (uint16)g_object_get_data(G_OBJECT(slider), "num_layer");
	unsigned int i;	// for���p�̃J�E���^
	((CHANGE_BRIGHT_CONTRAST*)filter_data)->bright
		= (int8)gtk_adjustment_get_value(slider);

	// ��x���̃s�N�Z���f�[�^�ɖ߂�
	for(i=0; i<num_layer; i++)
	{
		(void)memcpy(layers[i]->pixels, pixel_data[i],
			layers[i]->width * layers[i]->height * layers[i]->channel);
	}

	ChangeBrightContrastFilter(window, layers, num_layer, filter_data);
}

/***************************************
* ChangeContrastValueCallBack�֐�      *
* �R���g���X�g�ύX���̃R�[���o�b�N�֐� *
* ����                                 *
* slider		: �X���C�_�̃A�W���X�^ *
* filter_data	: �t�B���^�[�p�̃f�[�^ *
***************************************/
static void ChangeContrastValueCallBack(
	GtkAdjustment* slider,
	void* filter_data
)
{
	// �`��̈�
	DRAW_WINDOW *window = (DRAW_WINDOW*)g_object_get_data(G_OBJECT(slider), "draw_window");
	// �K�p���郌�C���[
	LAYER **layers = (LAYER**)g_object_get_data(G_OBJECT(slider), "layers");
	// ���̃s�N�Z���f�[�^
	uint8 **pixel_data = (uint8**)g_object_get_data(G_OBJECT(slider), "pixel_data");
	// �K�p���郌�C���[�̐�
	uint16 num_layer = (uint16)g_object_get_data(G_OBJECT(slider), "num_layer");
	unsigned int i;	// for���p�̃J�E���^
	((CHANGE_BRIGHT_CONTRAST*)filter_data)->contrast
		= (int8)gtk_adjustment_get_value(slider);

	// ��x���̃s�N�Z���f�[�^�ɖ߂�
	for(i=0; i<num_layer; i++)
	{
		(void)memcpy(layers[i]->pixels, pixel_data[i],
			layers[i]->width * layers[i]->height * layers[i]->channel);
	}

	ChangeBrightContrastFilter(window, layers, num_layer, filter_data);
}

/*****************************************************
* ExecuteChangeBrightContrastFilter�֐�              *
* ���邳�R���g���X�g�t�B���^�����s                   *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
void ExecuteChangeBrightContrastFilter(APPLICATION* app)
{
	// �g�傷��s�N�Z�������w�肷��_�C�A���O
	GtkWidget* dialog = gtk_dialog_new_with_buttons(
		app->labels->menu.bright_contrast,
		GTK_WINDOW(app->window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL
	);
	// ��������`��̈�
	DRAW_WINDOW* window = app->draw_window[app->active_window];
	// ���邳�R���g���X�g�̒����l
	CHANGE_BRIGHT_CONTRAST change_value = {0, 0};
	// ���邳�R���g���X�g�����l�w��p�̃E�B�W�F�b�g
	GtkWidget* label, *scale;
	// �����l�ݒ�X���C�_�p�̃A�W���X�^
	GtkAdjustment* adjust;
	// �t�B���^�[��K�p���郌�C���[�̐�
	uint16 num_layer;
	// �t�B���^�[��K�p���郌�C���[
	LAYER **layers = GetLayerChain(window, &num_layer);
	// �����O�̃s�N�Z���f�[�^
	uint8 **pixel_data = (uint8**)MEM_ALLOC_FUNC(sizeof(uint8*)*num_layer);
	// �_�C�A���O�̌���
	gint result;
	// int�^�Ń��C���[�̐����L�����Ă���(�L���X�g�p)
	int int_num_layer = num_layer;
	unsigned int i;	// for���p�̃J�E���^

	// �s�N�Z���f�[�^�̃R�s�[���쐬
	for(i=0; i<num_layer; i++)
	{
		size_t buff_size = layers[i]->width * layers[i]->height * layers[i]->channel;
		pixel_data[i] = MEM_ALLOC_FUNC(buff_size);
		(void)memcpy(pixel_data[i], layers[i]->pixels, buff_size);
	}

	// �_�C�A���O�ɃE�B�W�F�b�g������
		// ���邳��
	adjust = GTK_ADJUSTMENT(gtk_adjustment_new(0, -127, 127, 1, 1, 1));
	scale = gtk_hscale_new(adjust);
	g_object_set_data(G_OBJECT(adjust), "draw_window", window);
	g_object_set_data(G_OBJECT(adjust), "pixel_data", pixel_data);
	g_object_set_data(G_OBJECT(adjust), "layers", layers);
	g_object_set_data(G_OBJECT(adjust), "num_layer", GINT_TO_POINTER(int_num_layer));
	(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
		G_CALLBACK(ChangeBrightnessValueCallBack), &change_value);
	label = gtk_label_new(app->labels->tool_box.brightness);
	gtk_scale_set_digits(GTK_SCALE(scale), 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		scale, FALSE, TRUE, 0);

	// �R���g���X�g��
	adjust = GTK_ADJUSTMENT(gtk_adjustment_new(0, -127, 127, 1, 1, 1));
	scale = gtk_hscale_new(adjust);
	g_object_set_data(G_OBJECT(adjust), "draw_window", window);
	g_object_set_data(G_OBJECT(adjust), "pixel_data", pixel_data);
	g_object_set_data(G_OBJECT(adjust), "layers", layers);
	g_object_set_data(G_OBJECT(adjust), "num_layer", GINT_TO_POINTER(int_num_layer));
	(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
		G_CALLBACK(ChangeContrastValueCallBack), &change_value);
	label = gtk_label_new(app->labels->tool_box.contrast);
	gtk_scale_set_digits(GTK_SCALE(scale), 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		scale, FALSE, TRUE, 0);

	gtk_widget_show_all(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));

	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if(result == GTK_RESPONSE_ACCEPT)
	{	// O.K.�{�^���������ꂽ
			// ��x�A�s�N�Z���f�[�^�����ɖ߂���
		for(i=0; i<num_layer; i++)
		{
			(void)memcpy(window->temp_layer->pixels, layers[i]->pixels,
				window->pixel_buf_size);
			(void)memcpy(layers[i]->pixels, pixel_data[i], window->pixel_buf_size);
			(void)memcpy(pixel_data[i], window->temp_layer->pixels, window->pixel_buf_size);
		}

		// ��ɗ����f�[�^���c��
		AddFilterHistory(app->labels->menu.blur, &change_value, sizeof(change_value),
			FILTER_FUNC_BRIGHTNESS_CONTRAST, layers, num_layer, window);

		// ���邳�R���g���X�g�������s��̃f�[�^�ɖ߂�
		for(i=0; i<num_layer; i++)
		{
			(void)memcpy(layers[i]->pixels, pixel_data[i],
				layers[i]->width * layers[i]->height * layers[i]->channel);
		}
	}
	else
	{	// �L�����Z�����ꂽ��
			// �s�N�Z���f�[�^�����ɖ߂�
		for(i=0; i<num_layer; i++)
		{
			(void)memcpy(layers[i]->pixels, pixel_data[i],
				layers[i]->width * layers[i]->height * layers[i]->channel);
		}
	}

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;

	for(i=0; i<num_layer; i++)
	{
		MEM_FREE_FUNC(pixel_data[i]);
	}
	MEM_FREE_FUNC(pixel_data);

	MEM_FREE_FUNC(layers);

	// �_�C�A���O������
	gtk_widget_destroy(dialog);
}

/*****************************************
* CHANGE_HUE_SATURATION�\����            *
* �F���E�ʓx�E�P�x�ύX�t�B���^�[�̃f�[�^ *
*****************************************/
typedef struct _CHANGE_HUE_SATURATION_DATA
{
	int16 hue;			// �F��
	int16 saturation;	// �ʓx
	int16 vivid;		// �P�x
} CHANGE_HUE_SATURATION_DATA;

/***************************************
* ChangeHueSaturationFilter�֐�        *
* �F���E�ʓx�E�P�x��ύX����t�B���^�[ *
* ����                                 *
* window	: �`��̈�̏��           *
* layers	: �������s�����C���[�z��   *
* num_layer	: �������s�����C���[�̐�   *
* data		: �ڂ��������̏ڍ׃f�[�^   *
***************************************/
void ChangeHueSaturationFilter(
	DRAW_WINDOW* window,
	LAYER** layers,
	uint16 num_layer,
	void* data
)
{
	// �F���E�ʓx�E�P�x�̃f�[�^�ɃL���X�g
	CHANGE_HUE_SATURATION_DATA *filter_data =
		(CHANGE_HUE_SATURATION_DATA*)data;
	// �摜��HSV�f�[�^�ɉ�����l
	int change_h = filter_data->hue,
		change_s = (int)(filter_data->saturation * 0.01 * 255),
		change_v = (int)(filter_data->vivid * 0.01 * 255);
	// for���p�̃J�E���^
	int i, j;

	// �e���C���[�ɑ΂��F���E�ʓx�E�P�x�̕ύX�����s
	for(i=0; i<(int)num_layer; i++)
	{
		// �ʏ탌�C���[�Ȃ�
		if(layers[i]->layer_type == TYPE_NORMAL_LAYER)
		{
			(void)memset(window->mask_temp->pixels, 0, window->pixel_buf_size);
			// �S�Ẵs�N�Z���ɑ΂��ď������s
#ifdef _OPENMP
#pragma omp parallel for
#endif
			for(j=0; j<(int)(layers[i]->width*layers[i]->height); j++)
			{
				// �ϊ����HSV�̒l
				int next_h, next_s, next_v;
				// 1�s�N�Z�����̃f�[�^
				uint8 rgb[3];
				// HSV�f�[�^
				HSV hsv;
				// �s�N�Z���z��̃C���f�b�N�X
				int index;

				index = j * layers[i]->channel;

				// �s�N�Z���f�[�^���R�s�[
				rgb[0] = layers[i]->pixels[index];
				rgb[1] = layers[i]->pixels[index+1];
				rgb[2] = layers[i]->pixels[index+2];

				// HSV�f�[�^�ɕϊ�
				RGB2HSV_Pixel(rgb, &hsv);

				// ���̓f�[�^�𔽉f
				next_h = hsv.h + change_h;
				if(next_h < 0)
				{
					next_h = 360 + next_h;
				}
				else if(next_h >= 360)
				{
					next_h = next_h - 360;
				}
				next_s = hsv.s + change_s;
				if(next_s < 0)
				{
					next_s = 0;
				}
				else if(next_s > 255)
				{
					next_s = 255;
				}
				next_v = hsv.v + change_v;
				if(next_v < 0)
				{
					next_v = 0;
				}
				else if(next_v > 255)
				{
					next_v = 255;
				}

				hsv.h = (int16)next_h;
				hsv.s = (uint8)next_s;
				hsv.v = (uint8)next_v;

				// HSV��RGB�ɖ߂�
				HSV2RGB_Pixel(&hsv, rgb);

				// �s�N�Z���f�[�^���X�V
				window->temp_layer->pixels[index] = rgb[0];
				window->temp_layer->pixels[index+1] = rgb[1];
				window->temp_layer->pixels[index+2] = rgb[2];
				window->temp_layer->pixels[index+3] = 0xff;
				window->mask_temp->pixels[index+3] = layers[i]->pixels[index+3];
			}	// �S�Ẵs�N�Z���ɑ΂��ď������s
				// for(j=0; j<(unsigned int)(layers[i]->width*layers[i]->height); j++)

			if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) != 0)
			{
				uint8 select_value;
				for(j=0; j<(int)(window->width*window->height); j++)
				{
					select_value = window->selection->pixels[j];
					window->temp_layer->pixels[j*4+0] = ((0xff-select_value)*layers[i]->pixels[j*4+0]
						+ window->temp_layer->pixels[j*4+0]*select_value) / 255;
					window->temp_layer->pixels[j*4+1] = ((0xff-select_value)*layers[i]->pixels[j*4+1]
						+ window->temp_layer->pixels[j*4+1]*select_value) / 255;
					window->temp_layer->pixels[j*4+2] = ((0xff-select_value)*layers[i]->pixels[j*4+2]
						+ window->temp_layer->pixels[j*4+2]*select_value) / 255;
					window->temp_layer->pixels[j*4+3] = ((0xff-select_value)*layers[i]->pixels[j*4+3]
						+ window->temp_layer->pixels[j*4+3]*select_value) / 255;
				}
			}

			for(j=0; j<(int)(layers[i]->width*layers[i]->height); j++)
			{
				layers[i]->pixels[j*4] = MINIMUM(window->temp_layer->pixels[j*4], layers[i]->pixels[j*4+3]);
				layers[i]->pixels[j*4+1] = MINIMUM(window->temp_layer->pixels[j*4+1], layers[i]->pixels[j*4+3]);
				layers[i]->pixels[j*4+2] = MINIMUM(window->temp_layer->pixels[j*4+2], layers[i]->pixels[j*4+3]);
			}
		}	// �ʏ탌�C���[�Ȃ�
			// if(layers[i]->layer_type == TYPE_NORMAL_LAYER)
	}	// �e���C���[�ɑ΂��F���E�ʓx�E�P�x�̕ύX�����s
		// for(i=0; i<num_layer; i++)

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	gtk_widget_queue_draw(window->window);
}

/***************************************
* ChangeHueValueCallBack�֐�           *
* �F���ύX���̃R�[���o�b�N�֐�         *
* ����                                 *
* slider		: �X���C�_�̃A�W���X�^ *
* filter_data	: �t�B���^�[�p�̃f�[�^ *
***************************************/
static void ChangeHueValueCallBack(
	GtkAdjustment* slider,
	void* filter_data
)
{
	// �`��̈�
	DRAW_WINDOW *window = (DRAW_WINDOW*)g_object_get_data(G_OBJECT(slider), "draw_window");
	// �K�p���郌�C���[
	LAYER **layers = (LAYER**)g_object_get_data(G_OBJECT(slider), "layers");
	// ���̃s�N�Z���f�[�^
	uint8 **pixel_data = (uint8**)g_object_get_data(G_OBJECT(slider), "pixel_data");
	// �K�p���郌�C���[�̐�
	uint16 num_layer = (uint16)g_object_get_data(G_OBJECT(slider), "num_layer");
	unsigned int i;	// for���p�̃J�E���^
	((CHANGE_HUE_SATURATION_DATA*)filter_data)->hue
		= (int16)gtk_adjustment_get_value(slider);

	// ��x���̃s�N�Z���f�[�^�ɖ߂�
	for(i=0; i<num_layer; i++)
	{
		(void)memcpy(layers[i]->pixels, pixel_data[i],
			layers[i]->width * layers[i]->height * layers[i]->channel);
	}

	ChangeHueSaturationFilter(window, layers, num_layer, filter_data);
}

/***************************************
* ChangeSaturationValueCallBack�֐�    *
* �ʓx�ύX���̃R�[���o�b�N�֐�         *
* ����                                 *
* slider		: �X���C�_�̃A�W���X�^ *
* filter_data	: �t�B���^�[�p�̃f�[�^ *
***************************************/
static void ChangeSaturationValueCallBack(
	GtkAdjustment* slider,
	void* filter_data
)
{
	// �`��̈�
	DRAW_WINDOW *window = (DRAW_WINDOW*)g_object_get_data(G_OBJECT(slider), "draw_window");
	// �K�p���郌�C���[
	LAYER **layers = (LAYER**)g_object_get_data(G_OBJECT(slider), "layers");
	// ���̃s�N�Z���f�[�^
	uint8 **pixel_data = (uint8**)g_object_get_data(G_OBJECT(slider), "pixel_data");
	// �K�p���郌�C���[�̐�
	uint16 num_layer = (uint16)g_object_get_data(G_OBJECT(slider), "num_layer");
	unsigned int i;	// for���p�̃J�E���^
	((CHANGE_HUE_SATURATION_DATA*)filter_data)->saturation
		= (int16)gtk_adjustment_get_value(slider);

	// ��x���̃s�N�Z���f�[�^�ɖ߂�
	for(i=0; i<num_layer; i++)
	{
		(void)memcpy(layers[i]->pixels, pixel_data[i],
			layers[i]->width * layers[i]->height * layers[i]->channel);
	}

	ChangeHueSaturationFilter(window, layers, num_layer, filter_data);
}

/***************************************
* ChangeVividValueCallBack�֐�         *
* �P�x�ύX���̃R�[���o�b�N�֐�         *
* ����                                 *
* slider		: �X���C�_�̃A�W���X�^ *
* filter_data	: �t�B���^�[�p�̃f�[�^ *
***************************************/
static void ChangeVividValueCallBack(
	GtkAdjustment* slider,
	void* filter_data
)
{
	// �`��̈�
	DRAW_WINDOW *window = (DRAW_WINDOW*)g_object_get_data(G_OBJECT(slider), "draw_window");
	// �K�p���郌�C���[
	LAYER **layers = (LAYER**)g_object_get_data(G_OBJECT(slider), "layers");
	// ���̃s�N�Z���f�[�^
	uint8 **pixel_data = (uint8**)g_object_get_data(G_OBJECT(slider), "pixel_data");
	// �K�p���郌�C���[�̐�
	uint16 num_layer = (uint16)g_object_get_data(G_OBJECT(slider), "num_layer");
	unsigned int i;	// for���p�̃J�E���^
	((CHANGE_HUE_SATURATION_DATA*)filter_data)->vivid
		= (int16)gtk_adjustment_get_value(slider);

	// ��x���̃s�N�Z���f�[�^�ɖ߂�
	for(i=0; i<num_layer; i++)
	{
		(void)memcpy(layers[i]->pixels, pixel_data[i],
			layers[i]->width * layers[i]->height * layers[i]->channel);
	}

	ChangeHueSaturationFilter(window, layers, num_layer, filter_data);
}

/*****************************************************
* ExecuteChangeHueSaturationFilter�֐�               *
* �F���E�ʓx�t�B���^�����s                           *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
void ExecuteChangeHueSaturationFilter(APPLICATION* app)
{
	// �g�傷��s�N�Z�������w�肷��_�C�A���O
	GtkWidget* dialog = gtk_dialog_new_with_buttons(
		app->labels->menu.bright_contrast,
		GTK_WINDOW(app->window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL
	);
	// ��������`��̈�
	DRAW_WINDOW* window = app->draw_window[app->active_window];
	// ���邳�R���g���X�g�̒����l
	CHANGE_HUE_SATURATION_DATA change_value = {0, 0, 0};
	// ���邳�R���g���X�g�����l�w��p�̃E�B�W�F�b�g
	GtkWidget* label, *scale;
	// �����l�ݒ�X���C�_�p�̃A�W���X�^
	GtkAdjustment* adjust;
	// �t�B���^�[��K�p���郌�C���[�̐�
	uint16 num_layer;
	// �t�B���^�[��K�p���郌�C���[
	LAYER **layers = GetLayerChain(window, &num_layer);
	// �����O�̃s�N�Z���f�[�^
	uint8 **pixel_data = (uint8**)MEM_ALLOC_FUNC(sizeof(uint8*)*num_layer);
	// �_�C�A���O�̌���
	gint result;
	// int�^�Ń��C���[�̐����L�����Ă���(�L���X�g�p)
	int int_num_layer = num_layer;
	unsigned int i;	// for���p�̃J�E���^

	// �s�N�Z���f�[�^�̃R�s�[���쐬
	for(i=0; i<num_layer; i++)
	{
		size_t buff_size = layers[i]->width * layers[i]->height * layers[i]->channel;
		pixel_data[i] = MEM_ALLOC_FUNC(buff_size);
		(void)memcpy(pixel_data[i], layers[i]->pixels, buff_size);
	}

	// �_�C�A���O�ɃE�B�W�F�b�g������
		// �F����
	adjust = GTK_ADJUSTMENT(gtk_adjustment_new(0, -180, 180, 1, 1, 1));
	scale = gtk_hscale_new(adjust);
	g_object_set_data(G_OBJECT(adjust), "draw_window", window);
	g_object_set_data(G_OBJECT(adjust), "pixel_data", pixel_data);
	g_object_set_data(G_OBJECT(adjust), "layers", layers);
	g_object_set_data(G_OBJECT(adjust), "num_layer", GINT_TO_POINTER(int_num_layer));
	(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
		G_CALLBACK(ChangeHueValueCallBack), &change_value);
	label = gtk_label_new(app->labels->tool_box.hue);
	gtk_scale_set_digits(GTK_SCALE(scale), 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		scale, FALSE, TRUE, 0);

	// �ʓx��
	adjust = GTK_ADJUSTMENT(gtk_adjustment_new(0, -100, 100, 1, 1, 1));
	scale = gtk_hscale_new(adjust);
	g_object_set_data(G_OBJECT(adjust), "draw_window", window);
	g_object_set_data(G_OBJECT(adjust), "pixel_data", pixel_data);
	g_object_set_data(G_OBJECT(adjust), "layers", layers);
	g_object_set_data(G_OBJECT(adjust), "num_layer", GINT_TO_POINTER(int_num_layer));
	(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
		G_CALLBACK(ChangeSaturationValueCallBack), &change_value);
	label = gtk_label_new(app->labels->tool_box.saturation);
	gtk_scale_set_digits(GTK_SCALE(scale), 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		scale, FALSE, TRUE, 0);

	// �P�x��
	adjust = GTK_ADJUSTMENT(gtk_adjustment_new(0, -100, 100, 1, 1, 1));
	scale = gtk_hscale_new(adjust);
	g_object_set_data(G_OBJECT(adjust), "draw_window", window);
	g_object_set_data(G_OBJECT(adjust), "pixel_data", pixel_data);
	g_object_set_data(G_OBJECT(adjust), "layers", layers);
	g_object_set_data(G_OBJECT(adjust), "num_layer", GINT_TO_POINTER(int_num_layer));
	(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
		G_CALLBACK(ChangeVividValueCallBack), &change_value);
	label = gtk_label_new(app->labels->tool_box.brightness);
	gtk_scale_set_digits(GTK_SCALE(scale), 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		scale, FALSE, TRUE, 0);

	gtk_widget_show_all(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));

	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if(result == GTK_RESPONSE_ACCEPT)
	{	// O.K.�{�^���������ꂽ
			// ��x�A�s�N�Z���f�[�^�����ɖ߂���
		for(i=0; i<num_layer; i++)
		{
			(void)memcpy(window->temp_layer->pixels, layers[i]->pixels,
				window->pixel_buf_size);
			(void)memcpy(layers[i]->pixels, pixel_data[i], window->pixel_buf_size);
			(void)memcpy(pixel_data[i], window->temp_layer->pixels, window->pixel_buf_size);
		}

		// ��ɗ����f�[�^���c��
		AddFilterHistory(app->labels->menu.bright_contrast, &change_value, sizeof(change_value),
			FILTER_FUNC_HUE_SATURATION, layers, num_layer, window);

		// �F���E�ʓx�E�P�x�������s��̃f�[�^�ɖ߂�
		for(i=0; i<num_layer; i++)
		{
			(void)memcpy(layers[i]->pixels, pixel_data[i],
				layers[i]->width * layers[i]->height * layers[i]->channel);
		}
	}
	else
	{	// �L�����Z�����ꂽ��
			// �s�N�Z���f�[�^�����ɖ߂�
		for(i=0; i<num_layer; i++)
		{
			(void)memcpy(layers[i]->pixels, pixel_data[i],
				layers[i]->width * layers[i]->height * layers[i]->channel);
		}
	}

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;

	for(i=0; i<num_layer; i++)
	{
		MEM_FREE_FUNC(pixel_data[i]);
	}
	MEM_FREE_FUNC(pixel_data);

	MEM_FREE_FUNC(layers);

	// �_�C�A���O������
	gtk_widget_destroy(dialog);
}

/*************************************
* Luminosity2OpacityFilter�֐�       *
* �P�x�𓧖��x�ɂ���                 *
* ����                               *
* window	: �`��̈�̏��         *
* layers	: �������s�����C���[�z�� *
* num_layer	: �������s�����C���[�̐� *
* data		: �_�~�[�f�[�^           *
*************************************/
void Luminosity2OpacityFilter(DRAW_WINDOW* window, LAYER** layers, uint16 num_layer, void* data)
{
	HSV hsv;
	uint8 buff;
	unsigned int i;
	int j;

	for(i=0; i<num_layer; i++)
	{
		(void)memset(window->temp_layer->pixels, 0, window->pixel_buf_size);

#ifdef _OPENMP
#pragma omp parallel for
#endif
		for(j=0; j<layers[i]->width*layers[i]->height; j++)
		{
			uint8 *pix = &layers[i]->pixels[j*4];
			uint8 *dst = &window->temp_layer->pixels[j*4];
			RGB2HSV_Pixel(pix, &hsv);
			buff = hsv.v;//0xff - hsv.v;
			dst[3] = (pix[3] > buff) ? pix[3] - buff : 0;
		}

		if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) != 0)
		{
#ifdef _OPENMP
#pragma omp parallel for
#endif
			for(j=0; j<window->width*window->height; j++)
			{
				uint8 select_value = window->selection->pixels[j];
				window->temp_layer->pixels[j*4+0] = ((0xff-select_value)*layers[i]->pixels[j*4+0]
					+ window->temp_layer->pixels[j*4+0]*select_value) / 255;
				window->temp_layer->pixels[j*4+1] = ((0xff-select_value)*layers[i]->pixels[j*4+1]
					+ window->temp_layer->pixels[j*4+1]*select_value) / 255;
				window->temp_layer->pixels[j*4+2] = ((0xff-select_value)*layers[i]->pixels[j*4+2]
					+ window->temp_layer->pixels[j*4+2]*select_value) / 255;
				window->temp_layer->pixels[j*4+3] = ((0xff-select_value)*layers[i]->pixels[j*4+3]
					+ window->temp_layer->pixels[j*4+3]*select_value) / 255;
			}
		}

		(void)memset(window->mask_temp->pixels, 0, window->pixel_buf_size);
		cairo_set_source_surface(window->mask_temp->cairo_p, layers[i]->surface_p, 0, 0);
		cairo_mask_surface(window->mask_temp->cairo_p, window->temp_layer->surface_p, 0, 0);

		(void)memcpy(layers[i]->pixels, window->mask_temp->pixels, window->pixel_buf_size);
	}

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	gtk_widget_queue_draw(window->window);
}

/*****************************************************
* ExecuteLuminosity2OpacityFilter�֐�                *
* �P�x�𓧖��x�֕ϊ������s                           *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
void ExecuteLuminosity2OpacityFilter(APPLICATION* app)
{
	DRAW_WINDOW* window =	// ��������`��̈�
		app->draw_window[app->active_window];
	// �_�~�[�f�[�^
	int32 dummy = 0;
	// ���C���[�̐�
	uint16 num_layer;
	// ���������s���郌�C���[
	LAYER** layers = GetLayerChain(window, &num_layer);

	// ��ɗ����f�[�^���c��
	AddFilterHistory(app->labels->menu.blur, &dummy, sizeof(dummy),
		FILTER_FUNC_LUMINOSITY2OPACITY, layers, num_layer, window);

	// �ϊ����s
	Luminosity2OpacityFilter(window, layers, num_layer, &dummy);

	(void)memcpy(window->active_layer->pixels, window->temp_layer->pixels, window->pixel_buf_size);

	MEM_FREE_FUNC(layers);

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
}

typedef struct _COLOR2ALPHA
{
	uint8 color[3];
	uint8 threshold;
} COLOR2ALPHA;

/*****************************************
* Color2AlphaFilter�֐�                  *
* �w��F�𓧖��ɂ���                     *
* ����                                   *
* window	: �`��̈�̏��             *
* layers	: �������s�����C���[�z��     *
* num_layer	: �������s�����C���[�̐�     *
* data		: �t�B���^�[�̏ڍאݒ�f�[�^ *
*****************************************/
static void Color2AlphaFilter(DRAW_WINDOW* window, LAYER** layers, uint16 num_layer, void* data)
{
	// �t�B���^�[�̏ڍאݒ�̓��e�ɃA�N�Z�X�ł���悤�ɃL���X�g
	COLOR2ALPHA *setting = (COLOR2ALPHA*)data;
	// �w��F
	uint8 color[3];
	// ���ȏ�̃��l�����s�N�Z���͕s�����ɂ���
	uint8 upper_threshold;
	// �w��F�Ƃ̍�
	int difference;
	// �s�N�Z���̃��l
	int alpha;
	// �s�N�Z���̃C���f�b�N�X
	int index;
	// for���p�̃J�E���^
	unsigned int i;
	int j;

	(void)memcpy(color, setting->color, sizeof(color));
#if defined(USE_BGR_COLOR_SPACE) && USE_BGR_COLOR_SPACE != 0
	{
		uint8 r;
		r = color[0];
		color[0] = color[2];
		color[2] = r;
	}
#endif
	upper_threshold = 0xff - setting->threshold;

	for(i=0; i<num_layer; i++)
	{
		if(layers[i]->layer_type == TYPE_NORMAL_LAYER)
		{
			for(j=0; j<layers[i]->width * layers[i]->height; j++)
			{
				index = j*4;
				difference = abs((int)setting->color[0] - (int)layers[i]->pixels[index+0]);
				difference += abs((int)setting->color[1] - (int)layers[i]->pixels[index+1]);
				difference += abs((int)setting->color[2] - (int)layers[i]->pixels[index+2]);
				if(difference > 0xff)
				{
					difference = 0xff;
				}
				alpha = layers[i]->pixels[index+3] - (0xff - difference);
				if(alpha < setting->threshold)
				{
					alpha = setting->threshold;
				}
				else if(alpha > upper_threshold)
				{
					alpha = 0xff;
				}
				layers[i]->pixels[index+0] = MINIMUM(alpha, layers[i]->pixels[index+0]);
				layers[i]->pixels[index+1] = MINIMUM(alpha, layers[i]->pixels[index+1]);
				layers[i]->pixels[index+2] = MINIMUM(alpha, layers[i]->pixels[index+2]);
				layers[i]->pixels[index+3] = alpha;
			}
		}
	}
}

/*****************************************************
* ExecuteColor2AlphaFilter�֐�                       *
* �w��F�𓧖��x�֕ϊ������s                         *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
void ExecuteColor2AlphaFilter(APPLICATION* app)
{
	DRAW_WINDOW* window =	// ��������`��̈�
		app->draw_window[app->active_window];
	// �t�B���^�[�̐ݒ�f�[�^
	COLOR2ALPHA filter_data = {0};
	// �����ɂ���F
	GdkColor color;
	// ���C���[�̐�
	uint16 num_layer;
	// ���������s���郌�C���[
	LAYER** layers = GetLayerChain(window, &num_layer);
	// �t�B���^�[�̐ݒ�p�_�C�A���O�E�B�W�F�b�g
	GtkWidget *dialog;
	// �F�w��E�B�W�F�b�g
	GtkWidget *color_button;
	// 臒l�ݒ�E�B�W�F�b�g
	GtkWidget *threshold_spin_button;

	dialog = gtk_dialog_new_with_buttons(
		app->labels->menu.color2alpha,
		GTK_WINDOW(app->window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		NULL
	);
	// �f�t�H���g�͔�
	color.red = 0xffff;
	color.green = 0xffff;
	color.blue = 0xffff;
	color_button = gtk_color_button_new_with_color(&color);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		color_button, FALSE, FALSE, 0);
	threshold_spin_button = gtk_spin_button_new_with_range(0, 0xff, 1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(threshold_spin_button), 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		threshold_spin_button, FALSE, FALSE, 0);

	gtk_widget_show_all(dialog);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
	{
		// �ݒ肳�ꂽ�f�[�^���擾
		gtk_color_button_get_color(GTK_COLOR_BUTTON(color_button), &color);
		filter_data.color[0] = color.red / 256;
		filter_data.color[1] = color.green / 256;
		filter_data.color[2] = color.blue / 256;
		filter_data.threshold = (uint8)gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON(threshold_spin_button));
		// ��ɗ����f�[�^���c��
		AddFilterHistory(app->labels->menu.color2alpha, &filter_data, sizeof(filter_data),
			FILTER_FUNC_COLOR2ALPHA, layers, num_layer, window);

		// �ϊ����s
		Color2AlphaFilter(window, layers, num_layer, &filter_data);
	}

	MEM_FREE_FUNC(layers);

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
}

typedef enum _eCOLORIZE_WITH_UNDER_TARGET
{
	COLORIZE_WITH_UNDER_LAYER,
	COLORIZE_WITH_MIXED_UNDER
} eCOLORIZE_WITH_UNDER_TARGET;

typedef struct _COLORIZE_WITH_UNDER
{
	int16 add_h, add_s, add_v;	// �F�̒����l
	int16 size;					// �F����Ɏg���s�N�Z���͈̔�
	int8 color_from;			// �F�������Ă���Ώ�
} COLORIZE_WITH_UNDER;

/*************************************
* ColorizeWithUnderFilter�֐�        *
* ���̃��C���[�Œ��F�t�B���^�[       *
* ����                               *
* window	: �`��̈�̏��         *
* layers	: �������s�����C���[�z�� *
* num_layer	: �������s�����C���[�̐� *
* data		: �F�����E�擾�͈̓f�[�^ *
*************************************/
void ColorizeWithUnderFilter(DRAW_WINDOW* window, LAYER** layers, uint16 num_layer, void* data)
{
	COLORIZE_WITH_UNDER *adjust = (COLORIZE_WITH_UNDER*)data;
	LAYER *target;
	int delete_target = 0;
	int x, y;

	cairo_set_operator(window->temp_layer->cairo_p, CAIRO_OPERATOR_OVER);

	if(adjust->color_from == COLORIZE_WITH_UNDER_LAYER)
	{
		if(((*layers)->prev->flags & LAYER_MASKING_WITH_UNDER_LAYER) == 0)
		{
			target = (*layers)->prev;
		}
		else
		{
			LAYER *base = (*layers)->prev;
			LAYER *src;

			while(base->prev != NULL && (base->flags & LAYER_MASKING_WITH_UNDER_LAYER) != 0)
			{
				base = base->prev;
			}

			(void)memcpy(window->mask->pixels, base->pixels, window->pixel_buf_size);
			src = base->next;
			while(src != (*layers))
			{
				(void)memset(window->temp_layer->pixels, 0, window->pixel_buf_size);
				cairo_set_source_surface(window->temp_layer->cairo_p, src->surface_p, 0, 0);
				cairo_mask_surface(window->temp_layer->cairo_p, base->surface_p, 0, 0);
				cairo_set_source_surface(window->mask->cairo_p, window->temp_layer->surface_p, 0, 0);
				cairo_paint_with_alpha(window->mask->cairo_p, src->alpha * 0.01);
				src = src->next;
			}
		}
	}
	else
	{
		target = GetBlendedUnderLayer(*layers, window, TRUE);
		delete_target++;
	}

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for(y=0; y<(*layers)->height; y++)
	{
		uint8 src_color[4];
		HSV hsv;
		int h, s, v;
		int x;

		for(x=0; x<(*layers)->width; x++)
		{
			if((*layers)->pixels[y*(*layers)->stride+x*4+3] > 0)
			{
				GetAverageColor(target, x, y, adjust->size, src_color);
				if(src_color[3] > 0)
				{
					RGB2HSV_Pixel(src_color, &hsv);
					h = hsv.h + adjust->add_h;
					s = hsv.s + adjust->add_s;
					v = hsv.v + adjust->add_v;

					if(h < 0)
					{
						h = 0;
					}
					else if(h >= 360)
					{
						h = 360 - 1;
					}
					if(s < 0)
					{
						s = 0;
					}
					else if(s > 0xff)
					{
						s = 0xff;
					}
					if(v < 0)
					{
						v = 0;
					}
					else if(v > 0xff)
					{
						v = 0xff;
					}

					hsv.h = (int16)h;
					hsv.s = (uint8)s;
					hsv.v = (uint8)v;
					HSV2RGB_Pixel(&hsv, src_color);

					window->mask_temp->pixels[y*window->mask_temp->stride+x*4+0] = src_color[0];
					window->mask_temp->pixels[y*window->mask_temp->stride+x*4+1] = src_color[1];
					window->mask_temp->pixels[y*window->mask_temp->stride+x*4+2] = src_color[2];
					window->mask_temp->pixels[y*window->mask_temp->stride+x*4+3] = 0xff;
				}
				else
				{
					window->mask_temp->pixels[y*window->mask_temp->stride+x*4+0]
						= (*layers)->pixels[y*(*layers)->stride+x*4+0];
					window->mask_temp->pixels[y*window->mask_temp->stride+x*4+1]
						= (*layers)->pixels[y*(*layers)->stride+x*4+1];
					window->mask_temp->pixels[y*window->mask_temp->stride+x*4+2]
						= (*layers)->pixels[y*(*layers)->stride+x*4+2];
					window->mask_temp->pixels[y*window->mask_temp->stride+x*4+3] = 0xff;
				}
			}
			else
			{
				window->mask_temp->pixels[y*window->mask_temp->stride+x*4+0]
					= (*layers)->pixels[y*(*layers)->stride+x*4+0];
				window->mask_temp->pixels[y*window->mask_temp->stride+x*4+1]
					= (*layers)->pixels[y*(*layers)->stride+x*4+1];
				window->mask_temp->pixels[y*window->mask_temp->stride+x*4+2]
					= (*layers)->pixels[y*(*layers)->stride+x*4+2];
				window->mask_temp->pixels[y*window->mask_temp->stride+x*4+3] = 0xff;
			}
		}
	}

	if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) != 0)
	{
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for(x=0; x<window->width*window->height; x++)
		{
			uint8 select_value = window->selection->pixels[x];
			window->mask_temp->pixels[x*4+0] = ((0xff-select_value)*target->pixels[x*4+0]
				+ window->mask_temp->pixels[x*4+0]*select_value) / 255;
			window->mask_temp->pixels[x*4+1] = ((0xff-select_value)*target->pixels[x*4+1]
				+ window->mask_temp->pixels[x*4+1]*select_value) / 255;
			window->mask_temp->pixels[x*4+2] = ((0xff-select_value)*target->pixels[x*4+2]
				+ window->mask_temp->pixels[x*4+2]*select_value) / 255;
		}
	}

	(void)memset(window->temp_layer->pixels, 0, window->pixel_buf_size);
	cairo_set_source_surface(window->temp_layer->cairo_p, window->mask_temp->surface_p, 0, 0);
	cairo_mask_surface(window->temp_layer->cairo_p, (*layers)->surface_p, 0, 0);
	(void)memcpy((*layers)->pixels, window->temp_layer->pixels, window->pixel_buf_size);

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	gtk_widget_queue_draw(window->window);
}

static void ColorizeWithUnderSetTarget(GtkWidget* radio_button, COLORIZE_WITH_UNDER* filter_data)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_button)) != FALSE)
	{
		filter_data->color_from = (int8)GPOINTER_TO_INT(
			g_object_get_data(G_OBJECT(radio_button), "filter_target"));
	}
}

/*****************************************************
* ExecuteColorizeWithUnderFilter�֐�                 *
* ���̃��C���[�Œ��F�����s                           *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
void ExecuteColorizeWithUnderFilter(APPLICATION* app)
{
	DRAW_WINDOW* window =	// ��������`��̈�
		app->draw_window[app->active_window];
	COLORIZE_WITH_UNDER filter_data =
		{0, 0, 0, 5, COLORIZE_WITH_UNDER_LAYER};
	LAYER *target = window->active_layer;
	GtkWidget *dialog;
	GtkWidget *control;
	GtkWidget *label;
	GtkWidget *hbox;
	GtkAdjustment *adjust;
	gint result;

	// �����\�Ȃ̂͒ʏ탌�C���[�̂�
	if(target->layer_type != TYPE_NORMAL_LAYER)
	{
		return;
	}

	// ���Ƀ��C���[���Ȃ�ΏI��
	if(target->prev == NULL)
	{
		return;
	}

	dialog = gtk_dialog_new_with_buttons(
		app->labels->menu.colorize_with_under,
		GTK_WINDOW(app->window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL
	);

	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(app->labels->tool_box.scale);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	adjust = GTK_ADJUSTMENT(gtk_adjustment_new(5, 1, 30, 1, 1, 0));
	(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
		G_CALLBACK(AdjustmentChangeValueCallBackInt16), &filter_data.size);
	control = gtk_spin_button_new(adjust, 1, 0);
	gtk_box_pack_start(GTK_BOX(hbox), control, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		hbox, FALSE, FALSE, 0);

	label = gtk_label_new(app->labels->tool_box.hue);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		label, FALSE, FALSE, 0);
	adjust = GTK_ADJUSTMENT(gtk_adjustment_new(0, -180, 180, 1, 1, 0));
	(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
		G_CALLBACK(AdjustmentChangeValueCallBackInt16), &filter_data.add_h);
	control = gtk_hscale_new(adjust);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		control, FALSE, FALSE, 0);

	label = gtk_label_new(app->labels->tool_box.saturation);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		label, FALSE, FALSE, 0);
	adjust = GTK_ADJUSTMENT(gtk_adjustment_new(0, -100, 100, 1, 1, 0));
	(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
		G_CALLBACK(AdjustmentChangeValueCallBackInt16), &filter_data.add_s);
	control = gtk_hscale_new(adjust);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		control, FALSE, FALSE, 0);

	label = gtk_label_new(app->labels->tool_box.brightness);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		label, FALSE, FALSE, 0);
	adjust = GTK_ADJUSTMENT(gtk_adjustment_new(0, -100, 100, 1, 1, 0));
	(void)g_signal_connect(G_OBJECT(adjust), "value_changed",
		G_CALLBACK(AdjustmentChangeValueCallBackInt16), &filter_data.add_v);
	control = gtk_hscale_new(adjust);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		control, FALSE, FALSE, 0);

	control = gtk_radio_button_new_with_label(NULL, app->labels->layer_window.under_layer);
	(void)g_signal_connect(G_OBJECT(control), "toggled",
		G_CALLBACK(ColorizeWithUnderSetTarget), &filter_data);
	g_object_set_data(G_OBJECT(control), "filter_target", GINT_TO_POINTER(COLORIZE_WITH_UNDER_LAYER));
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		control, FALSE, FALSE, 0);
	control = gtk_radio_button_new_with_label(gtk_radio_button_get_group(
		GTK_RADIO_BUTTON(control)), app->labels->layer_window.mixed_under_layer);
	(void)g_signal_connect(G_OBJECT(control), "toggled",
		G_CALLBACK(ColorizeWithUnderSetTarget), &filter_data);
	g_object_set_data(G_OBJECT(control), "filter_target", GINT_TO_POINTER(COLORIZE_WITH_MIXED_UNDER));
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		control, FALSE, FALSE, 0);

	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if(result == GTK_RESPONSE_ACCEPT)
	{
		filter_data.add_s = (int16)(filter_data.add_s * 2.55);
		filter_data.add_v = (int16)(filter_data.add_v * 2.55);

		AddFilterHistory(app->labels->menu.colorize_with_under, (void*)&filter_data, sizeof(filter_data),
			FILTER_FUNC_COLORIZE_WITH_UNDER, &window->active_layer, 1, window);

		ColorizeWithUnderFilter(window, &window->active_layer, 1, (void*)&filter_data);
	}

	gtk_widget_destroy(dialog);
}

typedef enum _eGRADATION_MAP_FLAGS
{
	GRADATION_MAP_DETECT_MAX = 0x01,
	GRADATION_MAP_TRANSPARANCY_AS_WHITE = 0x02,
	GRADATION_MAP_MASK_WITH_UNDER = 0x04,
	GRADATION_MAP_LOCK_OPACITY = 0x08
} eGRADATION_MAP_FLAGS;

typedef struct _GRADATION_MAP
{
	guint32 flags;
	uint8 fore_ground[3];
	uint8 back_ground[3];
} GRADATION_MAP;

/*************************************
* GradationMapFilter�֐�             *
* �O���f�[�V�����}�b�v�t�B���^�[     *
* ���̃��C���[�Œ��F�t�B���^�[       *
* ����                               *
* window	: �`��̈�̏��         *
* layers	: �������s�����C���[�z�� *
* num_layer	: �������s�����C���[�̐� *
* data		: �F�����E�擾�͈̓f�[�^ *
*************************************/
void GradationMapFilter(DRAW_WINDOW* window, LAYER** layers, uint16 num_layer, void* data)
{
	GRADATION_MAP *filter_data = (GRADATION_MAP*)data;
	FLOAT_T rate;
	int max_value, min_value;
	int i, j;

#ifdef _OPENMP
#pragma omp parallel
	{
#endif
	for(i=0; i<num_layer; i++)
	{
		if((filter_data->flags & GRADATION_MAP_DETECT_MAX) != 0)
		{
			int gray_value;
			uint8 alpha;
			int index;
			int k;

			max_value = 0,	min_value = 0xff;
#ifdef _OPENMP
#pragma omp for
#endif
			for(j=0; j<layers[i]->height; j++)
			{
				for(k=0; k<layers[i]->width; k++)
				{
					index = j*layers[i]->stride+k*4;
					gray_value = (layers[i]->pixels[index+0] + layers[i]->pixels[index+1]
						+ layers[i]->pixels[index+2]) / 3;
					alpha = layers[i]->pixels[index+3];

					if((filter_data->flags & GRADATION_MAP_TRANSPARANCY_AS_WHITE) != 0)
					{
						gray_value = (int)((0xff - gray_value) * (alpha * DIV_PIXEL));
					}
					else
					{
						gray_value = 0xff - gray_value;
					}

					if(max_value < gray_value)
					{
						max_value = gray_value;
					}
					if(min_value > gray_value)
					{
						min_value = gray_value;
					}
				}	// for(k=0; k<layers[i]->height; k++)
			}	// for(j=0; j<layers[i]->height; j++)
		}
		else
		{
			max_value = 0xff,	min_value = 0;
		}

		rate = (FLOAT_T)0xff / (max_value - min_value);

#ifdef _OPENMP
#pragma omp for
#endif
		for(j=0; j<layers[i]->height; j++)
		{
			int gray_value;
			uint8 alpha;
			int index;
			int k;

			for(k=0; k<layers[i]->width; k++)
			{
				index = j*layers[i]->stride+k*4;
				gray_value = 0xff - (layers[i]->pixels[index+0] + layers[i]->pixels[index+1]
						+ layers[i]->pixels[index+2]) / 3;
				alpha = layers[i]->pixels[index+3];

				gray_value = (int)(gray_value * rate);
				if(gray_value > 0xff)
				{
					gray_value = 0xff;
				}

				if((filter_data->flags & GRADATION_MAP_TRANSPARANCY_AS_WHITE) != 0)
				{
					gray_value = (int)(gray_value * (alpha * DIV_PIXEL));
					// gray_value = (int)((0xff - gray_value) * ((0xff - alpha) * DIV_PIXEL));
				}

				window->temp_layer->pixels[index+0] =
					(gray_value * filter_data->back_ground[0]
						+ (0xff - gray_value) * filter_data->fore_ground[0]) >> 8;
				window->temp_layer->pixels[index+1] =
					(gray_value * filter_data->back_ground[1]
						+ (0xff - gray_value) * filter_data->fore_ground[1]) >> 8;
				window->temp_layer->pixels[index+2] =
					(gray_value * filter_data->back_ground[2]
						+ (0xff - gray_value) * filter_data->fore_ground[2]) >> 8;
				window->temp_layer->pixels[index+3] = 0xff;
			}
		}

		if((filter_data->flags & GRADATION_MAP_MASK_WITH_UNDER) != 0
			&& layers[i]->prev != NULL)
		{
			(void)memset(window->mask_temp->pixels, 0, window->pixel_buf_size);
			cairo_set_operator(window->mask_temp->cairo_p, CAIRO_OPERATOR_OVER);
			cairo_set_source_surface(window->mask_temp->cairo_p, window->temp_layer->surface_p, 0, 0);
			cairo_mask_surface(window->mask_temp->cairo_p, layers[i]->prev->surface_p, 0, 0);
		}
		else
		{
			(void)memcpy(window->mask_temp->pixels, window->temp_layer->pixels, window->pixel_buf_size);
		}

		if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) != 0)
		{
#ifdef _OPENMP
#pragma omp for
#endif
			for(j=0; j<window->width*window->height; j++)
			{
				uint8 select_value = window->selection->pixels[j];
				window->mask_temp->pixels[j*4+0] = ((0xff-select_value)*layers[i]->pixels[j*4+0]
					+ window->mask_temp->pixels[j*4+0]*select_value) / 255;
				window->mask_temp->pixels[j*4+1] = ((0xff-select_value)*layers[i]->pixels[j*4+1]
					+ window->mask_temp->pixels[j*4+1]*select_value) / 255;
				window->mask_temp->pixels[j*4+2] = ((0xff-select_value)*layers[i]->pixels[j*4+2]
					+ window->mask_temp->pixels[j*4+2]*select_value) / 255;
				window->mask_temp->pixels[j*4+3] = ((0xff-select_value)*layers[i]->pixels[j*4+3]
					+ window->mask_temp->pixels[j*4+3]*select_value) / 255;
			}
		}

		if((filter_data->flags & GRADATION_MAP_LOCK_OPACITY) != 0)
		{
			(void)memset(window->mask->pixels, 0, window->pixel_buf_size);
			cairo_set_operator(window->mask->cairo_p, CAIRO_OPERATOR_OVER);
			cairo_set_source_surface(window->mask->cairo_p, window->mask_temp->surface_p, 0, 0);
			cairo_mask_surface(window->mask->cairo_p, layers[i]->surface_p, 0, 0);
			(void)memcpy(layers[i]->pixels, window->mask->pixels, window->pixel_buf_size);
		}
		else
		{
			(void)memcpy(layers[i]->pixels, window->mask_temp->pixels, window->pixel_buf_size);
		}
	}	// for(i=0; i<num_layer; i++)
#ifdef _OPENMP
	}
#endif

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	gtk_widget_queue_draw(window->window);
}

/*****************************************************
* ExecuteGradationMapFilter�֐�                      *
* �O���f�[�V�����}�b�v�����s                         *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
void ExecuteGradationMapFilter(APPLICATION* app)
{
	// �O���f�[�V�����̐ݒ���w�肷��_�C�A���O
	GtkWidget* dialog = gtk_dialog_new_with_buttons(
		app->labels->menu.gradation_map,
		GTK_WINDOW(app->window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL
	);
	// ��������`��̈�
	DRAW_WINDOW* window = GetActiveDrawWindow(app);
	// �������@�̐ݒ�
	GRADATION_MAP filter_data = {0};
	// �ݒ�ύX�p�E�B�W�F�b�g
	GtkWidget *control;
	// �t�B���^�[��K�p���郌�C���[�̐�
	uint16 num_layer;
	// �t�B���^�[��K�p���郌�C���[
	LAYER **layers = GetLayerChain(window, &num_layer);
	// �_�C�A���O�̌���
	gint result;
	// int�^�Ń��C���[�̐����L�����Ă���(�L���X�g�p)
	int int_num_layer = num_layer;

#if defined(USE_BGR_COLOR_SPACE) && USE_BGR_COLOR_SPACE != 0
	filter_data.fore_ground[0] = app->tool_window.color_chooser->rgb[2];
	filter_data.fore_ground[1] = app->tool_window.color_chooser->rgb[1];
	filter_data.fore_ground[2] = app->tool_window.color_chooser->rgb[0];
	filter_data.back_ground[0] = app->tool_window.color_chooser->back_rgb[2];
	filter_data.back_ground[1] = app->tool_window.color_chooser->back_rgb[1];
	filter_data.back_ground[2] = app->tool_window.color_chooser->back_rgb[0];
#else
	filter_data.fore_ground[0] = app->tool_window.color_chooser->rgb[0];
	filter_data.fore_ground[1] = app->tool_window.color_chooser->rgb[1];
	filter_data.fore_ground[2] = app->tool_window.color_chooser->rgb[2];
	filter_data.back_ground[0] = app->tool_window.color_chooser->back_rgb[0];
	filter_data.back_ground[1] = app->tool_window.color_chooser->back_rgb[1];
	filter_data.back_ground[2] = app->tool_window.color_chooser->back_rgb[2];
#endif

	// �_�C�A���O�ɃE�B�W�F�b�g������
		// �O���[�X�P�[���̍ł��������������o���ă}�b�s���O
	control = gtk_check_button_new_with_label(app->labels->menu.detect_max);
	CheckButtonSetFlagsCallBack(control, &filter_data.flags, GRADATION_MAP_DETECT_MAX);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		control, FALSE, FALSE, 0);
	// ���������𔒐F�Ƃ݂Ȃ�
	control = gtk_check_button_new_with_label(app->labels->menu.tranparancy_as_white);
	CheckButtonSetFlagsCallBack(control, &filter_data.flags, GRADATION_MAP_TRANSPARANCY_AS_WHITE);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		control, FALSE, FALSE, 0);
	// ���̃��C���[�Ń}�X�L���O
	control = gtk_check_button_new_with_label(app->labels->layer_window.mask_with_under);
	CheckButtonSetFlagsCallBack(control, &filter_data.flags, GRADATION_MAP_MASK_WITH_UNDER);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		control, FALSE, FALSE, 0);
	// �s�����ی�
	control = gtk_check_button_new_with_label(app->labels->layer_window.lock_opacity);
	CheckButtonSetFlagsCallBack(control, &filter_data.flags, GRADATION_MAP_LOCK_OPACITY);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		control, FALSE, FALSE, 0);

	// �_�C�A���O��\�����Ď��s
	gtk_widget_show_all(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
	result = gtk_dialog_run(GTK_DIALOG(dialog));

	if(result == GTK_RESPONSE_ACCEPT)
	{	// O.K.�{�^���������ꂽ
			// ��ɗ����f�[�^���c��
		AddFilterHistory(app->labels->menu.gradation_map, &filter_data, sizeof(filter_data),
			FILTER_FUNC_GRADATION_MAP, layers, num_layer, window);

		// �O���f�[�V�����}�b�v���s
		GradationMapFilter(window, layers, num_layer, &filter_data);
	}

	MEM_FREE_FUNC(layers);

	// �_�C�A���O������
	gtk_widget_destroy(dialog);
}

/**********************************************
* AddVectorLinePath�֐�                       *
* �x�N�g������Cairo�̃p�X�ɂ���             *
* ����                                        *
* cairo_p	: �p�X��ǉ�����Cairo�R���e�L�X�g *
* line		: �ǉ�����x�N�g�����            *
* reverse	: �����𔽓]���Ēǉ����邩�ۂ�    *
**********************************************/
void AddVectorLinePath(
	cairo_t* cairo_p,
	VECTOR_LINE* line,
	gboolean reverse
)
{
	int i;

	if(reverse == FALSE)
	{
		if(line->num_points == 2 || line->vector_type == VECTOR_LINE_STRAIGHT)
		{
			for(i=0; i<line->num_points; i++)
			{
				cairo_line_to(cairo_p, line->points[i].x, line->points[i].y);
			}
		}
		else if(line->vector_type == VECTOR_LINE_BEZIER_OPEN)
		{
			BEZIER_POINT calc[4], inter[2];
			int j;

			cairo_line_to(cairo_p, line->points[0].x, line->points[0].y);

			for(j=0; j<3; j++)
			{
				calc[j].x = line->points[j].x;
				calc[j].y = line->points[j].y;
			}
			MakeBezier3EdgeControlPoint(calc, inter);
			cairo_curve_to(cairo_p, line->points[0].x, line->points[0].y,
				inter[0].x, inter[0].y, line->points[1].x, line->points[1].y
			);

			for(i=0; i<line->num_points-3; i++)
			{
				for(j=0; j<4; j++)
				{
					calc[j].x = line->points[i+j].x;
					calc[j].y = line->points[i+j].y;
				}
				MakeBezier3ControlPoints(calc, inter);
				cairo_curve_to(cairo_p, inter[0].x, inter[0].y,
					inter[1].x, inter[1].y, line->points[i+2].x, line->points[i+2].y
				);
			}

			for(j=0; j<3; j++)
			{
				calc[j].x = line->points[i+j].x;
				calc[j].y = line->points[i+j].y;
			}
			MakeBezier3EdgeControlPoint(calc, inter);
			cairo_curve_to(cairo_p, inter[1].x, inter[1].y,
				line->points[i+2].x, line->points[i+2].y, line->points[i+2].x, line->points[i+2].y);
		}
		else if(line->vector_type == VECTOR_LINE_STRAIGHT_CLOSE)
		{
			for(i=0; i<line->num_points; i++)
			{
				cairo_line_to(cairo_p, line->points[i].x, line->points[i].y);
			}
			cairo_line_to(cairo_p, line->points[0].x, line->points[0].y);
		}
		else if(line->vector_type == VECTOR_LINE_BEZIER_CLOSE)
		{
			BEZIER_POINT calc[4], inter[2];
			int j;

			cairo_line_to(cairo_p, line->points[0].x, line->points[0].y);

			calc[0].x = line->points[line->num_points-1].x;
			calc[0].y = line->points[line->num_points-1].y;

			for(j=0; j<3; j++)
			{
				calc[j+1].x = line->points[j].x;
				calc[j+1].y = line->points[j].y;
			}
			MakeBezier3ControlPoints(calc, inter);
			cairo_curve_to(cairo_p, inter[0].x, inter[0].y, inter[1].x, inter[1].y,
				line->points[1].x, line->points[1].y
			);

			for(i=0; i<line->num_points-3; i++)
			{
				for(j=0; j<4; j++)
				{
					calc[j].x = line->points[i+j].x,	calc[j].y = line->points[i+j].y;
				}
				MakeBezier3ControlPoints(calc, inter);
				cairo_curve_to(cairo_p, inter[0].x, inter[0].y, inter[1].x, inter[1].y,
					line->points[i+2].x, line->points[i+2].y
				);
			}

			for(j=0; j<3; j++)
			{
				calc[j].x = line->points[i+j].x,	calc[j].y = line->points[i+j].y;
			}
			calc[3].x = line->points[i+1].x,	calc[3].y = line->points[i+1].y;
			MakeBezier3ControlPoints(calc, inter);
			cairo_curve_to(cairo_p, inter[0].x, inter[0].y, inter[1].x, inter[1].y,
				line->points[i+2].x, line->points[i+2].y
			);

			calc[0].x = line->points[i+1].x, calc[0].y = line->points[i+1].y;
			calc[1].x = line->points[i+2].x, calc[1].y = line->points[i+2].y;
			calc[2].x = line->points[0].x, calc[2].y = line->points[0].y;
			calc[3].x = line->points[1].x, calc[3].y = line->points[1].y;
			MakeBezier3ControlPoints(calc, inter);
			cairo_curve_to(cairo_p, inter[0].x, inter[0].y, inter[1].x, inter[1].y,
				line->points[0].x, line->points[0].y
			);
		}
	}
	else
	{
		if(line->num_points == 2 || line->vector_type == VECTOR_LINE_STRAIGHT)
		{
			for(i=0; i<line->num_points; i++)
			{
				cairo_line_to(cairo_p,
					line->points[line->num_points-i-1].x, line->points[line->num_points-i-1].y);
			}
		}
		else if(line->vector_type == VECTOR_LINE_BEZIER_OPEN)
		{
			BEZIER_POINT calc[4], inter[2];
			int j;

			cairo_line_to(cairo_p,
				line->points[line->num_points-1].x, line->points[line->num_points-1].y);

			for(j=0; j<3; j++)
			{
				calc[j].x = line->points[line->num_points-j-1].x;
				calc[j].y = line->points[line->num_points-j-1].y;
			}
			MakeBezier3EdgeControlPoint(calc, inter);
			cairo_curve_to(cairo_p, line->points[line->num_points-1].x, line->points[line->num_points-1].y,
				inter[0].x, inter[0].y, line->points[line->num_points-2].x, line->points[line->num_points-2].y
			);

			for(i=0; i<line->num_points-3; i++)
			{
				for(j=0; j<4; j++)
				{
					calc[j].x = line->points[line->num_points-i-j-1].x;
					calc[j].y = line->points[line->num_points-i-j-1].y;
				}
				MakeBezier3ControlPoints(calc, inter);
				cairo_curve_to(cairo_p, inter[0].x, inter[0].y,
					inter[1].x, inter[1].y, line->points[line->num_points-i-3].x, line->points[line->num_points-i-3].y
				);
			}

			for(j=0; j<3; j++)
			{
				calc[j].x = line->points[2-j].x;
				calc[j].y = line->points[2-j].y;
			}
			MakeBezier3EdgeControlPoint(calc, inter);
			cairo_curve_to(cairo_p, inter[1].x, inter[1].y,
				line->points[0].x, line->points[0].y, line->points[0].x, line->points[0].y);
		}
		else if(line->vector_type == VECTOR_LINE_STRAIGHT_CLOSE)
		{
			for(i=0; i<line->num_points; i++)
			{
				cairo_line_to(cairo_p, line->points[i].x, line->points[i].y);
			}
			cairo_line_to(cairo_p, line->points[0].x, line->points[0].y);
		}
		else if(line->vector_type == VECTOR_LINE_BEZIER_CLOSE)
		{
			BEZIER_POINT calc[4], inter[2];
			int j;

			cairo_line_to(cairo_p, line->points[0].x, line->points[0].y);

			calc[0].x = line->points[line->num_points-1].x;
			calc[0].y = line->points[line->num_points-1].y;

			for(j=0; j<3; j++)
			{
				calc[j+1].x = line->points[j].x;
				calc[j+1].y = line->points[j].y;
			}
			MakeBezier3ControlPoints(calc, inter);
			cairo_curve_to(cairo_p, inter[0].x, inter[0].y, inter[1].x, inter[1].y,
				line->points[1].x, line->points[1].y
			);

			for(i=0; i<line->num_points-3; i++)
			{
				for(j=0; j<4; j++)
				{
					calc[j].x = line->points[i+j].x,	calc[j].y = line->points[i+j].y;
				}
				MakeBezier3ControlPoints(calc, inter);
				cairo_curve_to(cairo_p, inter[0].x, inter[0].y, inter[1].x, inter[1].y,
					line->points[i+2].x, line->points[i+2].y
				);
			}

			for(j=0; j<3; j++)
			{
				calc[j].x = line->points[i+j].x,	calc[j].y = line->points[i+j].y;
			}
			calc[3].x = line->points[i+1].x,	calc[3].y = line->points[i+1].y;
			MakeBezier3ControlPoints(calc, inter);
			cairo_curve_to(cairo_p, inter[0].x, inter[0].y, inter[1].x, inter[1].y,
				line->points[i+2].x, line->points[i+2].y
			);

			calc[0].x = line->points[i+1].x, calc[0].y = line->points[i+1].y;
			calc[1].x = line->points[i+2].x, calc[1].y = line->points[i+2].y;
			calc[2].x = line->points[0].x, calc[2].y = line->points[0].y;
			calc[3].x = line->points[1].x, calc[3].y = line->points[1].y;
			MakeBezier3ControlPoints(calc, inter);
			cairo_curve_to(cairo_p, inter[0].x, inter[0].y, inter[1].x, inter[1].y,
				line->points[0].x, line->points[0].y
			);
		}
	}
}

typedef enum _eFILL_WITH_VECTOR_LINE_MODE
{
	FILL_WITH_VECTOR_LINE_MODE_WINDING,
	FILL_WITH_VECTOR_LINE_MODE_ODD
} eFILL_WITH_VECTOR_LINE_MODE;

typedef struct _FILL_WITH_VECTOR_LINE
{
	eFILL_WITH_VECTOR_LINE_MODE mode;
	uint8 color[4];
} FILL_WITH_VECTOR_LINE;

/*************************************
* FillWithVectorLineFilter�֐�       *
* �x�N�g�����C���[�𗘗p���ēh��ׂ� *
* ���̃��C���[�Œ��F�t�B���^�[       *
* ����                               *
* window	: �`��̈�̏��         *
* layers	: �������s�����C���[�z�� *
* num_layer	: �������s�����C���[�̐� *
* data		: �F�����E�擾�͈̓f�[�^ *
*************************************/
void FillWithVectorLineFilter(DRAW_WINDOW* window, LAYER** layers, uint16 num_layer, void* data)
{
	FILL_WITH_VECTOR_LINE *fill_data = (FILL_WITH_VECTOR_LINE*)data;
	VECTOR_LINE *line;
	VECTOR_LINE *neighbour;
	VECTOR_POINT target;
	FLOAT_T distance;
	FLOAT_T min_distance;
	FLOAT_T dx, dy;
	gboolean reverse;
	int end_flag;
	int num_lines = 0;
	int i;

	for(i=0; i<num_layer-1; i++)
	{
		line = layers[i]->layer_data.vector_layer_p->base->next;
		if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) == 0)
		{
			while(line != NULL)
			{
				line->flags |= VECTOR_LINE_MARKED;
				line = line->next;
				num_lines++;
			}
		}
		else
		{
			while(line != NULL)
			{
				if(line->layer != NULL)
				{
					if(window->selection_area.max_x >= line->layer->x
						&& window->selection_area.max_y >= line->layer->y
						&& window->selection_area.min_x <= line->layer->x + line->layer->width
						&& window->selection_area.min_y <= line->layer->y + line->layer->height
					)
					{
						if(IsVectorLineInSelectionArea(window,
							window->selection->pixels, line) != 0)
						{
							line->flags |= VECTOR_LINE_MARKED;
							num_lines++;
						}
					}
				}

				line = line->next;
			}
		}
	}

	if(num_lines < 0)
	{
		for(i=0; i<num_layer; i++)
		{
			line = layers[i]->layer_data.vector_layer_p->base->next;

			while(line != NULL)
			{
				line->flags &= ~(VECTOR_LINE_MARKED);
				line = line->next;
			}
		}
	}

	cairo_save(layers[0]->prev->cairo_p);
	cairo_new_path(layers[0]->prev->cairo_p);

	end_flag = 0;
	for(i=0; i<num_layer-1 && end_flag == 0; i++)
	{
		line = layers[i]->layer_data.vector_layer_p->base->next;
		while(line != NULL &&(line->flags & VECTOR_LINE_MARKED) == 0)
		{
			line = line->next;
		}
		if(line != NULL)
		{
			end_flag++;
			line->flags &= ~(VECTOR_LINE_MARKED);
			AddVectorLinePath(layers[0]->prev->cairo_p, line, FALSE);
			target = line->points[line->num_points-1];
		}
	}
	num_lines--;

	while(num_lines > 0)
	{
		reverse = FALSE;
		end_flag = 0;
		for(i=0; i<num_layer-1 && end_flag == 0; i++)
		{
			neighbour = layers[i]->layer_data.vector_layer_p->base->next;
			while(neighbour != NULL && (neighbour->flags & VECTOR_LINE_MARKED) == 0)
			{
				neighbour = neighbour->next;
			}
			if(neighbour != NULL)
			{
				end_flag++;
			}
			else
			{
				if(layers[i+1]->layer_type == TYPE_VECTOR_LAYER)
				{
					neighbour = layers[i+1]->layer_data.vector_layer_p->base->next;
				}
			}
		}

		dx = neighbour->points[0].x - target.x;
		dy = neighbour->points[0].y - target.y;
		min_distance = dx*dx + dy*dy;

		dx = neighbour->points[neighbour->num_points-1].x - target.x;
		dy = neighbour->points[neighbour->num_points-1].y - target.y;
		distance = dx*dx + dy*dy;

		if(min_distance > distance)
		{
			min_distance = distance;
			reverse = TRUE;
		}

		for(i=0; i<num_layer-1; i++)
		{
			line = layers[i]->layer_data.vector_layer_p->base->next;
			while(line != NULL)
			{
				if(line != neighbour && (line->flags & VECTOR_LINE_MARKED) != 0)
				{
					dx = line->points[0].x - target.x;
					dy = line->points[0].y - target.y;
					distance = dx*dx + dy*dy;
					if(min_distance > distance)
					{
						min_distance = distance;
						reverse = FALSE;
						neighbour = line;
					}

					dx = line->points[line->num_points-1].x - target.x;
					dy = line->points[line->num_points-1].y - target.y;
					distance = dx*dx + dy*dy;
					if(min_distance > distance)
					{
						min_distance = distance;
						reverse = TRUE;
						neighbour = line;
					}
				}

				line = line->next;
			}
		}

		AddVectorLinePath(layers[0]->prev->cairo_p, neighbour, reverse);
		neighbour->flags &= ~(VECTOR_LINE_MARKED);

		if(reverse == FALSE)
		{
			target = neighbour->points[neighbour->num_points-1];
		}
		else
		{
			target = neighbour->points[0];
		}

		num_lines--;
	}

	cairo_close_path(layers[0]->prev->cairo_p);
	cairo_set_source_rgb(layers[0]->prev->cairo_p,
		fill_data->color[0]*DIV_PIXEL, fill_data->color[1]*DIV_PIXEL, fill_data->color[2]*DIV_PIXEL
	);
	cairo_fill(layers[0]->prev->cairo_p);

	cairo_restore(layers[0]->prev->cairo_p);

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	gtk_widget_queue_draw(window->window);
}

/*****************************************************
* ExecuteFillWithVectorLineFilter�֐�                *
* �x�N�g�����C���[�𗘗p���ĉ��̃��C���[��h��ׂ�   *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
void ExecuteFillWithVectorLineFilter(APPLICATION* app)
{
	FILL_WITH_VECTOR_LINE fill_data = {0};
	DRAW_WINDOW *window = app->draw_window[app->active_window];
	LAYER *layers[256];
	LAYER *layer = window->layer;
	uint16 num_layer = 1;

	if(window->active_layer->layer_type != TYPE_VECTOR_LAYER
		|| window->active_layer->prev == NULL)
	{
		return;
	}

	while(layer != NULL)
	{
		if(layer != window->active_layer && layer->layer_type == TYPE_VECTOR_LAYER
			&& (layer->flags & LAYER_CHAINED) != 0)
		{
			layers[num_layer] = layer;
			num_layer++;
		}
		layer = layer->next;
	}

	if(window->active_layer->prev->layer_type != TYPE_NORMAL_LAYER)
	{
		return;
	}

	fill_data.color[0] = app->tool_window.color_chooser->rgb[0];
	fill_data.color[1] = app->tool_window.color_chooser->rgb[1];
	fill_data.color[2] = app->tool_window.color_chooser->rgb[2];

	layers[0] = window->active_layer;
	layers[num_layer] = window->active_layer->prev;
	num_layer++;

	AddFilterHistory(app->labels->menu.fill_with_vector, (void*)&fill_data,
		sizeof(fill_data), FILTER_FUNC_FILL_WITH_VECTOR, layers, num_layer, window);
	FillWithVectorLineFilter(window, layers, num_layer, (void*)&fill_data);
}

typedef enum _ePERLIN_NOISE_INTERPOLATION_TYPE
{
	PERLIN_NOISE_LINEAR_INTERPOLATION,
	PERLIN_NOISE_COSINE_INTERPOLATION,
	PERLIN_NOISE_CUBIC_INTERPOLATION,
	NUM_PERLIN_NOISE_INTERPOLATION_TYPE
} ePERLIN_NOISE_INTERPOLATION_TYPE;

typedef struct _PERLIN_NOISE_DATA
{
	uint16 num_octaves;
	uint16 frequency;
	uint16 interpolation_type;
	uint8 color[3];
	uint8 multi_color;
	uint8 opacity;
	FLOAT_T persistence;
	int width, height;
	int seed;
} PERLIN_NOISE_DATA;

typedef struct _PERLIN_NOISE_WIDGET
{
	GtkWidget *canvas;
	PERLIN_NOISE_DATA *filter_data;
	GtkWidget *use_random;
	GtkWidget *immediate_update;
	uint8 *pixels;
	uint8 back_ground[3];
	cairo_surface_t *surface_p;
	int canvas_width, canvas_height;
} PERLIN_NOISE_WIDGET;

typedef struct _PERLIN_NOISE_RANDOM
{
	int value1, value2, value3;
} PERLIN_NOISE_RANDOM;

static void MakePerlinNoiseRandom(PERLIN_NOISE_RANDOM* random)
{
	do
	{
		random->value1 = rand() & 0xFFFF;
	} while(random->value1 < 8000);

	random->value2 = 650000 + rand() % 450000;

	random->value3 = 1000000000 + rand() % 600000000;
}

static FLOAT_T LinearInterpolation(FLOAT_T a, FLOAT_T b, FLOAT_T x)
{
	return a * (1 - x) + b * x;
}

static FLOAT_T CosineInterpolation(FLOAT_T a, FLOAT_T b, FLOAT_T x)
{
	FLOAT_T ft = x * 3.1415927;
	FLOAT_T f = (1 - cos(ft)) * 0.5;

	return a * (1 - f) + b * f;
}

static FLOAT_T CubicInterpolation(
	FLOAT_T v0,
	FLOAT_T v1,
	FLOAT_T v2,
	FLOAT_T v3,
	FLOAT_T x
)
{
	FLOAT_T p = (v3 - v2) - (v0 - v1);
	FLOAT_T q = (v0 - v1) - p;
	FLOAT_T r = v2 - v0;
	FLOAT_T s = v1;

	return p * (x * x * x) + q * x * x + r * x + s;
}

static FLOAT_T Noise2(int x, int y, PERLIN_NOISE_RANDOM* random)
{
	int n = x + y * 57;
	n = (n << 13) ^ n;
	return (1.0 - ((n * (n * n * random->value1 + random->value2) + random->value3) & 0x7FFFFFFF) / 1073741824.0);
}

static FLOAT_T SmoothNoise2(FLOAT_T x, FLOAT_T y, PERLIN_NOISE_RANDOM* random)
{
	FLOAT_T corners, sides, center;
	int int_x = (int)x,	int_y = (int)y;
	corners = (Noise2(int_x-1, int_y-1, random) + Noise2(int_x+1, int_y-1, random)
		+ Noise2(int_x-1, int_y+1, random), Noise2(int_x+1, int_y+1, random)) / 16;
	sides = (Noise2(int_x-1, int_y, random) + Noise2(int_x+1, int_y, random) + Noise2(int_x, int_y-1, random)
		+ Noise2(int_x, int_y+1, random)) / 8;
	center = Noise2(int_x, int_y, random) / 4;
	return corners + sides + center;
}

static int PerlinNoisePoint(FLOAT_T data, int points)
{
	int floor_value = (int)data;
	if(floor_value > data)
	{
		floor_value--;
	}

	return ((floor_value % points) + points) % points;
}

static FLOAT_T InterpolatedNoise2(
	PERLIN_NOISE_DATA* data,
	FLOAT_T x,
	FLOAT_T y,
	int points,
	PERLIN_NOISE_RANDOM* random
)
{
	int nx0 = PerlinNoisePoint(x, points);
	int nx1 = (nx0 + 1) % points;
	FLOAT_T fractional_x = x - (int)x;
	int ny0 = PerlinNoisePoint(y, points);
	int ny1 = (ny0 + 1) % points;
	FLOAT_T fractional_y = y - (int)y;
	FLOAT_T v1, v2, v3, v4;
	FLOAT_T i1, i2;
	FLOAT_T ret;

	v1 = SmoothNoise2(nx0, ny0, random);
	v2 = SmoothNoise2(nx1, ny0, random);
	v3 = SmoothNoise2(nx0, ny1, random);
	v4 = SmoothNoise2(nx1, ny1, random);

	switch(data->interpolation_type)
	{
	case PERLIN_NOISE_LINEAR_INTERPOLATION:
		i1 = LinearInterpolation(v1, v2, fractional_x);
		i2 = LinearInterpolation(v3, v4, fractional_x);
		ret = LinearInterpolation(i1, i2, fractional_y);
		break;
	case PERLIN_NOISE_COSINE_INTERPOLATION:
		i1 = CosineInterpolation(v1, v2, fractional_x);
		i2 = CosineInterpolation(v3, v4, fractional_x);
		ret = CosineInterpolation(i1, i2, fractional_y);
		break;
	case PERLIN_NOISE_CUBIC_INTERPOLATION:
		ret = CubicInterpolation(v1, v2, v3, v4, fractional_x);
		break;
	default:
		ret = 0;
	}

	return ret;
}

static FLOAT_T PerlinNoise2D(
	PERLIN_NOISE_DATA* data,
	FLOAT_T x,
	FLOAT_T y,
	PERLIN_NOISE_RANDOM* random
)
{
	FLOAT_T total = 0;
	FLOAT_T persistence = data->persistence;
	FLOAT_T amplitude = persistence;
	FLOAT_T maximum = 0;
	int frequency;
	const int n = data->num_octaves;
	int i;

	for(i=0; i<n; i++)
	{
		frequency = data->frequency * (1 << i) + 1;
		total += InterpolatedNoise2(data, (x * frequency) / data->width,
			(y * frequency) / data->height, frequency, random) * amplitude;
		maximum += amplitude;
		amplitude *= persistence;
	}

	return total / maximum;
}

static void FillMonoColorPelinNoise2D(
	PERLIN_NOISE_DATA* data,
	uint8* pixels
)
{
	PERLIN_NOISE_RANDOM random;
	const int width = data->width;
	const int height = data->height;
	const int stride = data->width * 4;
	int y;
	FLOAT_T boost = (1 - data->persistence) * 2.5;
	if(boost < - 0.5)
	{
		boost = 256;
	}
	else
	{
		boost = 256 + boost * 256;
	}

	srand(data->seed);
	MakePerlinNoiseRandom(&random);

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for(y=0; y<height; y++)
	{
		int pixel_value;
		int x;

		for(x=0; x<width; x++)
		{
			pixel_value =
				(int)(PerlinNoise2D(data, x, y, &random) * boost);
			if(pixel_value > 255)
			{
				pixel_value = 255;
			}
			else if(pixel_value < 0)
			{
				pixel_value = 0;
			}
			if(data->color[0] > pixel_value)
			{
				pixels[y*stride+x*4+0] = (uint8)pixel_value;
			}
			else
			{
				pixels[y*stride+x*4+0] = data->color[0];
			}
			if(data->color[1] > pixel_value)
			{
				pixels[y*stride+x*4+1] = (uint8)pixel_value;
			}
			else
			{
				pixels[y*stride+x*4+1] = data->color[1];
			}
			if(data->color[2] > pixel_value)
			{
				pixels[y*stride+x*4+2] = (uint8)pixel_value;
			}
			else
			{
				pixels[y*stride+x*4+2] = data->color[2];
			}
			pixels[y*stride+x*4+3] = (uint8)pixel_value;
		}
	}
}

static void FillMultiColorPelinNoise2D(
	PERLIN_NOISE_DATA* data,
	uint8* pixels
)
{
	PERLIN_NOISE_RANDOM random1, random2, random3, random4;
	const int width = data->width;
	const int height = data->height;
	const int stride = data->width * 4;
	int y;
	FLOAT_T boost = (1 - data->persistence) * 2.5;
	if(boost < - 0.5)
	{
		boost = 256;
	}
	else
	{
		boost = 256 + boost * 256;
	}

	srand(data->seed);
	MakePerlinNoiseRandom(&random1);
	MakePerlinNoiseRandom(&random2);
	MakePerlinNoiseRandom(&random3);
	MakePerlinNoiseRandom(&random4);

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for(y=0; y<height; y++)
	{
		int pixel_value;
		int x;

		for(x=0; x<width; x++)
		{
			pixel_value = (int)(PerlinNoise2D(data, x, y, &random1) * boost);
			if(pixel_value > 255)
			{
				pixel_value = 255;
			}
			else if(pixel_value < 0)
			{
				pixel_value = 0;
			}
			pixels[y*stride+x*4+0] = (uint8)pixel_value;

			pixel_value = (int)(PerlinNoise2D(data, width + x, height + y, &random2) * boost);
			if(pixel_value > 255)
			{
				pixel_value = 255;
			}
			else if(pixel_value < 0)
			{
				pixel_value = 0;
			}
			pixels[y*stride+x*4+1] = (uint8)pixel_value;

			pixel_value = (int)(PerlinNoise2D(data, width * 2 + x, height * 2 + y, &random3) * boost);
			if(pixel_value > 255)
			{
				pixel_value = 255;
			}
			else if(pixel_value < 0)
			{
				pixel_value = 0;
			}
			pixels[y*stride+x*4+2] = (uint8)pixel_value;

			pixel_value = (int)(PerlinNoise2D(data, width * 3 + x, height * 3 + y, &random4) * boost)
				+ data->opacity;
			if(pixel_value > 255)
			{
				pixel_value = 255;
			}
			else if(pixel_value < 0)
			{
				pixel_value = 0;
			}
			pixels[y*stride+x*4+3] = pixel_value;

			if(pixels[y*stride+x*4+0] > pixel_value)
			{
				pixels[y*stride+x*4+0] = pixel_value;
			}
			if(pixels[y*stride+x*4+1] > pixel_value)
			{
				pixels[y*stride+x*4+1] = pixel_value;
			}
			if(pixels[y*stride+x*4+2] > pixel_value)
			{
				pixels[y*stride+x*4+2] = pixel_value;
			}
		}
	}
}

/*************************************
* PerlinNoiseFilter�֐�              *
* �p�[�����m�C�Y�ŉ��h��             *
* ����                               *
* window	: �`��̈�̏��         *
* layers	: �������s�����C���[�z�� *
* num_layer	: �������s�����C���[�̐� *
* data		: �F�����E�擾�͈̓f�[�^ *
*************************************/
void PerlinNoiseFilter(DRAW_WINDOW* window, LAYER** layers, uint16 num_layer, void* data)
{
	PERLIN_NOISE_DATA *filter_data = (PERLIN_NOISE_DATA*)data;
	cairo_surface_t *surface_p;
	uint8 *pixels;
	int i;

	pixels = (uint8*)MEM_ALLOC_FUNC(filter_data->width * filter_data->height * 4);
	surface_p = cairo_image_surface_create_for_data(pixels, CAIRO_FORMAT_ARGB32,
		filter_data->width, filter_data->height, filter_data->width * 4);

	if(filter_data->multi_color == FALSE)
	{
		FillMonoColorPelinNoise2D(filter_data, pixels);
	}
	else
	{
		FillMultiColorPelinNoise2D(filter_data, pixels);
	}

	for(i=0; i<num_layer; i++)
	{
		if(layers[i]->layer_type == TYPE_NORMAL_LAYER)
		{
			if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) == 0)
			{
				cairo_set_source_surface(layers[i]->cairo_p, surface_p, 0, 0);
				cairo_paint(layers[i]->cairo_p);
			}
			else
			{
				cairo_set_source_surface(layers[i]->cairo_p, surface_p, 0, 0);
				cairo_mask_surface(layers[i]->cairo_p, window->selection->surface_p, 0, 0);
			}
		}
	}

	cairo_surface_destroy(surface_p);
	MEM_FREE_FUNC(pixels);
}

static gboolean DisplayPerlinNoisePreview(
	GtkWidget* widget,
#if GTK_MAJOR_VERSION <= 2
	GdkEventExpose* event_info,
#else
	cairo_t* cairo_p,
#endif
	PERLIN_NOISE_WIDGET* widgets
)
{
#if GTK_MAJOR_VERSION <= 2
	cairo_t *cairo_p = gdk_cairo_create(widget->window);
#endif

	if(widgets->filter_data->multi_color == FALSE)
	{
		FillMonoColorPelinNoise2D(widgets->filter_data, widgets->pixels);
	}
	else
	{
		FillMultiColorPelinNoise2D(widgets->filter_data, widgets->pixels);
	}

#if defined(USE_BGR_COLOR_SPACE) && USE_BGR_COLOR_SPACE != 0
	cairo_set_source_rgb(cairo_p, widgets->back_ground[2] * DIV_PIXEL,
		widgets->back_ground[1] * DIV_PIXEL, widgets->back_ground[0] * DIV_PIXEL);
#else
	cairo_set_source_rgb(cairo_p, widgets->back_ground[0] * DIV_PIXEL,
		widgets->back_ground[1] * DIV_PIXEL, widgets->back_ground[2] * DIV_PIXEL);
#endif
	cairo_rectangle(cairo_p, 0, 0, widgets->canvas_width, widgets->canvas_height);
	cairo_paint(cairo_p);
	cairo_set_source_surface(cairo_p, widgets->surface_p, 0, 0);
	cairo_rectangle(cairo_p, 0, 0, widgets->canvas_width, widgets->canvas_height);
	cairo_paint(cairo_p);

#if GTK_MAJOR_VERSION <= 2
	cairo_destroy(cairo_p);
#endif

	return TRUE;
}

static void OnChangePerlinNoiseSetting(
	GtkWidget* widget,
	PERLIN_NOISE_DATA* filter_data
)
{
	PERLIN_NOISE_WIDGET *widgets = (PERLIN_NOISE_WIDGET*)g_object_get_data(
		G_OBJECT(widget), "setting-widgets");

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets->immediate_update)) == FALSE)
	{
		return;
	}

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets->use_random)) != FALSE)
	{
		filter_data->seed = (int)time(NULL);
	}

	gtk_widget_queue_draw(widgets->canvas);
}

static void OnChangePerlinNoiseSeed(GtkWidget* spin, PERLIN_NOISE_DATA* filter_data)
{
	filter_data->seed = gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(spin));
	OnChangePerlinNoiseSetting(spin, filter_data);
}

static void OnChangePerlinNoiseUseRandom(GtkWidget* button, GtkWidget* control)
{
	PERLIN_NOISE_DATA *filter_data = (PERLIN_NOISE_DATA*)g_object_get_data(
		G_OBJECT(button), "filter_data");
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)) == FALSE)
	{
		filter_data->seed = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON(control));
	}
	else
	{
		filter_data->seed = (int)time(NULL);
	}

	gtk_widget_set_sensitive(control, !gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(button)));
	OnChangePerlinNoiseSetting(button, filter_data);
}

static void OnChangePerlinNoiseOctaves(GtkWidget* spin, PERLIN_NOISE_DATA* filter_data)
{
	filter_data->num_octaves = gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(spin));
	OnChangePerlinNoiseSetting(spin, filter_data);
}

static void OnChangePerlinNoiseFrequency(GtkWidget* spin, PERLIN_NOISE_DATA* filter_data)
{
	filter_data->frequency = (uint16)gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON(spin));
	OnChangePerlinNoiseSetting(spin, filter_data);
}

static void OnChangePerlinNoisePersistence(GtkWidget* spin, PERLIN_NOISE_DATA* filter_data)
{
	filter_data->persistence = gtk_spin_button_get_value(
		GTK_SPIN_BUTTON(spin));
	OnChangePerlinNoiseSetting(spin, filter_data);
}

static void OnChangePerlinNoiseInterpolation(GtkWidget* button, PERLIN_NOISE_DATA* filter_data)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)) != FALSE)
	{
		filter_data->interpolation_type =
			(uint16)GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "interpolation"));
		OnChangePerlinNoiseSetting(button, filter_data);
	}
}

static void OnChangePerlinNoiseCloudColor(GtkWidget* button, PERLIN_NOISE_DATA* filter_data)
{
	GdkColor color;
	gtk_color_button_get_color(GTK_COLOR_BUTTON(button), &color);

#if defined(USE_BGR_COLOR_SPACE) && USE_BGR_COLOR_SPACE != 0
	filter_data->color[0] = color.blue / 256;
	filter_data->color[1] = color.green / 256;
	filter_data->color[2] = color.red / 256;
#else
	filter_data->color[0] = color.red / 256;
	filter_data->color[1] = color.green / 256;
	filter_data->color[2] = color.blue / 256;
#endif
	OnChangePerlinNoiseSetting(button, filter_data);
}

static void OnChangePerlinNoiseColorMode(GtkWidget* button, PERLIN_NOISE_DATA* filter_data)
{
	filter_data->multi_color = (uint8)gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(button));
	OnChangePerlinNoiseSetting(button, filter_data);
}

static void OnChangePerlinNoiseOpacity(GtkAdjustment* slider, PERLIN_NOISE_DATA* filter_data)
{
	filter_data->opacity = (uint8)(gtk_adjustment_get_value(slider) * 2.555);
	OnChangePerlinNoiseSetting(GTK_WIDGET(slider), filter_data);
}

static void OnClickedPerlinNoiseUpdateButton(GtkWidget* button, PERLIN_NOISE_DATA* filter_data)
{
	PERLIN_NOISE_WIDGET *widgets = (PERLIN_NOISE_WIDGET*)g_object_get_data(
		G_OBJECT(button), "setting-widgets");

	if(widgets->use_random != NULL)
	{
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets->use_random)) != FALSE)
		{
			filter_data->seed = (int)time(NULL);
		}
	}

	gtk_widget_queue_draw(widgets->canvas);
}

static int CheckPerlinoiseReverseColor(DRAW_WINDOW* window, uint8* color)
{
	FLOAT_T float_color[4] = {0};
	FLOAT_T alpha;
	int i;

	for(i=0; i<window->width*window->height; i++)
	{
		alpha = window->mixed_layer->pixels[i*4+3] * DIV_PIXEL;
		float_color[0] += window->mixed_layer->pixels[i*4+0] * DIV_PIXEL
			* alpha;
		float_color[1] += window->mixed_layer->pixels[i*4+1] * DIV_PIXEL
			* alpha;
		float_color[2] += window->mixed_layer->pixels[i*4+2] * DIV_PIXEL
			* alpha;
		float_color[3] += alpha;
	}
	alpha = 1 / (alpha * window->width * window->height);
	float_color[0] *= alpha,	float_color[1] *= alpha,	float_color[2] *= alpha;
	color[0] = (uint8)(255 * float_color[0]);
	color[1] = (uint8)(255 * float_color[1]);
	color[2] = (uint8)(255 * float_color[2]);
	return float_color[0] + float_color[1] + float_color[2] > 0.5 * 3;
}

/*****************************************************
* ExecutePerlinNoiseFilter�֐�                       *
* �p�[�����m�C�Y�ŉ��h������s                       *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
void ExecutePerlinNoiseFilter(APPLICATION* app)
{
#define PERLIN_NOISE_DEFAULT_OCTAVES 6
#define PERLIN_NOISE_DEFAULT_PERSISTENCE 0.6
#define PERLIN_NOISE_DEFAULT_FREQUENCY 10
#define PERLIN_NOISE_PREVIEW_SIZE 500
	// �p�[�����m�C�Y���w�肷��_�C�A���O
	GtkWidget* dialog = gtk_dialog_new_with_buttons(
		app->labels->menu.cloud,
		GTK_WINDOW(app->window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL
	);
	// ��������`��̈�
	DRAW_WINDOW* window = GetActiveDrawWindow(app);
	// �������@�̐ݒ�
	PERLIN_NOISE_DATA filter_data = {0};
	// �ݒ�ύX�p�E�B�W�F�b�g
	PERLIN_NOISE_WIDGET widgets = {0};
	GdkColor color;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *layout;
	GtkWidget *control;
	GtkWidget *spin;
	GtkWidget *buttons[3];
	GtkWidget *scroll;
	GtkAdjustment *adjustment;
	// ���x���ɐݒ肷�镶����
	char str[4096];
	// �t�B���^�[��K�p���郌�C���[�̐�
	uint16 num_layer;
	// �t�B���^�[��K�p���郌�C���[
	LAYER **layers = GetLayerChain(window, &num_layer);
	// �_�C�A���O�̌���
	gint result;
	// int�^�Ń��C���[�̐����L�����Ă���(�L���X�g�p)
	int int_num_layer = num_layer;

	filter_data.num_octaves = PERLIN_NOISE_DEFAULT_OCTAVES;
	filter_data.persistence = PERLIN_NOISE_DEFAULT_PERSISTENCE;
	filter_data.frequency = PERLIN_NOISE_DEFAULT_FREQUENCY;
	filter_data.interpolation_type = (uint16)PERLIN_NOISE_COSINE_INTERPOLATION;

	widgets.filter_data = &filter_data;
	if(window->width >= window->height)
	{
		widgets.canvas_width = PERLIN_NOISE_PREVIEW_SIZE;
		widgets.canvas_height = (window->height * PERLIN_NOISE_PREVIEW_SIZE) / window->width;
	}
	else
	{
		widgets.canvas_height = PERLIN_NOISE_PREVIEW_SIZE;
		widgets.canvas_width = (window->width * PERLIN_NOISE_PREVIEW_SIZE) / window->height;
	}
	widgets.pixels = (uint8*)MEM_ALLOC_FUNC(widgets.canvas_width * widgets.canvas_height * 4);

	if(CheckPerlinoiseReverseColor(window, widgets.back_ground) != FALSE)
	{
		color.red = color.green = color.blue = 0;
	}
	else
	{
		color.red = color.green = color.blue = 0xffff;
		filter_data.color[0] = filter_data.color[1] = filter_data.color[2] = 0xff;
	}

	widgets.surface_p = cairo_image_surface_create_for_data(widgets.pixels,
		CAIRO_FORMAT_ARGB32, widgets.canvas_width, widgets.canvas_height, widgets.canvas_width * 4);
	widgets.canvas = gtk_drawing_area_new();
	gtk_widget_set_events(widgets.canvas, GDK_EXPOSURE_MASK);
#if GTK_MAJOR_VERSION <= 2
	(void)g_signal_connect(G_OBJECT(widgets.canvas), "expose_event",
		G_CALLBACK(DisplayPerlinNoisePreview), &widgets);
#else
	(void)g_signal_connect(G_OBJECT(widgets.canvas), "draw",
		G_CALLBACK(DisplayPerlinNoisePreview), &widgets);
#endif
	gtk_widget_set_size_request(widgets.canvas, widgets.canvas_width, widgets.canvas_height);

	filter_data.width = widgets.canvas_width;
	filter_data.height = widgets.canvas_height;

	scroll = gtk_scrolled_window_new(NULL, NULL);
	layout = gtk_vbox_new(FALSE, 0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll), layout);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(
		GTK_DIALOG(dialog))), scroll, TRUE, TRUE, 0);
	gtk_widget_set_size_request(scroll, 720, 480);
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(layout), hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), widgets.canvas, FALSE, FALSE, 2);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(layout), vbox, TRUE, TRUE, 2);

	(void)sprintf(str, "%s : ", app->labels->tool_box.rand_seed);
	label = gtk_label_new(str);
	spin = gtk_spin_button_new_with_range(0, INT_MAX, 1);
	g_object_set_data(G_OBJECT(spin), "setting-widgets", &widgets);
	(void)g_signal_connect(G_OBJECT(spin), "value_changed",
		G_CALLBACK(OnChangePerlinNoiseSeed), &filter_data);
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	widgets.use_random = gtk_check_button_new_with_label(app->labels->tool_box.use_random);
	(void)g_signal_connect(G_OBJECT(widgets.use_random), "toggled",
		G_CALLBACK(OnChangePerlinNoiseUseRandom), spin);
	g_object_set_data(G_OBJECT(widgets.use_random), "filter_data", &filter_data);
	g_object_set_data(G_OBJECT(widgets.use_random), "setting-widgets", &widgets);
	gtk_box_pack_start(GTK_BOX(vbox), widgets.use_random, FALSE, FALSE, 0);

	g_object_set_data(G_OBJECT(spin), "use_random", widgets.use_random);

	(void)sprintf(str, "%s : ", app->labels->tool_box.num_octaves);
	label = gtk_label_new(str);
	spin = gtk_spin_button_new_with_range(1, 30, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), PERLIN_NOISE_DEFAULT_OCTAVES);
	g_object_set_data(G_OBJECT(spin), "setting-widgets", &widgets);
	(void)g_signal_connect(G_OBJECT(spin), "value_changed",
		G_CALLBACK(OnChangePerlinNoiseOctaves), &filter_data);
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	(void)sprintf(str, "%s : ", app->labels->tool_box.frequency);
	label = gtk_label_new(str);
	spin = gtk_spin_button_new_with_range(1, 30, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), PERLIN_NOISE_DEFAULT_FREQUENCY);
	g_object_set_data(G_OBJECT(spin), "setting-widgets", &widgets);
	(void)g_signal_connect(G_OBJECT(spin), "value_changed",
		G_CALLBACK(OnChangePerlinNoiseFrequency), &filter_data);
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	(void)sprintf(str, "%s : ", app->labels->tool_box.persistence);
	label = gtk_label_new(str);
	spin = gtk_spin_button_new_with_range(0.01, 3, 0.1);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(spin), 2);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), PERLIN_NOISE_DEFAULT_PERSISTENCE);
	g_object_set_data(G_OBJECT(spin), "setting-widgets", &widgets);
	(void)g_signal_connect(G_OBJECT(spin), "value_changed",
		G_CALLBACK(OnChangePerlinNoisePersistence), &filter_data);
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	(void)sprintf(str, "%s : ", app->labels->tool_box.cloud_color);
	label = gtk_label_new(str);
	control = gtk_color_button_new_with_color(&color);
	g_object_set_data(G_OBJECT(control), "setting-widgets", &widgets);
	(void)g_signal_connect(G_OBJECT(control), "color-set",
		G_CALLBACK(OnChangePerlinNoiseCloudColor), &filter_data);
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), control, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	buttons[0] = gtk_radio_button_new_with_label(NULL, app->labels->tool_box.linear);
	g_object_set_data(G_OBJECT(buttons[0]), "interpolation",
		GINT_TO_POINTER(PERLIN_NOISE_LINEAR_INTERPOLATION));
	g_object_set_data(G_OBJECT(buttons[0]), "setting-widgets", &widgets);
	(void)g_signal_connect(G_OBJECT(buttons[0]), "toggled",
		G_CALLBACK(OnChangePerlinNoiseInterpolation), &filter_data);
	gtk_box_pack_start(GTK_BOX(vbox), buttons[0], FALSE, FALSE, 0);
	buttons[1] = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(buttons[0]), app->labels->tool_box.cosine);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[1]), TRUE);
	g_object_set_data(G_OBJECT(buttons[1]), "interpolation",
		GINT_TO_POINTER(PERLIN_NOISE_COSINE_INTERPOLATION));
	g_object_set_data(G_OBJECT(buttons[1]), "setting-widgets", &widgets);
	(void)g_signal_connect(G_OBJECT(buttons[1]), "toggled",
		G_CALLBACK(OnChangePerlinNoiseInterpolation), &filter_data);
	gtk_box_pack_start(GTK_BOX(vbox), buttons[1], FALSE, FALSE, 0);
	buttons[2] = gtk_radio_button_new_with_label_from_widget(
		GTK_RADIO_BUTTON(buttons[0]), app->labels->tool_box.cubic);
	g_object_set_data(G_OBJECT(buttons[2]), "setting-widgets", &widgets);
	g_object_set_data(G_OBJECT(buttons[2]), "interpolation",
		GINT_TO_POINTER(PERLIN_NOISE_CUBIC_INTERPOLATION));
	(void)g_signal_connect(G_OBJECT(buttons[2]), "toggled",
		G_CALLBACK(OnChangePerlinNoiseInterpolation), &filter_data);
	gtk_box_pack_start(GTK_BOX(vbox), buttons[2], FALSE, FALSE, 0);

	control = gtk_check_button_new_with_label(app->labels->tool_box.colorize);
	g_object_set_data(G_OBJECT(control), "setting-widgets", &widgets);
	(void)g_signal_connect(G_OBJECT(control), "toggled",
		G_CALLBACK(OnChangePerlinNoiseColorMode), &filter_data);
	gtk_box_pack_start(GTK_BOX(vbox), control, FALSE, FALSE, 0);

	adjustment = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 100, 1, 1, 0));
	g_object_set_data(G_OBJECT(adjustment), "filter_data", &filter_data);
	g_object_set_data(G_OBJECT(adjustment), "setting-widgets", &widgets);
	control = SpinScaleNew(adjustment, app->labels->layer_window.opacity, 0);
	(void)g_signal_connect(G_OBJECT(adjustment), "value_changed",
		G_CALLBACK(OnChangePerlinNoiseOpacity), &filter_data);
	gtk_box_pack_start(GTK_BOX(vbox), control, FALSE, FALSE, 0);

	widgets.immediate_update =
		gtk_check_button_new_with_label(app->labels->tool_box.update_immediately);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets.immediate_update), TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), widgets.immediate_update, FALSE, FALSE, 0);

	control = gtk_button_new_with_label(app->labels->tool_box.update);
	g_object_set_data(G_OBJECT(control), "setting-widgets", &widgets);
	(void)g_signal_connect(G_OBJECT(control), "clicked",
		G_CALLBACK(OnClickedPerlinNoiseUpdateButton), &filter_data);
	gtk_box_pack_start(GTK_BOX(vbox), control, FALSE, FALSE, 0);

	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	if(result == GTK_RESPONSE_OK)
	{// O.K.�{�^���������ꂽ
		filter_data.width = window->width;
		filter_data.height = window->height;

		// ��ɗ����f�[�^���c��
		AddFilterHistory(app->labels->menu.cloud, &filter_data, sizeof(filter_data),
			FILTER_FUNC_PERLIN_NOISE, layers, num_layer, window);

		PerlinNoiseFilter(window, layers, num_layer, &filter_data);
	}

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	gtk_widget_queue_draw(window->window);

	cairo_surface_destroy(widgets.surface_p);
	MEM_FREE_FUNC(widgets.pixels);

	MEM_FREE_FUNC(layers);

	// �_�C�A���O������
	gtk_widget_destroy(dialog);
}

/*************************************
* FractalFilter�֐�                  *
* �t���N�^���}�`�̕`����s           *
* ����                               *
* window	: �`��̈�̏��         *
* layers	: �������s�����C���[�z�� *
* num_layer	: �������s�����C���[�̐� *
* data		: �F�����E�擾�͈̓f�[�^ *
*************************************/
void FractalFilter(DRAW_WINDOW* window, LAYER** layers, uint16 num_layer, void* data)
{
	MEMORY_STREAM stream = {(uint8*)data, 0, 8, 1};
	guint32 data_size;
	cairo_surface_t *surface_p;
	uint8 *pixels;
	gint32 width, height, stride;
	// �t�B���^�[��K�p���郌�C���[
	int i;

	(void)MemRead(&data_size, sizeof(data_size), 1, &stream);
	stream.data_size = data_size;

	pixels = ReadPNGStream((void*)&stream, (stream_func_t)MemRead,
		&width, &height, &stride);
	surface_p = cairo_image_surface_create_for_data(pixels,
		CAIRO_FORMAT_ARGB32, width, height, stride);

	for(i=0; i<num_layer; i++)
	{
		if(layers[i]->layer_type == TYPE_NORMAL_LAYER)
		{
			cairo_set_source_surface(layers[i]->cairo_p, surface_p, 0, 0);
			cairo_set_operator(layers[i]->cairo_p, CAIRO_OPERATOR_OVER);

			if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) == 0)
			{
				cairo_paint(layers[i]->cairo_p);
			}
			else
			{
				cairo_mask_surface(layers[i]->cairo_p, window->selection->surface_p, 0, 0);
			}
		}
	}

	cairo_surface_destroy(surface_p);
	MEM_FREE_FUNC(pixels);
}

/*****************************************************
* ExecuteFractal�֐�                                 *
* �t���N�^���}�`�ŉ��h����s                         *
* ����                                               *
* app	: �A�v���P�[�V�������Ǘ�����\���̂̃A�h���X *
*****************************************************/
void ExecuteFractal(APPLICATION* app)
{
#define FRACTAL_PREVIEW_SIZE 400
	// �g�傷��s�N�Z�������w�肷��_�C�A���O
	GtkWidget* dialog = gtk_dialog_new_with_buttons(
		app->labels->menu.fractal,
		GTK_WINDOW(app->window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL
	);
	// ��������`��̈�
	DRAW_WINDOW* window = GetActiveDrawWindow(app);
	// �������@�̐ݒ�
	FRACTAL fractal;
	FRACTAL_EDITOR_WIDGET *editor;
	// �����f�[�^
	MEMORY_STREAM *stream = CreateMemoryStream(window->pixel_buf_size);
	// �ݒ�ύX�p�E�B�W�F�b�g
	GtkWidget *vbox;
	// �_�C�A���O�̈ʒu�A�T�C�Y�ݒ�p
	gint x, y;
	GtkAllocation allocation;
	// �����f�[�^�̃T�C�Y
	size_t history_size;
	// �t�B���^�[��K�p���郌�C���[�̐�
	uint16 num_layer;
	// �t�B���^�[��K�p���郌�C���[
	LAYER **layers = GetLayerChain(window, &num_layer);
	// �L�����o�X�ւ̕`��p�T�[�t�F�X
	cairo_surface_t *surface_p;
	// �s�N�Z���f�[�^
	uint8 *pixels;
	// �v���r���[�̃T�C�Y
	int width, height;
	// �ݒ��ʂ��X�N���[���Ɏ��܂肫��Ȃ��̂ŃX�N���[������
	GtkWidget *scroll;
	// for���p�̃J�E���^
	int i;

	if(window->width >= window->height)
	{
		width = FRACTAL_PREVIEW_SIZE;
		height = FRACTAL_PREVIEW_SIZE * window->height / window->width;
	}
	else
	{
		height = FRACTAL_PREVIEW_SIZE;
		width = FRACTAL_PREVIEW_SIZE * window->width / window->height;
	}

	// �t���N�^���}�`�`��̏�����
	InitializeFractal(&fractal, width, height);
	fractal.random_start = (int)time(NULL);
	FractalRandomizeControlPoint(&fractal, &fractal.control_point,
		2, 3, 0);
	FractalRender(&fractal, fractal.random_start);

	vbox = FractalEditorNew(&fractal, FRACTAL_PREVIEW_SIZE,
		FRACTAL_PREVIEW_SIZE, app->fractal_labels, &editor);
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll), vbox);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#if GTK_MAJOR_VERSION <= 2
	allocation = app->window->allocation;
#else
	gtk_widget_get_allocation(app->window, &allocation);
#endif
	gtk_widget_set_size_request(scroll, allocation.width, allocation.height - 80);
	gtk_window_get_position(GTK_WINDOW(app->window), &x, &y);
	gtk_window_move(GTK_WINDOW(dialog), x, y);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		scroll, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
	{
		guint32 data_size = 0;

		(void)MemWrite(&data_size, sizeof(data_size), 1, stream);

		ResizeFractal(&fractal, window->width, window->height);
		FractalRender(&fractal, fractal.random_start);

		pixels = (uint8*)fractal.pixels;
		WritePNGStream((void*)stream, (stream_func_t)MemWrite,
			NULL, pixels, window->width, window->height, window->stride,
			window->channel, 0, Z_DEFAULT_COMPRESSION);
		history_size = stream->data_point;
		data_size = (guint32)(history_size - sizeof(data_size));
		(void)MemSeek(stream, 0, SEEK_SET);
		(void)MemWrite(&data_size, sizeof(data_size), 1, stream);

		AddFilterHistory(app->labels->menu.fractal, stream->buff_ptr, history_size,
			FILTER_FUNC_FRACTAL, layers, num_layer, window);

		surface_p = cairo_image_surface_create_for_data(pixels,
			CAIRO_FORMAT_ARGB32, window->width, window->height, window->stride);

		for(i=0; i<num_layer; i++)
		{
			if(layers[i]->layer_type == TYPE_NORMAL_LAYER)
			{
				cairo_set_source_surface(layers[i]->cairo_p, surface_p, 0, 0);
				cairo_set_operator(layers[i]->cairo_p, CAIRO_OPERATOR_OVER);
				if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) == 0)
				{
					cairo_paint(layers[i]->cairo_p);
				}
				else
				{
					cairo_mask_surface(layers[i]->cairo_p, window->selection->surface_p, 0, 0);
				}
			}
		}

		cairo_surface_destroy(surface_p);
	}

	ReleaseFractal(&fractal);
	(void)DeleteMemoryStream(stream);
	MEM_FREE_FUNC(layers);

	gtk_widget_destroy(dialog);

	// �L�����o�X���X�V
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	gtk_widget_queue_draw(window->window);
}

filter_func g_filter_funcs[] =
{
	BlurFilter,
	MotionBlurFilter,
	ChangeBrightContrastFilter,
	ChangeHueSaturationFilter,
	Luminosity2OpacityFilter,
	Color2AlphaFilter,
	ColorizeWithUnderFilter,
	GradationMapFilter,
	FillWithVectorLineFilter,
	PerlinNoiseFilter,
	FractalFilter
};

selection_filter_func g_selection_filter_funcs[] =
{
	SelectionBlurFilter,
	SelectionMotionBlurFilter,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

#ifdef __cplusplus
}
#endif
