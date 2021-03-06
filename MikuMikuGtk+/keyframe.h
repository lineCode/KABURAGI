#ifndef _INCLUDED_KEYFRAME_H_
#define _INCLUDED_KEYFRAME_H_

#include "types.h"
#include "utils.h"

typedef enum _eKEYFRAME_TYPE
{
	KEYFRAME_TYPE_ASSET,
	KEYFRAME_TYPE_BONE,
	KEYFRAME_TYPE_CAMERA,
	KEYFRAME_TYPE_EFFECT,
	KEYFRAME_TYPE_LIGHT,
	KEYFRAME_TYPE_MODEL,
	KEYFRAME_TYPE_MORPH,
	KEYFRAME_TYPE_PROJECT,
	MAX_KEYFRAME_TYPE
} eKEYFRAME_TYPE;

typedef struct _KEYFRAME_INTERFACE
{
	eKEYFRAME_TYPE type;
	char *name;
	int layer_index;
	FLOAT_T time_index;
	struct _APPLICATION *application;
} KEYFRAME_INTERFACE;

typedef enum _eBONE_KEYFRAME_INTERPOLATION_TYPE
{
	BONE_KEYFRAME_INTERPOLATION_TYPE_X,
	BONE_KEYFRAME_INTERPOLATION_TYPE_Y,
	BONE_KEYFRAME_INTERPOLATION_TYPE_Z,
	BONE_KEYFRAME_INTERPOLATION_TYPE_ROTATION,
	MAX_BONE_KEYFRAME_INTERPOLATION_TYPE
} eBONE_KEYFRAME_INTERPOLATION_TYPE;

typedef struct _BONE_KEYFRAME_INTERFACE
{
	KEYFRAME_INTERFACE base_data;
	struct _BONE_INTERFACE *bone;
	VECTOR3 position;
	QUATERNION rotation;
	void (*set_default_interpolation_parameter)(void*);
	void (*set_local_translation)(void*, float*);
	void (*set_local_rotation)(void*, float*);
} BONE_KEYFRAME_INTERFACE;

typedef struct _BONE_KEYFARAME_INTERPOLIATION_PARAMETER
{
	float x[4];
	float y[4];
	float z[4];
	float rotation[4];
} BONE_KEYFARAME_INTERPOLIATION_PARAMETER;

typedef struct _MODEL_KEYFRAME_INTERFACE
{
	KEYFRAME_INTERFACE base_data;
} MODEL_KEYFRAME_INTERFACE;

typedef struct _MORPH_KEYFRAME_INTERFACE
{
	KEYFRAME_INTERFACE base_data;
} MORPH_KEYFRAME_INTERFACE;

#ifdef __cplusplus
extern "C" {
#endif

extern int FindKeyframeIndex(
	FLOAT_T key,
	KEYFRAME_INTERFACE** keyframes,
	const int num_keyframes
);

#ifdef __cplusplus
}
#endif

#endif	// _INCLUDED_KEYFRAME_H_
