#ifndef _INLUCDED_INI_FILE_H_
#define _INLUCDED_INI_FILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EXTERN
# undef EXTERN
#endif

#ifdef _MSC_VER
# ifdef __cplusplus
#  define EXTERN extern "C" __declspec(dllexport)
# else
#  define EXTERN extern __declspec(dllexport)
# endif
#else
# define EXTERN extern
#endif

#define KEY_BUFF_SIZE 256		// �L�[�ɂ����閼�O�̍ő咷
#define SECTION_BUFF_SIZE 256	// �Z�N�V�����ɂ����閼�O�̍ő咷
#define INI_ALLOC_SIZE 10		// �T�C�Y�X�V���̍X�V�l

#define NUM_STRING_LEN 64	// ���l�f�[�^�̕�����̒����̍ő�l

#define COMMENT_ON 1	// �R�����g�ł���(�t���O�̒l)
#define COMMENT_OFF 0	// �R�����g�łȂ�(�t���O�̒l)

// �ǂݍ��݂��������݂�
typedef enum _eINI_MODE
{
	INI_READ,
	INI_WRITE
} eINI_MODE;

typedef struct _KEY
{
	char key_name[KEY_BUFF_SIZE];	// �L�[�̖��O���i�[
	int hash;	// �n�b�V���l
	char *string;
	char comment_flag;	// �R�����g�ł��邩�ǂ����̃t���O
} KEY, *KEY_PTR;

typedef struct _SECTION
{
	// �Z�N�V�����̖��O���i�[
	char section_name[SECTION_BUFF_SIZE];
	int hash;	// �n�b�V���l
	KEY_PTR key;	// �L�[�̔z��
	int key_count;	// �L�[�̐�
	int key_size;	// �L�[�̃T�C�Y
} SECTION, *SECTION_PTR;

typedef struct _INI_FILE
{
	SECTION_PTR section;	// ini�t�@�C���̓��e���i�[
	int section_size;	// �Z�N�V�����̃T�C�Y
	int section_count;	// �Z�N�V�����̐�
	void* io;			// ini�t�@�C���f�[�^�ւ̃t�@�C���|�C���^
	char* file_path;	// ini�t�@�C���ւ̃p�X

	// �������J���p�̊֐��ւ̃|�C���^
	void (*delete_func)(struct _INI_FILE *ini);
} INI_FILE, *INI_FILE_PTR;

// �֐��̃v���g�^�C�v�錾
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
EXTERN INI_FILE_PTR CreateIniFile(
	void* io,
	size_t (*read_func)(void*, size_t, size_t, void*),
	size_t data_size,
	eINI_MODE mode
);

/********************************************************
* WriteIniFile�֐�                                      *
* ini�t�@�C���ɓ��e�������o��                           *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* write_func	: �����o���p�֐��ւ̃|�C���^            *
* �Ԃ�l                                                *
*	����I��(0)�A�ȏ�I��(0�ȊO)                        *
********************************************************/
EXTERN int WriteIniFile(
	INI_FILE_PTR ini,
	size_t (*write_func)(void*, size_t, size_t, void*)
);

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
EXTERN long IniFileGetString(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name,
	char* ret,
	const long size
);

/********************************************************
* IniFileStrdup�֐�                                   *
* ���������m�ۂ��ĕ�������m��                          *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* section_name	: ��������Z�N�V�����̖��O              *
* key_name		: ��������L�[�̖��O                    *
* �Ԃ�l                                                *
*	�m�ۂ����������̃A�h���X(���s����NULL)              *
********************************************************/
EXTERN char* IniFileStrdup(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name
);

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
EXTERN int IniFileGetInteger(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name
);

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
EXTERN double IniFileGetDouble(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name
);

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
EXTERN int IniFileGetArray(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name,
	int* array_ptr,
	int array_size
);

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
EXTERN int IniFileAddString(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name,
	const char* str
);

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
EXTERN int IniFileAddInteger(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name,
	const int num,
	const int radix
);

/********************************************************
* IniFileAddDouble�֐�                                  *
* ini�t�@�C���ɏ����f�[�^��ǉ�����                     *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* section_name	: ��������Z�N�V�����̖��O              *
* key_name		: ��������L�[�̖��O                    *
* num			: �ǉ����鐔�l                          *
* �Ԃ�l                                                *
*	����I��(0)�A�ȏ�I��(���̒l)                       *
********************************************************/
EXTERN int IniFileAddDouble(
	INI_FILE_PTR ini,
	const char* section_name,
	const char* key_name,
	const double num
);

/********************************************************
* Change_INI_FILE_MODE�֐�                              *
* ini�t�@�C���̏������݁A�ǂݍ��݂̃��[�h��؂�ւ���   *
* ����                                                  *
* ini			: ini�t�@�C�����Ǘ�����\���̂̃A�h���X *
* �Ԃ�l                                                *
*	����I��(0)�A�ُ�I��(���̒l)                       *
********************************************************/
EXTERN int Change_INI_FILE_MODE(INI_FILE_PTR ini, eINI_MODE mode);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INLUCDED_INI_FILE_H_
