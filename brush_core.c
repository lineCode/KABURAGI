#include <string.h>
#include <gtk/gtk.h>
#include "application.h"
#include "brush_core.h"
#include "memory_stream.h"
#include "layer.h"
#include "memory.h"

#ifdef __cplusplus
extern "C" {
#endif

void ChangeBrush(
	BRUSH_CORE* core,
	void* brush_data,
	brush_core_func press_func,
	brush_core_func motion_func,
	brush_core_func release_func
)
{
	core->brush_data = brush_data;
	core->press_func = press_func;
	core->motion_func = motion_func;
	core->release_func = release_func;

	BrushCorePointReset(core);
}

void BrushCorePointReset(BRUSH_CORE* core)
{
	core->min_x = -1.0;
	core->min_y = -1.0;
	core->max_x = -1.0;
	core->max_y = -1.0;
}

/***************************************
* BrushCoreSetCirclePattern�֐�        *
* �u���V�̉~�`�摜�p�^�[�����쐬       *
* ����                                 *
* core				: �u���V�̊�{��� *
* r					: ���a             *
* outline_hardness	: �֊s�̍d��       *
* blur				: �{�P��           *
* alpha				: �s�����x         *
* color				: �F               *
***************************************/
void BrushCoreSetCirclePattern(
	BRUSH_CORE* core,
	FLOAT_T r,
	FLOAT_T outline_hardness,
	FLOAT_T blur,
	FLOAT_T alpha,
	const uint8 color[3]
)
{
	cairo_t *cairo_p;
	cairo_pattern_t *radial_pattern;
	FLOAT_T float_color[3] = {color[0] * DIV_PIXEL, color[1] * DIV_PIXEL, color[2] * DIV_PIXEL};

	if(core->brush_pattern != NULL)
	{
		cairo_pattern_destroy(core->brush_pattern);
	}

	if(core->brush_surface != NULL)
	{
		cairo_surface_destroy(core->brush_surface);
	}

	if(core->temp_pattern != NULL)
	{
		cairo_pattern_destroy(core->temp_pattern);
	}

	if(core->temp_surface != NULL)
	{
		cairo_surface_destroy(core->temp_surface);
	}

	if(core->temp_cairo != NULL)
	{
		cairo_destroy(core->temp_cairo);
	}

	core->brush_surface = cairo_image_surface_create_for_data(*core->brush_pattern_buff,
		CAIRO_FORMAT_ARGB32, (int)(r*2+1), (int)(r*2+1), (int)(r*2+1)*4);
	cairo_p = cairo_create(core->brush_surface);
	cairo_set_operator(cairo_p, CAIRO_OPERATOR_SOURCE);
	radial_pattern = cairo_pattern_create_radial(r, r, 0, r, r, r);
	cairo_pattern_set_extend(radial_pattern, CAIRO_EXTEND_NONE);
	cairo_pattern_add_color_stop_rgba(radial_pattern, 0, float_color[0],
		float_color[1], float_color[2], alpha);
	cairo_pattern_add_color_stop_rgba(radial_pattern, 1.0-blur, float_color[0],
		float_color[1], float_color[2], alpha);
	cairo_pattern_add_color_stop_rgba(radial_pattern, 1, float_color[0],
		float_color[1], float_color[2], alpha * outline_hardness);
	cairo_set_source(cairo_p, radial_pattern);
	cairo_paint(cairo_p);

	core->temp_surface = cairo_image_surface_create_for_data(*core->temp_pattern_buff,
		CAIRO_FORMAT_ARGB32, (int)(r*2+1), (int)(r*2+1), (int)(r*2+1)*4);
	core->temp_cairo = cairo_create(core->temp_surface);
	cairo_set_operator(core->temp_cairo, CAIRO_OPERATOR_SOURCE);
	core->temp_pattern = cairo_pattern_create_for_surface(core->temp_surface);

	core->brush_pattern = cairo_pattern_create_for_surface(core->brush_surface);

	cairo_pattern_destroy(radial_pattern);
	cairo_destroy(cairo_p);
}

/***********************************************
* BrushCoreSetGrayCirclePattern�֐�            *
* �u���V�̃O���[�X�P�[���~�`�摜�p�^�[�����쐬 *
* ����                                         *
* core				: �u���V�̊�{���         *
* r					: ���a                     *
* outline_hardness	: �֊s�̍d��               *
* blur				: �{�P��                   *
* alpha				: �s�����x                 *
***********************************************/
void BrushCoreSetGrayCirclePattern(
	BRUSH_CORE* core,
	FLOAT_T r,
	FLOAT_T outline_hardness,
	FLOAT_T blur,
	FLOAT_T alpha
)
{
	cairo_t *cairo_p;
	cairo_pattern_t *radial_pattern;
	int width = (int)(r * 2 + 1);

	core->stride = width + (4 - (width % 4)) % 4;

	if(core->brush_pattern != NULL)
	{
		cairo_pattern_destroy(core->brush_pattern);
	}

	if(core->brush_surface != NULL)
	{
		cairo_surface_destroy(core->brush_surface);
	}

	if(core->temp_pattern != NULL)
	{
		cairo_pattern_destroy(core->temp_pattern);
	}

	if(core->temp_surface != NULL)
	{
		cairo_surface_destroy(core->temp_surface);
	}

	if(core->temp_cairo != NULL)
	{
		cairo_destroy(core->temp_cairo);
	}

	core->brush_surface = cairo_image_surface_create_for_data(*core->brush_pattern_buff,
		CAIRO_FORMAT_A8, width, width, core->stride);
	cairo_p = cairo_create(core->brush_surface);
	cairo_set_operator(cairo_p, CAIRO_OPERATOR_SOURCE);
	radial_pattern = cairo_pattern_create_radial(r, r, 0, r, r, r);
	cairo_pattern_add_color_stop_rgba(radial_pattern, 0, 0, 0, 0, alpha);
	cairo_pattern_add_color_stop_rgba(radial_pattern, 1-blur, 0, 0, 0, alpha);
	cairo_pattern_add_color_stop_rgba(radial_pattern, 1, 0, 0, 0, alpha*outline_hardness);
	cairo_pattern_set_extend(radial_pattern, CAIRO_EXTEND_NONE);
	cairo_set_source(cairo_p, radial_pattern);
	cairo_paint(cairo_p);

	core->temp_surface = cairo_image_surface_create_for_data(*core->temp_pattern_buff,
		CAIRO_FORMAT_A8, width, width, core->stride);
	core->temp_cairo = cairo_create(core->temp_surface);
	cairo_set_operator(core->temp_cairo, CAIRO_OPERATOR_SOURCE);
	core->temp_pattern = cairo_pattern_create_for_surface(core->temp_surface);
	cairo_pattern_set_extend(core->temp_pattern, CAIRO_EXTEND_NONE);

	core->brush_pattern = cairo_pattern_create_for_surface(core->brush_surface);

	cairo_pattern_destroy(radial_pattern);
	cairo_destroy(cairo_p);
}


typedef struct _BRUSH_HISTORY_DATA
{
	int32 x, y;
	int32 width, height;
	int32 name_len;
	gchar *layer_name;
	uint8 *pixels;
} BRUSH_HISTORY_DATA;

void AddBrushHistory(
	BRUSH_CORE* core,
	LAYER* active
)
{
	BRUSH_HISTORY_DATA data;
	MEMORY_STREAM_PTR stream;
	int i;

	data.x = (int32)core->min_x - 1;
	data.y = (int32)core->min_y - 1;
	data.width = (int32)(core->max_x + 1.5 - core->min_x);
	data.height = (int32)(core->max_y + 1.5 - core->min_y);
	if(data.x < 0) data.x = 0;
	else if(data.x > active->width) return;
	if(data.y < 0) data.y = 0;
	else if(data.y > active->height) return;
	if (data.width < 0) return;
	else if(data.x + data.width > active->width) data.width = active->width - data.x;
	if(data.height < 0) return;
	else if(data.y + data.height > active->height) data.height = active->height - data.y;
	data.name_len = (int32)strlen(active->name) + 1;

	stream = CreateMemoryStream(
		offsetof(BRUSH_HISTORY_DATA, layer_name)
		+ data.name_len+data.height*data.width*active->channel);
	(void)MemWrite(&data, offsetof(BRUSH_HISTORY_DATA, layer_name), 1, stream);
	(void)MemWrite(active->name, 1, data.name_len, stream);
	for(i=0; i<data.height; i++)
	{
		(void)MemWrite(&active->pixels[(data.y+i)*active->stride+data.x*active->channel],
			1, data.width * active->channel, stream);
	}
	AddHistory(
		&active->window->history,
		core->name,
		stream->buff_ptr,
		(uint32)stream->data_size,
		BrushCoreUndoRedo,
		BrushCoreUndoRedo
	);
	(void)DeleteMemoryStream(stream);
}

void BrushCoreUndoRedo(DRAW_WINDOW* window, void* p)
{
	BRUSH_HISTORY_DATA data;
	LAYER* layer = window->layer;
	uint8* buff = (uint8*)p;
	uint8* before_data;
	int i;

	(void)memcpy(&data, buff, offsetof(BRUSH_HISTORY_DATA, layer_name));
	buff += offsetof(BRUSH_HISTORY_DATA, layer_name);
	data.layer_name = (gchar*)buff;
	buff += data.name_len;
	data.pixels = buff;

	while(strcmp(layer->name, data.layer_name) != 0)
	{
		layer = layer->next;
	}

	before_data = (uint8*)MEM_ALLOC_FUNC(data.height*data.width*layer->channel);
	for(i=0; i<data.height; i++)
	{
		(void)memcpy(&before_data[i*data.width*layer->channel],
			&layer->pixels[(data.y+i)*layer->stride+data.x*layer->channel],
			data.width*layer->channel);
	}

	for(i=0; i<data.height; i++)
	{
		(void)memcpy(&layer->pixels[(data.y+i)*layer->stride+data.x*layer->channel],
			&data.pixels[i*data.width*layer->channel], data.width*layer->channel);
	}

	(void)memcpy(data.pixels, before_data, data.height*data.width*layer->channel);

	MEM_FREE_FUNC(before_data);
}

typedef struct _EDIT_SELECTION_DATA
{
	int32 x, y;
	int32 width, height;
	uint8 *pixels;
} EDIT_SELECTION_DATA;

void EditSelectionUndoRedo(DRAW_WINDOW* window, void* p)
{
	EDIT_SELECTION_DATA *data = (EDIT_SELECTION_DATA*)p;
	uint8 *buff = (uint8*)p;
	uint8 *pixels, *src, *before_data;
	int i;

	before_data = (uint8*)MEM_ALLOC_FUNC(data->width*data->height);
	src = &window->selection->pixels[data->y*window->selection->stride+data->x];
	pixels = before_data;
	for(i=0; i<data->height; i++)
	{
		(void)memcpy(pixels, src, data->width);
		pixels += data->width;
		src += window->selection->stride;
	}

	pixels = &window->selection->pixels[data->y*window->selection->stride+data->x];
	src = &buff[offsetof(EDIT_SELECTION_DATA, pixels)];
	for(i=0; i<data->height; i++)
	{
		(void)memcpy(pixels, src, data->width);
		pixels += window->selection->stride;
		src += data->width;
	}

	(void)memcpy(&buff[offsetof(EDIT_SELECTION_DATA, pixels)], before_data,
		data->width * data->height);

	MEM_FREE_FUNC(before_data);

	if((window->flags & DRAW_WINDOW_EDIT_SELECTION) == 0)
	{
		if(UpdateSelectionArea(&window->selection_area, window->selection, window->temp_layer) == FALSE)
		{
			window->flags &= ~(DRAW_WINDOW_HAS_SELECTION_AREA);
		}
		else
		{
			window->flags |= DRAW_WINDOW_HAS_SELECTION_AREA;
		}
	}
}

void AddSelectionEditHistory(BRUSH_CORE* core, LAYER* selection)
{
	EDIT_SELECTION_DATA data;
	uint8 *buff, *pixels, *src;
	int i;

	data.x = (int32)core->min_x - 1;
	data.y = (int32)core->min_y - 1;
	data.width = (int32)(core->max_x + 1 - core->min_x);
	data.height = (int32)(core->max_y + 1 - core->min_y);
	if(data.x < 0) data.x = 0;
	if(data.y < 0) data.y = 0;
	if(data.x + data.width > selection->width) data.width = selection->width - data.x;
	if(data.y + data.height > selection->height) data.height = selection->height - data.y;

	buff = (uint8*)MEM_ALLOC_FUNC(offsetof(EDIT_SELECTION_DATA, pixels) + data.width*data.height);

	(void)memcpy(buff, &data, offsetof(EDIT_SELECTION_DATA, pixels));

	pixels = &buff[offsetof(EDIT_SELECTION_DATA, pixels)];
	src = &selection->pixels[data.y*selection->stride+data.x];
	for(i=0; i<data.height; i++)
	{
		(void)memcpy(pixels, src, data.width);
		pixels += data.width;
		src += selection->stride;
	}

	AddHistory(
		&selection->window->history,
		core->name,
		buff,
		offsetof(EDIT_SELECTION_DATA, pixels) + data.width * data.height,
		EditSelectionUndoRedo,
		EditSelectionUndoRedo
	);

	MEM_FREE_FUNC(buff);
}

/*************************************************
* DrawCircleBrushWorkLayer�֐�                   *
* �u���V����ƃ��C���[�ɕ`�悷��                 *
* ����                                           *
* window	: �L�����o�X                         *
* core		: �u���V�̊�{���                   *
* x			: �`��͈͂̍����X���W              *
* y			: �`��͈͂̍����Y���W              *
* width		: �`��͈͂̕�                       *
* height	: �`��͈͂̍���                     *
* mask		: ��ƃ��C���[�ɃR�s�[����ۂ̃}�X�N *
* zoom		: �g��E�k����                       *
* alpha		: �s�����x                           *
*************************************************/
void DrawCircleBrushWorkLayer(
	DRAW_WINDOW* window,
	BRUSH_CORE* core,
	gdouble x,
	gdouble y,
	gdouble width,
	gdouble height,
	uint8** mask,
	gdouble zoom,
	gdouble alpha
)
{
	// �`����s�����߂�Cairo���
	cairo_t *update;
	cairo_surface_t *update_surface;
	// �`�掞�̊g��E�k���A�ʒu�ݒ�p
	cairo_matrix_t matrix;
	// �摜��s���̃o�C�g��
	int stride = (int)width * 4;
	// �s�N�Z���f�[�^�����Z�b�g������W
	int start_x = (int)x, start_y = (int)y;
	// for���p�̃J�E���^
	int i;

	// �`��p��Cairo�쐬
	update_surface = cairo_surface_create_for_rectangle(
		window->mask_temp->surface_p, x, y,
			width, height);
	update = cairo_create(update_surface);

	*mask = window->mask_temp->pixels;
	if(window->app->textures.active_texture == 0)
	{	// �e�N�X�`����	
		if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) == 0)
		{	// �I��͈͖�
			if((window->active_layer->flags & LAYER_LOCK_OPACITY) == 0)
			{	// �s�����ی얳
					// �u���V�̃T�C�Y��ݒ肵�ĕ`��
				cairo_matrix_init_scale(&matrix, zoom, zoom);
				cairo_pattern_set_matrix(core->brush_pattern, &matrix);
				cairo_set_source(update, core->brush_pattern);
				cairo_paint_with_alpha(update, alpha);
			}
			else
			{	// �s�����ی�L
					// �u���V�̃T�C�Y��ݒ肵��
				cairo_matrix_init_scale(&matrix, zoom, zoom);
				cairo_pattern_set_matrix(core->brush_pattern, &matrix);
				cairo_set_source(core->temp_cairo, core->brush_pattern);
				// ��x�A�ꎞ�ۑ��̈�ɕ`��
				cairo_paint_with_alpha(core->temp_cairo, alpha);
				// �`�悷��ʒu�����Z�b�g
				cairo_matrix_init_translate(&matrix, 0,0);
				cairo_pattern_set_matrix(core->temp_pattern, &matrix);
				// �`�挋�ʂ��g����
				cairo_set_source(update, core->temp_pattern);
				// �A�N�e�B�u�ȃ��C���[�Ń}�X�N
				cairo_mask_surface(update,
					window->active_layer->surface_p, - x, - y);
			}
		}
		else
		{	// �I��͈͗L
			if((window->active_layer->flags & LAYER_LOCK_OPACITY) == 0)
			{	// �s�����ی얳
					// �u���V�̃T�C�Y��ݒ肵��
				cairo_matrix_init_scale(&matrix, zoom, zoom);
				cairo_pattern_set_matrix(core->brush_pattern, &matrix);
				cairo_set_source(core->temp_cairo, core->brush_pattern);
				// ��x�ꎞ�ۑ��̈�ɕ`��
				cairo_paint_with_alpha(core->temp_cairo, alpha);
				// �`�悷��ʒu�����Z�b�g
				cairo_matrix_init_translate(&matrix, 0, 0);
				cairo_pattern_set_matrix(core->temp_pattern, &matrix);
				// �`�挋�ʂ��g����
				cairo_set_source(update, core->temp_pattern);
				// �I��͈͂Ń}�X�N
				cairo_mask_surface(window->mask_temp->cairo_p,
					window->selection->surface_p, - x, - y);
			}
			else
			{
				// �I��͈͗L
					// �A�N�e�B�u�ȃ��C���[�ƑI��͈͂Ń}�X�N����
				// �`��p�Ɉꎞ�I��Cairo���쐬
				cairo_surface_t *temp_surface = cairo_surface_create_for_rectangle(
					window->temp_layer->surface_p, x, y, width, height);
				cairo_t *update_temp = cairo_create(temp_surface);

				// �܂��͈ꎞ�ۑ����C���[�ɑI��͈͂Ń}�X�N���ĕ`��
				cairo_matrix_init_scale(&matrix, zoom, zoom);
				cairo_pattern_set_matrix(core->brush_pattern, &matrix);
				cairo_set_source(core->temp_cairo, core->brush_pattern);
				cairo_paint_with_alpha(core->temp_cairo, alpha);
				cairo_matrix_init_translate(&matrix, 0, 0);
				cairo_pattern_set_matrix(core->temp_pattern, &matrix);
				cairo_set_source(update, core->temp_pattern);

				// �`��O�Ƀs�N�Z���f�[�^�����������Ă���
				for(i=0; i<height; i++)
				{
					(void)memset(&window->temp_layer->pixels[
						(i+start_y)*window->temp_layer->stride+start_x*4],
						0, stride
					);
				}

				cairo_mask_surface(update,
					window->selection->surface_p, - x, - y);
				cairo_set_source_surface(update_temp,
					update_surface, 0, 0);
				cairo_mask_surface(update_temp,
					window->active_layer->surface_p, - x, - y);

				// �g�p����}�X�N��ύX
				*mask = window->temp_layer->pixels;

				// �ꎞ�I�ɍ쐬����Cairo��j��
				cairo_surface_destroy(temp_surface);
				cairo_destroy(update_temp);
			}
		}
	}
	else
	{	// �e�N�X�`���L
			// �ꎞ�ۑ��p��Cairo�����쐬
		cairo_surface_t *temp_surface = cairo_surface_create_for_rectangle(
			window->temp_layer->surface_p, x, y, width, height);
		cairo_t *update_temp = cairo_create(temp_surface);

		// �`��O�Ɉꎞ�ۑ����C���[�̃s�N�Z���f�[�^��������
		for(i=0; i<height; i++)
		{
			(void)memset(&window->temp_layer->pixels[
				(i+start_y)*window->temp_layer->stride+start_x*4],
				0, stride
			);
		}

		if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) == 0)
		{	// �I��͈͖�
			if((window->active_layer->flags & LAYER_LOCK_OPACITY) == 0)
			{	// �s�����ی얳
					// �g��E�k�����A�s�����x��ݒ肵�ĕ`��
				cairo_matrix_init_scale(&matrix, zoom, zoom);
				cairo_pattern_set_matrix(core->brush_pattern, &matrix);
				cairo_set_source(update_temp, core->brush_pattern);
				cairo_paint_with_alpha(update_temp, alpha);
			}
			else
			{	// �s�����ی�L
					// �A�N�e�B�u�ȃ��C���[�Ń}�X�N���ĕ`��
				cairo_matrix_init_scale(&matrix, zoom, zoom);
				cairo_pattern_set_matrix(core->brush_pattern, &matrix);
				cairo_set_source(core->temp_cairo, core->brush_pattern);
				cairo_paint_with_alpha(core->temp_cairo, alpha);
				cairo_matrix_init_translate(&matrix, 0, 0);
				cairo_pattern_set_matrix(core->temp_pattern, &matrix);
				cairo_set_source(update_temp, core->temp_pattern);
				cairo_mask_surface(update_temp,
					window->active_layer->surface_p, - x, - y);
			}

			cairo_set_source_surface(update, temp_surface, 0, 0);
			cairo_mask_surface(update, window->texture->surface_p, - x, - y);

			*mask = window->temp_layer->pixels;
		}
		else
		{	// �I��͈͗L
			if((window->active_layer->flags & LAYER_LOCK_OPACITY) == 0)
			{	// �s�����ی얳
					// �I��͈͂ƃe�N�X�`���Ń}�X�N
				// �܂��͈ꎞ�ۑ��̈�ɕs�����x���w�肵�ĕ`�悵�Ă���
				cairo_matrix_init_scale(&matrix, zoom, zoom);
				cairo_pattern_set_matrix(core->brush_pattern, &matrix);
				cairo_set_source(core->temp_cairo, core->brush_pattern);
				cairo_paint_with_alpha(core->temp_cairo, alpha);
				// �`�挋�ʂ��g����
				cairo_matrix_init_translate(&matrix, 0, 0);
				cairo_pattern_set_matrix(core->temp_pattern, &matrix);
				cairo_set_source(update, core->temp_pattern);
				// �I��͈͂Ń}�X�N
				cairo_mask_surface(update,
					window->selection->surface_p, - x, - y);

				// �ꎞ�ۑ��̈�̃s�N�Z���f�[�^����x���Z�b�g����
				for(i=0; i<height; i++)
				{
					(void)memset(&window->temp_layer->pixels[
						(i+start_y)*window->temp_layer->stride+start_x*4],
						0, stride
					);
				}

				// �e�N�X�`���Ń}�X�N���ĕ`��
				cairo_set_source_surface(update_temp, update_surface, 0, 0);
				cairo_mask_surface(update_temp, window->texture->surface_p, - x, - y);
			}
			else
			{	// �s�����ی�L
					// �I��͈́A�e�N�X�`���A�A�N�e�B�u�ȃ��C���[�Ń}�X�N
				// �܂��͕s�����x��ݒ肵�ĕ`�悵�Ă���
				cairo_matrix_init_scale(&matrix, zoom, zoom);
				cairo_pattern_set_matrix(core->brush_pattern, &matrix);
				cairo_set_source(core->temp_cairo, core->brush_pattern);
				cairo_paint_with_alpha(core->temp_cairo, alpha);
				// �`�挋�ʂ��g����
				cairo_matrix_init_translate(&matrix, 0, 0);
				cairo_pattern_set_matrix(core->temp_pattern, &matrix);
				cairo_set_source(update, core->temp_pattern);

				// �`��O�Ɉꎞ�ۑ��̈�̃s�N�Z���f�[�^��������
				for(i=0; i<height; i++)
				{
					(void)memset(&window->temp_layer->pixels[
						(i+start_y)*window->temp_layer->stride+start_x*4],
						0, stride
					);
				}

				// �I��͈͂Ń}�X�N
				cairo_mask_surface(update,
					window->selection->surface_p, - x, - y);
				cairo_set_source_surface(update_temp,
					update_surface, 0, 0);
				// �A�N�e�B�u�ȃ��C���[�Ń}�X�N
				cairo_mask_surface(update_temp,
					window->active_layer->surface_p, - x, - y);

				// 2�߂̈ꎞ�ۑ��̈�̃s�N�Z���f�[�^��������
				for(i=0; i<height; i++)
				{
					(void)memset(&window->mask_temp->pixels[
						(i+start_y)*window->mask_temp->stride+start_x*4],
						0, stride
					);
				}

				// �e�N�X�`���Ń}�X�N���ĕ`��
				cairo_set_source_surface(update, temp_surface, 0, 0);
				cairo_mask_surface(update, window->texture->surface_p, - x, - y);
			}
		}

		// �ꎞ�I�ɍ쐬����Cairo����j��
		cairo_surface_destroy(temp_surface);
		cairo_destroy(update_temp);
	}

	// �X�V�p�ɍ쐬����Cairo����j��
	cairo_surface_destroy(update_surface);
	cairo_destroy(update);
}

/*****************************************************
* SetBrushBaseScale�֐�                              *
* �u���V�T�C�Y�̔{����ݒ肷��                       *
* ����                                               *
* widget	: �{���ݒ�p�̃R���{�{�b�N�X�E�B�W�F�b�g *
* index		: �{���̃C���f�b�N�X��ێ�����A�h���X   *
*****************************************************/
void SetBrushBaseScale(GtkWidget* widget, int* index)
{
	GtkWidget *scale = GTK_WIDGET(g_object_get_data(
		G_OBJECT(widget), "scale"));

	*index = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));

	switch(*index)
	{
	case 0:
		SpinScaleSetScaleLimits((SPIN_SCALE*)scale, 0.1, 10);
		SpinScaleSetStep((SPIN_SCALE*)scale, 0.1);
		SpinScaleSetPage((SPIN_SCALE*)scale, 0.1);
		break;
	case 1:
		SpinScaleSetScaleLimits((SPIN_SCALE*)scale, 1, 100);
		SpinScaleSetStep((SPIN_SCALE*)scale, 1);
		SpinScaleSetPage((SPIN_SCALE*)scale, 1);
		break;
	case 2:
		SpinScaleSetScaleLimits((SPIN_SCALE*)scale, 5, 500);
		SpinScaleSetStep((SPIN_SCALE*)scale, 1);
		SpinScaleSetPage((SPIN_SCALE*)scale, 1);
		break;
	}
}

/***************************************
* DefaultToolUpdate�֐�                *
* �f�t�H���g�̃c�[���A�b�v�f�[�g�̊֐� *
* ����                                 *
* window	: �A�N�e�B�u�ȕ`��̈�     *
* x			: �}�E�X�J�[�\����X���W    *
* y			: �}�E�X�J�[�\����Y���W    *
* dummy		: �_�~�[�|�C���^           *
***************************************/
void DefaultToolUpdate(DRAW_WINDOW* window, gdouble x, gdouble y, void* dummy)
{
	window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_OVER;
	gtk_widget_queue_draw(window->window);
}

#ifdef __cplusplus
}
#endif
