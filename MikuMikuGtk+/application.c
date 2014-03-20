// Visual Studio 2005�ȍ~�ł͌Â��Ƃ����֐����g�p����̂�
	// �x�����o�Ȃ��悤�ɂ���
#if defined _MSC_VER && _MSC_VER >= 1400
# define _CRT_SECURE_NO_DEPRECATE
# define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <math.h>
#include <zlib.h>
#include "application.h"
#include "map_funcs.h"
#include "utils.h"
#include "memory.h"
#include "load_image.h"
#include "bullet.h"

void* ApplicationContextNew(int default_width, int default_height)
{
	APPLICATION *application = MikuMikuGtkNew(
		default_width, default_height, FALSE, NULL);

	return (void*)application;
}

static void LoadDefaultData(APPLICATION* application)
{
	application->paths.shader_directory_path = "./";
	application->paths.image_directory_path = "./image/";
	application->paths.icon_directory_path = "./icons/";
	application->paths.model_directory_path = "./models/";
}

static void InitializeDefaultData(APPLICATION* application)
{
	static const float basis[] = IDENTITY_MATRIX3x3;

	InitializeDefaultBone(&application->default_data.bone, application);
	InitializeDefaultMaterial(&application->default_data.material, application);

	application->default_data.identity_transform = BtTransformNew(basis);
}

APPLICATION* MikuMikuGtkNew(
	int widget_width,
	int widget_height,
	int make_first_project,
	const char* initialize_file_path
)
{
	static const float basis[] = IDENTITY_MATRIX3x3;
	APPLICATION *ret;

	// �������m��
	ret = (APPLICATION*)MEM_ALLOC_FUNC(sizeof(*ret));
	(void)memset(ret, 0, sizeof(*ret));

	ret->label = LoadDefaultUiLabel();

	// �ŏ��̃v���W�F�N�g���쐬
	if(make_first_project != FALSE)
	{
		ret->projects[0] = ProjectNew((void*)ret,
			widget_width, widget_height, PROJECT_FLAG_DRAW_GRID | PROJECT_FLAG_ALWAYS_PHYSICS);
		ret->num_projects++;
	}

	if(initialize_file_path != NULL)
	{
	}
	else
	{
		LoadDefaultData(ret);
	}

	InitializeTextEncode(&ret->encode, "UTF-16LE", "UTF-8");
	InitializeDefaultData(ret);

	ret->transform = BtTransformNew(basis);

	ret->texture_chache_map = ght_create(DEFAULT_BUFFER_SIZE*2);
	ght_set_hash(ret->texture_chache_map, (ght_fn_hash_t)GetStringHash);

	return ret;
}

char* LoadSubstituteShaderSource(
	const char* file_path
)
{
	char *ret;

	static const uint8 asset_model_vsh[] =
	{
#include "asset_model_vsh.txt"
	};
	static const uint8 asset_model_fsh[] =
	{
#include "asset_model_fsh.txt"
	};
	static const uint8 asset_z_plot_vsh[] =
	{
#include "asset_zplot_vsh.txt"
	};
	static const uint8 asset_z_plot_fsh[] =
	{
#include "asset_zplot_fsh.txt"
	};
	static const uint8 pmx_edge_vsh[] =
	{
#include "pmx_edge_vsh.txt"
	};
	static const uint8 pmx_edge_fsh[] =
	{
#include "pmx_edge_fsh.txt"
	};
	static const uint8 pmx_model_vsh[] =
	{
#include "pmx_model_vsh.txt"
	};
	static const uint8 pmx_model_fsh[] =
	{
#include "pmx_model_fsh.txt"
	};
	static const uint8 pmx_shadow_vsh[] =
	{
#include "pmx_shadow_vsh.txt"
	};
	static const uint8 pmx_shadow_fsh[] =
	{
#include "pmx_shadow_fsh.txt"
	};
	static const uint8 pmx_z_plot_vsh[] =
	{
#include "pmx_zplot_vsh.txt"
	};
	static const uint8 pmx_z_plot_fsh[] =
	{
#include "pmx_zplot_fsh.txt"
	};
	static const uint8 pmx_skinning_edge_vsh[] =
	{
#include "pmx_skinning_edge_vsh.txt"
	};
	static const uint8 pmx_skinning_model_vsh[] =
	{
#include "pmx_skinning_model_vsh.txt"
	};
	static const uint8 pmx_skinning_shadow_vsh[] =
	{
#include "pmx_skinning_shadow_vsh.txt"
	};
	static const uint8 pmx_skinning_z_plot_vsh[] =
	{
#include "pmx_skinning_zplot_vsh.txt"
	};
	static const uint8 gui_handle_vsh[] =
	{
#include "gui_handle_vsh.txt"
	};
	static const uint8 gui_handle_fsh[] =
	{
#include "gui_handle_fsh.txt"
	};
	static const uint8 gui_grid_vsh[] =
	{
#include "gui_grid_vsh.txt"
	};
	static const uint8 gui_grid_fsh[] =
	{
#include "gui_grid_fsh.txt"
	};
	static const uint8 gui_texture_vsh[] =
	{
#include "gui_texture_vsh.txt"
	};
	static const uint8 gui_texture_fsh[] =
	{
#include "gui_texture_fsh.txt"
	};

	z_stream stream = {0};
	const uint8 *source_data;
	int32 source_length = 0;
	unsigned int length = 0;

	if(strstr(file_path, "asset/model.vsh") != NULL)
	{
		source_data = asset_model_vsh;
		length = sizeof(asset_model_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "asset/model.fsh") != NULL)
	{
		source_data = asset_model_fsh;
		length = sizeof(asset_model_fsh) - sizeof(int32);
	}
	else if(strstr(file_path, "asset/zplot.vsh") != NULL)
	{
		source_data = asset_z_plot_vsh;
		length = sizeof(asset_z_plot_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "asset/zplot.fsh") != NULL)
	{
		source_data = asset_z_plot_fsh;
		length = sizeof(asset_z_plot_fsh) - sizeof(int32);
	}
	else if(strstr(file_path, "pmx/edge.vsh") != NULL)
	{
		source_data = pmx_edge_vsh;
		length = sizeof(pmx_edge_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "pmx/edge.fsh") != NULL)
	{
		source_data = pmx_edge_fsh;
		length = sizeof(pmx_edge_fsh) - sizeof(int32);
	}
	else if(strstr(file_path, "pmx/model.vsh") != NULL)
	{
		source_data = pmx_model_vsh;
		length = sizeof(pmx_model_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "pmx/model.fsh") != NULL)
	{
		source_data = pmx_model_fsh;
		length = sizeof(pmx_model_fsh) - sizeof(int32);
	}
	else if(strstr(file_path, "pmx/shadow.vsh") != NULL)
	{
		source_data = pmx_shadow_vsh;
		length = sizeof(pmx_shadow_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "pmx/shadow.fsh") != NULL)
	{
		source_data = pmx_shadow_fsh;
		length = sizeof(pmx_shadow_fsh) - sizeof(int32);
	}
	else if(strstr(file_path, "pmx/zplot.vsh") != NULL)
	{
		source_data = pmx_z_plot_vsh;
		length = sizeof(pmx_z_plot_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "pmx/zplot.fsh") != NULL)
	{
		source_data = pmx_z_plot_fsh;
		length = sizeof(pmx_z_plot_fsh) - sizeof(int32);
	}
	else if(strstr(file_path, "pmx/skinning/edge.vsh") != NULL)
	{
		source_data = pmx_skinning_edge_vsh;
		length = sizeof(pmx_skinning_edge_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "pmx/skinning/model.vsh") != NULL)
	{
		source_data = pmx_skinning_model_vsh;
		length = sizeof(pmx_skinning_model_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "pmx/skinning/shadow.vsh") != NULL)
	{
		source_data = pmx_skinning_shadow_vsh;
		length = sizeof(pmx_skinning_shadow_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "pmx/skinning/zplot.vsh") != NULL)
	{
		source_data = pmx_skinning_z_plot_vsh;
		length = sizeof(pmx_skinning_z_plot_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "gui/handle.vsh") != NULL)
	{
		source_data = gui_handle_vsh;
		length = sizeof(gui_handle_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "gui/handle.fsh") != NULL)
	{
		source_data = gui_handle_fsh;
		length = sizeof(gui_handle_fsh) - sizeof(int32);
	}
	else if(strstr(file_path, "gui/grid.vsh") != NULL)
	{
		source_data = gui_grid_vsh;
		length = sizeof(gui_grid_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "gui/grid.fsh") != NULL)
	{
		source_data = gui_grid_fsh;
		length = sizeof(gui_grid_fsh) - sizeof(int32);
	}
	else if(strstr(file_path, "gui/texture.vsh") != NULL)
	{
		source_data = gui_texture_vsh;
		length = sizeof(gui_texture_vsh) - sizeof(int32);
	}
	else if(strstr(file_path, "gui/texture.fsh") != NULL)
	{
		source_data = gui_texture_fsh;
		length = sizeof(gui_texture_fsh) - sizeof(int32);
	}
	else
	{
		return NULL;
	}

	(void)memcpy(&source_length, source_data, sizeof(source_length));
	ret = (char*)MEM_ALLOC_FUNC(source_length+1);
	(void)inflateInit(&stream);
	stream.avail_in = length;
	stream.next_in = (Bytef*)&source_data[sizeof(int32)];
	stream.avail_out = (uInt)source_length;
	stream.next_out = (Bytef*)ret;
	(void)inflate(&stream, Z_NO_FLUSH);
	ret[source_length] = '\0';

	(void)inflateEnd(&stream);

	return ret;
}

uint8* LoadSubstituteModel(const char* path, size_t* data_size)
{
	uint8 *ret;
	static const uint8 rotation_3ds[] =
	{
#include "rotation_3ds.txt"
	};
	static const uint8 translation_3ds[] =
	{
#include "translation_3ds.txt"
	};

	z_stream stream = {0};
	const uint8 *source_data;
	int32 data_length = 0;
	unsigned int length = 0;

	if(strstr(path, "rotation.3ds") != NULL)
	{
		source_data = rotation_3ds;
		length = sizeof(rotation_3ds) - sizeof(int32);
	}
	else if(strstr(path, "translation.3ds") != NULL)
	{
		source_data = translation_3ds;
		length = sizeof(translation_3ds) - sizeof(int32);
	}
	else
	{
		return NULL;
	}

	(void)memcpy(&data_length, source_data, sizeof(data_length));
	ret = (uint8*)MEM_ALLOC_FUNC(data_length);
	(void)inflateInit(&stream);
	stream.avail_in = length;
	stream.next_in = (Bytef*)&source_data[sizeof(int32)];
	stream.avail_out = (uInt)data_length;
	stream.next_out = (Bytef*)ret;
	(void)inflate(&stream, Z_NO_FLUSH);

	*data_size = data_length;

	return ret;
}

char* LoadShaderSource(
	APPLICATION* application,
	eSHADER_TYPE shader_type,
	MODEL_INTERFACE* model,
	void* user_data
)
{
	FILE *fp;
	long data_size;
	char *ret;
	char file_path[8192] = {0};
	char *system_dir = g_locale_from_utf8(
		application->paths.shader_directory_path, -1, NULL, NULL, NULL);

	(void)strcat(file_path, system_dir);
	g_free(system_dir);

	switch(model->type)
	{
	case MODEL_TYPE_ASSET_MODEL:
		(void)strcat(file_path, "/asset");
		break;
	case MODEL_TYPE_PMD_MODEL:
	case MODEL_TYPE_PMX_MODEL:
		(void)strcat(file_path, "/pmx");
		break;
	}

	switch(shader_type)
	{
	case SHADER_TYPE_EDGE_VERTEX:
		(void)strcat(file_path, "/edge.vsh");
		break;
	case SHADER_TYPE_EDGE_FRAGMENT:
		(void)strcat(file_path, "/edge.fsh");
		break;
	case SHADER_TYPE_MODEL_VERTEX:
		(void)strcat(file_path, "/model.vsh");
		break;
	case SHADER_TYPE_MODEL_FRAGMENT:
		(void)strcat(file_path, "/model.fsh");
		break;
	case SHADER_TYPE_SHADOW_VERTEX:
		(void)strcat(file_path, "/shadow.vsh");
		break;
	case SHADER_TYPE_SHADOW_FRAGMENT:
		(void)strcat(file_path, "/shadow.fsh");
		break;
	case SHADER_TYPE_Z_PLOT_VERTEX:
		(void)strcat(file_path, "/zplot.vsh");
		break;
	case SHADER_TYPE_Z_PLOT_FRAGMENT:
		(void)strcat(file_path, "/zplot.fsh");
		break;
	case SHADER_TYPE_EDGE_WITH_SKINNING_VERTEX:
		(void)strcat(file_path, "/skinning/edge.vsh");
		break;
	case SHADER_TYPE_MODEL_WITH_SKINNING_VERTEX:
		(void)strcat(file_path, "/skinning/model.vsh");
		break;
	case SHADER_TYPE_SHADOW_WITH_SKINNING_VERTEX:
		(void)strcat(file_path, "/skinning/shadow.vsh");
		break;
	case SHADER_TYPE_Z_PLOT_WITH_SKINNING_VERTEX:
		(void)strcat(file_path, "/skinning/zplot.vsh");
		break;
	case SHADER_TYPE_TRANSFORM_FEEDBACK_VERTEX:
		(void)strcat(file_path, "transform.vsh");
		break;
	default:
		return NULL;
	}

	fp = fopen(file_path, "rb");
	if(fp != NULL)
	{
		(void)fseek(fp, 0, SEEK_END);
		data_size = ftell(fp);
		rewind(fp);

		ret = (char*)MEM_ALLOC_FUNC(data_size);
		(void)fread(ret, 1, data_size, fp);

		(void)fclose(fp);
	}
	else
	{
		ret = LoadSubstituteShaderSource(file_path);
	}

	return ret;
}

int FindTextureCache(const char* path, TEXTURE_DATA_BRIDGE* bridge, APPLICATION* application)
{
	void *value;

	value = ght_get(application->texture_chache_map, (unsigned int)strlen(path), path);
	if(value != NULL)
	{
		bridge->texture = (TEXTURE_INTERFACE*)value;

		return TRUE;
	}

	return FALSE;
}

#define LOAD_DEFAULT_FORMAT_RGB(FORMAT) \
	(FORMAT).external = GL_RGB; \
	(FORMAT).internal = GL_RGB8; \
	(FORMAT).type = GL_UNSIGNED_BYTE; \
	(FORMAT).target = GL_TEXTURE_2D;

#define LOAD_DEFAULT_FORMAT_RGBA(FORMAT) \
	(FORMAT).external = GL_RGBA; \
	(FORMAT).internal = GL_RGBA8; \
	(FORMAT).type = GL_UNSIGNED_INT_8_8_8_8_REV; \
	(FORMAT).target = GL_TEXTURE_2D;

TEXTURE_INTERFACE* CreateTexture(
	uint8* pixels,
	TEXTURE_FORMAT* format,
	int* size,
	int mipmap
)
{
	TEXTURE_2D *ret = (TEXTURE_2D*)MEM_ALLOC_FUNC(sizeof(*ret));
	InitializeTexture2D(ret, format, size, 0);
	BaseTextureMake(&ret->base_data);
	BaseTextureBind(&ret->base_data);
	if(CheckHasExtension("ARB_texture_storage"))
	{
		glTexStorage2D(format->target, 1, format->internal, size[0], size[1]);
		glTexSubImage2D(format->target, 0, 0, 0, size[0], size[1], format->external, format->type, pixels);
	}
	else
	{
		glTexImage2D(format->target, 0, format->internal, size[0], size[1], 0, format->external, format->type, pixels);
	}
	BaseTextureUnbind(&ret->base_data);

	return (TEXTURE_INTERFACE*)ret;
}

int CacheTexture(
	const char* key,
	TEXTURE_INTERFACE* texture,
	TEXTURE_DATA_BRIDGE* bridge,
	APPLICATION* application
)
{
	GLuint name;

	if(key == NULL)
	{
		return FALSE;
	}

	name = texture->name;
	glBindTexture(GL_TEXTURE_2D, name);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	if((bridge->flags & TEXTURE_FLAG_SYSTEM_TOON_TEXTURE) != 0)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	bridge->texture = texture;
	if(texture != NULL)
	{
		(void)ght_insert(application->texture_chache_map, texture, (unsigned int)strlen(key), (void*)key);
	}

	return TRUE;
}

int UploadTexture(const char* path, TEXTURE_DATA_BRIDGE* bridge, APPLICATION* application)
{
	TEXTURE_INTERFACE *texture;
	TEXTURE_FORMAT format;
	uint8 *pixels;
	int size[3];
	int channel;

	if(path[strlen(path)-1] == '/')
	{
		return TRUE;
	}
	else if(FindTextureCache(path, bridge, application) != FALSE)
	{
		return TRUE;
	}

	pixels = LoadImage(path, &size[0], &size[1], &channel);
	if(pixels == NULL)
	{
		return FALSE;
	}

	if(channel == 3)
	{
		LOAD_DEFAULT_FORMAT_RGB(format);
	}
	else
	{
		LOAD_DEFAULT_FORMAT_RGBA(format);
	}

	texture = CreateTexture(pixels, &format, size, bridge->flags & TEXTURE_FLAG_GENERATE_TEXTURE_MIPMAP);

	MEM_FREE_FUNC(pixels);

	return CacheTexture(path, texture, bridge, application);
}

int UploadWhiteTexture(int width, int height, APPLICATION* application)
{
	TEXTURE_DATA_BRIDGE bridge;
	TEXTURE_FORMAT format;
	TEXTURE_INTERFACE *texture;
	int size[3];
	uint8 *pixels;

	if(FindTextureCache(WHITE_TEXTURE_NAME, &bridge, application) != FALSE)
	{
		return TRUE;
	}

	pixels = (uint8*)MEM_ALLOC_FUNC(width * height * 4);
	(void)memset(pixels, 0xff, width * height * 4);
	LOAD_DEFAULT_FORMAT_RGBA(format);

	size[0] = width,	size[1] = height;
	texture = CreateTexture(pixels, &format, size, 0);

	MEM_FREE_FUNC(pixels);

	return CacheTexture(WHITE_TEXTURE_NAME, texture, &bridge, application);
}
