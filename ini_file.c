// Visual Studio 2005�ȍ~�ł͌Â��Ƃ����֐����g�p����̂�
	// �x�����o�Ȃ��悤�ɂ���
#if defined _MSC_VER && _MSC_VER >= 1400
# define _CRT_SECURE_NO_DEPRECATE
# define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <stdio.h>
#include "memory.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "ini_file.h"
#include "memory.h"

#ifdef __cplusplus
extern "C" {
#endif

// �֐��̃v���g�^�C�v�錾
static void Delete_INI_FILE(INI_FILE_PTR ini);

/*********************************************
* Trim�֐�                                   *
* ������̑O��̃X�y�[�X�A�^�u���X�L�b�v���� *
* ����                                       *
* buff	: ����������������                   *
*********************************************/
static void Trim(char* buff)
{
	char* p = buff;	// �X�y�[�X�A�^�u�̒T���p
	char* r = buff + strlen((const char*)buff) - 1;	// �V����������̍Ō�Ƀk�������ǉ��p

	// �O��̃X�y�[�X���������|�C���^���擾
	for( ; (*p == ' ' || *p == '\t') && *p != '\0'; p++)
	{
	}

	for( ; r > p && (*r == ' ' || *r == '\t'); r--)
	{
	}

	*(r+1) = '\0';	// ������̍Ō�Ƀk��������ǉ�

	// buff�̒��g���X�V����
	(void)strcpy(buff, p);
}

/*******************************************
* Str2Hash�֐�                             *
* ������̃n�b�V���l���擾����             *
* ����                                     *
* string	: �n�b�V���l���擾������������ *
* �Ԃ�l                                   *
*	�n�b�V���l                             *
*******************************************/
static int Str2Hash(const char* string)
{
	int hash = 0;	// �Ԃ�l

	for( ; *string != '\0'; string++)
	{
		if(*string != ' ')
		{
			hash = ((hash << 1) + tolower(*string));
		}
	}

	return hash;
}

/********************************************************
* SectionAdd�֐�                                        *
* �Z�N�V������ǉ�����                                  *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* section_name	: �ǉ�����Z�N�V�����̖��O              *
* �Ԃ�l                                                *
*	����I��(0)�A�ُ�I��(0�ȊO)                        *
********************************************************/
static int SectionAdd(INI_FILE_PTR ini, const char* section_name)
{
	SECTION_PTR tmp_section;

	if(section_name == NULL || *section_name == '\0')
	{
		return -1;
	}

	// �V���Ƀ��������K�v���ǂ���
	if(ini->section_size < ini->section_count + 1)
	{
		// �Ċm��
		ini->section_size += INI_ALLOC_SIZE;
		tmp_section = (SECTION_PTR)MEM_ALLOC_FUNC(sizeof(SECTION)*ini->section_size);

		// �������m�ې����̃`�F�b�N
#ifdef _DEBUG
		if(tmp_section == NULL)
		{
			(void)printf("Memory allocate error.\n(In SectionAdd)\n");

			return -2;
		}
#endif
		(void)memset(tmp_section, 0, sizeof(SECTION)*ini->section_size);

		if(ini->section != NULL)
		{
			(void)memcpy(tmp_section, ini->section, sizeof(SECTION)*ini->section_count);
			MEM_FREE_FUNC(ini->section);
		}

		ini->section = tmp_section;
	}	// if(ini->section_size < section_count + 1)

	// �Z�N�V������ǉ�
	(void)memset((ini->section + ini->section_count), 0, sizeof(SECTION));
	(void)strcpy((ini->section + ini->section_count)->section_name, section_name);
	Trim((ini->section + ini->section_count)->section_name);
	(ini->section + ini->section_count)->hash =	// �n�b�V���l���v�Z
		Str2Hash((ini->section + ini->section_count)->section_name);

	ini->section_count++;

	return 0;
}

/********************************************************
* SectionFind�֐�                                       *
* �Z�N�V��������������                                  *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* section_name	: ��������Z�N�V�����̖��O              *
* �Ԃ�l                                                *
*	����I��(�Z�N�V������ID)�A�ُ�I��(���̒l)          *
********************************************************/
static int SectionFind(INI_FILE_PTR ini, const char *section_name)
{
	int hash;	// ��������Z�N�V�����̃n�b�V���l
	int i;	// for���p�̃J�E���^

	// ������A�������[�h��NULL��������A�������[�h���Ȃ�������
		// �G���[�ŏI��
	if(ini->section == NULL || section_name == NULL
		|| *section_name == '\0')
	{
		return -1;
	}

	// ��������Z�N�V�����̃n�b�V���l�����߂�
	hash = Str2Hash(section_name);

	for(i=0; i<ini->section_count; i++)
	{
		// �Z�N�V�����̃n�b�V���l�����߂���̂Ɠ�����?
		if((ini->section + i)->hash != hash)
		{	// ��������蒼��
			continue;
		}

		// �O�̂��ߓ��������񂩂ǂ������m���߂Ă���
		if(strcmp((ini->section + i)->section_name, section_name) == 0)
		{	// �����Ȃ�ID(i�̒l)��Ԃ�
			return i;
		}
	}	// for(i=0; i<ini->section_count; i++)

	// �����܂ł��Ă��܂�����G���[
	return -2;
}

/*********************************************************************
* KeyAdd�֐�                                                         *
* �L�[��ǉ�                                                         *
* ����                                                               *
* section		: �L�[��ǉ�����Z�N�V�������Ǘ�����\���̂̃A�h���X *
* key_name		: �L�[�̖��O                                         *
* string		: �L�[�ɒǉ����镶����                               *
* comment_flag	: �R�����g���ǂ����̃t���O                           *
* �Ԃ�l                                                             *
*	����I��(0)�A�ُ�I��(-1)                                        *
*********************************************************************/
static int KeyAdd(SECTION_PTR section, const char* key_name, char* string,
			char comment_flag)
{
	KEY_PTR tmp_key;	// �V���ɒǉ�����L�[�̃������̈�
	int index = -1;		// �ǉ����ꂽ�L�[�̃C���f�b�N�X

	// �ǉ����镶����NULL�������蕶���񂪂Ȃ�������G���[
	if(key_name == NULL || *key_name == '\0' || string == NULL)
	{
		return -1;
	}

	// �������̍Ċm�ۂ��K�v���ǂ���
	if(section->key_size < section->key_count + 1)
	{
		// �Ċm��
		section->key_size += INI_ALLOC_SIZE;
		tmp_key = (KEY_PTR)MEM_ALLOC_FUNC(sizeof(KEY)*section->key_size);
		// �������m�ې����̃`�F�b�N
#ifdef _DEBUG
		if(tmp_key == NULL)
		{
			(void)printf("Memory allocate error.\n(In Key_Add)\n");

			return -2;
		}
#endif
		if(section->key != NULL)
		{
			(void)memcpy(tmp_key, section->key, sizeof(KEY)*section->key_count);
			MEM_FREE_FUNC(section->key);
		}
		section->key = tmp_key;
	}

	// �L�[��ǉ�
	// ��������R�s�[
	(void)strcpy((section->key + section->key_count)->key_name, key_name);
	Trim((section->key + section->key_count)->key_name);

	// �R�����g�łȂ���΃n�b�V���l�����߂�
	if(comment_flag == COMMENT_OFF)
	{
		(section->key + section->key_count)->hash =
			Str2Hash((section->key + section->key_count)->key_name);
	}

	// �L�[�̕�������쐬����
	(section->key + section->key_count)->string =
		MEM_STRDUP_FUNC(string);

	// �������m�ې����̃`�F�b�N
#ifdef _DEBUG
	if((section->key + section->key_count)->string == NULL)
	{
		(void)printf("Memory allocate error.\n(In Key_Add)\n");

		return -3;
	}
#endif
	// �R�����g���ǂ����̃t���O��ݒ�
	(section->key + section->key_count)->comment_flag = comment_flag;

	section->key_count++;	// �Z�N�V�����������L�[�̐����X�V����
	return 0;
}

/*************************************************************
* KeyFind�֐�                                                *
* �L�[������                                                 *
* ����                                                       *
* section	: �L�[��{���Z�N�V�������Ǘ�����\���̂̃A�h���X *
* key_name	: ��������L�[�̖��O                             *
* �Ԃ�l                                                     *
*	����I��(�L�[��ID)�A�ُ�I��(���̒l)                     *
*************************************************************/
static int KeyFind(const SECTION_PTR section, const char* key_name)
{
	int hash;	// �������镶����̃n�b�V���l
	int i;		// for���p�̃J�E���^���Ԃ�l

	// �������镶����̃|�C���^��NULL��������A
		// �����񂪂Ȃ�������G���[
	if(section->key == NULL || key_name == NULL
		|| *key_name == '\0')
	{
		return -1;
	}

	// �n�b�V���l�����߂�
	hash = Str2Hash(key_name);

	// �L�[������
	for(i=0; i<section->key_count; i++)
	{
		// �L�[���R�����g�ł�������A�n�b�V���l������Ă������蒼��
		if((section->key + i)->comment_flag == COMMENT_ON
			|| (section->key + i)->hash != hash)
		{
			continue;
		}

		// �O�̂��ߕ�������r���Ă���
		if(strcmp((section->key + i)->key_name, key_name) == 0)
		{	// ��v������ID��Ԃ�
			return i;
		}
	}	// for(i=0; i<section->key_count; i++)

	return -2;	// �L�[��������Ȃ�����
}

/************************************************
* CreateIniFile�֐�                             *
* ini�t�@�C���������\���̂̃������̊m�ۂƏ����� *
* ����                                          *
* io		: ini�t�@�C���f�[�^�ւ̃|�C���^     *
* read_func	: �ǂݍ��ݗp�̊֐�                  *
* data_size	: ini�t�@�C���̃o�C�g��             *
* mode		: �ǂݍ��݃��[�h���������݃��[�h��  *
* �Ԃ�l                                        *
*	���������ꂽ�\���̂̃A�h���X                *
************************************************/
INI_FILE_PTR CreateIniFile(
	void* io,
	size_t (*read_func)(void*, size_t, size_t, void*),
	size_t data_size,
	eINI_MODE mode
)
{
	INI_FILE_PTR ret;	// �Ԃ�l
	char tmp[KEY_BUFF_SIZE];	// �ꎞ�I�ɕ�������i�[
	char* buff;		// ini�t�@�C���̓��e���i�[

	// ���������m�ۂ���
	ret = (INI_FILE_PTR)MEM_ALLOC_FUNC(sizeof(INI_FILE));

	// �������m�ې����̃`�F�b�N
#ifdef _DEBUG
	if(ret == NULL)
	{
		(void)printf("Memory allocate error.\n(In CreateIniFile_STRUCT)\n");

		return NULL;
	}
#endif

	// �������J���p�̊֐��|�C���^��ݒ�
	ret->delete_func = Delete_INI_FILE;

	// �������݃��[�h��?
	if(mode == INI_WRITE)
	{	// �������݃��[�h�Ȃ�t�@�C�����J���A
			// �Z�N�V�����̏������������ďI��
		ret->io = io;
		ret->section = NULL;

		return ret;
	}

	// �ǂݎ��̈�̊m��
	buff = (char*)MEM_ALLOC_FUNC(sizeof(char)*(data_size+1));

	// �������m�ې����̃`�F�b�N
#ifdef _DEBUG
	if(buff == NULL)
	{
		(void)printf("Memory allocate error.\n(In CreateIniFile_STRUCT)\n");

		MEM_FREE_FUNC(ret);

		return NULL;
	}
#endif

	// �t�@�C����ǂݍ���
	ret->io = io;
	(void)read_func(buff, sizeof(char), data_size, ret->io);

	// �o�b�t�@�̍Ō�Ƀk��������ǉ�����
	buff[data_size] = '\0';

	// �Z�N�V�����̊m��
	ret->section = (SECTION_PTR)MEM_ALLOC_FUNC(sizeof(SECTION)*INI_ALLOC_SIZE);

	// �������m�ې����̃`�F�b�N
#ifdef _DEBUG
	if(ret->section == NULL)
	{
		(void)printf("Memory allocate error.\n(In CreateIniFile_STRUCT)\n");

		MEM_FREE_FUNC(buff);
		MEM_FREE_FUNC(ret);

		return NULL;
	}
#endif

	// �Z�N�V�����̏��̏�����
	(void)memset(ret->section, 0, sizeof(SECTION)*INI_ALLOC_SIZE);
	ret->section_count = 1;
	ret->section_size = INI_ALLOC_SIZE;

	// �t�@�C���̓��e�𒲂ׂ�u���b�N
	{
		char *p, *r, *s;

		p = buff;

		// �t�@�C���̏I�[�܂ł��`�F�b�N
		while((data_size > (size_t)(p - buff)) && *p != '\0')
		{
			// r�����s�̎��̉��s�Ɉړ�����
			for(r = p; (data_size > (size_t)(r - buff) && (*r != '\r') && *r != '\n'); r++)
			{
				// �X�y�[�X���A�^�u�̂��Ƃ̉��s�Ȃ炻�̂܂ܓǂݍ��ݑ�����
				if((*r == ' ' || *r == '\t') && (*(r + 1) == '\n' || *(r + 1) == '\r'))
				{
					if(*(r + 1) == '\r')
					{
						r++;
					}
					r++;
				}
			}

			// p�ɂ��镶���̓��e�ŏ�����؂�ւ�
			switch(*p)
			{
			case '[':	// �u[�v�Ȃ�
				// �Z�N�V������ǉ�����
				if(p == r || *(r-1) != ']')
				{	// �u[�v������ꂽ��I��
					break;
				}
				*(r-1) = '\0';
				SectionAdd(ret, p+1);
				break;
			case '\r':
			case '\n':
				// ���s�Ȃ�Ȃɂ����Ȃ�
				break;

			default:	// ����ȊO�Ȃ�L�[���R�����g
				if(ret->section == NULL || p == r)
				{	// �Z�N�V�������Ċm�ۂł��Ȃ�������A���s�܂ł������Ă�����I��
					break;
				}
				
				if(*p == '#')
				{	// �R�����g
					for(s = tmp; p < r; p++, s++)
					{	// ���e�̓ǂݍ���
						*s = *p;
					}
					*s = '\0';	// �Ō�Ƀk��������ǉ�
					// �R�����g�̃t���O��ON�ɂ��ăL�[��ǉ�
					KeyAdd((ret->section + ret->section_count - 1), tmp, "", COMMENT_ON);
				}	// if(*p == '#')
				else
				{
					// �L�[��ǉ�����
					for(s = tmp; p < r; p++, s++)
					{
						if(*p == '=')
						{	// �u=�v��������I��
							break;
						}
						*s = *p;
					}
					*s = '\0';	// ������̍Ō�Ƀk��������ǉ�

					if(*p == '=')
					{	// p�̎w�����e���u=�v�Ȃ�
						p++;	// ���̒l��T��
					}
					*r = '\0';	// �Ō�Ƀk��������ǉ�
					KeyAdd((ret->section + ret->section_count - 1), tmp, p, COMMENT_OFF);
				}	// if(*p == '#') else

				if(data_size > (size_t)(r - buff))
				{	// �t�@�C���̏I�[�܂ŒB���Ă��Ȃ������玟�̍s��
					r++;
				}
			}	// switch(*p)

			// ���̕�����p���ړ�����
			p = r;

			for( ; (data_size > (size_t)(p - buff)) && (*p == '\r'
				|| *p == '\n'); p++)
			{
			}
		}	// while((data_size > (p - buff)) && *p != '\0')
	}

	MEM_FREE_FUNC(buff);
	return ret;
}

/********************************************************
* WriteIniFile�֐�                                      *
* ini�t�@�C���ɓ��e�������o��                           *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* write_func	: �����o���p�֐��ւ̃|�C���^            *
* �Ԃ�l                                                *
*	����I��(0)�A�ȏ�I��(0�ȊO)                        *
********************************************************/
int WriteIniFile(
	INI_FILE_PTR ini,
	size_t (*write_func)(void*, size_t, size_t, void*)
)
{
	char *buff;	// �����o�����e���i�[����o�b�t�@
	char *p;	// �������݂̕�����r�p
	int len;	// �����o���T�C�Y
	int i, j;	// for���p�̃J�E���^

	// �Z�N�V�����̏�񂪂Ȃ���΃G���[
	if(ini->section == NULL)
	{
		return -1;
	}

	// �ۑ��T�C�Y���v�Z����
	len = 0;
	for(i=0; i<ini->section_count; i++)
	{
		// �Z�N�V�������̒����Ɓu[�v�u]�v�𑫂�
		len += (int)strlen((ini->section + i)->section_name) + 4;
		// �L�[�̏�񂪂Ȃ��Ȃ�����X�L�b�v
		if((ini->section + i)->key == NULL)
		{
			continue;
		}

		for(j=0; j<(ini->section + i)->key_count; j++)
		{
			// �L�[�̖��O�����������񂪂Ȃ���΃X�L�b�v
			if(*((ini->section + i)->key + j)->key_name == '\0')
			{
				continue;
			}

			// �L�[��
			len += (int)strlen(((ini->section + i)->key + j)->key_name);
			// �L�[���R�����g�łȂ��Ȃ當����̒����𑫂�
			if(((ini->section + i)->key + j)->comment_flag == COMMENT_OFF)
			{
				len++;
				// �����񂪑��݂��邩?
				if(((ini->section + i)->key + j)->string != NULL)
				{
					// �T�C�Y�ɒǉ�
					len += (int)strlen(((ini->section + i)->key + j)->string);
				}
			}	// if(((ini->section + i)->key + j)->comment_flag == COMMENT_OFF)
			len += 2;	// �u[�v�Ɓu]�v�̕��̃T�C�Y�𑫂�
		}	// for(j=0; j<(ini->section + i)->key_count; j++)
		len += 2;	// �u[�v�Ɓu]�v�̕��̃T�C�Y�𑫂�
	}	// for(i=0; i<ini->section_count; i++)

	// �o�b�t�@�̃������̊m��
	p = buff = (char*)MEM_ALLOC_FUNC(sizeof(char)*len*2);

	// �������m�ۂ̃`�F�b�N
#ifdef _DEBUG
	if(p == NULL)
	{
		(void)printf("Memory allocate error.\n(In Write_Ini_File)\n");

		return -2;
	}
#endif

	// �ۑ�������̍쐬
	for(i=0; i<ini->section_count; i++)
	{
		// �Z�N�V�����ɃL�[�̏�񂪂Ȃ�����X�L�b�v
		if((ini->section + i)->key == NULL)
		{
			continue;
		}

		// �Z�N�V���������쐬
		if(i != 0)
		{
			*(p++) = '[';	// �u[�v��ǉ�
			(void)strcpy(p, (ini->section + i)->section_name);
			p += strlen(p);	// ������̒�����p�̈ʒu�����߂�
			*(p++) = ']';	// �u]�v��ǉ�
			//*(p++) = '\r';	// �L�����b�W���^�[����ǉ�
			*(p++) = '\n';	// ���s��ǉ�
		}	// if(i != 0)

		// �L�[�̕������ǉ�
		for(j=0; j<(ini->section + i)->key_count; j++)
		{
			// �L�[�̕����񂪑��݂��邩�𔻒f
			if(*((ini->section + i)->key + j)->key_name == '\0')
			{
				continue;
			}

			(void)strcpy(p, ((ini->section + i)->key + j)->key_name);
			p += strlen(p);	// ������̒�����p�̈ʒu�����߂�
			// �R�����g�łȂ��Ȃ�Ώ���ǉ�
			if(((ini->section + i)->key + j)->comment_flag == COMMENT_OFF)
			{
				*(p++) = '=';	// �u=�v��ǉ�
				// �����񂪂��邩�ǂ����𔻒f���Ă���ǉ�����
				if(((ini->section + i)->key + j)->string != NULL)
				{
					(void)strcpy(p, ((ini->section + i)->key + j)->string);
					p += strlen(p);	// ������̒�����p�̈ʒu�����߂�
				}
			}	// if(((ini->section + i)->key + j)->comment_flag == COMMENT_OFF)

			//*(p++) = '\r';	// �L�����b�W���^�[����ǉ�
			*(p++) = '\n';	// ���s��ǉ�
		}	// for(j=0; j<ini->section + i)->key_count; j++)

		//*(p++) = '\r';	// �L�����b�W���^�[����ǉ�
		*(p++) = '\n';	// ���s��ǉ�
	}	// for(i=0; i<ini->section_count; i++)
	*p = '\0';	// �Ō�Ƀk��������ǉ����Ă���

	(void)write_func(buff, 1, strlen(buff), ini->io);

	// �������̊J��
	MEM_FREE_FUNC(buff);

	return 0;
}

/********************************************************
* Delete_INI_FILE�֐�                                   *
* ini�t�@�C�����Ǘ�����\���̂̃��������J������         *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
********************************************************/
static void Delete_INI_FILE(INI_FILE_PTR ini)
{
	int i, j;	// for���p�̃J�E���^

	// �Z�N�V������񂪂���Ȃ炻��̃��������J������
	if(ini->section != NULL)
	{
		for(i=0; i<ini->section_count; i++)
		{	// �Z�N�V�����̃L�[���������J������
			// �L�[�̏�񂪂Ȃ��Ȃ�X�L�b�v
			if((ini->section + i)->key == NULL)
			{
				continue;
			}

			// �L�[�̃��������J������
			for(j=0; j<(ini->section + i)->key_count; j++)
			{
				// ����������J������
				MEM_FREE_FUNC(((ini->section + i)->key + j)->string);
			}

			MEM_FREE_FUNC((ini->section + i)->key);
		}	// for(i=0; i<ini->section_count; i++)

		MEM_FREE_FUNC(ini->section);
	}	// if(ini->section != NULL)

	MEM_FREE_FUNC(ini);
}

/********************************************************
* IniFileGetString�֐�                               *
* ini�t�@�C�����當������擾����                       *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* section_name	: ��������Z�N�V�����̖��O              *
* key_name		: ��������L�[�̖��O                    *
* ret			: ����������������i�[����z��          *
* size			: ret�̃T�C�Y                           *
* �Ԃ�l                                                *
*	��������������̒���(������Ȃ�������0)           *
********************************************************/
long IniFileGetString(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name,
	char* ret,
	const long size
)
{
	char* buff;	// �L�[�̕�����R�s�[�p�̈ꎞ�ۑ�
	char* p;	// �L�[�̕����񑀍�p
	int section_index;	// �Z�N�V�����̌�������
	int key_index;		// �L�[�̌�������
	int len;	// ������̒���

	// �Z�N�V�����̌���
	if((section_index = SectionFind(ini, section_name)) < 0)
	{	// �Z�N�V������������Ȃ�������I��
		return 0;
	}

	// �L�[�̌���
	key_index = KeyFind((ini->section + section_index), key_name);
	// �L�[��������Ȃ�������A�L�[�ɕ����񂪂Ȃ�������I��
	if(key_index < 0 || ((ini->section + section_index)->key + key_index)->string == NULL)
	{
		return 0;
	}

	// ���e�̎擾
		// ���buff�̃��������m�ۂ��Ă���
	buff = (char*)MEM_ALLOC_FUNC(sizeof(char)
		* (strlen(((ini->section + section_index)->key + key_index)->string) + 1));

	// �������m�ې����̃`�F�b�N
#ifdef _DEBUG
	if(buff == NULL)
	{
		(void)printf("Memory allocate error.\n(In IniFileGetString)\n");

		return 0;
	}
#endif
	(void)strcpy(buff, ((ini->section + section_index)->key + key_index)->string);
	Trim(buff);	// �X�y�[�X�A�^�u���폜
	// buff�̓��e���u"�v�ň͂܂�Ă�����u"�v�폜����
	p = (*buff == '\"') ? buff + 1 : buff;
	if((len = (int)strlen(p)) > 0 && *(p + len - 1) == '\"')
	{
		*(p + len - 1) = '\0';	// ������̍Ō���k�������ɂ���
	}
	(void)strncpy(ret, p, size);
	// �o�b�t�@�̃��������J��
	MEM_FREE_FUNC(buff);

	return (int)strlen(ret);
}

/********************************************************
* IniFileStrdup�֐�                                     *
* ���������m�ۂ��ĕ�������m��                          *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* section_name	: ��������Z�N�V�����̖��O              *
* key_name		: ��������L�[�̖��O                    *
* �Ԃ�l                                                *
*	�m�ۂ����������̃A�h���X(���s����NULL)              *
********************************************************/
char* IniFileStrdup(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name
)
{
	char* ret;	// �Ԃ�l
	char* p;	// �u"�v(�_�u���N�I�[�e�[�V����)�폜�p
	int section_index;	// �Z�N�V�����̌�������
	int key_index;		// �L�[�̌�������
	long len;

	// �Z�N�V�����̌���
	if((section_index = SectionFind(ini, section_name)) < 0)
	{	// �Z�N�V������������Ȃ�������I��
		return NULL;
	}

	// �L�[�̌���
	key_index = KeyFind((ini->section + section_index), key_name);
	// �L�[��������Ȃ�������A�L�[�ɕ����񂪂Ȃ�������I��
	if(key_index < 0 || ((ini->section + section_index)->key + key_index)->string == NULL)
	{
		return NULL;
	}

	// ���e�̎擾
		// ���buff�̃��������m�ۂ��Ă���
	ret = (char*)MEM_ALLOC_FUNC(sizeof(char)
		* (strlen(((ini->section + section_index)->key + key_index)->string) + 1));

	// �������m�ې����̃`�F�b�N
#ifdef _DEBUG
	if(ret == NULL)
	{
		(void)printf("Memory allocate error.\n(In Ini_File_Alloc_String)\n");

		return NULL;
	}
#endif

	// ��x�o�b�t�@�ɕ�������R�s�[���Ă��瑀��
	(void)strcpy(ret, ((ini->section + section_index)->key + key_index)->string);
	// �����񂪁u"�v�ň͂܂�Ă�����u"�v���폜
	if(*ret == '\"')
	{
		p = ret;
		while(*(p+1) != '\0')
		{
			*p = *(p+1);
			p++;
		}

		if((len = (int)strlen(ret)) > 0 && *(ret + len - 2) == '\"')
		{
			*(ret + len - 2) = '\0';	// �Ō�̕������k�������ɕύX
		}
	}

	return ret;
}

/********************************************************
* IniFileGetInteger�֐�                                 *
* ini�t�@�C�����琔�l���擾����                         *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* section_name	: ��������Z�N�V�����̖��O              *
* key_name		: ��������L�[�̖��O                    *
* �Ԃ�l                                                *
*	�L�[�ɏ�����Ă��鐔�l(���s���͏��0)               *
********************************************************/
int IniFileGetInteger(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name
)
{
	char buff[NUM_STRING_LEN];	// ���l�̕�������i�[����
	char *p;	// �����񑀍�p
	int section_index;	// �Z�N�V�����̌�������
	int key_index;		// �L�[�̌�������
	int len;	// ������̒���

	// �Z�N�V�����̌���
	if((section_index = SectionFind(ini, section_name)) < 0)
	{	// �Z�N�V������������Ȃ�������I��
		return 0;
	}

	// �L�[�̌���
	key_index = KeyFind((ini->section + section_index), key_name);
	// �L�[��������Ȃ�������A�L�[�ɕ����񂪂Ȃ�������I��
	if(key_index < 0 || ((ini->section + section_index)->key + key_index)->string == NULL)
	{
		return 0;
	}

	// ���l���o�b�t�@���I�[�o�[���Ȃ������`�F�b�N
	if((len = (int)strlen(((ini->section + section_index)->key + key_index)->string)) > NUM_STRING_LEN)
	{
		(void)printf("Buffer over flow\n(In IniFileGetInteger)\n");

		return 0;
	}

	// ��x�o�b�t�@�ɕ�������R�s�[
	(void)strcpy(buff, ((ini->section + section_index)->key + key_index)->string);
	// �����񂪁u"�v�ň͂܂�Ă�����u"�v���폜����
	p = (*buff == '\"') ? (len--, buff + 1) : buff;
	if(len > 0 && *(p + len - 1) == '\"')
	{
		*(p + len - 1) = '\0';	// �Ō�̕������k�������ɕς���
	}

	return atoi(buff);
}

/********************************************************
* IniFileGetDouble�֐�                                  *
* ini�t�@�C�����珬�����l���擾����                     *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* section_name	: ��������Z�N�V�����̖��O              *
* key_name		: ��������L�[�̖��O                    *
* �Ԃ�l                                                *
*	�L�[�ɏ�����Ă��鐔�l(���s���͏��0)               *
********************************************************/
double IniFileGetDouble(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name
)
{
	char buff[NUM_STRING_LEN];	// ���l�̕�������i�[����
	char *p;	// �����񑀍�p
	int section_index;	// �Z�N�V�����̌�������
	int key_index;		// �L�[�̌�������
	int len;	// ������̒���

	// �Z�N�V�����̌���
	if((section_index = SectionFind(ini, section_name)) < 0)
	{	// �Z�N�V������������Ȃ�������I��
		return 0;
	}

	// �L�[�̌���
	key_index = KeyFind((ini->section + section_index), key_name);
	// �L�[��������Ȃ�������A�L�[�ɕ����񂪂Ȃ�������I��
	if(key_index < 0 || ((ini->section + section_index)->key + key_index)->string == NULL)
	{
		return 0;
	}

	// ���l���o�b�t�@���I�[�o�[���Ȃ������`�F�b�N
	if((len = (int)strlen(((ini->section + section_index)->key + key_index)->string)) > NUM_STRING_LEN)
	{
		(void)printf("Buffer over flow\n(In IniFileGetInteger)\n");

		return 0;
	}

	// ��x�o�b�t�@�ɕ�������R�s�[
	(void)strcpy(buff, ((ini->section + section_index)->key + key_index)->string);
	// �����񂪁u"�v�ň͂܂�Ă�����u"�v���폜����
	p = (*buff == '\"') ? (len--, buff + 1) : buff;
	if(len > 0 && *(p + len - 1) == '\"')
	{
		*(p + len - 1) = '\0';	// �Ō�̕������k�������ɕς���
	}

	return atof(buff);
}

/********************************************************
* IniFileGetArray�֐�                                   *
* ini�t�@�C�����畡���̐��l�f�[�^���擾����             *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* section_name	: ��������Z�N�V�����̖��O              *
* key_name		: ��������L�[�̖��O                    *
* array_ptr		: �f�[�^��������z��                  *
* array_size	: �z��̃T�C�Y                          *
* �Ԃ�l                                                *
*	����I��(0)�A�ȏ�I��(���̒l)                       *
********************************************************/
int IniFileGetArray(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name,
	int* array_ptr,
	int array_size
)
{
	char* buff;	// �L�[�̕�����R�s�[�p�̈ꎞ�ۑ�
	char* p;	// �L�[�̕����񑀍�p
	int section_index;	// �Z�N�V�����̌�������
	int key_index;		// �L�[�̌�������
	int len;	// ������̒���
	int i, j;	// for���p�̃J�E���^

	// �Z�N�V�����̌���
	if((section_index = SectionFind(ini, section_name)) < 0)
	{	// �Z�N�V������������Ȃ�������I��
		return -1;
	}

	// �L�[�̌���
	key_index = KeyFind((ini->section + section_index), key_name);
	// �L�[��������Ȃ�������A�L�[�ɕ����񂪂Ȃ�������I��
	if(key_index < 0 || ((ini->section + section_index)->key + key_index)->string == NULL)
	{
		return -1;
	}

	// ���e�̎擾
		// ���buff�̃��������m�ۂ��Ă���
	buff = (char*)MEM_ALLOC_FUNC(sizeof(char)
		* (strlen(((ini->section + section_index)->key + key_index)->string) + 1));

	// �������m�ې����̃`�F�b�N
#ifdef _DEBUG
	if(buff == NULL)
	{
		(void)printf("Memory allocate error.\n(In IniFileGetString)\n");

		return 0;
	}
#endif
	(void)strcpy(buff, ((ini->section + section_index)->key + key_index)->string);
	Trim(buff);	// �X�y�[�X�A�^�u���폜
	// buff�̓��e���u"�v�ň͂܂�Ă�����u"�v�폜����
	p = (*buff == '\"') ? buff + 1 : buff;
	if((len = (int)strlen(p)) > 0 && *(p + len - 1) == '\"')
	{
		*(p + len - 1) = '\0';	// ������̍Ō���k�������ɂ���
	}

	// �v�����ꂽ�������f�[�^��ǂݍ���
	for(i=0; (i<array_size && p != '\0'); i++)
	{
		// ������𐔒l�ɕϊ����邽�߂̃o�b�t�@
		char num_str[NUM_STRING_LEN];

		// p���������̂������ɂȂ�܂�p�����߂�
		while(isdigit(*p) == 0)
		{
			if(*p == '\0')
			{	// ������̏I�[�ɂ��Ă��܂����烋�[�v����E�o
				break;
			}

			p++;
		}

		// ���l�̕������num_str�ɓ����
		for(j=0; (isalnum(*p)!=0 && j < NUM_STRING_LEN); j++, p++)
		{
			num_str[j] = *p;
		}
		num_str[j] = '\0';	// atoi���g�����߂ɍŌ�̕������k�������ɂ���

		// �z��Ƀf�[�^������
		array_ptr[i] = atoi(num_str);
	}	// for(i=0; (i<array_size && p != '\0'); i++)

	// �o�b�t�@�̃��������J��
	MEM_FREE_FUNC(buff);

	return 0;
}

/********************************************************
* IniFileWriteAdd�֐�                                   *
* ini�t�@�C���Ƀf�[�^��ǉ�����                         *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* section_name	: ��������Z�N�V�����̖��O              *
* key_name		: ��������L�[�̖��O                    *
* str			: �ǉ�����f�[�^�̕�����                *
* �Ԃ�l                                                *
*	����I��(0)�A�ȏ�I��(-1)                           *
********************************************************/
static int IniFileAddData(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name,
	char* str
)
{
	int section_index;	// �Z�N�V�����̌�������
	int key_index;		// �L�[�̌�������
	int i;	// for���p�̃J�E���^

	// �Z�N�V���������w�肳��Ă��Ȃ�������G���[
	if(section_name == NULL)
	{
		return -1;
	}

	// �Z�N�V�����̏�񂪂Ȃ������烁�������m�ۂ���
	if(ini->section == NULL)
	{
		ini->section = (SECTION_PTR)MEM_ALLOC_FUNC(sizeof(SECTION) * INI_ALLOC_SIZE);

		// �������m�ې����̃`�F�b�N
#ifdef _DEBUG
		if(ini->section == NULL)
		{
			return -2;
		}
#endif
		ini->section_count = 1;
		ini->section_size = INI_ALLOC_SIZE;
		(void)memset(ini->section, 0, sizeof(SECTION) * INI_ALLOC_SIZE);
	}

	// �Z�N�V�����̌���
	if((section_index = SectionFind(ini, section_name)) < 0)
	{	// ������Ȃ�������Z�N�V������ǉ�����
		if(SectionAdd(ini, section_name) != 0)
		{
			return -3;
		}
		// �Z�N�V�����̃C���f�b�N�X��ݒ�
		section_index = ini->section_count - 1;
	}

	// �L�[�̖��O���Ȃ��A�L�[�����݂��Ă�����L�[���폜����
		// (���̃Z�N�V�����ɂ̓L�[���Ȃ����ƂɂȂ邽��)
	if(key_name == NULL)
	{
		if((ini->section + section_index)->key != NULL)
		{	// �L�[�̍폜
			for(i=0; i<(ini->section + section_index)->key_count; i++)
			{
				// �L�[�ɕ����񂪑��݂��Ă����炻�̃��������J������
				MEM_FREE_FUNC(((ini->section + section_index)->key + i)->string);
			}

			// �L�[�Ɏg���Ă������������J�����A�l�����Z�b�g
			MEM_FREE_FUNC((ini->section + section_index)->key);
			(ini->section + section_index)->key = NULL;
			(ini->section + section_index)->key_count = 0;
			(ini->section + section_index)->key_size = 0;
		}	// if((ini->section + section_index)->key != NULL)

		return 0;	// �ȉ��̏����͂���Ȃ��̂ŏI��
	}	// if(key_name == NULL)

	// �L�[�̌���
	if((key_index = KeyFind((ini->section + section_index), key_name)) < 0)
	{	// ������Ȃ�������L�[��ǉ�
		return KeyAdd((ini->section + section_index), key_name, str, COMMENT_OFF);
	}
	else
	{	// ������������e��ύX����
		// �����񂪑��݂����炻�̃��������ɊJ��
		MEM_FREE_FUNC(((ini->section + section_index)->key + key_index)->string);

		if(str == NULL)
		{	// �L�[�ɓ�����񂪂Ȃ��΂��̗p�ɐݒ�
			*((ini->section + section_index)->key + key_index)->key_name = '\0';
			((ini->section + section_index)->key + key_index)->string = NULL;
			return 0;
		}

		// �L�[�ɕ������ǉ�����
		((ini->section + section_index)->key + key_index)->string =
			MEM_STRDUP_FUNC(str);
		// �������m�ې����̃`�F�b�N
#ifdef _DEBUG
		if(((ini->section + section_index)->key + key_index)->string == NULL)
		{
			(void)printf("Memory allocate error.\n(In Ini_File_Write_Data)\n");

			return -4;
		}
#endif
	}	// if((key_index = KeyFind((ini->section + section_index), key_name)) < 0) else

	return 0;
}

/********************************************************
* IniFileAddString�֐�                                  *
* ini�t�@�C���ɕ������ǉ�����                         *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* section_name	: ��������Z�N�V�����̖��O              *
* key_name		: ��������L�[�̖��O                    *
* str			: �ǉ����镶����                        *
* �Ԃ�l                                                *
*	����I��(0)�A�ȏ�I��(���̒l)                       *
********************************************************/
int IniFileAddString(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name,
	const char* str
)
{
	char* buff;	// �������ޕ�������i�[����
	char* p;	// �����񑀍�p
	int ret;	// �Ԃ�l

	// �����񂪂Ȃ������炻�̂܂܏����o��
	if(str == NULL || *str == '\0')
	{
		return IniFileAddData(ini, section_name, key_name, "");
	}

	// �o�b�t�@�̃��������m�ۂ���
		// �u"�v�ň͂ޕ��̃��������܂�
	buff = (char*)MEM_ALLOC_FUNC(sizeof(char)*(strlen(str)+3));

	// �������m�ې����̃`�F�b�N
#ifdef _DEBUG
	if(buff == NULL)
	{
		(void)printf("Memory allocate error.\n(In Ini_File_Write_String)\n");

		return -1;
	}
#endif

	// �u"�v�ŕ�������͂�
	p = buff;
	*(p++) = '\"';
	(void)strcpy(p, str);
	p += strlen(p);
	*(p++) = '\"';
	*(p++) = '\0';	// ������̍Ō�Ƀk��������ǉ�
	ret = IniFileAddData(ini, section_name, key_name, buff);

	// �o�b�t�@�̃��������J������
	MEM_FREE_FUNC(buff);

	return ret;
}

/*********************************************************
* IniFileAddInteger�֐�                                  *
* ini�t�@�C���ɐ��l��ǉ�����                            *
* ����                                                   *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X  *
* section_name	: ��������Z�N�V�����̖��O               *
* key_name		: ��������L�[�̖��O                     *
* num			: �ǉ����鐔�l                           *
* radix			: ���l�̃t�H�[�}�b�g(10�i���A16�i���Ȃ�) *
* �Ԃ�l                                                 *
*	����I��(0)�A�ȏ�I��(���̒l)                        *
*********************************************************/
extern int IniFileAddInteger(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name,
	const int num,
	const int radix
)
{
	// �������ރf�[�^���i�[
	char buff[NUM_STRING_LEN];

	// ���l�𕶎���ɕϊ�����
	itoa(num, buff, radix);

	// �f�[�^��ǉ�
	return IniFileAddData(ini, section_name, key_name, buff);
}

/********************************************************
* IniFileAddDouble�֐�                                  *
* ini�t�@�C���ɏ����f�[�^��ǉ�����                     *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* section_name	: ��������Z�N�V�����̖��O              *
* key_name		: ��������L�[�̖��O                    *
* num			: �ǉ����鐔�l                          *
* digigt		: �����_�ȉ��̌���(-1�ȉ��Őݒ薳��)    *
* �Ԃ�l                                                *
*	����I��(0)�A�ȏ�I��(���̒l)                       *
********************************************************/
int IniFileAddDouble(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name,
	const double num,
	const int digit
)
{
	// �������ރf�[�^���i�[
	char buff[NUM_STRING_LEN];

	// �����_�ȉ��̐ݒ肪�����
	if(digit >= 0)
	{
		// �����쐬�p
		char format[16];

		// �t�H�[�}�b�g�p������쐬
		(void)sprintf(format, "%%.%df", digit);
		// ���l�𕶎���ɕϊ�����
		(void)sprintf(buff, format, num);
	}
	else
	{
		// ���l�𕶎���ɕϊ�����
		(void)sprintf(buff, "%.2f", num);
	}

	// �f�[�^��ǉ�
	return IniFileAddData(ini, section_name, key_name, buff);
}

#ifdef __cplusplus
}
#endif
