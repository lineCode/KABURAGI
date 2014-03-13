#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include "application.h"
#include "draw_window.h"
#include "vector.h"
#include "vector_layer.h"
#include "bezier.h"
#include "anti_alias.h"
#include "memory_stream.h"
#include "memory.h"

#ifdef __cplusplus
extern "C" {
#endif

VECTOR_LINE_LAYER* CreateVectorLineLayer(
	LAYER* work,
	VECTOR_LINE* line,
	VECTOR_LAYER_RECTANGLE* rect
)
{
	VECTOR_LINE_LAYER* ret =
		(VECTOR_LINE_LAYER*)MEM_ALLOC_FUNC(sizeof(*ret));
	int width, height;
	int i;

	(void)memset(ret, 0, sizeof(*ret));

	if(rect->min_x < 0)
	{
		rect->min_x = 0;
	}
	if(rect->min_y < 0)
	{
		rect->min_y = 0;
	}

	width = (int32)(rect->max_x - rect->min_x)+1;
	height = (int32)(rect->max_y - rect->min_y)+1;

	if((int32)rect->min_x + width > work->width)
	{
		width = work->width - (int32)rect->min_x;
	}
	if((int32)rect->min_y + height > work->height)
	{
		height = work->height - (int32)rect->min_y;
	}

	ret->x = (int32)rect->min_x;
	ret->y = (int32)rect->min_y;
	ret->width = width;
	ret->height = height;
	ret->stride = width*4;
	ret->pixels = (uint8*)MEM_ALLOC_FUNC(height*ret->stride);

	for(i=0; i<height; i++)
	{
		(void)memcpy(&ret->pixels[ret->stride*i],
			&work->pixels[(ret->y+i)*work->stride+ret->x*4], width*4);
	}

	ret->surface_p = cairo_image_surface_create_for_data(
		ret->pixels, CAIRO_FORMAT_ARGB32, width, height, ret->stride);
	ret->cairo_p = cairo_create(ret->surface_p);

	return ret;
}

void DeleteVectorLineLayer(VECTOR_LINE_LAYER** layer)
{
	if(*layer == NULL)
	{
		return;
	}

	cairo_surface_destroy((*layer)->surface_p);
	cairo_destroy((*layer)->cairo_p);

	MEM_FREE_FUNC((*layer)->pixels);
	MEM_FREE_FUNC(*layer);

	*layer = NULL;
}

void StrokeStraightLine(
	DRAW_WINDOW* window,
	VECTOR_LINE* line,
	VECTOR_LAYER_RECTANGLE* rect
)
{
	gdouble r, half_r, d, div_d, arg, dist;
	int32 min_x, min_y, max_x, max_y;
	gdouble draw_x, draw_y;
	gdouble dx, dy, cos_x, sin_y;
	gdouble ad, bd;
	gdouble red, green, blue, alpha;
	cairo_pattern_t *brush;
	int i, x, y;

	cairo_set_operator(window->temp_layer->cairo_p, CAIRO_OPERATOR_OVER);
	rect->min_x = rect->max_x = line->points->x;
	rect->min_y = rect->max_y = line->points->y;

	for(i=0; i<line->num_points-1; i++)
	{
		dx = line->points[i].x-line->points[i+1].x;
		dy = line->points[i].y-line->points[i+1].y;
		d = sqrt(dx*dx+dy*dy);

		if(d < 0.5)
		{
			continue;
		}

		div_d = 1.0 / d;
		dist = 0.0;
		arg = atan2(dy, dx);
		cos_x = cos(arg), sin_y = sin(arg);
		draw_x = line->points[i].x, draw_y = line->points[i].y;

		dx = d;

		do
		{
			ad = (d - dist) * div_d, bd = dist * div_d;
			r = line->points[i].size * line->points[i].pressure * 0.01f * ad
				+ line->points[i+1].size * line->points[i+1].pressure * 0.01f * bd;
			half_r = r * 0.125;
			if(half_r < 0.5)
			{
				half_r = 0.5;
			}
			dist += half_r;

			min_x = (int32)(draw_x - r - 1);
			min_y = (int32)(draw_y - r - 1);
			max_x = (int32)(draw_x + r + 1);
			max_y = (int32)(draw_y + r + 1);

			if(min_x < 0)
			{
				min_x = 0;
			}
			else if(min_x > window->width)
			{
				min_x = window->width;
			}
			if(min_y < 0)
			{
				min_y = 0;
			}
			else if(min_y > window->height)
			{
				min_y = window->height;
			}
			if(max_x > window->width)
			{
				max_x = window->width;
			}
			else if(max_x < 0)
			{
				max_x = 0;
			}
			if(max_y > window->height)
			{
				max_y = window->height;
			}
			else if(max_y < 0)
			{
				max_y = 0;
			}

			if(rect->min_x > min_x)
			{
				rect->min_x = min_x;
			}
			if(rect->min_y > min_y)
			{
				rect->min_y = min_y;
			}
			if(rect->max_x < max_x)
			{
				rect->max_x = max_x;
			}
			if(rect->max_y < max_y)
			{
				rect->max_y = max_y;
			}
			
			for(y=min_y; y<max_y; y++)
			{
				(void)memset(&window->temp_layer->pixels[y*window->temp_layer->stride+min_x*window->temp_layer->channel],
					0, (max_x - min_x)*window->temp_layer->channel);
			}

			ad *= DIV_PIXEL, bd *= DIV_PIXEL;
			red = (line->points[i].color[0] * ad) + (line->points[i+1].color[0] * bd);
			green = (line->points[i].color[1] * ad) + (line->points[i+1].color[1] * bd);
			blue = (line->points[i].color[2] * ad) + (line->points[i+1].color[2] * bd);
			alpha = (line->points[i].color[3] * ad) + (line->points[i+1].color[3] * bd);

			brush = cairo_pattern_create_radial(draw_x, draw_y, r * 0.0, draw_x, draw_y, r);
			cairo_pattern_set_extend(brush, CAIRO_EXTEND_NONE);
			cairo_pattern_add_color_stop_rgba(brush, 0.0, red, green, blue, alpha);
			cairo_pattern_add_color_stop_rgba(brush, 1.0-line->blur*0.01,
				red, green, blue, alpha);
			cairo_pattern_add_color_stop_rgba(brush, 1.0, red, green, blue,
				line->outline_hardness*0.01*alpha);
			cairo_set_source(window->temp_layer->cairo_p, brush);

			cairo_arc(window->temp_layer->cairo_p, draw_x, draw_y, r, 0, 2*G_PI);
			cairo_fill(window->temp_layer->cairo_p);

			for(y=min_y; y<max_y; y++)
			{
				for(x=min_x; x<max_x; x++)
				{
					if(window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >
						window->work_layer->pixels[y*window->work_layer->stride+x*window->channel+3])
					{
						window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel] =
							(uint8)(((int)window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel]
							- (int)window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel])
								* window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >> 8)
								+ window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel];
						window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+1] =
							(uint8)(((int)window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+1]
							- (int)window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+1])
								* window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >> 8)
								+ window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+1];
						window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+2] =
							(uint8)(((int)window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+2]
							- (int)window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+2])
								* window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >> 8)
								+ window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+2];
						window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+3] =
							(uint8)(((int)window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3]
							- (int)window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+3])
								* window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >> 8)
								+ window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+3];
					}
				}
			}

			cairo_pattern_destroy(brush);

			dx -= half_r;
			draw_x -= cos_x*half_r, draw_y -= sin_y*half_r;
		} while(dx >= half_r);
	}
}

void StrokeStraightCloseLine(
	DRAW_WINDOW* window,
	VECTOR_LINE* line,
	VECTOR_LAYER_RECTANGLE* rect
)
{
	gdouble r, half_r, d, div_d, arg, dist;
	int32 min_x, min_y, max_x, max_y;
	gdouble draw_x, draw_y;
	gdouble dx, dy, cos_x, sin_y;
	gdouble ad, bd;
	gdouble red, green, blue, alpha;
	cairo_pattern_t *brush;
	int i, x, y;

	cairo_set_operator(window->temp_layer->cairo_p, CAIRO_OPERATOR_OVER);
	rect->min_x = rect->max_x = line->points->x;
	rect->min_y = rect->max_y = line->points->y;

	for(i=0; i<line->num_points-1; i++)
	{
		dx = line->points[i].x-line->points[i+1].x;
		dy = line->points[i].y-line->points[i+1].y;
		d = sqrt(dx*dx+dy*dy);

		if(d < 0.5)
		{
			continue;
		}

		div_d = 1.0 / d;
		dist = 0.0;
		arg = atan2(dy, dx);
		cos_x = cos(arg), sin_y = sin(arg);
		draw_x = line->points[i].x, draw_y = line->points[i].y;

		dx = d;

		do
		{
			ad = (d - dist) * div_d, bd = dist * div_d;
			r = line->points[i].size * line->points[i].pressure * 0.01f * ad
				+ line->points[i+1].size * line->points[i+1].pressure * 0.01f * bd;
			half_r = r * 0.125;
			if(half_r < 0.5)
			{
				half_r = 0.5;
			}
			dist += half_r;

			min_x = (int32)(draw_x - r - 1);
			min_y = (int32)(draw_y - r - 1);
			max_x = (int32)(draw_x + r + 1);
			max_y = (int32)(draw_y + r + 1);

			if(min_x < 0)
			{
				min_x = 0;
			}
			else if(min_x > window->width)
			{
				min_x = window->width;
			}
			if(min_y < 0)
			{
				min_y = 0;
			}
			else if(min_y > window->height)
			{
				min_y = window->height;
			}
			if(max_x > window->width)
			{
				max_x = window->width;
			}
			else if(max_x < 0)
			{
				max_x = 0;
			}
			if(max_y > window->height)
			{
				max_y = window->height;
			}
			else if(max_y < 0)
			{
				max_y = 0;
			}

			if(rect->min_x > min_x)
			{
				rect->min_x = min_x;
			}
			if(rect->min_y > min_y)
			{
				rect->min_y = min_y;
			}
			if(rect->max_x < max_x)
			{
				rect->max_x = max_x;
			}
			if(rect->max_y < max_y)
			{
				rect->max_y = max_y;
			}
			
			for(y=min_y; y<max_y; y++)
			{
				(void)memset(&window->temp_layer->pixels[y*window->temp_layer->stride+min_x*window->temp_layer->channel],
					0, (max_x - min_x)*window->temp_layer->channel);
			}

			ad *= DIV_PIXEL, bd *= DIV_PIXEL;
			red = (line->points[i].color[0] * ad) + (line->points[i+1].color[0] * bd);
			green = (line->points[i].color[1] * ad) + (line->points[i+1].color[1] * bd);
			blue = (line->points[i].color[2] * ad) + (line->points[i+1].color[2] * bd);
			alpha = (line->points[i].color[3] * ad) + (line->points[i+1].color[3] * bd);

			brush = cairo_pattern_create_radial(draw_x, draw_y, r * 0.0, draw_x, draw_y, r);
			cairo_pattern_set_extend(brush, CAIRO_EXTEND_NONE);
			cairo_pattern_add_color_stop_rgba(brush, 0.0, red, green, blue, alpha);
			cairo_pattern_add_color_stop_rgba(brush, 1.0-line->blur*0.01,
				red, green, blue, alpha);
			cairo_pattern_add_color_stop_rgba(brush, 1.0, red, green, blue,
				line->outline_hardness*0.01f*alpha);
			cairo_set_source(window->temp_layer->cairo_p, brush);

			cairo_arc(window->temp_layer->cairo_p, draw_x, draw_y, r, 0, 2*G_PI);
			cairo_fill(window->temp_layer->cairo_p);

			for(y=min_y; y<max_y; y++)
			{
				for(x=min_x; x<max_x; x++)
				{
					if(window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >
						window->work_layer->pixels[y*window->work_layer->stride+x*window->channel+3])
					{
						window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel] =
							(uint8)(((int)window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel]
							- (int)window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel])
								* window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >> 8)
								+ window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel];
						window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+1] =
							(uint8)(((int)window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+1]
							- (int)window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+1])
								* window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >> 8)
								+ window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+1];
						window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+2] =
							(uint8)(((int)window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+2]
							- (int)window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+2])
								* window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >> 8)
								+ window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+2];
						window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+3] =
							(uint8)(((int)window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3]
							- (int)window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+3])
								* window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >> 8)
								+ window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+3];
					}
				}
			}

			cairo_pattern_destroy(brush);

			dx -= half_r;
			draw_x -= cos_x*half_r, draw_y -= sin_y*half_r;
		} while(dx >= half_r);
	}

	dx = line->points[i].x-line->points[0].x;
	dy = line->points[i].y-line->points[0].y;
	d = sqrt(dx*dx+dy*dy);

	if(d < 0.5)
	{
		return;
	}

	div_d = 1.0 / d;
	dist = 0.0;
	arg = atan2(dy, dx);
	cos_x = cos(arg), sin_y = sin(arg);
	draw_x = line->points[i].x, draw_y = line->points[i].y;

	dx = d;

	do
	{
		ad = (d - dist) * div_d, bd = dist * div_d;
		r = line->points[i].size * line->points[i].pressure * 0.01f * ad
			+ line->points[0].size * line->points[0].pressure * 0.01f * bd;
		half_r = r * 0.125;
		if(half_r < 0.5)
		{
			half_r = 0.5;
		}
		dist += half_r;

		min_x = (int32)(draw_x - r - 1);
		min_y = (int32)(draw_y - r - 1);
		max_x = (int32)(draw_x + r + 1);
		max_y = (int32)(draw_y + r + 1);

		if(min_x < 0)
		{
			min_x = 0;
		}
		else if(min_x > window->width)
		{
			min_x = window->width;
		}
		if(min_y < 0)
		{
			min_y = 0;
		}
		else if(min_y > window->height)
		{
			min_y = window->height;
		}
		if(max_x > window->width)
		{
			max_x = window->width;
		}
		else if(max_x < 0)
		{
			max_x = 0;
		}
		if(max_y > window->height)
		{
			max_y = window->height;
		}
		else if(max_y < 0)
		{
			max_y = 0;
		}

		if(rect->min_x > min_x)
		{
			rect->min_x = min_x;
		}
		if(rect->min_y > min_y)
		{
			rect->min_y = min_y;
		}
		if(rect->max_x < max_x)
		{
			rect->max_x = max_x;
		}
		if(rect->max_y < max_y)
		{
			rect->max_y = max_y;
		}

		for(y=min_y; y<max_y; y++)
		{
			(void)memset(&window->temp_layer->pixels[y*window->temp_layer->stride+min_x*window->temp_layer->channel],
				0, (max_x - min_x)*window->temp_layer->channel);
		}

		ad *= DIV_PIXEL, bd *= DIV_PIXEL;
		red = (line->points[i].color[0] * ad) + (line->points[0].color[0] * bd);
		green = (line->points[i].color[1] * ad) + (line->points[0].color[1] * bd);
		blue = (line->points[i].color[2] * ad) + (line->points[0].color[2] * bd);
		alpha = (line->points[i].color[3] * ad) + (line->points[0].color[3] * bd);

		brush = cairo_pattern_create_radial(draw_x, draw_y, r * 0.0, draw_x, draw_y, r);
		cairo_pattern_set_extend(brush, CAIRO_EXTEND_NONE);
		cairo_pattern_add_color_stop_rgba(brush, 0.0, red, green, blue, alpha);
		cairo_pattern_add_color_stop_rgba(brush, 1.0-line->blur*0.01,
			red, green, blue, alpha);
		cairo_pattern_add_color_stop_rgba(brush, 1.0, red, green, blue,
			line->outline_hardness*0.01f*alpha);
		cairo_set_source(window->temp_layer->cairo_p, brush);

		cairo_arc(window->temp_layer->cairo_p, draw_x, draw_y, r, 0, 2*G_PI);
		cairo_fill(window->temp_layer->cairo_p);

		for(y=min_y; y<max_y; y++)
		{
			for(x=min_x; x<max_x; x++)
			{
				if(window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >
					window->work_layer->pixels[y*window->work_layer->stride+x*window->channel+3])
				{
					window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel] =
						(uint8)(((int)window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel]
						- (int)window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel])
							* window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >> 8)
							+ window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel];
					window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+1] =
						(uint8)(((int)window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+1]
						- (int)window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+1])
							* window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >> 8)
							+ window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+1];
					window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+2] =
						(uint8)(((int)window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+2]
						- (int)window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+2])
							* window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >> 8)
							+ window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+2];
					window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+3] =
						(uint8)(((int)window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3]
						- (int)window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+3])
							* window->temp_layer->pixels[y*window->temp_layer->stride+x*window->temp_layer->channel+3] >> 8)
							+ window->work_layer->pixels[y*window->work_layer->stride+x*window->work_layer->channel+3];
				}
			}
		}

		cairo_pattern_destroy(brush);

		dx -= half_r;
		draw_x -= cos_x*half_r, draw_y -= sin_y*half_r;
	} while(dx >= half_r);
}

void RasterizeVectorLine(
	DRAW_WINDOW* window,
	VECTOR_LAYER* layer,
	VECTOR_LINE* line,
	VECTOR_LAYER_RECTANGLE* rect
)
{
	switch(line->vector_type)
	{
	case VECTOR_LINE_STRAIGHT:
		StrokeStraightLine(window, line, rect);
		break;
	case VECTOR_LINE_BEZIER_OPEN:
		if(line->num_points < 3)
		{
			StrokeStraightLine(window, line, rect);
		}
		else
		{
			StrokeBezierLine(window, line, rect);
		}
		break;
	case VECTOR_LINE_STRAIGHT_CLOSE:
		if(line->num_points < 3)
		{
			StrokeStraightLine(window, line, rect);
		}
		else
		{
			StrokeStraightCloseLine(window, line, rect);
		}
		break;
	case VECTOR_LINE_BEZIER_CLOSE:
		if(line->num_points < 3)
		{
			StrokeStraightLine(window, line, rect);
		}
		else
		{
			StrokeBezierLineClose(window, line, rect);
		}
		break;
	}

	if((line->flags & VECTOR_LINE_ANTI_ALIAS) != 0)
	{
		ANTI_ALIAS_RECTANGLE range;

		range.x = (int)rect->min_x;
		range.y = (int)rect->min_y;
		range.width = (int)(rect->max_x - rect->min_x) + 1;
		range.height = (int)(rect->max_y - rect->min_y) + 1;

		AntiAliasVectorLine(window->work_layer, window->temp_layer, &range);
	}
}

void BlendVectorLineLayer(VECTOR_LINE_LAYER* src, VECTOR_LINE_LAYER* dst)
{
	cairo_set_source_surface(dst->cairo_p, src->surface_p, src->x, src->y);
	cairo_paint(dst->cairo_p);
}

VECTOR_LINE* CreateVectorLine(VECTOR_LINE* prev, VECTOR_LINE* next)
{
	VECTOR_LINE* ret = (VECTOR_LINE*)MEM_ALLOC_FUNC(sizeof(*ret));
	(void)memset(ret, 0, sizeof(*ret));

	if(prev != NULL)
	{
		prev->next = ret;
	}
	if(next != NULL)
	{
		next->prev = ret;
	}
	ret->prev = prev;
	ret->next = next;

	return ret;
}

void DeleteVectorLine(VECTOR_LINE** line)
{
	if((*line)->prev != NULL)
	{
		(*line)->prev->next = (*line)->next;
	}
	if((*line)->next != NULL)
	{
		(*line)->next->prev = (*line)->prev;
	}

	DeleteVectorLineLayer(&(*line)->layer);
	MEM_FREE_FUNC((*line)->points);
	MEM_FREE_FUNC(*line);

	*line = NULL;
}

void DeleteVectorLayer(VECTOR_LAYER** layer)
{
	VECTOR_LINE* delete_line, *next_delete;
	delete_line = (*layer)->base;

	while(delete_line != NULL)
	{
		next_delete = delete_line->next;

		DeleteVectorLine(&delete_line);

		delete_line = next_delete;
	}
}

void RasterizeVectorLayer(
	struct _DRAW_WINDOW* window,
	struct _LAYER* target,
	VECTOR_LAYER* layer
)
{
	VECTOR_LAYER_RECTANGLE rect;

	cairo_set_operator(window->temp_layer->cairo_p, CAIRO_OPERATOR_OVER);

	if((layer->flags & VECTOR_LAYER_RASTERIZE_TOP) != 0
		&& layer->top_line != NULL && layer->top_line != layer->base)
	{
		(void)memcpy(target->pixels, layer->mix->pixels, window->pixel_buf_size);
		(void)memset(window->work_layer->pixels, 0, window->pixel_buf_size);

		RasterizeVectorLine(window, layer, layer->top_line, &rect);

		g_layer_blend_funcs[LAYER_BLEND_NORMAL](window->work_layer, window->active_layer);

		if((layer->flags & VECTOR_LAYER_FIX_LINE) != 0)
		{
			layer->top_line->layer = CreateVectorLineLayer(window->work_layer, layer->top_line, &rect);
			(void)memcpy(layer->mix->pixels, window->active_layer->pixels, window->pixel_buf_size);
			layer->active_line = NULL;
			(void)memset(window->work_layer->pixels, 0, window->pixel_buf_size);
		}
	}
	else if((layer->flags & VECTOR_LAYER_RASTERIZE_ACTIVE) != 0)
	{
		VECTOR_LINE* dst = layer->base;
		VECTOR_LINE* src = layer->base->next;

		(void)memset(dst->layer->pixels, 0, dst->layer->stride*dst->layer->height);
		(void)memset(layer->mix->pixels, 0, window->active_layer->stride*window->active_layer->height);

		while(src != NULL)
		{
			if(src == layer->active_line)
			{
				(void)memset(window->work_layer->pixels, 0, window->pixel_buf_size);
				RasterizeVectorLine(window, layer, layer->active_line, &rect);

				if((layer->flags & VECTOR_LAYER_FIX_LINE) != 0)
				{
					DeleteVectorLineLayer(&src->layer);
					src->layer = CreateVectorLineLayer(window->work_layer, layer->active_line, &rect);
					BlendVectorLineLayer(src->layer, dst->layer);
				}
				else
				{
					cairo_set_source_surface(dst->layer->cairo_p, window->work_layer->surface_p, 0, 0);
					cairo_paint(dst->layer->cairo_p);
				}
			}
			else
			{
				if(src->layer != NULL)
				{
					BlendVectorLineLayer(src->layer, dst->layer);
				}
			}

			src = src->next;
		}

		(void)memcpy(window->active_layer->pixels, dst->layer->pixels, window->pixel_buf_size);
		if((layer->flags & VECTOR_LAYER_FIX_LINE) != 0)
		{
			(void)memcpy(layer->mix->pixels, window->active_layer->pixels, window->pixel_buf_size);
		}
	}
	else if((layer->flags & VECTOR_LAYER_RASTERIZE_CHECKED) != 0)
	{
		VECTOR_LINE* dst = layer->base;
		VECTOR_LINE* src = layer->base->next;

		(void)memset(dst->layer->pixels, 0, dst->layer->stride*dst->layer->height);
		(void)memset(layer->mix->pixels, 0, window->active_layer->stride*window->active_layer->height);

		while(src != NULL)
		{
			if((src->flags & VECTOR_LINE_FIX) != 0)
			{
				(void)memset(window->work_layer->pixels, 0, window->pixel_buf_size);
				RasterizeVectorLine(window, layer, src, &rect);

				DeleteVectorLineLayer(&src->layer);
				src->layer = CreateVectorLineLayer(window->work_layer, layer->active_line, &rect);
				BlendVectorLineLayer(src->layer, dst->layer);
				src->flags &= ~(VECTOR_LINE_FIX);
			}
			else
			{
				BlendVectorLineLayer(src->layer, dst->layer);
			}

			src = src->next;
		}

		(void)memcpy(target->pixels, dst->layer->pixels, window->pixel_buf_size);
		(void)memcpy(layer->mix->pixels, dst->layer->pixels, window->pixel_buf_size);
	}
	else if((layer->flags & VECTOR_LAYER_RASTERIZE_ALL) != 0)
	{
		VECTOR_LINE* rasterize = layer->base->next;
		(void)memset(layer->mix->pixels, 0, window->active_layer->stride*window->active_layer->height);
		(void)memset(target->pixels, 0, target->stride*target->height);

		if(rasterize == NULL)
		{
			goto end;
		}

		while(rasterize->next != NULL)
		{
			(void)memset(window->work_layer->pixels, 0, window->pixel_buf_size);

			RasterizeVectorLine(window, layer, rasterize, &rect);

			if((layer->flags & VECTOR_LAYER_FIX_LINE) != 0)
			{
				DeleteVectorLineLayer(&rasterize->layer);
				rasterize->layer = CreateVectorLineLayer(window->work_layer, layer->active_line, &rect);
			}

			g_layer_blend_funcs[LAYER_BLEND_NORMAL](window->work_layer, target);

			rasterize = rasterize->next;
		}

		(void)memset(window->work_layer->pixels, 0, window->pixel_buf_size);

		RasterizeVectorLine(window, layer, rasterize, &rect);

		if((layer->flags & VECTOR_LAYER_FIX_LINE) != 0)
		{
			DeleteVectorLineLayer(&rasterize->layer);
			rasterize->layer = CreateVectorLineLayer(window->work_layer, layer->active_line, &rect);
		}

		g_layer_blend_funcs[LAYER_BLEND_NORMAL](window->work_layer, target);

		(void)memcpy(layer->mix->pixels, target->pixels, window->pixel_buf_size);
	}
	else
	{
		(void)memcpy(target->pixels, layer->mix->pixels, window->pixel_buf_size);
	}

end:
	layer->flags = 0;
}

/***********************************************
* IsVectorLineInSelectionArea�֐�              *
* �����I��͈͓����ۂ��𔻒肷��               *
* ����                                         *
* window			: �`��̈�̏��           *
* selection_pixels	: �I��͈͂̃s�N�Z���f�[�^ *
* line				: ���肷���               *
* �Ԃ�l                                       *
*	�I��͈͓�:1	�I��͈͊O:0               *
***********************************************/
int IsVectorLineInSelectionArea(
	DRAW_WINDOW* window,
	uint8* selection_pixels,
	VECTOR_LINE* line
)
{
	FLOAT_T dx, dy;
	FLOAT_T length;
	FLOAT_T t;
	FLOAT_T dt;
	FLOAT_T x, y;
	int check_x, check_y;
	int stride = window->width;
	int i;

	if(line->num_points == 2)
	{
		dx = line->points[1].x - line->points[0].x;
		dy = line->points[1].y - line->points[0].y;
		length = sqrt(dx*dx + dy*dy);
		dx = dx / length,	dy = dy / length;
		x = line->points[0].x,	y = line->points[0].y;
		dt = 1 / length;

		for(t=0; t<1; t+=dt)
		{
			check_x = (int)x,	check_y = (int)y;
			if(check_x >= 0 && check_y >= 0
				&& check_x < window->width && check_y < window->height)
			{
				if(selection_pixels[check_y*stride + check_x] != 0)
				{
					return 1;
				}
			}
			x += dx;
			y += dy;
		}
	}
	else if(line->vector_type == VECTOR_LINE_STRAIGHT)
	{
		for(i=0; i<line->num_points-1; i++)
		{
			dx = line->points[i+1].x - line->points[i].x;
			dy = line->points[i+1].y - line->points[i].y;
			length = sqrt(dx*dx + dy*dy);
			dx = dx / length,	dy = dy / length;
			x = line->points[i].x,	y = line->points[i].y;
			dt = 1 / length;

			for(t=0; t<1; t+=dt)
			{
				check_x = (int)x,	check_y = (int)y;
				if(check_x >= 0 && check_y >= 0
					&& check_x < window->width && check_y < window->height)
				{
					if(selection_pixels[check_y*stride + check_x] != 0)
					{
						return 1;
					}
				}
				x += dx;
				y += dy;
			}
		}

		dx = line->points[0].x - line->points[i].x;
		dy = line->points[0].y - line->points[i].y;
		length = sqrt(dx*dx + dy*dy);
		dx = dx / length,	dy = dy / length;
		x = line->points[i].x,	y = line->points[i].y;
		dt = 1 / length;

		for(t=0; t<1; t+=dt)
		{
			check_x = (int)x,	check_y = (int)y;
			if(check_x >= 0 && check_y >= 0
				&& check_x < window->width && check_y < window->height)
			{
				if(selection_pixels[check_y*stride + check_x] != 0)
				{
					return 1;
				}
			}
			x += dx;
			y += dy;
		}
	}
	else if(line->vector_type == VECTOR_LINE_BEZIER_OPEN)
	{
		BEZIER_POINT calc[4], inter[2];
		BEZIER_POINT check;
		int j;

		for(j=0; j<3; j++)
		{
			calc[j].x = line->points[j].x;
			calc[j].y = line->points[j].y;
		}
		MakeBezier3EdgeControlPoint(calc, inter);
		calc[1].x = line->points[0].x,	calc[1].y = line->points[0].y;
		calc[2] = inter[0];
		calc[3].x = line->points[1].x,	calc[3].y = line->points[1].y;

		length = 0;
		for(j=1; j<3; j++)
		{
			length += sqrt((calc[j+1].x-calc[j].x)*(calc[j+1].x-calc[j].x)
				+ (calc[j+1].y-calc[j].y)*(calc[j+1].y-calc[j].y));
		}
		dt = 1 / length;

		for(t=0; t<1; t+=dt)
		{
			CalcBezier3(calc, t, &check);
			check_x = (int)check.x,	check_y = (int)check.y;
			if(check_x >= 0 && check_y >= 0
				&& check_x < window->width && check_y < window->height)
			{
				if(selection_pixels[check_y*stride + check_x] != 0)
				{
					return 1;
				}
			}
		}

		for(i=0; i<line->num_points-3; i++)
		{
			for(j=0; j<4; j++)
			{
				calc[j].x = line->points[i+j].x;
				calc[j].y = line->points[i+j].y;
			}
			MakeBezier3ControlPoints(calc, inter);
			calc[0].x = line->points[i+1].x,	calc[0].y = line->points[i+1].y;
			calc[1] = inter[0],	calc[2] = inter[1];
			calc[3].x = line->points[i+2].x,	calc[3].y = line->points[i+2].y;

			length = 0;
			for(j=0; j<3; j++)
			{
				length += sqrt((calc[j+1].x-calc[j].x)*(calc[j+1].x-calc[j].x)
					+ (calc[j+1].y-calc[j].y)*(calc[j+1].y-calc[j].y));
			}
			dt = 1 / length;

			for(t=0; t<1; t+=dt)
			{
				CalcBezier3(calc, t, &check);
				check_x = (int)check.x,	check_y = (int)check.y;
				if(check_x >= 0 && check_y >= 0
					&& check_x < window->width && check_y < window->height)
				{
					if(selection_pixels[check_y*stride + check_x] != 0)
					{
						return 1;
					}
				}
			}
		}

		for(j=0; j<3; j++)
		{
			calc[j].x = line->points[i+j].x;
			calc[j].y = line->points[i+j].y;
		}
		MakeBezier3EdgeControlPoint(calc, inter);

		calc[0].x = line->points[i+1].x,	calc[0].y = line->points[i+1].y;
		calc[1] = inter[1];
		calc[2].x = line->points[i+2].x,	calc[2].y = line->points[i+2].y;
		calc[3].x = line->points[i+2].x,	calc[3].y = line->points[i+2].y;

		length = 0;
		for(j=0; j<2; j++)
		{
			length += sqrt((calc[j+1].x-calc[j].x)*(calc[j+1].x-calc[j].x)
				+ (calc[j+1].y-calc[j].y)*(calc[j+1].y-calc[j].y));
		}
		dt = 1 / length;

		for(t=0; t<1; t+=dt)
		{
			CalcBezier3(calc, t, &check);
			check_x = (int)check.x,	check_y = (int)check.y;
			if(check_x >= 0 && check_y >= 0
				&& check_x < window->width && check_y < window->height)
			{
				if(selection_pixels[check_y*stride + check_x] != 0)
				{
					return 1;
				}
			}
		}
	}
	else if(line->vector_type == VECTOR_LINE_STRAIGHT_CLOSE)
	{
		for(i=0; i<line->num_points-1; i++)
		{
			dx = line->points[i+1].x - line->points[i].x;
			dy = line->points[i+1].y - line->points[i].y;
			length = sqrt(dx*dx + dy*dy);
			dx = dx / length,	dy = dy / length;
			x = line->points[i].x,	y = line->points[i].y;
			dt = 1 / length;

			for(t=0; t<1; t+=dt)
			{
				check_x = (int)x,	check_y = (int)y;
				if(check_x >= 0 && check_y >= 0
					&& check_x < window->width && check_y < window->height)
				{
					if(selection_pixels[check_y*stride + check_x] != 0)
					{
						return 1;
					}
				}
				x += dx;
				y += dy;
			}
		}
	}
	else if(line->vector_type == VECTOR_LINE_BEZIER_OPEN)
	{
		BEZIER_POINT calc[4], inter[2];
		BEZIER_POINT check;
		int j;

		calc[0].x = line->points[0].x,	calc[0].y = line->points[0].y;
		for(j=0; j<3; j++)
		{
			calc[j+1].x = line->points[j].x;
			calc[j+1].y = line->points[j].y;
		}
		MakeBezier3ControlPoints(calc, inter);
		calc[0].x = line->points[0].x,	calc[0].y = line->points[0].y;
		calc[1] = inter[0];
		calc[2] = inter[1];
		calc[3].x = line->points[1].x,	calc[3].y = line->points[1].y;

		length = 0;
		for(j=0; j<3; j++)
		{
			length += sqrt((calc[j+1].x-calc[j].x)*(calc[j+1].x-calc[j].x)
				+ (calc[j+1].y-calc[j].y)*(calc[j+1].y-calc[j].y));
		}
		dt = 1 / length;

		for(t=0; t<1; t+=dt)
		{
			CalcBezier3(calc, t, &check);
			check_x = (int)check.x,	check_y = (int)check.y;
			if(check_x >= 0 && check_y >= 0
				&& check_x < window->width && check_y < window->height)
			{
				if(selection_pixels[check_y*stride + check_x] != 0)
				{
					return 1;
				}
			}
		}

		for(i=0; i<line->num_points-3; i++)
		{
			for(j=0; j<4; j++)
			{
				calc[j].x = line->points[i+j].x;
				calc[j].y = line->points[i+j].y;
			}
			MakeBezier3ControlPoints(calc, inter);
			calc[0].x = line->points[i+1].x,	calc[0].y = line->points[i+1].y;
			calc[1] = inter[0],	calc[2] = inter[1];
			calc[3].x = line->points[i+2].x,	calc[3].y = line->points[i+2].y;

			length = 0;
			for(j=0; j<3; j++)
			{
				length += sqrt((calc[j+1].x-calc[j].x)*(calc[j+1].x-calc[j].x)
					+ (calc[j+1].y-calc[j].y)*(calc[j+1].y-calc[j].y));
			}
			dt = 1 / length;

			for(t=0; t<1; t+=dt)
			{
				CalcBezier3(calc, t, &check);
				check_x = (int)check.x,	check_y = (int)check.y;
				if(check_x >= 0 && check_y >= 0
					&& check_x < window->width && check_y < window->height)
				{
					if(selection_pixels[check_y*stride + check_x] != 0)
					{
						return 1;
					}
				}
			}
		}

		for(j=0; j<3; j++)
		{
			calc[j].x = line->points[i+j].x;
			calc[j].y = line->points[i+j].y;
		}
		calc[3].x = line->points[0].x,	calc[3].y = line->points[0].y;
		MakeBezier3EdgeControlPoint(calc, inter);

		calc[0].x = line->points[i+1].x,	calc[0].y = line->points[i+1].y;
		calc[1] = inter[0],	calc[2] = inter[1];
		calc[3].x = line->points[i+2].x,	calc[3].y = line->points[i+2].y;

		length = 0;
		for(j=0; j<3; j++)
		{
			length += sqrt((calc[j+1].x-calc[j].x)*(calc[j+1].x-calc[j].x)
				+ (calc[j+1].y-calc[j].y)*(calc[j+1].y-calc[j].y));
		}
		dt = 1 / length;

		for(t=0; t<1; t+=dt)
		{
			CalcBezier3(calc, t, &check);
			check_x = (int)check.x,	check_y = (int)check.y;
			if(check_x >= 0 && check_y >= 0
				&& check_x < window->width && check_y < window->height)
			{
				if(selection_pixels[check_y*stride + check_x] != 0)
				{
					return 1;
				}
			}
		}

		calc[0].x = line->points[i+1].x, calc[0].y = line->points[i+1].y;
		calc[1].x = line->points[i+2].x, calc[1].y = line->points[i+2].y;
		calc[2].x = line->points[0].x, calc[2].y = line->points[0].y;
		calc[3].x = line->points[1].x, calc[3].y = line->points[1].y;
		MakeBezier3ControlPoints(calc, inter);
		calc[0].x = line->points[i+2].x, calc[0].y = line->points[i+2].y;
		calc[1] = inter[0], calc[2] = inter[1];
		calc[3].x = line->points[0].x, calc[3].y = line->points[0].y;

		length = 0;
		for(j=0; j<3; j++)
		{
			length += sqrt((calc[j+1].x-calc[j].x)*(calc[j+1].x-calc[j].x)
				+ (calc[j+1].y-calc[j].y)*(calc[j+1].y-calc[j].y));
		}
		dt = 1 / length;

		for(t=0; t<1; t+=dt)
		{
			CalcBezier3(calc, t, &check);
			check_x = (int)check.x,	check_y = (int)check.y;
			if(check_x >= 0 && check_y >= 0
				&& check_x < window->width && check_y < window->height)
			{
				if(selection_pixels[check_y*stride + check_x] != 0)
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

/**********************************
* ADD_CONTROL_POINT_HISTORY�\���� *
* ����_�ǉ��̗����f�[�^          *
**********************************/
typedef struct ADD_CONTROL_POINT_HISTORY
{
	gdouble x, y;				// �ǉ���������_�̍��W
	gdouble pressure;			// �ǉ���������_�̕M��
	gdouble size;				// �ǉ���������_�̃T�C�Y
	uint8 color[4];				// �ǉ���������_�̐F
	uint32 line_id;				// ����_��ǉ��������C����ID
	uint32 point_id;			// �ǉ���������_��ID
	uint16 layer_name_length;	// ����_��ǉ��������C���[�̖��O�̒���
	char* layer_name;			// ����_��ǉ��������C���[�̖��O
} ADD_CONTROL_POINT_HISTORY;

/*****************************
* AddControlPointUndo�֐�    *
* ����_�̒ǉ������ɖ߂�     *
* ����                       *
* window	: �`��̈�̏�� *
* p			: �����f�[�^     *
*****************************/
static void AddControlPointUndo(DRAW_WINDOW* window, void* p)
{
	// �����f�[�^
	ADD_CONTROL_POINT_HISTORY history;
	// ����_���폜���郌�C���[
	LAYER* layer;
	// ����_���폜���郉�C��
	VECTOR_LINE* line;
	// �f�[�^���o�C�g�P�ʂň������߂ɃL���X�g
	int8* buff = (int8*)p;
	// �J�E���^
	unsigned int i;

	// �f�[�^���R�s�[
	(void)memcpy(&history, p, offsetof(ADD_CONTROL_POINT_HISTORY, layer_name));
	buff += offsetof(ADD_CONTROL_POINT_HISTORY, layer_name);
	history.layer_name = buff;

	// ���C���[�𖼑O�ŒT��
	layer = SearchLayer(window->layer, history.layer_name);

	// ����_���폜���郉�C����T��
	line = layer->layer_data.vector_layer_p->base;
	for(i=0; i<history.line_id; i++)
	{
		line = line->next;
	}

	if(line->num_points > history.point_id)
	{
		// ����_���𖄂߂č폜
		(void)memmove(&line->points[history.point_id],
			&line->points[history.point_id+1], sizeof(line->points[0])*(line->num_points-history.point_id));
	}

	line->num_points--;
}

/*****************************
* AddControlPointRedo�֐�    *
* ����_�̒ǉ�����蒼��     *
* ����                       *
* window	: �`��̈�̏�� *
* p			: �����f�[�^     *
*****************************/
static void AddControlPointRedo(DRAW_WINDOW* window, void* p)
{
	// �����f�[�^
	ADD_CONTROL_POINT_HISTORY history;
	// ����_��ǉ����郌�C���[
	LAYER* layer;
	// ����_��ǉ����郉�C��
	VECTOR_LINE* line;
	// �f�[�^���o�C�g�P�ʂň������߂ɃL���X�g
	int8* buff = (int8*)p;
	// �J�E���^
	unsigned int i;

	// �f�[�^���R�s�[
	(void)memcpy(&history, p, offsetof(ADD_CONTROL_POINT_HISTORY, layer_name));
	buff += offsetof(ADD_CONTROL_POINT_HISTORY, layer_name);
	history.layer_name = buff;

	// ���C���[�𖼑O�ŒT��
	layer = SearchLayer(window->layer, history.layer_name);

	// ����_��ǉ����郉�C����T��
	line = layer->layer_data.vector_layer_p->base;
	for(i=0; i<history.line_id; i++)
	{
		line = line->next;
	}

	(void)memmove(&line->points[history.point_id+1], &line->points[history.point_id],
		sizeof(line->points[0])*(line->num_points-history.point_id));

	line->points[history.point_id].x = history.x;
	line->points[history.point_id].y = history.y;
	line->points[history.point_id].pressure = history.pressure;
	line->points[history.point_id].size = history.size;
	(void)memcpy(line->points[history.point_id].color, history.color, 4);

	line->num_points++;
}

/*******************************************
* AddControlPointHistory�֐�               *
* ����_�ǉ��̗������쐬����               *
* ����                                     *
* window	: �`��̈�̏��               *
* layer		: ����_��ǉ��������C���[     *
* line		: ����_��ǉ��������C��       *
* point		: �ǉ���������_�̃A�h���X     *
* tool_name	: ����_��ǉ������c�[���̖��O *
*******************************************/
void AddControlPointHistory(
	DRAW_WINDOW* window,
	LAYER* layer,
	VECTOR_LINE* line,
	VECTOR_POINT* point,
	const char* tool_name
)
{
	// �����f�[�^
	ADD_CONTROL_POINT_HISTORY* history;
	// ���C���[���̒���
	uint16 name_length;
	// �����f�[�^�̃T�C�Y
	size_t data_size;
	// �f�[�^�R�s�[��̃A�h���X
	uint8* buff;
	// ���C����ID�v�Z�p
	VECTOR_LINE* comp_line = layer->layer_data.vector_layer_p->base;
	// for���p�̃J�E���^
	unsigned int i;

	// ���C���[���̒������擾
	name_length = (uint16)strlen(layer->name)+1;
	// �����f�[�^�̃o�C�g�����v�Z���ă������m��
	data_size = offsetof(ADD_CONTROL_POINT_HISTORY, layer_name) + name_length;
	history = (ADD_CONTROL_POINT_HISTORY*)MEM_ALLOC_FUNC(data_size);
	// ����_�̏��𗚗��f�[�^�ɃZ�b�g
	history->x = point->x, history->y = point->y;
	history->pressure = point->pressure;
	history->size = point->size;
	history->layer_name_length = name_length;
	(void)memcpy(history->color, point->color, 3);

	// ���C����ID���v�Z
	i = 0;
	while(comp_line != line)
	{
		i++;
		comp_line = comp_line->next;
	}
	history->line_id = i;

	// ����_��ID���v�Z
	for(i=0; i<=line->num_points; i++)
	{
		if(&line->points[i] == point)
		{
			history->point_id = i;
			break;
		}
	}

	// ���C���[�����R�s�[
	buff = (uint8*)history;
	buff += offsetof(ADD_CONTROL_POINT_HISTORY, layer_name);
	(void)memcpy(buff, layer->name, name_length);

	AddHistory(&window->history, tool_name, history, (uint32)data_size,
		AddControlPointUndo, AddControlPointRedo);
}

/***************************************************
* ADD_TOP_LINE_CONTROL_POINT_HISTORY�\����         *
* �܂���c�[�����Ȑ��c�[���ł̐���_�ǉ������f�[�^ *
***************************************************/
typedef struct _ADD_TOP_LINE_CONTROL_POINT_HISTORY
{
	gdouble x, y;				// �ǉ���������_�̍��W
	gdouble pressure;			// �ǉ���������_�̕M��
	gdouble size;				// �ǉ���������_�̃T�C�Y
	uint8 color[4];				// �ǉ���������_�̐F
	gdouble line_x, line_y;		// ���C���̊J�n���W
	gdouble line_pressure;		// ���C���̊J�n�M��
	gdouble line_size;			// ���C���̊J�n�T�C�Y
	uint8 line_color[4];		// ���C���̊J�n�F
	uint8 add_line_flag;		// ���C����ǉ�����t���O
	uint8 vector_type;			// ���C���̎��
	uint16 layer_name_length;	// ����_��ǉ��������C���[�̖��O�̒���
	char* layer_name;			// ����_��ǉ��������C���[�̖��O
} ADD_TOP_LINE_CONTROL_POINT_HISTORY;

/*****************************************************
* AddTopLineControlPointUndo�֐�                     *
* �܂���c�[�����Ȑ��c�[���ł̐���_�̒ǉ������ɖ߂� *
* ����                                               *
* window	: �`��̈�̏��                         *
* p			: �����f�[�^                             *
*****************************************************/
static void AddTopLineControlPointUndo(DRAW_WINDOW* window, void* p)
{
	// �����f�[�^
	ADD_TOP_LINE_CONTROL_POINT_HISTORY history;
	// ����_��ǉ����郌�C���[
	LAYER* layer;
	// ����_��ǉ����郉�C��
	VECTOR_LINE* line;
	// �f�[�^���o�C�g�P�ʂň������߂ɃL���X�g
	int8* buff = (int8*)p;
	// �ω��������C�����Œ肷��t���O
	int fix_flag = TRUE;
	// ���C���̃^�C�v
	uint8 line_type;

	// �f�[�^���R�s�[
	(void)memcpy(&history, p, offsetof(ADD_TOP_LINE_CONTROL_POINT_HISTORY, layer_name));
	buff += offsetof(ADD_TOP_LINE_CONTROL_POINT_HISTORY, layer_name);
	history.layer_name = buff;

	// ���C���[�𖼑O�ŒT��
	layer = SearchLayer(window->layer, history.layer_name);

	// ���C���ǉ��̗����ł���΃��C���폜
	if(history.add_line_flag != FALSE)
	{
		line = layer->layer_data.vector_layer_p->top_line;
		line_type = line->vector_type;
		layer->layer_data.vector_layer_p->top_line =
			layer->layer_data.vector_layer_p->top_line->prev;
		DeleteVectorLine(&line);
		fix_flag = FALSE;
	}
	else
	{	// �Ō�̓_���폜
		line = layer->layer_data.vector_layer_p->top_line;
		line_type = line->vector_type;
		line->num_points--;
	}

	if(layer == window->active_layer)
	{
		if(line_type == VECTOR_LINE_STRAIGHT
			&& window->app->tool_window.active_vector_brush[window->app->input]->brush_type == TYPE_POLY_LINE_BRUSH)
		{
			POLY_LINE_BRUSH* brush = (POLY_LINE_BRUSH*)window->app->tool_window.active_vector_brush[window->app->input]->brush_data;
			if((brush->flags & POLY_LINE_START) != 0)
			{
				if(history.add_line_flag != 0)
				{
					brush->flags &= ~(POLY_LINE_START);
					(void)memset(window->work_layer->pixels, 0, window->pixel_buf_size);
				}
				else if(line != NULL)
				{
					if(line->layer != NULL)
					{
						layer->layer_data.vector_layer_p->flags = VECTOR_LAYER_RASTERIZE_ACTIVE;
						layer->layer_data.vector_layer_p->active_line = line;
						brush->flags |= POLY_LINE_START;
						DeleteVectorLineLayer(&line->layer);
					}
				}
				fix_flag = FALSE;
			}
			else if(line != NULL)
			{
				if(line->layer != NULL)
				{
					brush->flags |= POLY_LINE_START;
					layer->layer_data.vector_layer_p->flags = VECTOR_LAYER_RASTERIZE_ACTIVE;
					layer->layer_data.vector_layer_p->active_line = line;
					brush->flags |= BEZIER_LINE_START;
					DeleteVectorLineLayer(&line->layer);
					line->num_points++;
					fix_flag = FALSE;
				}
			}
		}
		else if(line_type == VECTOR_LINE_BEZIER_OPEN
			&& window->app->tool_window.active_vector_brush[window->app->input]->brush_type == TYPE_BEZIER_LINE_BRUSH)
		{
			BEZIER_LINE_BRUSH* brush = (BEZIER_LINE_BRUSH*)window->app->tool_window.active_vector_brush[window->app->input]->brush_data;
			if((brush->flags & BEZIER_LINE_START) != 0)
			{
				if(history.add_line_flag != 0)
				{
					brush->flags &= ~(BEZIER_LINE_START);
					(void)memset(window->work_layer->pixels, 0, window->pixel_buf_size);
				}
				fix_flag = FALSE;
			}
			else if(line != NULL)
			{
				if(line->layer != NULL)
				{
					brush->flags |= BEZIER_LINE_START;
					layer->layer_data.vector_layer_p->flags = VECTOR_LAYER_RASTERIZE_ACTIVE;
					layer->layer_data.vector_layer_p->active_line = line;
					brush->flags |= BEZIER_LINE_START;
					DeleteVectorLineLayer(&line->layer);
					line->num_points++;
					fix_flag = FALSE;
				}
			}
		}
	}

	// ���C�������X�^���C�Y
	if(layer->layer_data.vector_layer_p->flags != VECTOR_LAYER_RASTERIZE_ACTIVE)
	{
		layer->layer_data.vector_layer_p->flags = VECTOR_LAYER_RASTERIZE_TOP;
	}
	if(fix_flag != FALSE)
	{
		layer->layer_data.vector_layer_p->flags |= VECTOR_LAYER_FIX_LINE;
	}
	RasterizeVectorLayer(window, layer, layer->layer_data.vector_layer_p);
	layer->layer_data.vector_layer_p->active_line = NULL;
	(void)memset(window->work_layer->pixels, 0, window->pixel_buf_size);

	// �\�����X�V
	if(layer == window->active_layer)
	{
		window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_OVER;
	}
	else
	{
		window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	}

	layer->layer_data.vector_layer_p->flags = VECTOR_LAYER_RASTERIZE_TOP;
}

/*****************************************************
* AddTopLineControlPointRedo�֐�                     *
* �܂���c�[�����Ȑ��c�[���ł̐���_�̒ǉ�����蒼�� *
* ����                                               *
* window	: �`��̈�̏��                         *
* p			: �����f�[�^                             *
*****************************************************/
static void AddTopLineControlPointRedo(DRAW_WINDOW* window, void* p)
{
	// �����f�[�^
	ADD_TOP_LINE_CONTROL_POINT_HISTORY history;
	// ����_��ǉ����郌�C���[
	LAYER* layer;
	// ����_��ǉ����郉�C��
	VECTOR_LINE* line;
	// �f�[�^���o�C�g�P�ʂň������߂ɃL���X�g
	int8* buff = (int8*)p;
	// �ω��������C�����Œ肷��t���O
	int fix_flag = TRUE;

	// �f�[�^���R�s�[
	(void)memcpy(&history, p, offsetof(ADD_TOP_LINE_CONTROL_POINT_HISTORY, layer_name));
	buff += offsetof(ADD_TOP_LINE_CONTROL_POINT_HISTORY, layer_name);
	history.layer_name = buff;

	// ���C���[�𖼑O�ŒT��
	layer = SearchLayer(window->layer, history.layer_name);

	// ���C���ǉ��̗����ł���΃��C���������m��
	if(history.add_line_flag == 0)
	{
		line = layer->layer_data.vector_layer_p->top_line;
	}
	else
	{	// ���C���ǉ�
		line = CreateVectorLine(layer->layer_data.vector_layer_p->top_line,
			layer->layer_data.vector_layer_p->top_line->next);
		// ����_�o�b�t�@�쐬
		line->buff_size = VECTOR_LINE_BUFFER_SIZE;
		line->points = (VECTOR_POINT*)MEM_ALLOC_FUNC(sizeof(*line->points)*line->buff_size);
		// �J�n�_��ݒ�
		line->points->x = history.line_x, line->points->y = history.line_y;
		line->points->pressure = history.line_pressure;
		line->points->size = history.line_size;
		(void)memcpy(line->points->color, history.line_color, 4);
		line->num_points = 1;
		// ��ԏ�̃��C���ύX
		layer->layer_data.vector_layer_p->top_line = line;
		line->vector_type = history.vector_type;
	}

	line->points[line->num_points].x = history.x;
	line->points[line->num_points].y = history.y;
	line->points[line->num_points].pressure = history.pressure;
	line->points[line->num_points].size = history.size;
	(void)memcpy(line->points[line->num_points].color, history.color, 4);

	if(layer == window->active_layer)
	{
		if(line->vector_type == VECTOR_LINE_STRAIGHT
			&& window->app->tool_window.active_vector_brush[window->app->input]->brush_type == TYPE_POLY_LINE_BRUSH)
		{
			POLY_LINE_BRUSH* brush = (POLY_LINE_BRUSH*)window->app->tool_window.active_vector_brush[window->app->input]->brush_data;
			brush->flags |= POLY_LINE_START;
			fix_flag = FALSE;
		}
		else if(line->vector_type == VECTOR_LINE_BEZIER_OPEN
			&& window->app->tool_window.active_vector_brush[window->app->input]->brush_type == TYPE_BEZIER_LINE_BRUSH)
		{
			BEZIER_LINE_BRUSH* brush = (BEZIER_LINE_BRUSH*)window->app->tool_window.active_vector_brush[window->app->input]->brush_data;
			brush->flags |= BEZIER_LINE_START;
			fix_flag = FALSE;
		}
	}

	line->num_points++;

	// ���C�������X�^���C�Y
	layer->layer_data.vector_layer_p->flags = VECTOR_LAYER_RASTERIZE_TOP;
	if(fix_flag != FALSE)
	{
		layer->layer_data.vector_layer_p->flags |= VECTOR_LAYER_FIX_LINE;
	}
	RasterizeVectorLayer(window, layer, layer->layer_data.vector_layer_p);

	// �\�����X�V
	if(layer == window->active_layer)
	{
		window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_OVER;
	}
	else
	{
		window->flags |= DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	}
}

/***************************************************
* AddTopLineControlPointHistory�֐�                *
* �܂���c�[�����Ȑ��c�[���ł̐���_�ǉ�������ǉ� *
* ����                                             *
* window		: �`��̈�̏��                   *
* layer			: ����_��ǉ��������C���[         *
* line			: ����_��ǉ��������C��           *
* point			: �ǉ���������_                   *
* tool_name		: ����_��ǉ������c�[���̖��O     *
* add_line_flag	: ���C���ǉ��������Ƃ��̃t���O     *
***************************************************/
void AddTopLineControlPointHistory(
	DRAW_WINDOW* window,
	LAYER* layer,
	VECTOR_LINE* line,
	VECTOR_POINT* point,
	const char* tool_name,
	uint8 add_line_flag
)
{
	// �����f�[�^
	ADD_TOP_LINE_CONTROL_POINT_HISTORY* history;
	// ���C���[���̒���
	uint16 name_length;
	// �����f�[�^�̃T�C�Y
	size_t data_size;
	// �f�[�^�R�s�[��̃A�h���X
	uint8* buff;

	// ���C���[���̒������擾
	name_length = (uint16)strlen(layer->name)+1;
	// �����f�[�^�̃o�C�g�����v�Z���ă������m��
	data_size = offsetof(ADD_TOP_LINE_CONTROL_POINT_HISTORY, layer_name) + name_length;
	history = (ADD_TOP_LINE_CONTROL_POINT_HISTORY*)MEM_ALLOC_FUNC(data_size);
	// ����_�̏��𗚗��f�[�^�ɃZ�b�g
	history->x = point->x, history->y = point->y;
	history->pressure = point->pressure;
	history->size = point->size;
	history->add_line_flag = add_line_flag;
	history->vector_type = line->vector_type;
	history->layer_name_length = name_length;
	(void)memcpy(history->color, point->color, 3);
	// ���C���̊J�n�ʒu�̏��𗚗��f�[�^�ɃZ�b�g
	history->line_x = line->points->x;
	history->line_y = line->points->y;
	history->line_pressure = line->points->pressure;
	history->line_size = line->points->size;
	(void)memcpy(history->line_color, line->points->color, 4);

	// ���C���[�����R�s�[
	buff = (uint8*)history;
	buff += offsetof(ADD_TOP_LINE_CONTROL_POINT_HISTORY, layer_name);
	(void)memcpy(buff, layer->name, name_length);

	AddHistory(&window->history, tool_name, history, (uint32)data_size,
		AddTopLineControlPointUndo, AddTopLineControlPointRedo);
}

void DivideLinesUndo(DRAW_WINDOW* window, void* p)
{
	DIVIDE_LINE_DATA data;
	LAYER *target = window->layer;
	VECTOR_LAYER *layer;
	VECTOR_LINE *line;
	uint8 *byte_data = (uint8*)p;
	uint32 update_flag = DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	int line_index = 0;
	int num_delete = 0;
	int num_change = 0;

	(void)memcpy(&data, p, offsetof(DIVIDE_LINE_DATA, layer_name));
	data.layer_name = (char*)(byte_data + offsetof(DIVIDE_LINE_DATA, layer_name));
	data.change = (DIVIDE_LINE_CHANGE_DATA*)&byte_data[
		offsetof(DIVIDE_LINE_DATA, layer_name) + data.layer_name_length];
	data.add = (DIVIDE_LINE_ADD_DATA*)&byte_data[
		offsetof(DIVIDE_LINE_DATA, layer_name) + data.layer_name_length + data.change_size];

	while(target != NULL)
	{
		if(target == window->active_layer)
		{
			update_flag = DRAW_WINDOW_UPDATE_ACTIVE_OVER;
		}

		if(strcmp(target->name, data.layer_name) == 0)
		{
			layer = target->layer_data.vector_layer_p;
			break;
		}
		target = target->next;
	}

	line = layer->base->next;
	while(line != NULL && num_delete < data.num_add)
	{
		while(data.add->index == line_index)
		{
			VECTOR_LINE *next_line = line->next;
			uint8 *byte_p = (uint8*)data.add;
			DeleteVectorLine(&line);
			data.add = (DIVIDE_LINE_ADD_DATA*)&byte_p[data.add->data_size];
			num_delete++;
			line = next_line;
		}

		if(line != NULL)
		{
			line = line->next;
		}
		line_index++;
	}

	line_index = 0;
	line = layer->base->next;
	while(line != NULL && num_change < data.num_change)
	{
		if(data.change->index == line_index)
		{
			uint8 *byte_p = (uint8*)data.change;
			line->flags |= VECTOR_LINE_FIX;
			line->vector_type = data.change->line_type;
			line->num_points = data.change->before_num_points;
			byte_p = &byte_p[offsetof(DIVIDE_LINE_CHANGE_DATA, before)];
			(void)memcpy(line->points, byte_p, sizeof(*line->points)*data.change->before_num_points);
			byte_p = (uint8*)data.change;
			data.change = (DIVIDE_LINE_CHANGE_DATA*)&byte_p[data.change->data_size];
			num_change++;
		}

		line = line->next;
		line_index++;
	}

	layer->flags |= VECTOR_LAYER_RASTERIZE_CHECKED;
	(void)memset(window->work_layer->pixels, 0, window->pixel_buf_size);
	RasterizeVectorLayer(window, target, layer);

	window->flags |= update_flag;
}

void DivideLinesRedo(DRAW_WINDOW* window, void* p)
{
	DIVIDE_LINE_DATA data;
	LAYER *target = window->layer;
	VECTOR_LAYER *layer;
	VECTOR_LINE *line;
	uint8 *byte_data = (uint8*)p;
	uint32 update_flag = DRAW_WINDOW_UPDATE_ACTIVE_UNDER;
	int line_index = 0;
	int num_add = 0;
	int num_change = 0;

	(void)memcpy(&data, p, offsetof(DIVIDE_LINE_DATA, layer_name));
	data.layer_name = (char*)&byte_data[offsetof(DIVIDE_LINE_DATA, layer_name)];
	data.change = (DIVIDE_LINE_CHANGE_DATA*)&byte_data[
		offsetof(DIVIDE_LINE_DATA, layer_name) + data.layer_name_length];
	data.add = (DIVIDE_LINE_ADD_DATA*)&byte_data[
		offsetof(DIVIDE_LINE_DATA, layer_name) + data.layer_name_length + data.change_size];

	while(target != NULL)
	{
		if(target == window->active_layer)
		{
			update_flag = DRAW_WINDOW_UPDATE_ACTIVE_OVER;
		}

		if(strcmp(target->name, data.layer_name) == 0)
		{
			layer = target->layer_data.vector_layer_p;
			break;
		}
		target = target->next;
	}

	line = layer->base->next;
	while(line != NULL && num_change < data.num_change)
	{
		while(data.add->index == line_index)
		{
			uint8 *byte_p = (uint8*)data.change;
			line->flags |= VECTOR_LINE_FIX;
			line->num_points = data.change->after_num_points;
			byte_p = &byte_p[offsetof(DIVIDE_LINE_CHANGE_DATA, before)];
			byte_p = &byte_p[sizeof(*line->points)*data.change->before_num_points];
			(void)memcpy(line->points, byte_p, sizeof(*line->points)*data.change->after_num_points);
			byte_p = (uint8*)data.change;
			data.change = (DIVIDE_LINE_CHANGE_DATA*)&byte_p[data.change->data_size];
			num_change++;
		}

		line = line->next;
		line_index++;
	}

	line_index = 0;
	line = layer->base->next;
	while(line != NULL && num_add < data.num_add)
	{
		while(data.change->index == line_index)
		{
			uint8 *byte_p = (uint8*)data.add;
			line = CreateVectorLine(line, line->next);
			line->flags = VECTOR_LINE_FIX;
			line->vector_type = data.add->line_type;
			line->num_points = data.add->num_points;
			
			line->buff_size =
				((line->num_points/VECTOR_LINE_BUFFER_SIZE)+1) * VECTOR_LINE_BUFFER_SIZE;
			line->points = (VECTOR_POINT*)MEM_ALLOC_FUNC(
				sizeof(*line->points)*line->buff_size);

			byte_p = &byte_p[offsetof(DIVIDE_LINE_ADD_DATA, after)];
			(void)memcpy(line->points, byte_p, sizeof(*line->points)*data.add->num_points);
			byte_p = (uint8*)data.add;
			data.add = (DIVIDE_LINE_ADD_DATA*)&byte_p[data.add->data_size];
			num_add++;
		}

		line = line->next;
		line_index++;
	}

	layer->flags |= VECTOR_LAYER_RASTERIZE_CHECKED;
	(void)memset(window->work_layer->pixels, 0, window->pixel_buf_size);
	RasterizeVectorLayer(window, target, layer);

	window->flags |= update_flag;
}

typedef struct _DELETE_LINES_HISTORY_DATA
{
	uint16 layer_name_length;
	uint32 num_lines;
	char *layer_name;
	VECTOR_LINE *lines;
	VECTOR_POINT **points;
	uint32 *indexes;
} DELETE_LINES_HISTORY_DATA;

static void DeleteLinesUndo(DRAW_WINDOW* window, void* p)
{
	DELETE_LINES_HISTORY_DATA data;
	uint8 *byte_data = (uint8*)p;
	LAYER *layer = window->layer;
	VECTOR_LINE *line;
	VECTOR_LINE *ressurect_line;
	uint32 exec_lines = 0;
	uint32 i, j;

	(void)memcpy(&data.layer_name_length, byte_data,
		sizeof(data.layer_name_length));
	byte_data += sizeof(data.layer_name_length);
	(void)memcpy(&data.num_lines, byte_data,
		sizeof(data.num_lines));
	byte_data += sizeof(data.num_lines);
	data.layer_name = (char*)byte_data;
	byte_data += data.layer_name_length;
	data.lines = (VECTOR_LINE*)byte_data;
	byte_data += sizeof(*data.lines)*data.num_lines;
	data.points = (VECTOR_POINT**)MEM_ALLOC_FUNC(
		sizeof(*data.points)*data.num_lines);
	for(i=0; i<data.num_lines; i++)
	{
		data.points[i] = (VECTOR_POINT*)byte_data;
		byte_data += sizeof(**data.points) * data.lines[i].num_points;
	}
	data.indexes = (uint32*)byte_data;

	while(strcmp(layer->name, data.layer_name) != 0)
	{
		layer = layer->next;
	}

	line = layer->layer_data.vector_layer_p->base;
	i = 0;
	while(line != NULL)
	{
		if(i == data.indexes[exec_lines])
		{
			ressurect_line = CreateVectorLine(line, line->next);
			if(line == layer->layer_data.vector_layer_p->top_line)
			{
				layer->layer_data.vector_layer_p->top_line = line;
			}
			ressurect_line->vector_type = data.lines[exec_lines].vector_type;
			ressurect_line->flags |= VECTOR_LINE_FIX;
			ressurect_line->num_points = data.lines[exec_lines].num_points;
			ressurect_line->buff_size = data.lines[exec_lines].buff_size;
			ressurect_line->blur = data.lines[exec_lines].blur;
			ressurect_line->outline_hardness = data.lines[exec_lines].outline_hardness;
			ressurect_line->points = (VECTOR_POINT*)MEM_ALLOC_FUNC(
				sizeof(*ressurect_line->points)*ressurect_line->buff_size);
			(void)memset(ressurect_line->points, 0,
				sizeof(*ressurect_line->points)*ressurect_line->buff_size);
			for(j=0; j<ressurect_line->num_points; j++)
			{
				ressurect_line->points[j] = data.points[exec_lines][j];
			}

			exec_lines++;
			if(exec_lines >= data.num_lines)
			{
				break;
			}
		}

		line = line->next;
		i++;
	}

	layer->layer_data.vector_layer_p->flags |=
		VECTOR_LAYER_FIX_LINE | VECTOR_LAYER_RASTERIZE_CHECKED;
	RasterizeVectorLayer(window, layer, layer->layer_data.vector_layer_p);

	MEM_FREE_FUNC(data.points);
}

static void DeleteLinesRedo(DRAW_WINDOW* window, void* p)
{
	DELETE_LINES_HISTORY_DATA data;
	uint8 *byte_data = (uint8*)p;
	LAYER *layer = window->layer;
	VECTOR_LINE *line;
	uint32 exec_lines = 0;
	uint32 i;

	(void)memcpy(&data.layer_name_length, byte_data,
		sizeof(data.layer_name_length));
	byte_data += sizeof(data.layer_name_length);
	(void)memcpy(&data.num_lines, byte_data,
		sizeof(data.num_lines));
	byte_data += sizeof(data.num_lines);
	data.layer_name = (char*)byte_data;
	byte_data += data.layer_name_length;
	data.lines = (VECTOR_LINE*)byte_data;
	byte_data += sizeof(*data.lines)*data.num_lines;
	for(i=0; i<data.num_lines; i++)
	{
		byte_data += sizeof(**data.points) * data.lines[i].num_points;
	}
	data.indexes = (uint32*)byte_data;

	while(strcmp(layer->name, data.layer_name) != 0)
	{
		layer = layer->next;
	}

	line = layer->layer_data.vector_layer_p->base->next;
	i = 0;
	while(line != NULL)
	{
		if(i == data.indexes[exec_lines])
		{
			VECTOR_LINE *prev_line = line->prev;
			DeleteVectorLine(&line);
			line = prev_line;

			exec_lines++;
			if(exec_lines >= data.num_lines)
			{
				break;
			}
		}

		line = line->next;
		i++;
	}

	layer->layer_data.vector_layer_p->flags |=
		VECTOR_LAYER_RASTERIZE_ACTIVE;
	RasterizeVectorLayer(window, layer, layer->layer_data.vector_layer_p);
	(void)memset(window->work_layer->pixels, 0, window->pixel_buf_size);
}

/***********************************************************
* AddDeleteLinesHistory�֐�                                *
* �����̐��𓯎��ɍ폜�����ۂ̗����f�[�^��ǉ�             *
* ����                                                     *
* window		: �`��̈�̏����Ǘ�����\���̂̃A�h���X *
* active_layer	: �A�N�e�B�u�ȃ��C���[                     *
* line_data		: �폜������̃f�[�^                       *
* line_indexes	: �폜������̈�ԉ����琔�����C���f�b�N�X *
* num_lines		: �폜������̐�                           *
* tool_name		: �폜�Ɏg�p�����c�[���̖��O               *
***********************************************************/
void AddDeleteLinesHistory(
	DRAW_WINDOW* window,
	LAYER* active_layer,
	VECTOR_LINE* line_data,
	uint32* line_indexes,
	uint32 num_lines,
	const char* tool_name
)
{
	MEMORY_STREAM_PTR stream = CreateMemoryStream(8096);
	uint16 name_length = (uint16)strlen(active_layer->name) + 1;
	uint32 i;

	(void)MemWrite(&name_length, sizeof(name_length), 1, stream);
	(void)MemWrite(&num_lines, sizeof(num_lines),1, stream);
	(void)MemWrite(active_layer->name, 1, name_length, stream);
	(void)MemWrite(line_data, sizeof(*line_data), num_lines, stream);

	for(i=0; i<num_lines; i++)
	{
		(void)MemWrite(line_data[i].points, sizeof(*line_data[0].points),
			line_data[i].num_points, stream);
	}

	(void)MemWrite(line_indexes, sizeof(*line_indexes), num_lines, stream);
	AddHistory(&window->history, tool_name, stream->buff_ptr,
		stream->data_point, DeleteLinesUndo, DeleteLinesRedo);

	DeleteMemoryStream(stream);
}

#ifdef __cplusplus
}
#endif
