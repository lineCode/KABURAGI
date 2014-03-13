 //---------------------------------------------------------------------------

#include <stdio.h>

#include "tlg6_encode.h"
#include "tlg6_bit_stream.h"
#include "golomb_table.h"
#include "memory.h"

#include "slide.h"

#define MAX_COLOR_COMPONENTS 4

#define FILTER_TRY_COUNT 16

#define W_BLOCK_SIZE 8
#define H_BLOCK_SIZE 8

//------------------------------ FOR DEBUG
//#define FILTER_TEST
//#define WRITE_ENTROPY_VALUES
//#define WRITE_VSTXT
//------------------------------


/*
	shift-jis

	TLG6 ���k�A���S���Y��(�T�v)

	TLG6 �͋g���g���Q�ŗp������摜���k�t�H�[�}�b�g�ł��B�O���[�X�P�[���A
	24bit�r�b�g�}�b�v�A8bit�A���t�@�`�����l���t��24bit�r�b�g�}�b�v�ɑΉ���
	�Ă��܂��B�f�U�C���̃R���Z�v�g�́u�����k���ł��邱�Ɓv�u�摜�W�J�ɂ�����
	�\���ɍ����Ȏ������ł��邱�Ɓv�ł��B

	TLG6 �͈ȉ��̏����ŉ摜�����k���܂��B

	1.   MED �܂��� ���ϖ@ �ɂ��P�x�\��
	2.   ���I�[�_�����O�ɂ����ʍ팸
	3.   �F���փt�B���^�ɂ����ʍ팸
	4.   �S�����E���C�X�����ɂ��G���g���s�[������



	1.   MED �܂��� ���ϖ@ �ɂ��P�x�\��

		TLG6 �� MED (Median Edge Detector) ���邢�͕��ϖ@�ɂ��P�x�\�����s
		���A�\���l�Ǝ��ۂ̒l�̌덷���L�^���܂��B

		MED �́A�\���ΏۂƂ���s�N�Z���ʒu(���}x�ʒu)�ɑ΂��A���ׂ̃s�N�Z��
		�̋P�x��a�A��ׂ̃s�N�Z���̋P�x��b�A����̃s�N�Z���̋P�x��c�Ƃ��A�\
		���Ώۂ̃s�N�Z���̋P�x x ���ȉ��̂悤�ɗ\�����܂��B

			+-----+-----+
			|  c  |  b  |
			+-----+-----+
			|  a  |  x  |
			+-----+-----+

			x = min(a,b)    (c > max(a,b) �̏ꍇ)
			x = max(a,b)    (c < min(a,b) �̏ꍇ)
			x = a + b - c   (���̑��̏ꍇ)

		MED�́A�P���ȁA���͂̃s�N�Z���̋P�x�̕��ςɂ��\���ȂǂƔ�ׁA�摜
		�̏c�����̃G�b�W�ł͏�̃s�N�Z�����A�������̃G�b�W�ł͍��̃s�N�Z����
		�Q�Ƃ��ė\�����s���悤�ɂȂ邽�߁A�����̂悢�\�����\�ł��B

		���ϖ@�ł́Aa �� b �̋P�x�̕��ς���\���Ώۂ̃s�N�Z���̋P�x x ��\
		�����܂��B�ȉ��̎���p���܂��B

			x = (a + b + 1) >> 1

		MED �� ���ϖ@�́A8x8 �ɋ�؂�ꂽ�摜�u���b�N���ŗ����ɂ�鈳�k����
		��r���A���k���̍����������̕��@���I������܂��B

		R G B �摜�� A R G B �摜�ɑ΂��Ă͂����F�R���|�[�l���g���Ƃɍs��
		�܂��B

	2.   ���I�[�_�����O�ɂ����ʍ팸

		�덷�݂̂ƂȂ����f�[�^�́A���I�[�_�����O(���ёւ�)���s���܂��B
		TLG6�ł́A�܂��摜��8�~8�̏����ȃu���b�N�ɕ������܂��B�u���b�N�͉�
		�����ɁA���̃u���b�N���珇�ɉE�̃u���b�N�Ɍ������A��񕪂̃u���b�N
		�̏�������������΁A���̈�񉺂̃u���b�N�A�Ƃ��������ŏ������s����
		���B
		���̌X�̃u���b�N���ł́A�s�N�Z�����ȉ��̏��Ԃɕ��ёւ��܂��B

		���ʒu�������̃u���b�N      ���ʒu����̃u���b�N
		 1  2  3  4  5  6  7  8     57 58 59 60 61 62 63 64
		16 15 14 13 12 11 10  9     56 55 54 53 52 51 50 49
		17 18 19 20 21 22 23 24     41 42 43 44 45 46 47 48
		32 31 30 29 28 27 26 25     40 39 38 37 36 35 34 33
		33 34 35 36 37 38 39 40     25 26 27 28 29 30 31 32
		48 47 46 45 44 43 42 41     24 23 22 21 20 19 18 17
		49 50 51 52 53 54 55 56      9 10 11 12 13 14 15 16
		64 63 62 61 60 59 58 57      8  7  6  5  4  3  2  1

		���̂悤�ȁu�W�O�U�O�v��̕��ёւ����s�����Ƃɂ��A����(�P�x��F���A
		�G�b�W�Ȃ�)�̎����s�N�Z�����A������\���������Ȃ�A��i�̃G���g��
		�s�[�������i�ł̃[���E�����E�����O�X���k�̌��ʂ����߂邱�Ƃ��ł��܂��B

	3.   �F���փt�B���^�ɂ����ʍ팸

		R G B �摜 ( A R G B �摜�������� ) �ɂ����ẮA�F�R���|�[�l���g��
		�̑��֐��𗘗p���ď��ʂ����点��\��������܂��BTLG6�ł́A�ȉ���
		16��ނ̃t�B���^���u���b�N���ƂɓK�p���A��i�̃S�����E���C�X�����ɂ�
		�鈳�k��T�C�Y�����Z���A�����Ƃ��T�C�Y�̏������Ȃ�t�B���^��K�p����
		���B�t�B���^�ԍ��� MED/���ϖ@�̕ʂ� LZSS �@�ɂ�舳�k����A�o�͂���
		�܂��B

		0          B        G        R              (�t�B���^����)
		1          B-G      G        R-G
		2          B        G-B      R-B-G
		3          B-R-G    G-R      R
		4          B-R      G-B-R    R-B-R-G
		5          B-R      G-B-R    R
		6          B-G      G        R
		7          B        G-B      R
		8          B        G        R-G
		9          B-G-R-B  G-R-B    R-B
		10         B-R      G-R      R
		11         B        G-B      R-B
		12         B        G-R-B    R-B
		13         B-G      G-R-B-G  R-B-G
		14         B-G-R    G-R      R-B-G-R
		15         B        G-(B<<1) R-(B<<1)

		�����̃t�B���^�́A�����̉摜�Ńe�X�g�������A�t�B���^�̎g�p�p�x��
		�ׁA�����Ƃ��K�p�̑�������16�̃t�B���^��I�o���܂����B

	4.   �S�����E���C�X�����ɂ��G���g���s�[������

		�ŏI�I�Ȓl�̓S�����E���C�X�����ɂ��r�b�g�z��Ɋi�[����܂��B������
		�ŏI�I�Ȓl�ɂ͘A������[�������ɑ������߁A�[���Ɣ�[���̘A������
		�K���}������p���ĕ��������܂�(�[���E�����E�����O�X)�B���̂��߁A�r�b
		�g�z�񒆂ɂ͊�{�I��

		1 ��[���̘A����(�����E�����O�X)�̃K���}����
		2 ��[���̒l�̃S�����E���C�X���� (�A������ �J��Ԃ����)
		3 �[���̘A����(�����E�����O�X)�̃K���}����

		�������񌻂�邱�ƂɂȂ�܂��B

		�K���}�����͈ȉ��̌`���ŁA0��ۑ�����K�v���Ȃ�����(����0�͂��蓾��
		���̂�)�A�\���ł���Œ�̒l��1�ƂȂ��Ă��܂��B

		                  MSB ��    �� LSB
		(v=1)                            1
		(2<=v<=3)                      x10          x = v-2
		(4<=v<=7)                    xx100         xx = v-4
		(8<=v<=15)                 xxx1000        xxx = v-8
		(16<=v<=31)             x xxx10000       xxxx = v-16
		(32<=v<=63)           xxx xx100000      xxxxx = v-32
		(64<=v<=127)        xxxxx x1000000     xxxxxx = v-64
		(128<=v<=255)     xxxxxxx 10000000    xxxxxxx = v-128
		(256<=v<=511)    xxxxxxx1 00000000   xxxxxxxx = v-256
		      :                    :                  :
		      :                    :                  :

		�S�����E���C�X�����͈ȉ��̕��@�ŕ��������܂��B�܂��A���������悤�Ƃ�
		��l��e�Ƃ��܂��Be�ɂ�1�ȏ�̐��̐�����-1�ȉ��̕��̐��������݂��܂��B
		�����0�ȏ�̐����ɕϊ����܂��B��̓I�ɂ͈ȉ��̎��ŕϊ����s���܂��B
		(�ϊ���̒l��m�Ƃ���)

		m = ((e >= 0) ? 2*e : -2*e-1) - 1

		���̕ϊ��ɂ��A���̕\�̂悤�ɁA����e��������m�ɁA����e�����m�ɕ�
		������܂��B

		 e     m
		-1     0
		 1     1
		-2     2
		 2     3
		-3     4
		 3     5
		 :     :
		 :     :

		���̒l m ���ǂꂮ�炢�̃r�b�g���ŕ\���ł��邩��\�����܂��B���̃r�b
		�g���� k �Ƃ��܂��Bk �� 0 �ȏ�̐����ł��B�������\�����͂���Am �� k
		�r�b�g�ŕ\���ł��Ȃ��ꍇ������܂��B���̏ꍇ�́Ak �r�b�g�ŕ\���ł�
		�Ȃ��������� m�A�܂� k>>m ���̃[�����o�͂��A�Ō�ɂ��̏I�[�L����
		���� 1 ���o�͂��܂��B
		m �� k�r�b�g�ŕ\���ł����ꍇ�́A�����P�� 1 ���o�͂��܂��B���̂��ƁA
		m �����ʃr�b�g���� k �r�b�g���o�͂��܂��B

		���Ƃ��� e �� 66 �� k �� 5 �̏ꍇ�͈ȉ��̂悤�ɂȂ�܂��B

		m �� m = ((e >= 0) ? 2*e : -2*e-1) - 1 �̎��ɂ�� 131 �ƂȂ�܂��B
		�\�������r�b�g�� 5 �ŕ\���ł���l�̍ő�l�� 31 �ł��̂ŁA131 �ł���
		m �� k �r�b�g�ŕ\���ł��Ȃ��Ƃ������ƂɂȂ�܂��B�ł��̂ŁAm >> k
		�ł��� 4 �� 0 ���o�͂��܂��B

		0000

		���ɁA����4�� 0 �̏I�[�L���Ƃ��� 1 ���o�͂��܂��B

		10000

		�Ō�� m �����ʃr�b�g���� k �r�b�g���o�͂��܂��B131 ���Q�i���ŕ\��
		����� 10000011 �ƂȂ�܂����A����̉��ʂ��� 5 �r�b�g�����o�͂��܂��B


        MSB��  ��LSB
		00011 1 0000      (�v10�r�b�g)
		~~~~~ ~ ~~~~
		  c   b   a

		a   (m >> k) �� 0
		b   a �� �I�[�L��
		c   m �̉��� k �r�b�g��

		���ꂪ e=66 k=5 �𕄍��������r�b�g��ƂȂ�܂��B

		�����I�ɂ� k �̗\���́A���܂łɏo�͂��� e �̐�Βl�̍��v�Ƃ���܂ł�
		�o�͂����������ɁA���炩���ߎ��ۂ̉摜�̓��v����쐬���ꂽ�e�[�u��
		���Q�Ƃ��邱�ƂŎZ�o���܂��B����� C++ ���ꕗ�ɋL�q����ƈȉ��̂悤
		�ɂȂ�܂��B

		n = 3; // �J�E���^
		a = 0; // e �̐�Βl�̍��v

		while(e ��ǂݍ���)
		{
			k = table[a][n]; // �e�[�u������ k ���Q��
			m = ((e >= 0) ? 2*e : -2*e-1) - 1;
			e �� m �� k �ŕ�����;
			a += m >> 1; // e �̐�Βl�̍��v (������ m >>1 �ő�p)
			if(--n < 0) a >>= 1, n = 3;
		}

		4 �񂲂Ƃ� a ���������� n �����Z�b�g����邽�߁A�O�̒l�̉e���͕�����
		���i�ނɂ�Ĕ����Ȃ�A���߂̒l��������I�� k ��\�����邱�Ƃ��ł�
		�܂��B


	�Q�l :
		Golomb codes
		http://oku.edu.mie-u.ac.jp/~okumura/compression/golomb/
		�O�d��w���� �������F���ɂ��S���������̊ȒP�ȉ��
*/

//---------------------------------------------------------------------------
#ifdef WRITE_VSTXT
FILE *vstxt = fopen("vs.txt", "wt");
#endif
#define GOLOMB_GIVE_UP_BYTES 4
void CompressValuesGolomb(TLG6BIT_STREAM *bs, char *buf, int size)
{
	// golomb encoding, -- http://oku.edu.mie-u.ac.jp/~okumura/compression/golomb/

	int count;

	int n = TVP_TLG6_GOLOMB_N_COUNT - 1; // ���̃J�E���^
	int a = 0; // �\���덷�̐�Βl�̘a

	int i;	// for���p�̃J�E���^

	// run-length golomb method
	TLG6PutValue(bs, buf[0]?1:0, 1);	// initial value state

	count = 0;
	for(i = 0; i < size; i++)
	{
		if(buf[i])
		{
			int ii;
			// write zero count
			if(count) TLG6PutGamma(bs, count);

			// count non-zero values
			count = 0;
			for(ii = i; ii < size; ii++)
			{
				if(buf[ii]) count++; else break;
			}

			// write non-zero count
			TLG6PutGamma(bs, count);

			// write non-zero values
			for(; i < ii; i++)
			{
				int e = buf[i];
#ifdef WRITE_VSTXT
				fprintf(vstxt, "%d ", e);
#endif
				int k = bs->table.GolombBitLengthTable[a][n];
				int m = ((e >= 0) ? 2*e : -2*e-1) - 1;
				int store_limit = bs->buffer_byte_pos + GOLOMB_GIVE_UP_BYTES;
				int put1 = 1;
				int c;

				for(c = (m >> k); c > 0; c--)
				{
					if(store_limit == bs->buffer_byte_pos)
					{
						TLG6PutValue(bs, m >> k, 8);
#ifdef WRITE_VSTXT
						fprintf(vstxt, "[ %d ] ", m>>k);
#endif
						put1 = 0;
						break;
					}
					TLG6Put1Bit(bs, 0);
				}
				if(store_limit == bs->buffer_byte_pos)
				{
					TLG6PutValue(bs, m >> k, 8);
#ifdef WRITE_VSTXT
					fprintf(vstxt, "[ %d ] ", m>>k);
#endif
					put1 = 0;
				}
				if(put1) TLG6Put1Bit(bs, 1);
				TLG6PutValue(bs, m, k);
				a += (m>>1);
				if (--n < 0) {
					a >>= 1; n = TVP_TLG6_GOLOMB_N_COUNT - 1;
				}
			}

#ifdef WRITE_VSTXT
			fprintf(vstxt, "\n");
#endif

			i = ii - 1;
			count = 0;
		}
		else
		{
			// zero
			count ++;
		}
	}

	if(count) TLG6PutGamma(bs, count);
}

typedef struct _TRY_COMPRESS_GOLOMB
{
	int total_bits;	// �r�b�g�̑���
	int count;		// ���������r�b�g��
	int n, a;
	int last_non_zero;
	GOLOMB_TABLE* table;
} TRY_COMPRESS_GOLOMB;

static void InitTryCompressGolomb(TRY_COMPRESS_GOLOMB* c, GOLOMB_TABLE* table)
{
	(void)memset(c, 0, sizeof(*c));
	c->total_bits = 1;
	c->n = TVP_TLG6_GOLOMB_N_COUNT - 1;
	c->table = table;
	InitGolombTable(c->table);
}

static int TryCompress(TRY_COMPRESS_GOLOMB* compress, char *buf, int size)
{
	int i;

	for(i = 0; i < size; i++)
	{
		if(buf[i])
		{
			// write zero count
			if(!compress->last_non_zero)
			{
				if(compress->count)
				{
					compress->total_bits +=
						GetGammaBitLength(compress->count);
				}

				// count non-zero values
				compress->count = 0;
			}

			// write non-zero values
			for( ; i < size; i++)
			{
				int unexp_bits;
				int e = buf[i];
				int k, m;
				if(!e) break;
				compress->count++;
				k = compress->table->GolombBitLengthTable[compress->a][compress->n];
				m = ((e >= 0) ? 2*e : -2*e-1) - 1;
				unexp_bits = (m>>k);

				if(unexp_bits >= (GOLOMB_GIVE_UP_BYTES*8-8/2))
				{
					unexp_bits = (GOLOMB_GIVE_UP_BYTES*8-8/2)+8;
				}

				compress->total_bits += unexp_bits + 1 + k;
				compress->a += (m>>1);
				if (--compress->n < 0)
				{
					compress->a >>= 1;
					compress->n = TVP_TLG6_GOLOMB_N_COUNT - 1;
				}
			}

			// write non-zero count

			i--;
			compress->last_non_zero = 1;
		}
		else
		{
			// zero
			if(compress->last_non_zero)
			{
				if(compress->count)
				{
					compress->total_bits += GetGammaBitLength(compress->count);
					compress->count = 0;
				}
			}

			compress->count++;
			compress->last_non_zero = 0;
		}
	}

	return compress->total_bits;
}

static int TryCompressFlush(TRY_COMPRESS_GOLOMB* compress)
{
	if(compress->count)
	{
		compress->total_bits += GetGammaBitLength(compress->count);
		compress->count = 0;
	}

	return compress->total_bits;
}

//---------------------------------------------------------------------------

void ApplyColorFilter(char * bufb, char * bufg, char * bufr, int len, int code)
{
	int d;
	unsigned char t;
	switch(code)
	{
	case 0:
		break;
	case 1:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			bufr[d] -= bufg[d];
			bufb[d] -= bufg[d];

			bufr[d+1] -= bufg[d+1];
			bufb[d+1] -= bufg[d+1];

			bufr[d+2] -= bufg[d+2];
			bufb[d+2] -= bufg[d+2];

			bufr[d+3] -= bufg[d+3];
			bufb[d+3] -= bufg[d+3];
		}

		len += 4;
		for( ; d<len; d++)
		{
			bufr[d] -= bufg[d];
			bufb[d] -= bufg[d];
		}

		break;
	case 2:
		for(d = 0; d < len; d++)
		{
			bufr[d] -= bufg[d],
			bufg[d] -= bufb[d];
		}

		break;
	case 3:
		for(d = 0; d < len; d++)
		{
			bufb[d] -= bufg[d],
			bufg[d] -= bufr[d];
		}

		break;
	case 4:
		for(d = 0; d < len; d++)
		{
			bufr[d] -= bufg[d],
			bufg[d] -= bufb[d],
			bufb[d] -= bufr[d];
		}

		break;
	case 5:
		for(d = 0; d < len; d++)
		{
			bufg[d] -= bufb[d],
			bufb[d] -= bufr[d];
		}

		break;
	case 6:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			bufb[d] -= bufg[d];
			bufb[d+1] -= bufg[d+1];
			bufb[d+2] -= bufg[d+2];
			bufb[d+3] -= bufg[d+3];
		}

		len += 4;
		for( ; d < len; d++)
		{
			bufb[d] -= bufg[d];
		}

		break;
	case 7:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			bufg[d] -= bufb[d];
			bufg[d+1] -= bufb[d+1];
			bufg[d+2] -= bufb[d+2];
			bufg[d+3] -= bufb[d+3];
		}

		len += 4;
		for( ; d < len; d++)
		{
			bufg[d] -= bufb[d];;
		}

		break;
	case 8:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			bufr[d] -= bufg[d];
			bufr[d+1] -= bufg[d+1];
			bufr[d+2] -= bufg[d+2];
			bufr[d+3] -= bufg[d+3];
		}

		len += 4;
		for( ; d < len; d++)
		{
			bufr[d] -= bufg[d];
		}

		break;
	case 9:
		for(d = 0; d < len; d++)
		{
			bufb[d] -= bufg[d],
			bufg[d] -= bufr[d],
			bufr[d] -= bufb[d];
		}

		break;
	case 10:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			bufg[d] -= bufr[d];
			bufb[d] -= bufr[d];

			bufg[d+1] -= bufr[d+1];
			bufb[d+1] -= bufr[d+1];

			bufg[d+2] -= bufr[d+2];
			bufb[d+2] -= bufr[d+2];

			bufg[d+3] -= bufr[d+3];
			bufb[d+3] -= bufr[d+3];
		}

		len += 4;
		for( ; d < len; d++)
		{
			bufg[d] -= bufr[d];
			bufb[d] -= bufr[d];
		}

		break;
	case 11:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			bufr[d+0] -= bufb[d+0];
			bufg[d+0] -= bufb[d+0];

			bufr[d+1] -= bufb[d+1];
			bufg[d+1] -= bufb[d+1];

			bufr[d+2] -= bufb[d+2];
			bufg[d+2] -= bufb[d+2];

			bufr[d+3] -= bufb[d+3];
			bufg[d+3] -= bufb[d+3];
		}

		len += 4;
		for( ; d < len; d++)
		{
			bufr[d] -= bufb[d];
			bufg[d] -= bufb[d];
		}

		break;
	case 12:
		for(d = 0; d < len; d++)
		{
			bufg[d] -= bufr[d],
			bufr[d] -= bufb[d];
		}

		break;
	case 13:
		for(d = 0; d < len; d++)
		{
			bufg[d] -= bufr[d],
			bufr[d] -= bufb[d],
			bufb[d] -= bufg[d];
		}

		break;
	case 14:
		for(d = 0; d < len; d++)
		{
			bufr[d] -= bufb[d],
			bufb[d] -= bufg[d],
			bufg[d] -= bufr[d];
		}

		break;
	case 15:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			t = bufb[d] << 1;
			bufr[d] -= t;
			bufg[d] -= t;

			t = bufb[d+1] << 1;
			bufr[d+1] -= t;
			bufg[d+1] -= t;

			t = bufb[d+2] << 1;
			bufr[d+2] -= t;
			bufg[d+2] -= t;

			t = bufb[d+3] << 1;
			bufr[d+3] -= t;
			bufg[d+3] -= t;
		}

		len += 4;
		for( ; d < len; d++)
		{
			t = bufb[d] << 1;
			bufr[d] -= t;
			bufg[d] -= t;
		}

		break;
	case 16:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			bufg[d] -= bufr[d];
			bufg[d+1] -= bufr[d+1];
			bufg[d+2] -= bufr[d+2];
			bufg[d+3] -= bufr[d+3];
		}

		len += 4;
		for( ; d < len; d++)
		{
			bufg[d] -= bufr[d];
		}

		break;
	case 17:
		for(d = 0; d < len; d++)
		{
			bufr[d] -= bufb[d],
			bufb[d] -= bufg[d];
		}

		break;
	case 18:
		for(d = 0; d < len; d++)
		{
			bufr[d] -= bufb[d];
		}

		break;
	case 19:
		for(d = 0; d < len; d++)
		{
			bufb[d] -= bufr[d],
			bufr[d] -= bufg[d];
		}

		break;
	case 20:
		for(d = 0; d < len; d++)
		{
			bufb[d] -= bufr[d];
		}

		break;
	case 21:
		for(d = 0; d < len; d++)
		{
			bufb[d] -= bufg[d]>>1;
		}

		break;
	case 22:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			bufg[d+0] -= bufb[d+0] >> 1;
			bufg[d+1] -= bufb[d+1] >> 1;
			bufg[d+2] -= bufb[d+2] >> 1;
			bufg[d+3] -= bufb[d+3] >> 1;
		}

		len += 4;
		for( ; d < len; d++)
		{
			bufg[d] -= bufb[d] >> 1;
		}

		break;
	case 23:
		for(d = 0; d < len; d++)
		{
			bufg[d] -= bufb[d],
			bufb[d] -= bufr[d],
			bufr[d] -= bufg[d];
		}

		break;
	case 24:
		for(d = 0; d < len; d++)
		{
			bufb[d] -= bufr[d],
			bufr[d] -= bufg[d],
			bufg[d] -= bufb[d];
		}

		break;
	case 25:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			bufg[d] -= bufr[d] >> 1;
			bufg[d+1] -= bufr[d+1] >> 1;
			bufg[d+2] -= bufr[d+2] >> 1;
			bufg[d+3] -= bufr[d+3] >> 1;
		}

		len += 4;
		for( ; d < len; d++)
		{
			bufg[d] -= bufr[d] >> 1;
		}

		break;
	case 26:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			bufr[d+0] -= bufg[d+0] >> 1;
			bufr[d+1] -= bufg[d+1] >> 1;
			bufr[d+2] -= bufg[d+2] >> 1;
			bufr[d+3] -= bufg[d+3] >> 1;
		}

		len += 4;
		for( ; d < len; d++)
		{
			bufr[d] -= bufg[d] >> 1;
		}

		break;
	case 27:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			t = bufr[d] >> 1;
			bufg[d] -= t;
			bufb[d] -= t;
			t = bufr[d+1] >> 1;
			bufg[d+1] -= t;
			bufb[d+1] -= t;
			t = bufr[d+2] >> 1;
			bufg[d+2] -= t;
			bufb[d+2] -= t;
			t = bufr[d+3] >> 1;
			bufg[d+3] -= t;
			bufb[d+3] -= t;
		}

		len += 4;
		for( ; d < len; d++)
		{
			t = bufr[d] >> 1;
			bufg[d] -= t;
			bufb[d] -= t;
		}

		break;
	case 28:
		for(d = 0; d < len; d++)
		{
			bufr[d] -= bufb[d]>>1;
		}

		break;
	case 29:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			t = bufg[d] >> 1;
			bufr[d] -= t;
			bufb[d] -= t;
			t = bufg[d+1] >> 1;
			bufr[d+1] -= t;
			bufb[d+1] -= t;
			t = bufg[d+2] >> 1;
			bufr[d+2] -= t;
			bufb[d+2] -= t;
			t = bufg[d+3] >> 1;
			bufr[d+3] -= t;
			bufb[d+3] -= t;
		}

		len += 4;
		for( ; d < len; d++)
		{
			t = bufg[d] >> 1;
			bufr[d] -= t;
			bufb[d] -= t;
		}

		break;
	case 30:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			t = bufb[d] >> 1;
			bufr[d] -= t;
			bufg[d] -= t;
			t = bufb[d+1] >> 1;
			bufr[d+1] -= t;
			bufg[d+1] -= t;
			t = bufb[d+2] >> 1;
			bufr[d+2] -= t;
			bufg[d+2] -= t;
			t = bufb[d+3] >> 1;
			bufr[d+3] -= t;
			bufg[d+3] -= t;
		}

		len += 4;
		for( ; d < len; d++)
		{
			t = bufb[d] >> 1;
			bufr[d] -= t;
			bufg[d] -= t;
		}

		break;
	case 31:
		for(d = 0; d < len; d++)
		{
			bufb[d] -= bufr[d]>>1;
		}

		break;
	case 32:
		for(d = 0; d < len; d++)
		{
			bufr[d] -= bufb[d]<<1;
		}

		break;
	case 33:
		for(d = 0; d < len; d++)
		{
			bufb[d] -= bufg[d]<<1;
		}

		break;
	case 34:
		len -= 4;
		for(d=0; d<len; d+=4)
		{
			t = bufr[d] << 1;
			bufg[d] -= t;
			bufb[d] -= t;
			t = bufr[d+1] << 1;
			bufg[d+1] -= t;
			bufb[d+1] -= t;
			t = bufr[d+2] << 1;
			bufg[d+2] -= t;
			bufb[d+2] -= t;
			t = bufr[d+3] << 1;
			bufg[d+3] -= t;
			bufb[d+3] -= t;
		}

		len += 4;
		for( ; d < len; d++)
		{
			t = bufr[d] << 1;
			bufg[d] -= t;
			bufb[d] -= t;
		}

		break;
	case 35:
		for(d = 0; d < len; d++)
		{
			bufg[d] -= bufb[d]<<1;
		}

		break;
	case 36:
		for(d = 0; d < len; d++)
		{
			bufr[d] -= bufg[d]<<1;
		}

		break;
	case 37:
		for(d = 0; d < len; d++)
		{
			bufr[d] -= bufg[d]<<1,
			bufb[d] -= bufg[d]<<1;
		}

		break;
	case 38:
		for(d = 0; d < len; d++)
		{
			bufg[d] -= bufr[d]<<1;
		}

		break;
	case 39:
		for(d = 0; d < len; d++)
		{
			bufb[d] -= bufr[d]<<1;
		}

		break;

	}

}
//---------------------------------------------------------------------------
int DetectColorFilter(GOLOMB_TABLE* table, char *b, char *g, char *r, int size, int *outsize)
{
#ifndef FILTER_TEST
	int minbits = -1;
	int mincode = -1;
	int code;

	char bbuf[H_BLOCK_SIZE*W_BLOCK_SIZE];
	char gbuf[H_BLOCK_SIZE*W_BLOCK_SIZE];
	char rbuf[H_BLOCK_SIZE*W_BLOCK_SIZE];
	TRY_COMPRESS_GOLOMB bc, gc, rc;
	// TryCompressGolomb bc, gc, rc;

	for(code = 0; code < FILTER_TRY_COUNT; code++)   // 17..27 are currently not used
	{
		int bits;

		// copy bbuf, gbuf, rbuf into b, g, r.
		memcpy(bbuf, b, sizeof(char)*size);
		memcpy(gbuf, g, sizeof(char)*size);
		memcpy(rbuf, r, sizeof(char)*size);

		// copy compressor
		InitTryCompressGolomb(&bc, table);
		InitTryCompressGolomb(&gc, table);
		InitTryCompressGolomb(&rc, table);

		// Apply color filter
		ApplyColorFilter(bbuf, gbuf, rbuf, size, code);

		// try to compress
		bits  = (TryCompress(&bc, bbuf, size), TryCompressFlush(&bc));
		if(minbits != -1 && minbits < bits) continue;
		bits += (TryCompress(&gc, gbuf, size), TryCompressFlush(&gc));
		if(minbits != -1 && minbits < bits) continue;
		bits += (TryCompress(&rc, rbuf, size), TryCompressFlush(&rc));

		if(minbits == -1 || minbits > bits)
		{
			minbits = bits, mincode = code;
		}
	}

	*outsize = minbits;

	return mincode;
#else
	static int filter = 0;

	int f = filter;

	filter++;
	if(filter >= FILTER_TRY_COUNT) filter = 0;

	outsize = 0;

	return f;
#endif
}

//---------------------------------------------------------------------------
static void TLG6InitializeColorFilterCompressor(SLIDE_COMPRESSOR *c)
{
	unsigned char code[4096];
	unsigned char dum[4096];
	unsigned char *p = code;
	long dumlen;
	int i, j;

	for(i = 0; i < 32; i++)
	{
		for(j = 0; j < 16; j++)
		{
			p[0] = p[1] = p[2] = p[3] = i;
			p += 4;
			p[0] = p[1] = p[2] = p[3] = j;
			p += 4;
		}
	}

	SlideEncode(c, code, 4096, dum, &dumlen);
}

//---------------------------------------------------------------------------
// int ftfreq[256] = {0};
void TLG6Encode(
	unsigned char* pixels,
	int width,
	int height,
	int colors,
	void *out,
	size_t (*write_func)(const void*, size_t, size_t, void*)
)
{
#ifdef WRITE_ENTROPY_VALUES
	FILE *vs = fopen("vs.bin", "wb");
#endif

	long max_bit_length = 0;

	unsigned char *buf[MAX_COLOR_COMPONENTS];
	char *block_buf[MAX_COLOR_COMPONENTS];
	unsigned char *filtertypes = NULL;
	// TMemoryStream *memstream = NULL;
	TLG6BIT_STREAM* bs = NULL;
	int i;
	// TVPTLG6InitGolombTable();

	// check pixel format
	if(!(colors == 1 || colors == 3 || colors == 4))
	{
		(void)fprintf(stderr, "No supported color.\n");

		return;
	}

	// output stream header
	{
		const char* write_string = "TLG6.0\x00raw\x1a\x00";
		unsigned char n = colors;
		(void)write_func(write_string, 1, 11, out);
		(void)write_func(&n, sizeof(n), 1, out);

		n = 0;
		(void)write_func(&n, sizeof(n), 1, out); // data flag (0)
		(void)write_func(&n, sizeof(n), 1, out); // color type (0)
		(void)write_func(&n, sizeof(n), 1, out); // external golomb table (0)

		(void)write_func(&width, sizeof(width), 1, out);
		(void)write_func(&height, sizeof(height), 1, out);
	}


/*
	// Near lossless filter
	if(colors == 3) bmp->PixelFormat = pf32bit;
	NearLosslessFilter(bmp);
	if(colors == 3) bmp->PixelFormat = pf24bit;
*/

	// compress
	for(i = 0; i < MAX_COLOR_COMPONENTS; i++)
	{
		buf[i] = NULL;
	}
	for(i = 0; i < MAX_COLOR_COMPONENTS; i++)
	{
		block_buf[i] = NULL;
	}

	{
		int w_block_count;
		int h_block_count;
		int x, y;
		int fc = 0, c;
		// memstream = new TMemoryStream();

		bs = CreateTLG6BIT_STREAM();
		// TLG6BitStream bs(memstream);

//		int buf_size = bmp->Width * bmp->Height;

		// allocate buffer
		for(c = 0; c < colors; c++)
		{
			buf[c] = (unsigned char*)MEM_ALLOC_FUNC(W_BLOCK_SIZE * H_BLOCK_SIZE * 3);
			block_buf[c] = (char*)MEM_ALLOC_FUNC(H_BLOCK_SIZE * width);
		}
		w_block_count = (int)((width - 1) / W_BLOCK_SIZE) + 1;
		h_block_count = (int)((height - 1) / H_BLOCK_SIZE) + 1;
		filtertypes = (unsigned char*)MEM_ALLOC_FUNC(w_block_count * h_block_count);

		for(y = 0; y < height; y += H_BLOCK_SIZE)
		{
			int ylim = y + H_BLOCK_SIZE;
			int gwp = 0, xp = 0, p;

			if(ylim > height)
			{
				ylim = height;
			}

			for(x = 0; x < width; x += W_BLOCK_SIZE, xp++)
			{
				int xlim = x + W_BLOCK_SIZE;
				int bw;
				int p0size; // size of MED method (p=0)
				int minp = 0; // most efficient method (0:MED, 1:AVG)
				int ft; // filter type
				int wp; // write point

				if(xlim > width)
				{
					xlim = width;
				}

				bw = xlim - x;

				for(p = 0; p < 2; p++)
				{
					int dbofs = (p+1) * (H_BLOCK_SIZE * W_BLOCK_SIZE);
					int yy;

					// do med(when p=0) or take average of upper and left pixel(p=1)
					for(c = 0; c < colors; c++)
					{
						int wp = 0;
						for(yy = y; yy < ylim; yy++)
						{
							const unsigned char * sl = x*colors +
								c + (const unsigned char *)&pixels[yy*width*colors];
								// c + (const unsigned char *)bmp->ScanLine[yy];
							const unsigned char * usl;
							int xx;

							if(yy >= 1)
								usl = x*colors + c + (const unsigned char *)&pixels[(yy-1)*width*colors];
								// usl = x*colors + c + (const unsigned char *)bmp->ScanLine[yy-1];
							else
								usl = NULL;
							for(xx = x; xx < xlim; xx++)
							{
								unsigned char pa = xx > 0 ? sl[-colors] : 0;
								unsigned char pb = usl ? *usl : 0;
								unsigned char px = *sl;

								unsigned char py;

//								py = 0;
								if(p == 0)
								{
									unsigned char pc = (xx > 0 && usl) ? usl[-colors] : 0;
									unsigned char min_a_b = pa>pb?pb:pa;
									unsigned char max_a_b = pa<pb?pb:pa;

									if(pc >= max_a_b)
										py = min_a_b;
									else if(pc < min_a_b)
										py = max_a_b;
									else
										py = pa + pb - pc;
								}
								else
								{
									py = (pa+pb+1)>>1;
								}
								
								buf[c][wp] = (unsigned char)(px - py);

								wp++;
								sl += colors;
								if(usl) usl += colors;
							}
						}
					}

					// reordering
					// Transfer the data into block_buf (block buffer).
					// Even lines are stored forward (left to right),
					// Odd lines are stored backward (right to left).

					wp = 0;
					for(yy = y; yy < ylim; yy++)
					{
						int ofs, xx;
						int dir; // false for forward, true for backward

						if(!(xp&1))
							ofs = (yy - y)*bw;
						else
							ofs = (ylim - yy - 1) * bw;
						if(!((ylim-y)&1))
						{
							// vertical line count per block is even
							dir = (yy&1) ^ (xp&1);
						}
						else
						{
							// otherwise;
							if((xp & 1) != 0)
							{
								dir = (yy&1);
							}
							else
							{
								dir = (yy&1) ^ (xp&1);
							}
						}

						if(dir == 0)
						{
							// forward
							for(xx = 0; xx < bw; xx++)
							{
								for(c = 0; c < colors; c++)
								{
									buf[c][wp + dbofs] =
										buf[c][ofs + xx];
								}
								wp++;
							}
						}
						else
						{
							// backward
							for(xx = bw - 1; xx >= 0; xx--)
							{
								for( c = 0; c < colors; c++)
								{
									buf[c][wp + dbofs] =
										buf[c][ofs + xx];
								}
								wp++;
							}
						}
					}
				}


				for(p = 0; p < 2; p++)
				{
					int dbofs = (p+1) * (H_BLOCK_SIZE * W_BLOCK_SIZE);
					// detect color filter
					int size = 0;
					int ft_;

					if(colors >= 3)
						ft_ = DetectColorFilter(
							&bs->table,
							(char*)(buf[0] + dbofs),
							(char*)(buf[1] + dbofs),
							(char*)(buf[2] + dbofs), wp, &size);
					else
						ft_ = 0;

					// select efficient mode of p (MED or average)
					if(p == 0)
					{
						p0size = size;
						ft = ft_;
					}
					else
					{
						if(p0size >= size)
							minp = 1, ft = ft_;
					}
				}

				// Apply most efficient color filter / prediction method
				{
					int dbofs = (minp + 1)  * (H_BLOCK_SIZE * W_BLOCK_SIZE);
					int xx, yy;
					wp = 0;
					for(yy = y; yy < ylim; yy++)
					{
						for(xx = 0; xx < bw; xx++)
						{
							for(c = 0; c < colors; c++)
								block_buf[c][gwp + wp] = buf[c][wp + dbofs];
							wp++;
						}
					}
				}

				ApplyColorFilter(block_buf[0] + gwp,
					block_buf[1] + gwp, block_buf[2] + gwp, wp, ft);

				filtertypes[fc++] = (ft<<1) + minp;
//				ftfreq[ft]++;
				gwp += wp;
			}

			// compress values (entropy coding)
			for(c = 0; c < colors; c++)
			{
				int method;
				long bitlength;
				CompressValuesGolomb(bs, block_buf[c], gwp);
				method = 0;
#ifdef WRITE_ENTROPY_VALUES
				fwrite(block_buf[c], 1, gwp, vs);
#endif
				bitlength = bs->buffer_byte_pos * 8 + bs->buffer_bit_pos;
				if((bitlength & 0xc0000000) != 0)
				{
					(void)fprintf(stderr, "SaveTLG6: Too large bit length (given image may be too large)");
					return;
				}

				// two most significant bits of bitlength are
				// entropy coding method;
				// 00 means Golomb method,
				// 01 means Gamma method (implemented but not used),
				// 10 means modified LZSS method (not yet implemented),
				// 11 means raw (uncompressed) data (not yet implemented).
				if(max_bit_length < bitlength) max_bit_length = bitlength;
				bitlength |= (method << 30);
				(void)MemWrite(&bitlength, sizeof(bitlength), 1, bs->out_stream);
				TLG6BitStreamFlush(bs);
			}

		}


		// write max bit length
		(void)write_func(&max_bit_length, sizeof(max_bit_length), 1, out);
		// WriteInt32(max_bit_length, out);

		// output filter types
		{
			SLIDE_COMPRESSOR *comp = CreateSlideCompressor();
			unsigned char* outbuf = (unsigned char*)MEM_ALLOC_FUNC(fc * 2);
			long outlen;

			TLG6InitializeColorFilterCompressor(comp);
			SlideEncode(comp, filtertypes, fc, outbuf, &outlen);
			(void)write_func(&outlen, sizeof(outlen), 1, out);
			(void)write_func(outbuf, 1, outlen, out);

			DeleteSlideCompressor(&comp);

/*
			FILE *f = fopen("ft.txt", "wt");
			int n = 0;
			for(int y = 0; y < h_block_count; y++)
			{
				for(int x = 0; x < w_block_count; x++)
				{
					int t = filtertypes[n++];
					char b;
					if(t & 1) b = 'A'; else b = 'M';
					t >>= 1;
					fprintf(f, "%c%x", b, t);
				}
				fprintf(f, "\n");
			}
			fclose(f);
*/
		}

		// copy memory stream to output stream
		(void)write_func(bs->out_stream->buff_ptr, 1, bs->out_stream->data_point, out);
	}

	for(i = 0; i < MAX_COLOR_COMPONENTS; i++)
	{
		if(buf[i] != NULL) MEM_FREE_FUNC(buf[i]);
		if(block_buf != NULL) MEM_FREE_FUNC(block_buf[i]);
	}
	if(filtertypes != NULL) MEM_FREE_FUNC(filtertypes);
	if(bs != NULL)
	{
		DeleteTLG6BIT_STREAM(&bs);
	}

#ifdef WRITE_ENTROPY_VALUES
	fclose(vs);
#endif
//	printf("med : %d ms\n", medend - medstart);
//	printf("reorder : %d ms\n", reordertick);
/*
	for(int i = 0; i < 256; i++)
	{
		if(ftfreq[i])
		{
			printf("%3d : %d\n", i, ftfreq[i]);
		}
	}
*/
}
//---------------------------------------------------------------------------
