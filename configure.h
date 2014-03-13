#ifndef _INCLUDED_CONFIGURE_H_
#define _INCLUDED_CONFIGURE_H_

#define CHECK_MEMORY_POOL0
#define USE_BGR_COLOR_SPACE 1

#define USE_STATIC_LIB 0

#ifdef _X64
# define BUILD64BIT 1
#endif

#define USE_3D_LAYER 1

#define USE_TBB 1

#define FRAME_RATE 60
#define AUTO_SAVE_INTERVAL 60

#define MAX_ZOOM 400	// �ő�g�嗦
#define MIN_ZOOM 10		// �ŏ��̊g�嗦

// �Œ�M��
#define MINIMUM_PRESSURE 0.005
// �y�����^�u���b�g���痣���������Ɣ��肷��臒l
#define RELEASE_PRESSURE 0.003

#define AUTO_SAVE(X) AutoSave((X))

// #define OLD_SELECTION_AREA

#endif	// #ifndef _INCLUDED_CONFIGURE_H_
