#ifndef _INCLUDED_PARAMETER_H_
#define _INCLUDED_PARAMETER_H_

#include <stdio.h>
#include <GL/glew.h>
#include "model.h"
#include "types.h"
#include "ght_hash_table.h"
#include "technique.h"
#include "pass.h"
#include "sampler_state.h"
#include "utils.h"

typedef enum _ePARAMETER_TYPE
{
	PARAMETER_TYPE_UNKNOWN,
	PARAMETER_TYPE_BOOLEAN,
	PARAMETER_TYPE_BOOLEAN1,
	PARAMETER_TYPE_BOOLEAN2,
	PARAMETER_TYPE_BOOLEAN3,
	PARAMETER_TYPE_BOOLEAN4,
	PARAMETER_TYPE_INTEGER,
	PARAMETER_TYPE_INTEGER1,
	PARAMETER_TYPE_INTEGER2,
	PARAMETER_TYPE_INTEGER3,
	PARAMETER_TYPE_INTEGER4,
	PARAMETER_TYPE_FLOAT,
	PARAMETER_TYPE_FLOAT1,
	PARAMETER_TYPE_FLOAT2,
	PARAMETER_TYPE_FLOAT3,
	PARAMETER_TYPE_FLOAT4,
	PARAMETER_TYPE_FLOAT2x2,
	PARAMETER_TYPE_FLOAT3x3,
	PARAMETER_TYPE_FLOAT4x4,
	PARAMETER_TYPE_STRING,
	PARAMETER_TYPE_TEXTURE,
	PARAMETER_TYPE_SAMPLER,
	PARAMETER_TYPE_SAMPLER1D,
	PARAMETER_TYPE_SAMPLER2D,
	PARAMETER_TYPE_SAMPLER3D,
	PARAMETER_TYPE_SAMPLER_CUBE,
	MAX_PARAMETER_TYPE
} ePARAMETER_TYPE;

typedef enum _eEFFECT_TYPE
{
	EFFECT_TYPE_FX,
	EFFECT_TYPE_NV_FX,
	NUM_EFFECT_TYPE
} eEFFECT_TYPE;

typedef enum _eVERTEX_ATTRIBUTE_TYPE
{
	VERTEX_ATTRIBUTE_UNKNOWN = -1,
	VERTEX_ATTRIBUTE_POSTION,
	VERTEX_ATTRIBUTE_NORMAL,
	VERTEX_ATTRIBUTE_TEXTURE_COORD,
	VERTEX_ATTRIBUTE_BONE_INDEX,
	VERTEX_ATTRIBUTE_BONE_WEIGHT,
	VERTEX_ATTRIBUTE_UVA1,
	VERTEX_ATTRIBUTE_UVA2,
	VERTEX_ATTRIBUTE_UVA3,
	VERTEX_ATTRIBUTE_UVA4,
	MAX_VERTEX_ATTRIBUTE
} eVERTEX_ATTRIBUTE_TYPE;

typedef struct _FX_PARAMETER
{
	struct _EFFECT *parent_effect;
	ght_hash_table_t *annotations;
	STRUCT_ARRAY *values;
	char *symbol;
	char *sematic;
	char *value;
//	FX_TYPE_UNION base;
//	FX_TYPE_UNION full;
} FX_PARAMETER;

#endif	// #ifndef _INCLUDED_PARAMETER_H_
