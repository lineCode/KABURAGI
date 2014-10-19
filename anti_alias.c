#include <string.h>
#include "types.h"
#include "anti_alias.h"
#include "layer.h"

#ifdef __cplusplus
extern "C" {
#endif

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
void AntiAlias(
	uint8* in_buff,
	uint8* out_buff,
	int width,
	int height,
	int stride,
	int channel
)
{
// �A���`�G�C���A�V���O���s��1�`�����l������臒l
#define ANTI_ALIAS_THRESHOLD (51*51)

	// �A���`�G�C���A�V���O���s��臒l
	int threshold = ANTI_ALIAS_THRESHOLD * channel;
	// �s�N�Z���f�[�^�z��̃C���f�b�N�X
	int index, now_index;
	// ���݂̃s�N�Z���Ǝ��͂̃s�N�Z���̐F��
	int color_diff[4];
	// �F���̍��v
	int sum_diff;
	// ���͂̃s�N�Z���̍��v�l
	int sum_color[4];
	int i, j, k;	// for���p�̃J�E���^

	// 1�s�ڂ͂��̂܂܃R�s�[
	(void)memcpy(out_buff, in_buff, stride);

	// 2�s�ڂ����ԉ���O�̍s�܂ŏ���
	for(i=1; i<height-1; i++)
	{
		// ��ԍ��͂��̂܂܃R�s�[
		index = i * stride;
		for(j=0; j<channel; j++)
		{
			out_buff[i+j] = in_buff[i+j];
		}

		for(j=1; j<width-1; j++)
		{	// �F�����v���N���A
			color_diff[3] = color_diff[2] = color_diff[1] = color_diff[0] = 0;
			// ���͂̃s�N�Z���̍��v�l���N���A
			sum_color[3] = sum_color[2] = sum_color[1] = sum_color[0] = 0;

			// ����8�s�N�Z���Ƃ̐F���ƒl�̍��v���v�Z
			now_index = i*stride + j * channel;
			index = (i-1)*stride + (j-1) * channel;
			for(k=0; k<channel; k++)
			{
				color_diff[k] += ((int)in_buff[now_index+k]-(int)in_buff[index+k])
					* ((int)in_buff[now_index+k]-(int)in_buff[index+k]);
				sum_color[k] += in_buff[now_index+k];
				sum_color[k] += in_buff[index+k];
			}
			index = (i-1)*stride + j * channel;
			for(k=0; k<channel; k++)
			{
				color_diff[k] += ((int)in_buff[now_index+k]-(int)in_buff[index+k])
					* ((int)in_buff[now_index+k]-(int)in_buff[index+k]);
				sum_color[k] += in_buff[index+k];
			}
			index = (i-1)*stride + (j+1) * channel;
			for(k=0; k<channel; k++)
			{
				color_diff[k] += ((int)in_buff[now_index+k]-(int)in_buff[index+k])
					* ((int)in_buff[now_index+k]-(int)in_buff[index+k]);
				sum_color[k] += in_buff[index+k];
			}
			index = i*stride + (j-1) * channel;
			for(k=0; k<channel; k++)
			{
				color_diff[k] += ((int)in_buff[now_index+k]-(int)in_buff[index+k])
					* ((int)in_buff[now_index+k]-(int)in_buff[index+k]);
				sum_color[k] += in_buff[index+k];
			}
			index = i*stride + (j+1) * channel;
			for(k=0; k<channel; k++)
			{
				color_diff[k] += ((int)in_buff[now_index+k]-(int)in_buff[index+k])
					* ((int)in_buff[now_index+k]-(int)in_buff[index+k]);
				sum_color[k] += in_buff[index+k];
			}
			index = (i+1)*stride + (j-1) * channel;
			for(k=0; k<channel; k++)
			{
				color_diff[k] += ((int)in_buff[now_index+k]-(int)in_buff[index+k])
					* ((int)in_buff[now_index+k]-(int)in_buff[index+k]);
				sum_color[k] += in_buff[index+k];
			}
			index = (i+1)*stride + j * channel;
			for(k=0; k<channel; k++)
			{
				color_diff[k] += ((int)in_buff[now_index+k]-(int)in_buff[index+k])
					* ((int)in_buff[now_index+k]-(int)in_buff[index+k]);
				sum_color[k] += in_buff[index+k];
			}
			index = (i+1)*stride + (j+1) * channel;
			for(k=0; k<channel; k++)
			{
				color_diff[k] += ((int)in_buff[now_index+k]-(int)in_buff[index+k])
					* ((int)in_buff[now_index+k]-(int)in_buff[index+k]);
				sum_color[k] += in_buff[index+k];
			}

			sum_diff = color_diff[0];
			for(k=1; k<channel; k++)
			{
				sum_diff += color_diff[k];
			}

			// �F���̍��v��臒l�ȏ�Ȃ�΃A���`�G�C���A�V���O���s
			if(sum_diff > threshold)
			{	// �ݒ肷��V���ȃf�[�^
				uint8 new_value;

				for(k=0; k<channel; k++)
				{
					new_value = (uint8)(sum_color[k] / 9);
					if(new_value > in_buff[now_index+k])
					{
						out_buff[now_index+k] = new_value;
					}
					else
					{
						out_buff[now_index+k] = in_buff[now_index+k];
					}
				}
			}
			else
			{
				for(k=0; k<channel; k++)
				{
					out_buff[now_index+k] = in_buff[now_index+k];
				}
			}
		}
	}	// 2�s�ڂ����ԉ���O�̍s�܂ŏ���
		// for(i=1; i<height-1; i++)

	// ��ԉ��̍s�͂��̂܂܃R�s�[
	(void)memcpy(&out_buff[i*stride], &in_buff[i*stride], stride);
#undef ANTI_ALIAS_THRESHOLD
}

/*********************************************************
* AntiAliasLayer�֐�                                     *
* ���C���[�ɑ΂��Ĕ͈͂��w�肵�ăA���`�G�C���A�X�������� *
* layer	: �A���`�G�C���A�X�������郌�C���[               *
* rect	: �A���`�G�C���A�X��������͈�                   *
*********************************************************/
void AntiAliasLayer(LAYER *layer, LAYER* temp, ANTI_ALIAS_RECTANGLE *rect)
{
// �A���`�G�C���A�V���O���s��1�`�����l������臒l
#define ANTI_ALIAS_THRESHOLD (51*51)
	// �s�N�Z���f�[�^�z��̃C���f�b�N�X
	int index, now_index;
	// ���݂̃s�N�Z���Ǝ��͂̃s�N�Z���̐F��
	int color_diff[4];
	// �F���̍��v
	int sum_diff;
	// ���͂̃s�N�Z���̍��v�l
	int sum_color[4];
	// �A���`�G�C���A�X�J�n�E�I���̍��W
	int x, y, end_x, end_y;
	// ����1�s���̃o�C�g��
	int stride;
	int i, j;	// for���p�̃J�E���^

	// ���W�Ɣ͈͂�ݒ�
	end_x = rect->width;
	end_y = rect->height;
	if(rect->x < 0)
	{
		end_x += rect->x;
		x = 0;
	}
	else if(rect->x > layer->width)
	{
		return;
	}
	else
	{
		x = rect->x;
	}
	end_x = x + end_x;
	if(end_x > layer->width)
	{
		end_x = layer->width;
	}
	if(end_x - x < 3)
	{
		return;
	}

	if(rect->y < 0)
	{
		end_y += rect->y;
		y = 0;
	}
	else if(rect->y > layer->height)
	{
		return;
	}
	else
	{
		y = rect->y;
	}
	end_y = y + end_y;
	if(end_y > layer->height)
	{
		end_y = layer->height;
	}

	// 2�s�ڂ����ԉ���O�̍s�܂ŏ���
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for(i=y+1; i<end_y-1; i++)
	{
		// ��ԍ��͂��̂܂܃R�s�[
		now_index = i * layer->stride + (x+1)*4;

		for(j=x+1; j<end_x-1; j++, now_index+=4)
		{

			// ����8�s�N�Z���Ƃ̐F���ƒl�̍��v���v�Z
			index = (i-1)*layer->stride + (j-1)*4;
			color_diff[0] = ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] = layer->pixels[now_index+0];
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] = ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] = layer->pixels[now_index+1];
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] = ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] = layer->pixels[now_index+2];
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] = ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] = layer->pixels[now_index+3];
			sum_color[3] += layer->pixels[index+3];

			index += 4;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			index += 4;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			index = i*layer->stride + (j-1)*4;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			index += 8;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			index = (i+1)*layer->stride + (j-1)*4;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			index += 4;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			index += 4;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			sum_diff = color_diff[0] + color_diff[1] + color_diff[2] + color_diff[3];

			// �F���̍��v��臒l�ȏ�Ȃ�΃A���`�G�C���A�V���O���s
			if(sum_diff > ANTI_ALIAS_THRESHOLD)
			{
				temp->pixels[now_index+0] = (uint8)(sum_color[0] / 9);
				temp->pixels[now_index+1] = (uint8)(sum_color[1] / 9);
				temp->pixels[now_index+2] = (uint8)(sum_color[2] / 9);
				temp->pixels[now_index+3] = (uint8)(sum_color[3] / 9);
				/*
				// �ݒ肷��V���ȃf�[�^
				uint8 new_value;

				new_value = (uint8)(sum_color[0] / 9);
				if(new_value > layer->pixels[now_index+0])
				{
					temp->pixels[now_index+0] = new_value;
				}
				else
				{
					temp->pixels[now_index+0] = layer->pixels[now_index+0];
				}
				new_value = (uint8)(sum_color[1] / 9);
				if(new_value > layer->pixels[now_index+1])
				{
					temp->pixels[now_index+1] = new_value;
				}
				else
				{
					temp->pixels[now_index+1] = layer->pixels[now_index+1];
				}
				new_value = (uint8)(sum_color[2] / 9);
				if(new_value > layer->pixels[now_index+2])
				{
					temp->pixels[now_index+2] = new_value;
				}
				else
				{
					temp->pixels[now_index+2] = layer->pixels[now_index+2];
				}
				new_value = (uint8)(sum_color[3] / 9);
				if(new_value > layer->pixels[now_index+3])
				{
					temp->pixels[now_index+3] = new_value;
				}
				else
				{
					temp->pixels[now_index+3] = layer->pixels[now_index+3];
				}
				*/
			}
			else
			{
				temp->pixels[now_index] = layer->pixels[now_index];
				temp->pixels[now_index+1] = layer->pixels[now_index+1];
				temp->pixels[now_index+2] = layer->pixels[now_index+2];
				temp->pixels[now_index+3] = layer->pixels[now_index+3];
			}
		}
	}	// 2�s�ڂ����ԉ���O�̍s�܂ŏ���
		// for(i=1; i<height-1; i++)

	stride = (end_x - x - 2) * 4;
	// ���ʂ�Ԃ�
	for(i=y+1; i<end_y-1; i++)
	{
		(void)memcpy(&layer->pixels[i*layer->stride+(x+1)*4],
			&temp->pixels[i*layer->stride+(x+1)*4], stride);
	}
}

/*********************************************************************
* AntiAliasVectorLine�֐�                                            *
* �x�N�g�����C���[�̐��ɑ΂��Ĕ͈͂��w�肵�ăA���`�G�C���A�X�������� *
* layer	: �A���`�G�C���A�X�������郌�C���[                           *
* rect	: �A���`�G�C���A�X��������͈�                               *
*********************************************************************/
void AntiAliasVectorLine(LAYER *layer, LAYER* temp, ANTI_ALIAS_RECTANGLE *rect)
{
// �A���`�G�C���A�V���O���s��1�`�����l������臒l
#define ANTI_ALIAS_THRESHOLD (51*51)
	// �A���`�G�C���A�X�J�n�E�I���̍��W
	int x, y, end_x, end_y;
	// ����1�s���̃o�C�g��
	int stride;
	int i;	// for���p�̃J�E���^

	// ���W�Ɣ͈͂�ݒ�
	end_x = rect->width;
	end_y = rect->height;
	if(rect->x < 0)
	{
		end_x += rect->x;
		x = 0;
	}
	else if(rect->x > layer->width)
	{
		return;
	}
	else
	{
		x = rect->x;
	}
	end_x = x + end_x;
	if(end_x > layer->width)
	{
		end_x = layer->width;
	}
	if(end_x - x < 3)
	{
		return;
	}

	if(rect->y < 0)
	{
		end_y += rect->y;
		y = 0;
	}
	else if(rect->y > layer->height)
	{
		return;
	}
	else
	{
		y = rect->y;
	}
	end_y = y + end_y;
	if(end_y > layer->height)
	{
		end_y = layer->height;
	}

#ifdef _OPENMP
#pragma omp parallel for
#endif
	// 2�s�ڂ����ԉ���O�̍s�܂ŏ���
	for(i=y+1; i<end_y-1; i++)
	{
		// �s�N�Z���f�[�^�z��̃C���f�b�N�X
		int index, now_index;
		// ���݂̃s�N�Z���Ǝ��͂̃s�N�Z���̐F��
		int color_diff[4];
		// �F���̍��v
		int sum_diff;
		// ���͂̃s�N�Z���̍��v�l
		int sum_color[4];
		int j;	// for���p�̃J�E���^

		// ��ԍ��͂��̂܂܃R�s�[
		now_index = i * layer->stride + (x+1)*4;

		for(j=x+1; j<end_x-1; j++, now_index+=4)
		{

			// ����8�s�N�Z���Ƃ̐F���ƒl�̍��v���v�Z
			index = (i-1)*layer->stride + (j-1)*4;
			color_diff[0] = ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] = layer->pixels[now_index+0];
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] = ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] = layer->pixels[now_index+1];
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] = ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] = layer->pixels[now_index+2];
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] = ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] = layer->pixels[now_index+3];
			sum_color[3] += layer->pixels[index+3];

			index += 4;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			index += 4;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			index = i*layer->stride + (j-1)*4;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			index += 8;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			index = (i+1)*layer->stride + (j-1)*4;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			index += 4;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			index += 4;
			color_diff[0] += ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0])
				* ((int)layer->pixels[now_index+0]-(int)layer->pixels[index+0]);
			sum_color[0] += layer->pixels[index+0];
			color_diff[1] += ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+1])
				* ((int)layer->pixels[now_index+1]-(int)layer->pixels[index+0]);
			sum_color[1] += layer->pixels[index+1];
			color_diff[2] += ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+2])
				* ((int)layer->pixels[now_index+2]-(int)layer->pixels[index+0]);
			sum_color[2] += layer->pixels[index+2];
			color_diff[3] += ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3])
				* ((int)layer->pixels[now_index+3]-(int)layer->pixels[index+3]);
			sum_color[3] += layer->pixels[index+3];

			sum_diff = color_diff[0] + color_diff[1] + color_diff[2] + color_diff[3];

			// �F���̍��v��臒l�ȏ�Ȃ�΃A���`�G�C���A�V���O���s
			if(sum_diff > ANTI_ALIAS_THRESHOLD)
			{	// �ݒ肷��V���ȃf�[�^
				uint8 new_value;

				new_value = (uint8)(sum_color[0] / 9);
				temp->pixels[now_index+0] = new_value;

				new_value = (uint8)(sum_color[1] / 9);
				temp->pixels[now_index+1] = new_value;
				
				new_value = (uint8)(sum_color[2] / 9);
				temp->pixels[now_index+2] = new_value;

				new_value = (uint8)(sum_color[3] / 9);
				temp->pixels[now_index+3] = new_value;
			}
			else
			{
				temp->pixels[now_index] = layer->pixels[now_index];
				temp->pixels[now_index+1] = layer->pixels[now_index+1];
				temp->pixels[now_index+2] = layer->pixels[now_index+2];
				temp->pixels[now_index+3] = layer->pixels[now_index+3];
			}
		}
	}	// 2�s�ڂ����ԉ���O�̍s�܂ŏ���
		// for(i=1; i<height-1; i++)

	stride = (end_x - x - 2) * 4;
	// ���ʂ�Ԃ�
	for(i=y+1; i<end_y-1; i++)
	{
		(void)memcpy(&layer->pixels[i*layer->stride+(x+1)*4],
			&temp->pixels[i*layer->stride+(x+1)*4], stride);
	}
}

#ifdef __cplusplus
}
#endif
