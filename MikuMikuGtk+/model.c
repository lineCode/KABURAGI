// Visual Studio 2005�ȍ~�ł͌Â��Ƃ����֐����g�p����̂�
	// �x�����o�Ȃ��悤�ɂ���
#if defined _MSC_VER && _MSC_VER >= 1400
# define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include "model.h"
#include "application.h"
#include "memory.h"

#ifdef __cplusplus
extern "C" {
#endif

void ReleaseModelInterface(MODEL_INTERFACE* model)
{
	MEM_FREE_FUNC(model->name);
	MEM_FREE_FUNC(model->english_name);
	MEM_FREE_FUNC(model->comment);
	MEM_FREE_FUNC(model->english_comment);
	MEM_FREE_FUNC(model->model_path);
	MEM_FREE_FUNC(model->texture_archive);
}

void ModelJoinWorld(MODEL_INTERFACE* model, WORLD* world)
{
	switch(model->type)
	{
	case MODEL_TYPE_ASSET_MODEL:
		break;
	case MODEL_TYPE_PMX_MODEL:
		PmxModelJoinWorld((PMX_MODEL*)model, world->world);
		break;
	}
}

void ModelLeaveWorld(MODEL_INTERFACE* model, WORLD* world)
{
	switch(model->type)
	{
	case MODEL_TYPE_ASSET_MODEL:
		break;
	case MODEL_TYPE_PMX_MODEL:
		PmxModelLeaveWorld((PMX_MODEL*)model, world->world);
		break;
	}
}

void DeleteModel(MODEL_INTERFACE* model)
{
	if(model == NULL)
	{
		return;
	}

	switch(model->type)
	{
	case MODEL_TYPE_ASSET_MODEL:
		ReleaseAssetModel((ASSET_MODEL*)model);
		MEM_FREE_FUNC(model);
		break;
	case MODEL_TYPE_PMX_MODEL:
		ReleasePmxModel((PMX_MODEL*)model);
		MEM_FREE_FUNC(model);
		break;
	}
}

typedef struct _SET_LABEL_DATA
{
	char *label;
	POINTER_ARRAY *child_names;
} SET_LABEL_DATA;

char** GetChildBoneNames(MODEL_INTERFACE* model, void* application_context)
{
	APPLICATION *application = (APPLICATION*)application_context;
	PROJECT *project = application->projects[application->active_project];
	STRUCT_ARRAY *set_label_data = StructArrayNew(sizeof(SET_LABEL_DATA), DEFAULT_BUFFER_SIZE);
	SET_LABEL_DATA *parent;
	SET_LABEL_DATA *data;
	LABEL_INTERFACE **labels;
	LABEL_INTERFACE *label;
	BONE_INTERFACE *bone;
	POINTER_ARRAY *added_bones;
	POINTER_ARRAY *names;
	char **ret;
	int num_labels;
	int num_children;
	int num_added_bones = 0;
	int i, j;

	if(model == NULL)
	{
		return NULL;
	}

	labels = (LABEL_INTERFACE**)model->get_labels(model, &num_labels);

	parent = (SET_LABEL_DATA*)StructArrayReserve(set_label_data);
	parent->child_names = PointerArrayNew(DEFAULT_BUFFER_SIZE);
	added_bones = PointerArrayNew(DEFAULT_BUFFER_SIZE);
	for(i=0; i<num_labels; i++)
	{
		label = labels[i];
		num_children = label->get_count(label);
		if(label->special != FALSE)
		{
			if(num_children > 0 && strcmp(label->name, "Root") == 0)
			{
				bone = label->get_bone(label, 0);
				if(bone != NULL)
				{
					parent->label = bone->name;
					PointerArrayAppend(added_bones, bone);
				}
			}
			if(parent->label == NULL)
			{
				continue;
			}
		}
		else
		{
			parent->label = label->name;
			PointerArrayAppend(added_bones, bone);
		}

		for(j=0; j<num_children; j++)
		{
			bone = label->get_bone(label, j);
			if(bone != NULL)
			{
				PointerArrayAppend(parent->child_names, bone->name);
				PointerArrayAppend(added_bones, bone);
			}
		}

		if(parent->child_names->num_data > 0)
		{
			parent = (SET_LABEL_DATA*)StructArrayReserve(set_label_data);
			parent->child_names = PointerArrayNew(DEFAULT_BUFFER_SIZE);
		}
	}

	data = (SET_LABEL_DATA*)set_label_data->buffer;
	names = PointerArrayNew(DEFAULT_BUFFER_SIZE);
	for(i=0; i<(int)set_label_data->num_data; i++)
	{
		parent = &data[i];
		bone = (BONE_INTERFACE*)added_bones->buffer[num_added_bones];
		num_added_bones++;

		for(j=0; j<(int)parent->child_names->num_data; j++)
		{
			bone = (BONE_INTERFACE*)added_bones->buffer[num_added_bones];
			PointerArrayAppend(names, MEM_STRDUP_FUNC(bone->name));
			num_added_bones++;
		}
	}
	PointerArrayAppend(names, NULL);
	ret = (char**)names->buffer;

	for(i=0; i<(int)set_label_data->num_data; i++)
	{
		parent = &data[i];
		PointerArrayDestroy(&parent->child_names, NULL);
	}
	StructArrayDestroy(&set_label_data, NULL);

	MEM_FREE_FUNC(labels);
	MEM_FREE_FUNC(names);

	return ret;
}

/*****************************************
* ReadModelData�֐�                      *
* ���f���f�[�^�Ə�Ԃ�ǂݍ���           *
* ����                                   *
* model		: �ǂݍ��ݐ�̃��f��         *
* src		: �f�[�^�X�g���[��           *
* data_size	: �f�[�^�̃o�C�g��           *
* read_func	: �ǂݍ��݂Ɏg���֐��|�C���^ *
* seek_func	: �V�[�N�Ɏg���֐��|�C���^   *
* �Ԃ�l                                 *
*	����:TRUE!(0)	���s:FALSE(0)        *
*****************************************/
int ReadModelData(
	MODEL_INTERFACE* model,
	void* scene,
	void* src,
	size_t (*read_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int)
)
{
	if(model == NULL)
	{
		return FALSE;
	}

	switch(model->type)
	{
	case MODEL_TYPE_ASSET_MODEL:
		return ReadAssetModelDataAndState(scene, (ASSET_MODEL*)model,
			src, read_func, seek_func);
	case MODEL_TYPE_PMD_MODEL:
		return ReadPmd2ModelDataAndState(scene, (PMD2_MODEL*)model,
			src, read_func, seek_func);
	case MODEL_TYPE_PMX_MODEL:
		return ReadPmxModelDataAndState(scene, (PMX_MODEL*)model,
			src, read_func, seek_func);
	case MODEL_TYPE_SHAPE:
		return ReadShapeModelDataAndState((SHAPE_MODEL*)model,
			src, read_func, seek_func, scene);
	}

	return FALSE;
}

/*****************************************************
* WriteModelData�֐�                                 *
* ���f���f�[�^�Ə�Ԃ������o��                       *
* ����                                               *
* model			: �f�[�^�Ə�Ԃ������o�����f��       *
* dst			: �����o����̃X�g���[��             *
* write_func	: �����o���Ɏg���֐��|�C���^         *
* seek_func		: �V�[�N�Ɏg���֐��|�C���^           *
* tell_func		: �V�[�N�ʒu�̎擾�Ɏg���֐��|�C���^ *
* �Ԃ�l                                             *
*	�����o�����o�C�g��                               *
*****************************************************/
size_t WriteModelData(
	MODEL_INTERFACE* model,
	void* dst,
	size_t (*write_func)(void*, size_t, size_t, void*),
	int (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	size_t* out_data_size
)
{
	switch(model->type)
	{
	case MODEL_TYPE_ASSET_MODEL:
		return WriteAssetModelDataAndState(
			(ASSET_MODEL*)model, dst, write_func, seek_func, tell_func);
	case MODEL_TYPE_PMD_MODEL:
		return WritePmd2ModelDataAndState(
			(PMD2_MODEL*)model, dst, write_func, seek_func, tell_func);
	case MODEL_TYPE_PMX_MODEL:
		return WritePmxModelDataAndState(
			(PMX_MODEL*)model, dst, write_func, seek_func, tell_func);
	case MODEL_TYPE_SHAPE:
		return WriteShapeModel((SHAPE_MODEL*)model,
			dst, write_func, seek_func, tell_func);
	}

	return 0;
}

/***************************************************************
* MakeModelContext�֐�                                         *
* �w�肳�ꂽ�^�C�v�̃��f���f�[�^�����������ĕԂ�               *
* ����                                                         *
* type					: ���������郂�f���̃^�C�v             *
* scene					: �V�[���`����Ǘ�����f�[�^           *
* application_context	: �A�v���P�[�V�����S�̂��Ǘ�����f�[�^ *
* �Ԃ�l                                                       *
*	���������ꂽ���f���f�[�^                                   *
***************************************************************/
MODEL_INTERFACE* MakeModelContext(
	eMODEL_TYPE type,
	void* scene,
	void* application_context
)
{
	APPLICATION *application = (APPLICATION*)application_context;
	MODEL_INTERFACE *model = NULL;

	switch(type)
	{
	case MODEL_TYPE_ASSET_MODEL:
		model = (MODEL_INTERFACE*)MEM_ALLOC_FUNC(sizeof(ASSET_MODEL));
		InitializeAssetModel((ASSET_MODEL*)model, (SCENE*)scene, application_context);
		break;
	case MODEL_TYPE_PMD_MODEL:
		model = (MODEL_INTERFACE*)MEM_ALLOC_FUNC(sizeof(PMD2_MODEL));
		InitializePmd2Model((PMD2_MODEL*)model, (SCENE*)scene, "./");
		break;
	case MODEL_TYPE_PMX_MODEL:
		model = (MODEL_INTERFACE*)MEM_ALLOC_FUNC(sizeof(PMX_MODEL));
		InitializePmxModel((PMX_MODEL*)model, (SCENE*)scene, &application->encode, "./");
		break;
	case MODEL_TYPE_SHAPE:
		model = (MODEL_INTERFACE*)MEM_ALLOC_FUNC(sizeof(SHAPE_MODEL_DATA));
		model->type = MODEL_TYPE_SHAPE;
		break;
	}

	return model;
}

#ifdef __cplusplus
}
#endif
