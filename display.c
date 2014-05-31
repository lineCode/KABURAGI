#include "configure.h"
#include <string.h>
#include <gtk/gtk.h>
#if defined(USE_3D_LAYER) && USE_3D_LAYER != 0
# include <GL/glew.h>
# include <gtk/gtkgl.h>
#endif
#include "application.h"
#include "display.h"
#include "draw_window.h"
#include "transform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _eUPDATE_MODE
{
	NO_UPDATE,
	UPDATE_ALL,
	UPDATE_PART
} eUPDATE_MODE;

/*****************************************
* DisplayDrawWindow�֐�                  *
* �`��̈�̉�ʍX�V����                 *
* ����                                   *
* widget		: �`��̈�̃E�B�W�F�b�g *
* event_info	: �`��X�V�̏��         *
* window		: �`��̈�̏��         *
* �Ԃ�l                                 *
*	���FALSE                            *
*****************************************/
gboolean DisplayDrawWindow(
	GtkWidget *widget,
	GdkEventExpose *event_info,
	struct _DRAW_WINDOW *window
)
{
	// �������郌�C���[
	LAYER *layer = NULL, *blend_layer;
	// �}�E�X�̏��擾�p
	GdkModifierType state;
	// �g��E�k�������ɖ߂����߂̒l
	gdouble rev_zoom = 1.0 / (window->zoom * 0.01);
	// for���p�̃J�E���^
	gint x, y;
	// ��ʍX�V�̃��[�h
	eUPDATE_MODE update_mode = UPDATE_ALL;
	// �A�N�e�B�u���C���[��艺�̍������ʂ��X�V����t���O
	int update_active_under = 0;
	// �������[�h
	int blend_mode;

	// ��ʕ\���pCairo���
	cairo_t *cairo_p;

	state = window->state;

	// �I��͈͂̕ҏW���łȂ����
	if((window->flags & DRAW_WINDOW_EDIT_SELECTION) == 0)
	{
		// �`��̈�̍X�V�t���O�̏�Ԃŕ���
		if((window->flags & DRAW_WINDOW_UPDATE_PART) == 0)
		{
			if((window->flags & DRAW_WINDOW_UPDATE_ACTIVE_UNDER) != 0)
			{	// �S���C���[������
				(void)memcpy(window->mixed_layer->pixels, window->back_ground, window->pixel_buf_size);
				// ��������ŏ��̃��C���[�͈�ԉ��̃��C���[
				layer = window->layer;
			}
			else if((window->flags & DRAW_WINDOW_UPDATE_ACTIVE_OVER) != 0)
			{	// �A�N�e�B�u���C���[�Ƃ��̏�̃��C���[������
					// ��������ŏ��̃��C���[�̓A�N�e�B�u���C���[
				if(window->active_layer == window->layer)
				{
					(void)memcpy(window->mixed_layer->pixels, window->back_ground, window->pixel_buf_size);
				}
				else
				{
					(void)memcpy(window->mixed_layer->pixels, window->under_active->pixels, window->pixel_buf_size);
				}
				layer = window->active_layer;
			}
			else
			{	// �X�V�̃t���O�������Ă��Ȃ��̂Ń��C���[�̍�������
				update_mode = NO_UPDATE;
			}
		}
		else
		{
			int stride;
			int end_y;
			if(window->update.x < 0)
			{
				window->update.x = 0;
			}
			stride = (int)((window->update.x + window->update.width < window->width) ?
				window->update.width * 4 : (window->width - window->update.x) * 4);
			if(stride > 0 && window->update.height > 0)
			{
				if(window->update.y < 0)
				{
					window->update.y = 0;
				}

				if(window->update.x + window->update.width > window->width)
				{
					window->update.width = window->width - window->update.x;
				}

				if(window->update.y + window->update.height <= window->height)
				{
					end_y = (int)window->update.height;
				}
				else
				{
					end_y = (int)(window->update.height = window->height - window->update.y);
				}

				if(end_y >= 0)
				{
					if(window->active_layer == window->layer)
					{	// �A�N�e�B�u���C���[����ԉ��Ȃ�Δw�i�̃s�N�Z���f�[�^���R�s�[
						for(y=0; y<end_y; y++)
						{
							(void)memcpy(&window->mixed_layer->pixels[(y+(int)window->update.y)*window->mixed_layer->stride+(int)window->update.x*4],
								&window->back_ground[(y+(int)window->update.y)*window->mixed_layer->stride+(int)window->update.x*4], stride);
						}
					}
					else
					{	// �����łȂ���΃A�N�e�B�u���C���[��艺�̍����ς݂̃f�[�^���R�s�[
						for(y=0; y<end_y; y++)
						{
							(void)memcpy(&window->mixed_layer->pixels[(y+(int)window->update.y)*window->mixed_layer->stride+(int)window->update.x*4],
								&window->under_active->pixels[(y+(int)window->update.y)*window->mixed_layer->stride+(int)window->update.x*4], stride);
						}
					}
					layer = window->active_layer;
				}
			}
			else
			{
				window->flags &= ~(DRAW_WINDOW_UPDATE_PART);
				goto execute_update;
			}

			window->update.surface_p = cairo_surface_create_for_rectangle(
				window->mixed_layer->surface_p, window->update.x, window->update.y,
					window->update.width, window->update.height);
			window->update.cairo_p = cairo_create(window->update.surface_p);
			window->temp_update = window->update;
			window->temp_update.surface_p = cairo_surface_create_for_rectangle(
				window->temp_layer->surface_p, window->update.x, window->update.y,
					window->update.width, window->update.height);
			window->temp_update.cairo_p = cairo_create(window->temp_update.surface_p);

			update_mode = UPDATE_PART;
		}
	}
	else
	{
		window->flags &= ~(DRAW_WINDOW_UPDATE_PART);
		update_mode = NO_UPDATE;
	}

	if(update_mode == UPDATE_ALL)
	{
		// ��ԏ�̃��C���[�ɒH�蒅���܂Ń��[�v
		while(layer != NULL)
		{
			// ���C���[�Z�b�g���̃��C���[�ł����
			if(layer->layer_set != NULL)
			{	// �S�X�V�Ȃ��
				if((window->flags & DRAW_WINDOW_UPDATE_ACTIVE_UNDER) != 0)
				{	// ���C���[�Z�b�g�����X�V
					MixLayerSet(layer, &layer, window);
				}	// �S�X�V�Ȃ��
					// if((window->flags & DRAW_WINDOW_UPDATE_ACTIVE_UNDER) != 0)
				else if(layer->layer_set == window->active_layer_set)
				{
					MixLayerSetActiveOver(layer, &layer, window);
				}	// else if(layer->layer_set == window->active_layer_set)
				else
				{
					layer = layer->layer_set;
				}
			}	// if(layer->layer_set != NULL)

			// �������C���[�ƍ������@����x�L������
			blend_layer = layer;
			blend_mode = layer->layer_mode;

			// ��\�����C���[�ɂȂ��Ă��Ȃ����Ƃ��m�F
			if((blend_layer->flags & LAYER_FLAG_INVISIBLE) == 0)
			{	// �����A�������郌�C���[���A�N�e�B�u���C���[�Ȃ�
				if(layer == window->active_layer)
				{
					if(layer->layer_type == TYPE_NORMAL_LAYER)
					{	// �ʏ탌�C���[��
							// ��ƃ��C���[�ƃA�N�e�B�u���C���[����x�������Ă��牺�̃��C���[�ƍ���
						(void)memcpy(window->temp_layer->pixels, layer->pixels, layer->stride*layer->height);
						g_layer_blend_funcs[window->work_layer->layer_mode](window->work_layer, window->temp_layer);
						blend_layer = window->temp_layer;
						blend_layer->alpha = layer->alpha;
						blend_layer->flags = layer->flags;
						blend_layer->prev = layer->prev;
					}
					else if(layer->layer_type == TYPE_VECTOR_LAYER)
					{	// �x�N�g�����C���[��
							// ���C���[�̃��X�^���C�Y�������s�Ȃ��Ă����ƃ��C���[�Ɖ��̃��C���[������
						RasterizeVectorLayer(window, layer, layer->layer_data.vector_layer_p);
						if(window->work_layer->layer_mode != LAYER_BLEND_NORMAL)
						{
							g_layer_blend_funcs[window->work_layer->layer_mode](window->work_layer, layer);
						}
					}
					else if(layer->layer_type == TYPE_TEXT_LAYER)
					{	// �e�L�X�g���C���[��
							// �e�L�X�g�̓��e�����X�^���C�Y�������Ă��牺�̃��C���[�ƍ���
						RenderTextLayer(window, layer, layer->layer_data.text_layer_p);
					}

					// �T���l�C���X�V
					if(layer->widget != NULL)
					{
						gtk_widget_queue_draw(layer->widget->thumbnail);
					}
				}

				// ��������Ώۂƕ��@���m�肵���̂ō��������s����
				g_layer_blend_funcs[blend_mode](blend_layer, window->mixed_layer);
				// ����������f�[�^�����ɖ߂�
				window->temp_layer->alpha = 100;
				window->temp_layer->flags = 0;
				window->temp_layer->prev = NULL;
				cairo_set_operator(window->temp_layer->cairo_p, CAIRO_OPERATOR_OVER);
			}	// ��\�����C���[�ɂȂ��Ă��Ȃ����Ƃ��m�F
			// if((blend_layer->flags & LAYER_FLAG_INVISIBLE) == 0)

			// ���̃��C���[��
			layer = layer->next;

			// ���ɍ������郌�C���[���A�N�e�B�u���C���[�Ȃ�
			if(layer == window->active_layer)
			{	// �A�N�e�B�u���C���[��艺�̃��C���[�̍����f�[�^���X�V
				(void)memcpy(window->under_active->pixels, window->mixed_layer->pixels, window->pixel_buf_size);
			}
		}	// ��ԏ�̃��C���[�ɒH�蒅���܂Ń��[�v
				// while(layer != NULL)
	}
	else
	{
		// ��ԏ�̃��C���[�ɒH�蒅���܂Ń��[�v
		while(layer != NULL)
		{
			// ���C���[�Z�b�g���̃��C���[�ł����
			if(layer->layer_set != NULL)
			{	// �S�X�V�Ȃ��
				if(layer->layer_set == window->active_layer_set)
				{
					MixLayerSetActiveOver(layer, &layer, window);
				}	// else if(layer->layer_set == window->active_layer_set)
				else
				{
					layer = layer->layer_set;
				}
			}	// if(layer->layer_set != NULL)

			// �������C���[�ƍ������@����x�L������
			blend_layer = layer;
			blend_mode = layer->layer_mode;

			// ��\�����C���[�ɂȂ��Ă��Ȃ����Ƃ��m�F
			if((blend_layer->flags & LAYER_FLAG_INVISIBLE) == 0)
			{	// �����A�������郌�C���[���A�N�e�B�u���C���[�Ȃ�
				if(layer == window->active_layer)
				{
					if(layer->layer_type == TYPE_NORMAL_LAYER)
					{	// �ʏ탌�C���[��
							// ��ƃ��C���[�ƃA�N�e�B�u���C���[����x�������Ă��牺�̃��C���[�ƍ���
						(void)memcpy(window->temp_layer->pixels, layer->pixels, layer->stride*layer->height);
						g_part_layer_blend_funcs[window->work_layer->layer_mode](window->work_layer, &window->temp_update);
						//g_layer_blend_funcs[window->work_layer->layer_mode](window->work_layer, window->temp_layer);
						blend_layer = window->temp_layer;
						blend_layer->alpha = layer->alpha;
						blend_layer->flags = layer->flags;
						blend_layer->prev = layer->prev;
					}
					else if(layer->layer_type == TYPE_VECTOR_LAYER)
					{	// �x�N�g�����C���[��
							// ���C���[�̃��X�^���C�Y�������s�Ȃ��Ă����ƃ��C���[�Ɖ��̃��C���[������
						RasterizeVectorLayer(window, layer, layer->layer_data.vector_layer_p);
						if(window->work_layer->layer_mode != LAYER_BLEND_NORMAL)
						{
							g_layer_blend_funcs[window->work_layer->layer_mode](window->work_layer, layer);
						}
					}
					else if(layer->layer_type == TYPE_TEXT_LAYER)
					{	// �e�L�X�g���C���[��
							// �e�L�X�g�̓��e�����X�^���C�Y�������Ă��牺�̃��C���[�ƍ���
						RenderTextLayer(window, layer, layer->layer_data.text_layer_p);
					}

					// �T���l�C���X�V
					gtk_widget_queue_draw(layer->widget->thumbnail);
				}

				// ��������Ώۂƕ��@���m�肵���̂ō��������s����
				g_part_layer_blend_funcs[blend_mode](blend_layer, &window->update);
				// ����������f�[�^�����ɖ߂�
				window->temp_layer->alpha = 100;
				window->temp_layer->flags = 0;
				window->temp_layer->prev = NULL;
				cairo_set_operator(window->temp_layer->cairo_p, CAIRO_OPERATOR_OVER);
			}	// ��\�����C���[�ɂȂ��Ă��Ȃ����Ƃ��m�F
			// if((blend_layer->flags & LAYER_FLAG_INVISIBLE) == 0)

			// ���̃��C���[��
			layer = layer->next;

			// ���ɍ������郌�C���[���A�N�e�B�u���C���[�Ȃ�
			if(layer == window->active_layer)
			{	// �A�N�e�B�u���C���[��艺�̃��C���[�̍����f�[�^���X�V
				(void)memcpy(window->under_active->pixels, window->mixed_layer->pixels, window->pixel_buf_size);
			}
		}	// ��ԏ�̃��C���[�ɒH�蒅���܂Ń��[�v
				// while(layer != NULL)
	}

	if(update_mode == UPDATE_ALL)
	{
		if(window->app->display_filter.filter_func != NULL)
		{
			window->app->display_filter.filter_func(window->mixed_layer->pixels,
				window->mixed_layer->pixels, window->width*window->height, window->app->display_filter.filter_data);
		}

		// ���݂̊g��k�����ŕ\���p�̃f�[�^�ɍ��������f�[�^��]��
		cairo_set_operator(window->scaled_mixed->cairo_p, CAIRO_OPERATOR_SOURCE);
		cairo_set_source(window->scaled_mixed->cairo_p, window->mixed_pattern);
		cairo_paint(window->scaled_mixed->cairo_p);
	}
	else if(update_mode == UPDATE_PART)
	{
		FLOAT_T zoom = window->zoom_rate;

		if(window->app->display_filter.filter_func != NULL)
		{
			int start_x = (int)window->update.x;
			int start_y = (int)window->update.y;
			int width = (int)window->update.width;
			int height = (int)window->update.height;
			int stride = width * 4;
			for(y=0; y<height; y++)
			{
				(void)memcpy(&window->temp_layer->pixels[y*stride],
					&window->mixed_layer->pixels[(start_y+y)*window->mixed_layer->stride + start_x*4], stride);
			}
			window->app->display_filter.filter_func(window->temp_layer->pixels,
				window->temp_layer->pixels, width*height, window->app->display_filter.filter_data);
			for(y=0; y<height; y++)
			{
				(void)memcpy(&window->mixed_layer->pixels[(start_y+y)*window->mixed_layer->stride + start_x*4],
					&window->temp_layer->pixels[y*stride], stride);
			}
		}

		cairo_save(window->scaled_mixed->cairo_p);
		cairo_rectangle(window->scaled_mixed->cairo_p, (int)(window->update.x * zoom), (int)(window->update.y * zoom),
			(int)(window->update.width * zoom), (int)(window->update.height * zoom));
		cairo_clip(window->scaled_mixed->cairo_p);
		cairo_set_operator(window->scaled_mixed->cairo_p, CAIRO_OPERATOR_OVER);
		cairo_set_source(window->scaled_mixed->cairo_p, window->mixed_pattern);
		cairo_paint(window->scaled_mixed->cairo_p);
		cairo_restore(window->scaled_mixed->cairo_p);
		//ScaleNearest(window);

		cairo_surface_destroy(window->update.surface_p);
		cairo_destroy(window->update.cairo_p);
		cairo_surface_destroy(window->temp_update.surface_p);
		cairo_destroy(window->temp_update.cairo_p);

		window->flags &= ~(DRAW_WINDOW_UPDATE_PART);
	}

	(void)memcpy(window->disp_layer->pixels, window->scaled_mixed->pixels,
		window->scaled_mixed->stride * window->scaled_mixed->height);

	// �}�E�X�J�[�\���̕`�揈��
		// �u���V�����J�[�\���\���p�̊֐����Ăяo��
	// �ό`�������̓J�[�\���\���͂��Ȃ�
	if(window->transform == NULL)
	{
		// �N���b�s���O�O�̏�Ԃ��L��
		cairo_save(window->disp_layer->cairo_p);

		if((window->app->tool_window.flags & TOOL_USING_BRUSH) == 0)
		{	// �ʏ�E�x�N�g�����C���[�����Ŏg����c�[���g�p��
			window->app->tool_window.active_common_tool->display_func(
				window, window->app->tool_window.active_common_tool);

			g_layer_blend_funcs[window->effect->layer_mode](window->effect, window->disp_layer);
			// �\���p�Ɏg�p�����f�[�^��������
			window->effect->layer_mode = LAYER_BLEND_NORMAL;
			(void)memset(window->effect->pixels, 0, window->effect->stride*window->effect->height);
		}
		else
		{
			(void)memset(window->disp_temp->pixels, 0, window->disp_temp->stride*window->disp_temp->height);
			if(window->active_layer->layer_type == TYPE_NORMAL_LAYER || ((window->flags & DRAW_WINDOW_EDIT_SELECTION) != 0))
			{	// �ʏ탌�C���[
				window->app->tool_window.active_brush[window->app->input]->draw_cursor(
					window, window->cursor_x, window->cursor_y, window->app->tool_window.active_brush[window->app->input]->brush_data);
			}
			else if(window->active_layer->layer_type == TYPE_VECTOR_LAYER)
			{	// �x�N�g�����C���[�̏ꍇ�AShift�L�[���邢��Ctrl�L�[��
					// ������Ă����琧��_����̕\�����@�ɂ���
				if((state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) != 0)
				{
					window->app->tool_window.vector_control_core.draw_cursor(
						window, window->cursor_x, window->cursor_y, window->app->tool_window.vector_control_core.brush_data);
				}
				else
				{	// �����łȂ���΃u���V�����J�[�\���\���֐����Ăяo��
					window->app->tool_window.active_vector_brush[window->app->input]->draw_cursor(
						window, window->cursor_x, window->cursor_y, window->app->tool_window.active_vector_brush[window->app->input]->brush_data);
				}
			}
			else
			{	// �e�L�X�g���C���[�Ȃ�e�L�X�g�̕`��̈��\��
				DisplayTextLayerRange(window, window->active_layer->layer_data.text_layer_p);
			}
			// �쐬�����f�[�^��\���f�[�^�ɍ���
			g_layer_blend_funcs[LAYER_BLEND_DIFFERENCE](window->disp_temp, window->disp_layer);
		}	// �}�E�X�J�[�\���̕`�揈��
					// �u���V�����J�[�\���\���p�̊֐����Ăяo��
			// if((window->app->tool_window.flags & TOOL_USING_BRUSH) == 0) else

		// Cairo�����̏�Ԃɖ߂�
		cairo_restore(window->disp_layer->cairo_p);

		// �I��͈͂̕ҏW���ł���Γ��e��`��
		if((window->flags & DRAW_WINDOW_EDIT_SELECTION) != 0)
		{
			DisplayEditSelection(window);
		}
		else
		{
			// �I��͈͂�����Ε\��
			if((window->flags & DRAW_WINDOW_HAS_SELECTION_AREA) != 0)
			{
				DrawSelectionArea(&window->selection_area, window);
				g_layer_blend_funcs[LAYER_BLEND_NORMAL](window->effect, window->disp_layer);
				(void)memset(window->effect->pixels, 0, window->effect->stride*window->effect->height);
			}
		}
	}	// �ό`�������̓J�[�\���\���͂��Ȃ�
		// if(window->transform == NULL)
	else
	{
		(void)memset(window->disp_temp->pixels, 0, window->disp_temp->stride*window->disp_temp->height);
		DisplayTransform(window);
	}

	// ���E���]�\�����Ȃ�\�����e�����E���]
	if((window->flags & DRAW_WINDOW_DISPLAY_HORIZON_REVERSE) != 0)
	{
		uint8 *ref, *src;

		for(y=0; y<window->disp_layer->height; y++)
		{
			ref = window->disp_temp->pixels;
			src = &window->disp_layer->pixels[(y+1)*window->disp_layer->stride-4];
			for(x=0; x<window->disp_layer->width; x++, ref += 4, src -= 4)
			{
				ref[0] = src[0], ref[1] = src[1], ref[2] = src[2], ref[3] = src[3];
			}
			(void)memcpy(src+4, window->disp_temp->pixels, window->disp_layer->stride);
		}
	}

execute_update:

	// ��]���� & �\��
# if GTK_MAJOR_VERSION <= 2
	cairo_p = gdk_cairo_create(window->window->window);
	gdk_cairo_region(cairo_p, event_info->region);
	cairo_clip(cairo_p);
# else
	cairo_p = (cairo_t*)event_info;
# endif
	cairo_set_source(cairo_p, window->rotate);
	cairo_paint(cairo_p);
# if GTK_MAJOR_VERSION <= 2
	cairo_destroy(cairo_p);
# endif

	// ��ʍX�V���I������̂Ńt���O�����낷
	window->flags &= ~(DRAW_WINDOW_UPDATE_ACTIVE_UNDER | DRAW_WINDOW_UPDATE_ACTIVE_OVER);

	// �i�r�Q�[�V��������уv���r���[�̓��e���X�V
	if(update_mode != NO_UPDATE)
	{
		if(window->app->navigation_window.draw_area != NULL)
		{
			gtk_widget_queue_draw(window->app->navigation_window.draw_area);
		}

		if(window->app->preview_window.window != NULL)
		{
			gtk_widget_queue_draw(window->app->preview_window.image);
		}
	}

	return TRUE;
}

/*******************************************************
* MixLayerForSave�֐�                                  *
* �ۑ����邽�߂ɔw�i�s�N�Z���f�[�^�����Ń��C���[������ *
* ����                                                 *
* window	: �`��̈�̏��                           *
* �Ԃ�l                                               *
*	�����������C���[�̃f�[�^                           *
*******************************************************/
LAYER* MixLayerForSave(DRAW_WINDOW* window)
{
	// ���C���[�̃��������m�ۂ���
	LAYER* ret = CreateLayer(
		0, 0, window->width, window->height, 4,
		TYPE_NORMAL_LAYER, NULL, NULL, NULL, window
	);
	LAYER* src = window->layer;

	// ��\���łȂ��S�Ẵ��C���[������
	while(src != NULL)
	{
		if((src->flags & LAYER_FLAG_INVISIBLE) == 0 && src->layer_type != TYPE_LAYER_SET)
		{
			if(!(src->layer_set != NULL && (src->layer_set->flags & LAYER_FLAG_INVISIBLE) != 0))
			{
				g_layer_blend_funcs[src->layer_mode](src, ret);
			}
		}

		src = src->next;
	}

	return ret;
}

/*******************************************************
* MixLayerForSaveWithBackGround�֐�                    *
* �ۑ����邽�߂ɔw�i�s�N�Z���f�[�^�L��Ń��C���[������ *
* ����                                                 *
* window	: �`��̈�̏��                           *
* �Ԃ�l                                               *
*	�����������C���[�̃f�[�^                           *
*******************************************************/
LAYER* MixLayerForSaveWithBackGround(DRAW_WINDOW* window)
{
	// ���C���[�̃��������m�ۂ���
	LAYER* ret = CreateLayer(
		0, 0, window->width, window->height, 4,
		TYPE_NORMAL_LAYER, NULL, NULL, NULL, window
	);
	LAYER* src = window->layer;

	(void)memcpy(ret->pixels, window->back_ground, window->pixel_buf_size);

	// ��\���łȂ��S�Ẵ��C���[������
	while(src != NULL)
	{
		if((src->flags & LAYER_FLAG_INVISIBLE) == 0 && src->layer_type != TYPE_LAYER_SET)
		{
			if(!(src->layer_set != NULL && (src->layer_set->flags & LAYER_FLAG_INVISIBLE) != 0))
			{
				g_layer_blend_funcs[src->layer_mode](src, ret);
			}
		}

		src = src->next;
	}

	return ret;
}

#ifdef __cplusplus
}
#endif
