// Visual Studio 2005�ȍ~�ł͌Â��Ƃ����֐����g�p����̂�
	// �x�����o�Ȃ��悤�ɂ���
#if defined _MSC_VER && _MSC_VER >= 1400
# define _CRT_SECURE_NO_DEPRECATE
#endif

#include <string.h>
#include "layer.h"
#include "layer_window.h"
#include "draw_window.h"
#include "application.h"
#include "memory.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
* DeleteLayerSet�֐�                       *
* ���C���[�Z�b�g�̍폜���s��               *
* ����                                     *
* layer_set	: �폜���郌�C���[�Z�b�g       *
* window	: ���C���[�Z�b�g�����`��̈� *
*******************************************/
void DeleteLayerSet(LAYER* layer_set, DRAW_WINDOW* window)
{
	LAYER *layer = window->layer;
	LAYER *new_layer_set = layer_set->layer_set;
	int hierarchy = 0;

	while(new_layer_set != NULL)
	{
		hierarchy++;
	}

	while(layer != layer_set)
	{
		if(layer->layer_set == layer_set)
		{
			gtk_alignment_set_padding(GTK_ALIGNMENT(layer->widget->alignment),
				0, 0, LAYER_SET_DISPLAY_OFFSET * hierarchy, 0);
			layer->layer_set = new_layer_set;
		}

		layer = layer->next;
	}

	DeleteLayer(&layer_set->layer_data.layer_set_p->active_under);
	MEM_FREE_FUNC(layer_set->layer_data.layer_set_p);
}

typedef struct _CHANGE_LAYER_SET_HISTORY
{
	uint16 layer_name_length;
	uint16 before_parent_name_length;
	uint16 after_parent_name_length;
	char *layer_name;
	char *before_parent_name;
	char *after_parent_name;
} CHANGE_LAYER_SET_HISTORY;

static void ChangeLayerSetUndo(DRAW_WINDOW* window, void* data)
{
	CHANGE_LAYER_SET_HISTORY history;
	LAYER *change_layer;
	uint8 *buff = (uint8*)data;

	(void)memcpy(&history, buff, offsetof(CHANGE_LAYER_SET_HISTORY, layer_name));
	buff += offsetof(CHANGE_LAYER_SET_HISTORY, layer_name);
	history.layer_name = (char*)buff;
	buff += history.layer_name_length;
	history.before_parent_name = (char*)buff;

	change_layer = SearchLayer(window->layer, history.layer_name);
	if(*history.before_parent_name != '\0')
	{
		LAYER *parent = SearchLayer(window->layer, history.before_parent_name);
		int hierarchy = 1;

		change_layer->layer_set = parent;
		while(parent->layer_set != NULL)
		{
			hierarchy++;
			parent = parent->layer_set;
		}

		gtk_alignment_set_padding(GTK_ALIGNMENT(change_layer->widget->alignment),
			0, 0, LAYER_SET_DISPLAY_OFFSET * hierarchy, 0);
	}
	else
	{
		gtk_alignment_set_padding(GTK_ALIGNMENT(change_layer->widget->alignment),
			0, 0, 0, 0);
	}
}

static void ChangeLayerSetRedo(DRAW_WINDOW* window, void* data)
{
	CHANGE_LAYER_SET_HISTORY history;
	LAYER *change_layer;
	uint8 *buff = (uint8*)data;

	(void)memcpy(&history, buff, offsetof(CHANGE_LAYER_SET_HISTORY, layer_name));
	buff += offsetof(CHANGE_LAYER_SET_HISTORY, layer_name);
	history.layer_name = (char*)buff;
	buff += history.layer_name_length + history.before_parent_name_length;
	history.after_parent_name = (char*)buff;

	change_layer = SearchLayer(window->layer, history.layer_name);
	if(*history.after_parent_name != '\0')
	{
		LAYER *parent = SearchLayer(window->layer, history.after_parent_name);
		int hierarchy = 1;

		change_layer->layer_set = parent;
		while(parent->layer_set != NULL)
		{
			hierarchy++;
			parent = parent->layer_set;
		}

		gtk_alignment_set_padding(GTK_ALIGNMENT(change_layer->widget->alignment),
			0, 0, LAYER_SET_DISPLAY_OFFSET * hierarchy, 0);
	}
	else
	{
		gtk_alignment_set_padding(GTK_ALIGNMENT(change_layer->widget->alignment),
			0, 0, 0, 0);
	}
}

/***********************************************************
* AddChangeLyaerSetHistory�֐�                             *
* ���C���[�̏������C���[�Z�b�g�ύX�̗����f�[�^��ǉ�       *
* ����                                                     *
* change_layer	: �������C���[�Z�b�g��ύX���郌�C���[     *
* before_parent	: ���C���[�Z�b�g�ύX�O�̏������C���[�Z�b�g *
* after_parent	: ���C���[�Z�b�g�ύX��̏������C���[�Z�b�g *
***********************************************************/
void AddChangeLayerSetHistory(
	const LAYER* change_layer,
	const LAYER* before_parent,
	const LAYER* after_parent
)
{
	CHANGE_LAYER_SET_HISTORY history;
	size_t data_size = 0;
	uint8 *buff, *data;

	data_size += history.layer_name_length = (uint16)strlen(change_layer->name) + 1;
	if(before_parent != NULL)
	{
		history.before_parent_name_length = (uint16)strlen(before_parent->name) + 1;
	}
	else
	{
		history.before_parent_name_length = 1;
	}
	data_size += history.before_parent_name_length;
	if(after_parent != NULL)
	{
		history.after_parent_name_length = (uint16)strlen(after_parent->name) + 1;
	}
	else
	{
		history.after_parent_name_length = 1;
	}
	data_size += history.after_parent_name_length;
	data_size += offsetof(CHANGE_LAYER_SET_HISTORY, layer_name);

	data = buff = (uint8*)MEM_ALLOC_FUNC(data_size);
	(void)memset(buff, 0, data_size);
	(void)memcpy(buff, &history, offsetof(CHANGE_LAYER_SET_HISTORY, layer_name));
	buff += offsetof(CHANGE_LAYER_SET_HISTORY, layer_name);
	(void)strcpy(buff, change_layer->name);
	buff += history.layer_name_length;
	if(before_parent != NULL)
	{
		(void)strcpy(buff, before_parent->name);
	}
	buff += history.before_parent_name_length;
	if(after_parent != NULL)
	{
		(void)strcpy(buff, after_parent->name);
	}

	AddHistory(&change_layer->window->history, change_layer->window->app->labels->layer_window.reorder,
		data, data_size, ChangeLayerSetUndo, ChangeLayerSetRedo);

	MEM_FREE_FUNC(data);
}

/***************************************************
* LayerSetShowChildren�֐�                         *
* ���C���[�Z�b�g�̎q���C���[��\������             *
* ����                                             *
* layer_set	: �q��\�����郌�C���[�Z�b�g           *
* prev		: �֐��I����̎��Ƀ`�F�b�N���郌�C���[ *
***************************************************/
void LayerSetShowChildren(LAYER* layer_set, LAYER **prev)
{
	LAYER *layer = *prev;

	while(layer != NULL)
	{
		if(layer->layer_set == layer_set)
		{	// �\��ON
			gtk_widget_show_all(layer->widget->box);
			if(layer->layer_type == TYPE_LAYER_SET)
			{	// �q���C���Ƀ��C���[�Z�b�g������΂�������\��
				LayerSetShowChildren(layer, &layer);
			}
			else
			{
				layer = layer->prev;
			}
		}
		else
		{	// ���C���[�Z�b�g�O�ɏo���̂ŏI��
			break;
		}
	}

	*prev = layer;
}

/***************************************************
* LayerSetHideChildren�֐�                         *
* ���C���[�Z�b�g�̎q���C���[���\������           *
* ����                                             *
* layer_set	: �q��\�����郌�C���[�Z�b�g           *
* prev		: �֐��I����̎��Ƀ`�F�b�N���郌�C���[ *
***************************************************/
void LayerSetHideChildren(LAYER* layer_set, LAYER **prev)
{
	LAYER *layer = *prev;

	while(layer != NULL)
	{
		if(layer->layer_set == layer_set)
		{	// �\��OFF
			gtk_widget_hide(layer->widget->box);
			if(layer->layer_type == TYPE_LAYER_SET)
			{	// �q���C���Ƀ��C���[�Z�b�g������΂�������\��
				LayerSetHideChildren(layer, &layer);
			}
			else
			{
				layer = layer->prev;
			}
		}
		else
		{	// ���C���[�Z�b�g�O�ɏo���̂ŏI��
			break;
		}
	}

	*prev = layer;
}

static void LayerSetChildButtonCallBack(GtkWidget *button, LAYER* layer_set)
{
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)) == FALSE)
	{
		LAYER *prev = layer_set->prev;

		layer_set->flags |= LAYER_SET_CLOSE;
		LayerSetHideChildren(layer_set, &prev);

		gtk_image_set_from_pixbuf(GTK_IMAGE(layer_set->layer_data.layer_set_p->button_image),
			layer_set->window->app->layer_window.close);
	}
	else
	{
		LAYER *prev = layer_set->prev;

		layer_set->flags &= ~(LAYER_SET_CLOSE);
		LayerSetShowChildren(layer_set, &prev);

		gtk_image_set_from_pixbuf(GTK_IMAGE(layer_set->layer_data.layer_set_p->button_image),
			layer_set->window->app->layer_window.open);
	}
}

/*****************************************************
* CreateLayerSetChildButton�֐�                      *
* ���C���[�Z�b�g�̎q���C���[�\���E��\���{�^�����쐬 *
* ����                                               *
* layer_set	: �{�^���쐬�Ώۂ̃��C���[�Z�b�g         *
* �Ԃ�l                                             *
*	�쐬�����{�^���E�B�W�F�b�g                       *
*****************************************************/
GtkWidget* CreateLayerSetChildButton(LAYER* layer_set)
{
	layer_set->layer_data.layer_set_p->button_image = gtk_image_new();
	layer_set->layer_data.layer_set_p->show_child_button = gtk_toggle_button_new();

	gtk_image_set_from_pixbuf(GTK_IMAGE(layer_set->layer_data.layer_set_p->button_image),
		layer_set->window->app->layer_window.open);
	gtk_container_add(GTK_CONTAINER(layer_set->layer_data.layer_set_p->show_child_button),
		layer_set->layer_data.layer_set_p->button_image);
	gtk_toggle_button_set_active(
		GTK_TOGGLE_BUTTON(layer_set->layer_data.layer_set_p->show_child_button),
			(layer_set->flags & LAYER_SET_CLOSE) == 0);

	g_signal_connect(G_OBJECT(layer_set->layer_data.layer_set_p->show_child_button),
		"toggled", G_CALLBACK(LayerSetChildButtonCallBack), layer_set);

	return layer_set->layer_data.layer_set_p->show_child_button;
}

/*************************************************
* MixLayerSet�֐�                                *
* ���C���[�Z�b�g��������                         *
* ����                                           *
* bottom	: ���C���[�Z�b�g�̈�ԉ��̃��C���[   *
* next		: ������Ɏ��ɍ������郌�C���[       *
* window	: �`��̈���Ǘ�����\���̂̃A�h���X *
*************************************************/
void MixLayerSet(LAYER* bottom, LAYER** next, DRAW_WINDOW* window)
{
	// ���C���[�Z�b�g�̃s�N�Z���f�[�^�̃o�C�g��
	size_t pixel_bytes = bottom->layer_set->stride*bottom->layer_set->height;
	// �������C���[�Z�b�g
	LAYER *layer_set = bottom->layer_set;
	// ���C���[�����p
	LAYER *layer = bottom;
	LAYER *blend_layer;
	int blend_mode;

	// ���C���[�Z�b�g�̃s�N�Z���f�[�^�����Z�b�g
	(void)memset(layer_set->pixels, 0, pixel_bytes);

	if(layer_set == window->active_layer_set)
	{
		(void)memcpy(window->under_active->pixels, window->mixed_layer->pixels, window->pixel_buf_size);
	}

	// �������C���[�Z�b�g�܂Ń��[�v
	while(1)
	{
		blend_layer = layer;
		blend_mode = layer->layer_mode;

		// �A�N�e�B�u���C���[�Ȃ�
		if(blend_layer == window->active_layer)
		{
			// ���݂̍������ʂ��L��
			(void)memcpy(blend_layer->layer_set->layer_data.layer_set_p->active_under->pixels,
				blend_layer->layer_set->pixels, window->pixel_buf_size);

			// ������
			if((blend_layer->flags & LAYER_FLAG_INVISIBLE) == 0)
			{
				if(layer->layer_type == TYPE_NORMAL_LAYER)
				{	// �ʏ탌�C���[��
						// ��ƃ��C���[�ƃA�N�e�B�u���C���[����x�������Ă��牺�̃��C���[�ƍ���
					(void)memcpy(window->temp_layer->pixels, layer->pixels, layer->stride*layer->height);
					window->layer_blend_functions[window->work_layer->layer_mode](window->work_layer, window->temp_layer);
					blend_layer = window->temp_layer;
					blend_layer->alpha = layer->alpha;
					blend_layer->flags = layer->flags;
					blend_layer->prev = layer->prev;
				}
				else if(layer->layer_type == TYPE_VECTOR_LAYER)
				{	// �x�N�g�����C���[��
						// ���C���[�̃��X�^���C�Y�������s�Ȃ��Ă����ƃ��C���[�Ɖ��̃��C���[������
					RasterizeVectorLayer(window, layer, layer->layer_data.vector_layer_p);
					window->layer_blend_functions[window->work_layer->layer_mode](window->work_layer, layer);
				}
				else if(layer->layer_type == TYPE_TEXT_LAYER)
				{	// �e�L�X�g���C���[��
						// �e�L�X�g�̓��e�����X�^���C�Y�������Ă��牺�̃��C���[�ƍ���
					RenderTextLayer(window, layer, layer->layer_data.text_layer_p);
				}

				while(layer->next != NULL && layer->next->layer_type == TYPE_ADJUSTMENT_LAYER)
				{
					if((layer->next->flags & LAYER_FLAG_INVISIBLE) != 0)
					{
						layer->next->layer_data.adjustment_layer_p->filter_func(
							layer->layer_data.adjustment_layer_p, layer->pixels, layer->next->pixels,
								layer->width * layer->height, layer);
					}
					layer->next->layer_data.adjustment_layer_p->update(
						layer->layer_data.adjustment_layer_p, layer, window->mixed_layer,
							0, 0, layer->width, layer->height);
					blend_layer = layer->next;
					layer = layer->next;
				}

				// ��������Ώۂƕ��@���m�肵���̂ō��������s����
				window->layer_blend_functions[blend_mode](blend_layer, layer->layer_set);
				// ����������f�[�^�����ɖ߂�
				window->temp_layer->alpha = 100;
				window->temp_layer->flags = 0;
				window->temp_layer->prev = NULL;
				cairo_set_operator(window->temp_layer->cairo_p, CAIRO_OPERATOR_OVER);
			}	// ������
					// if((blend_layer->flags & LAYER_FLAG_INVISIBLE) == 0)

			// �T���l�C���X�V
			gtk_widget_queue_draw(layer->widget->thumbnail);
		}	// �A�N�e�B�u���C���[�Ȃ�
				// if(blend_layer == window->active_layer)
		else
		{
			// ������
			if((blend_layer->flags & LAYER_FLAG_INVISIBLE) == 0)
			{
				window->layer_blend_functions[blend_mode](blend_layer, layer_set);
			}
		}

		layer = layer->next;

		if(layer->layer_set != layer_set)
		{
			// �������C���[�Z�b�g�ɓ��B������
			if(layer == layer_set || layer->layer_set == NULL)
			{
				break;
			}

			MixLayerSet(layer, &layer, window);
		}
	}	// while(1)
			// �������C���[�Z�b�g�܂Ń��[�v

	// �T���l�C���X�V
	gtk_widget_queue_draw(layer->widget->thumbnail);

	*next = layer;
}

/***************************************************************
* MixLayerSetActiveOver�֐�                                    *
* ���C���[�Z�b�g���̃A�N�e�B�u���C���[�ȏ�̃��C���[���������� *
* ����                                                         *
* start		: �A�N�e�B�u���C���[                               *
* next		: ������̎��ɍ������郌�C���[                     *
* window	: �`��̈���Ǘ�����\���̂̃A�h���X               *
***************************************************************/
void MixLayerSetActiveOver(LAYER* start, LAYER** next, DRAW_WINDOW* window)
{
	// ���C���[�Z�b�g�̃s�N�Z���f�[�^�̃o�C�g��
	size_t pixel_bytes = start->layer_set->stride*start->layer_set->height;
	// �������C���[�Z�b�g
	LAYER *layer_set = start->layer_set;
	// ���C���[�����p
	LAYER *layer = start;
	LAYER *blend_layer;
	int blend_mode;

	// �A�N�e�B�u�ȃ��C���[��艺�̍������ʂ��R�s�[
	(void)memcpy(layer->layer_set->pixels,
		layer->layer_set->layer_data.layer_set_p->active_under->pixels, pixel_bytes);

	// �������C���[�Z�b�g�܂Ń��[�v
	while(1)
	{
		blend_layer = layer;
		blend_mode = layer->layer_mode;

		// ������
		if((blend_layer->flags & LAYER_FLAG_INVISIBLE) == 0)
		{	// �A�N�e�B�u���C���[�Ȃ�
			if(blend_layer == window->active_layer)
			{
				if(layer->layer_type == TYPE_NORMAL_LAYER)
				{	// �ʏ탌�C���[��
						// ��ƃ��C���[�ƃA�N�e�B�u���C���[����x�������Ă��牺�̃��C���[�ƍ���
					(void)memcpy(window->temp_layer->pixels, layer->pixels, layer->stride*layer->height);
					window->layer_blend_functions[window->work_layer->layer_mode](window->work_layer, window->temp_layer);
					blend_layer = window->temp_layer;
					blend_layer->alpha = layer->alpha;
					blend_layer->flags = layer->flags;
					blend_layer->prev = layer->prev;
				}
				else if(layer->layer_type == TYPE_VECTOR_LAYER)
				{	// �x�N�g�����C���[��
						// ���C���[�̃��X�^���C�Y�������s�Ȃ��Ă����ƃ��C���[�Ɖ��̃��C���[������
					RasterizeVectorLayer(window, layer, layer->layer_data.vector_layer_p);
					window->layer_blend_functions[window->work_layer->layer_mode](window->work_layer, layer);
				}
				else if(layer->layer_type == TYPE_TEXT_LAYER)
				{	// �e�L�X�g���C���[��
						// �e�L�X�g�̓��e�����X�^���C�Y�������Ă��牺�̃��C���[�ƍ���
					RenderTextLayer(window, layer, layer->layer_data.text_layer_p);
				}

				while(layer->next != NULL && layer->next->layer_type == TYPE_ADJUSTMENT_LAYER)
				{
					if((layer->next->flags & LAYER_FLAG_INVISIBLE) != 0)
					{
						layer->next->layer_data.adjustment_layer_p->filter_func(
							layer->layer_data.adjustment_layer_p, layer->pixels, layer->next->pixels,
								layer->width * layer->height, layer);
					}
					layer->next->layer_data.adjustment_layer_p->update(
						layer->layer_data.adjustment_layer_p, layer, window->mixed_layer,
							0, 0, layer->width, layer->height);
					blend_layer = layer->next;
					layer = layer->next;
				}

				// �T���l�C���X�V
				gtk_widget_queue_draw(layer->widget->thumbnail);
			}	// �A�N�e�B�u���C���[�Ȃ�
					// if(blend_layer == window->active_layer)

			// ��������Ώۂƕ��@���m�肵���̂ō��������s����
			window->layer_blend_functions[blend_mode](blend_layer, layer_set);
			// ����������f�[�^�����ɖ߂�
			window->temp_layer->alpha = 100;
			window->temp_layer->flags = 0;
			window->temp_layer->prev = NULL;
			cairo_set_operator(window->temp_layer->cairo_p, CAIRO_OPERATOR_OVER);
		}	// ������
				// if((blend_layer->flags & LAYER_FLAG_INVISIBLE) == 0)

		layer = layer->next;

		if(layer->layer_set != layer_set)
		{
			// �������C���[�Z�b�g�ɓ��B������
			if(layer == layer_set)
			{
				if(layer_set->layer_set == NULL)
				{
					break;
				}
				layer_set = layer_set->layer_set;
			}
		}
	}	// while(1)
			// �������C���[�Z�b�g�܂Ń��[�v

	// �T���l�C���X�V
	gtk_widget_queue_draw(layer->widget->thumbnail);

	*next = layer;
}

#ifdef __cplusplus
}
#endif
