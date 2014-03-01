/*
 * TilEm II
 *
 * Copyright (c) 2011-2013 Benjamin Moody
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define GDK_PIXBUF_ENABLE_BACKEND

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"
#include "colorquant.h"

#define GAMMA 2.2

struct _TilemAnimFrame {
	struct _TilemAnimFrame *next;
	unsigned duration : 24;
	unsigned contrast : 8;
	byte data[1];
};

struct _TilemAnimation {
	GdkPixbufAnimation parent;

	int num_frames;
	TilemAnimFrame *start;
	TilemAnimFrame *end;
	dword last_stamp;

	TilemLCDBuffer *temp_buffer;

	GdkPixbuf *static_pixbuf;

	int base_contrast;
	int display_width;
	int display_height;
	int lcdbuf_format;
	int frame_rowstride;
	int frame_size;
	int image_width;
	int image_height;
	dword *palette;
	gdouble speed;
	gdouble time_stretch;

	gboolean quantize_preview;
	ColorHistogram *cq_hist;
	ColorPalette *cq_pal;
	gboolean cq_grayscale;
	gboolean cq_rgb_fixed;
	int cq_color_cube_size;
	guint8 *cq_image_buf;
	int cq_image_buf_size;

	gboolean out_of_memory;
};

struct _TilemAnimationClass {
	GdkPixbufAnimationClass parent_class;
};

#define TILEM_TYPE_ANIM_ITER           (tilem_anim_iter_get_type())
#define TILEM_ANIM_ITER(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), TILEM_TYPE_ANIM_ITER, TilemAnimIter))
#define TILEM_ANIM_ITER_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST((cls), TILEM_TYPE_ANIM_ITER, TilemAnimIterClass))
#define TILEM_IS_ANIM_ITER(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj), TILEM_TYPE_ANIM_ITER))
#define TILEM_IS_ANIM_ITER_CLASS(cls)  (G_TYPE_CHECK_CLASS_TYPE((cls), TILEM_TYPE_ANIM_ITER))
#define TILEM_ANIM_ITER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), TILEM_TYPE_ANIM_ITER, TilemAnimIterClass))

typedef struct _TilemAnimIter {
	GdkPixbufAnimationIter parent;
	GTimeVal current_time;
	int time_elapsed;
	TilemAnimation *anim;
	TilemAnimFrame *frame;
	GdkPixbuf *pixbuf;
} TilemAnimIter;

typedef struct _TilemAnimIterClass {
	GdkPixbufAnimationIterClass parent_class;
} TilemAnimIterClass;

G_DEFINE_TYPE(TilemAnimation, tilem_animation,
              GDK_TYPE_PIXBUF_ANIMATION);

G_DEFINE_TYPE(TilemAnimIter, tilem_anim_iter,
              GDK_TYPE_PIXBUF_ANIMATION_ITER);

static TilemAnimFrame * alloc_frame(int bufsize)
{
	TilemAnimFrame *frm;

	frm = g_try_malloc(sizeof(TilemAnimFrame) + bufsize - 1);
	if (!frm)
		return NULL;
	frm->next = NULL;
	return frm;
}

static void free_frame(TilemAnimFrame *frm)
{
	g_free(frm);
}

static int adjust_contrast(TilemAnimation *anim, int contrast)
{
	TilemAnimFrame *frm;

	if (!contrast)
		return 0;

	if (!anim->base_contrast) {
		for (frm = anim->start; frm; frm = frm->next) {
			if (frm->contrast != 0) {
				anim->base_contrast = frm->contrast;
				break;
			}
		}
	}

	contrast = (contrast - anim->base_contrast + 32);
	return CLAMP(contrast, 0, 63);
}

static void set_lcdbuf_from_frame(TilemAnimation *anim, 
                                  TilemLCDBuffer *buf,
                                  const TilemAnimFrame *frm)
{
	buf->width = anim->display_width;
	buf->height = anim->display_height;
	buf->format = anim->lcdbuf_format;
	buf->rowstride = anim->frame_rowstride;
	buf->contrast = adjust_contrast(anim, frm->contrast);
	buf->data = (byte *) frm->data;
}

static void generate_cq_palette(TilemAnimation *anim)
{
	int csize = anim->cq_color_cube_size;

	if (anim->cq_pal || !anim->cq_hist)
		return;

	if (anim->cq_grayscale)
		anim->cq_pal = color_palette_new_gray(256);
	else if (anim->cq_rgb_fixed)
		anim->cq_pal = color_palette_new_rgb(256, NULL,
		                                     csize, csize, csize);
	else if (color_histogram_num_colors(anim->cq_hist) <= 256)
		anim->cq_pal = color_palette_new_rgb(256, anim->cq_hist,
		                                     0, 0, 0);
	else
		anim->cq_pal = color_palette_new_rgb(256, anim->cq_hist,
		                                     csize, csize, csize);
}

static void alloc_cq_image_buf(TilemAnimation *anim, int width, int height)
{
	if (anim->cq_image_buf_size >= width * height * 3)
		return;

	anim->cq_image_buf_size = width * height * 3;
	g_free(anim->cq_image_buf);
	anim->cq_image_buf = g_new(byte, anim->cq_image_buf_size);
}

static GdkPixbuf * frame_to_pixbuf(TilemAnimation *anim,
                                   const TilemAnimFrame *frm,
                                   gboolean preview)
{
	GdkPixbuf *pb;
	int width = anim->image_width, height = anim->image_height;
	byte * restrict bp;
	byte * restrict pix;
	const dword * restrict palcolors;
	int i, j, stride;

	pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
	pix = gdk_pixbuf_get_pixels(pb);
	stride = gdk_pixbuf_get_rowstride(pb);

	set_lcdbuf_from_frame(anim, anim->temp_buffer, frm);
	tilem_draw_lcd_image_rgb(anim->temp_buffer,
	                         pix, width, height, stride,
	                         3, anim->palette,
	                         TILEM_SCALE_SMOOTH);
	anim->temp_buffer->data = NULL;

	if (preview && anim->quantize_preview && anim->cq_hist) {
		generate_cq_palette(anim);
		alloc_cq_image_buf(anim, width, height);

		color_palette_quantize_image(anim->cq_pal, anim->cq_image_buf,
		                             pix, width, height, stride);

		bp = anim->cq_image_buf;
		palcolors = anim->cq_pal->colors;
		for (i = 0; i < height; i++) {
			for (j = 0; j < width; j++) {
				pix[3 * j] = palcolors[bp[j]] >> 16;
				pix[3 * j + 1] = palcolors[bp[j]] >> 8;
				pix[3 * j + 2] = palcolors[bp[j]];
			}
			bp += width;
			pix += stride;
		}
	}

	return pb;
}

static gboolean tilem_animation_is_static_image(GdkPixbufAnimation *ganim)
{
	TilemAnimation *anim = TILEM_ANIMATION(ganim);

	g_return_val_if_fail(TILEM_IS_ANIMATION(ganim), FALSE);

	if (anim->start == anim->end)
		return TRUE;
	else
		return FALSE;
}

static GdkPixbuf * tilem_animation_get_static_image(GdkPixbufAnimation *ganim)
{
	TilemAnimation *anim = TILEM_ANIMATION(ganim);

	g_return_val_if_fail(TILEM_IS_ANIMATION(anim), NULL);
	g_return_val_if_fail(anim->start != NULL, NULL);

	if (!anim->static_pixbuf)
		anim->static_pixbuf = frame_to_pixbuf(anim, anim->start, TRUE);

	return anim->static_pixbuf;
}

static void tilem_animation_get_size(GdkPixbufAnimation *ganim,
                                     int *width, int *height)
{
	TilemAnimation *anim = TILEM_ANIMATION(ganim);

	g_return_if_fail(TILEM_IS_ANIMATION(anim));

	if (width) *width = anim->image_width;
	if (height) *height = anim->image_height;
}

static GdkPixbufAnimationIter *
tilem_animation_get_iter(GdkPixbufAnimation *ganim,
                         const GTimeVal *start_time)
{
	TilemAnimation *anim = TILEM_ANIMATION(ganim);
	TilemAnimIter *iter;

	g_return_val_if_fail(TILEM_IS_ANIMATION(anim), NULL);

	iter = g_object_new(TILEM_TYPE_ANIM_ITER, NULL);
	iter->anim = anim;
	iter->frame = anim->start;
	iter->current_time = *start_time;

	g_object_ref(anim);

	return GDK_PIXBUF_ANIMATION_ITER(iter);
}

static void tilem_animation_init(G_GNUC_UNUSED TilemAnimation *anim)
{
}

static void tilem_animation_finalize(GObject *obj)
{
	TilemAnimation *anim = TILEM_ANIMATION(obj);
	TilemAnimFrame *frm;

	g_return_if_fail(TILEM_IS_ANIMATION(anim));

	while (anim->start) {
		frm = anim->start;
		anim->start = frm->next;
		free_frame(frm);
	}

	anim->start = anim->end = NULL;

	if (anim->temp_buffer)
		tilem_lcd_buffer_free(anim->temp_buffer);
	anim->temp_buffer = NULL;

	if (anim->palette)
		tilem_free(anim->palette);
	anim->palette = NULL;

	if (anim->static_pixbuf)
		g_object_unref(anim->static_pixbuf);
	anim->static_pixbuf = NULL;

	g_free(anim->cq_image_buf);
	anim->cq_image_buf = NULL;
	anim->cq_image_buf_size = 0;

	if (anim->cq_hist)
		color_histogram_free(anim->cq_hist);
	anim->cq_hist = NULL;

	if (anim->cq_pal)
		color_palette_free(anim->cq_pal);
	anim->cq_pal = NULL;

	if (G_OBJECT_CLASS(tilem_animation_parent_class)->finalize)
		(*G_OBJECT_CLASS(tilem_animation_parent_class)->finalize)(obj);
}

static void tilem_animation_class_init(TilemAnimationClass *klass)
{
	GdkPixbufAnimationClass *aclass = GDK_PIXBUF_ANIMATION_CLASS(klass);
	GObjectClass *oclass = G_OBJECT_CLASS(klass);

	aclass->is_static_image  = tilem_animation_is_static_image;
	aclass->get_static_image = tilem_animation_get_static_image;
	aclass->get_size         = tilem_animation_get_size;
	aclass->get_iter         = tilem_animation_get_iter;

	oclass->finalize = tilem_animation_finalize;
}

static int tilem_anim_iter_get_delay_time(GdkPixbufAnimationIter *giter)
{
	TilemAnimIter *iter = TILEM_ANIM_ITER(giter);

	g_return_val_if_fail(TILEM_IS_ANIM_ITER(iter), 0);
	g_return_val_if_fail(iter->anim != NULL, 0);
	g_return_val_if_fail(iter->frame != NULL, 0);

	if (iter->anim->start == iter->anim->end)
		return -1;
	else
		return ((iter->frame->duration - iter->time_elapsed)
		        * iter->anim->time_stretch);
}

static GdkPixbuf * tilem_anim_iter_get_pixbuf(GdkPixbufAnimationIter *giter)
{
	TilemAnimIter *iter = TILEM_ANIM_ITER(giter);

	g_return_val_if_fail(TILEM_IS_ANIM_ITER(iter), NULL);
	g_return_val_if_fail(iter->anim != NULL, NULL);
	g_return_val_if_fail(iter->frame != NULL, NULL);

	if (!iter->pixbuf)
		iter->pixbuf = frame_to_pixbuf(iter->anim, iter->frame, TRUE);

	return iter->pixbuf;
}

static gboolean tilem_anim_iter_on_currently_loading_frame(G_GNUC_UNUSED GdkPixbufAnimationIter *giter)
{
	return FALSE;
}

static gboolean
tilem_anim_iter_advance(GdkPixbufAnimationIter *giter,
                        const GTimeVal *current_time)
{
	TilemAnimIter *iter = TILEM_ANIM_ITER(giter);
	int ms;

	g_return_val_if_fail(TILEM_IS_ANIM_ITER(iter), FALSE);
	g_return_val_if_fail(iter->anim != NULL, FALSE);
	g_return_val_if_fail(iter->frame != NULL, FALSE);

	ms = ((current_time->tv_usec - iter->current_time.tv_usec) / 1000
	      + (current_time->tv_sec - iter->current_time.tv_sec) * 1000);

	g_time_val_add(&iter->current_time, ms * 1000);

	ms *= iter->anim->speed;

	ms += iter->time_elapsed;
	if (ms < iter->frame->duration) {
		iter->time_elapsed = ms;
		return FALSE;
	}

	if (iter->pixbuf)
		g_object_unref(iter->pixbuf);
	iter->pixbuf = NULL;

	while (ms >= iter->frame->duration) {
		ms -= iter->frame->duration;
		if (iter->frame->next)
			iter->frame = iter->frame->next;
		else
			iter->frame = iter->anim->start;
	}

	iter->time_elapsed = ms;
	return TRUE;
}

static void tilem_anim_iter_init(G_GNUC_UNUSED TilemAnimIter *iter)
{
}

static void tilem_anim_iter_finalize(GObject *obj)
{
	TilemAnimIter *iter = TILEM_ANIM_ITER(obj);

	g_return_if_fail(TILEM_IS_ANIM_ITER(obj));

	if (iter->anim)
		g_object_unref(iter->anim);
	iter->anim = NULL;

	if (iter->pixbuf)
		g_object_unref(iter->pixbuf);
	iter->pixbuf = NULL;

	if (G_OBJECT_CLASS(tilem_anim_iter_parent_class)->finalize)
		(*G_OBJECT_CLASS(tilem_anim_iter_parent_class)->finalize)(obj);
}

static void tilem_anim_iter_class_init(TilemAnimIterClass *klass)
{
	GdkPixbufAnimationIterClass *iclass = GDK_PIXBUF_ANIMATION_ITER_CLASS(klass);
	GObjectClass *oclass = G_OBJECT_CLASS(klass);

	iclass->get_delay_time             = tilem_anim_iter_get_delay_time;
	iclass->get_pixbuf                 = tilem_anim_iter_get_pixbuf;
	iclass->on_currently_loading_frame = tilem_anim_iter_on_currently_loading_frame;
	iclass->advance                    = tilem_anim_iter_advance;

	oclass->finalize = tilem_anim_iter_finalize;
}

/* Colors used by the system; these colors will be treated as "fixed"
   for palette generation */
static const dword ti84pc_system_colors[] = {
	0x0000FF, /* 001F */
	0xFF0000, /* F800 */
	0x000000, /* 0000 */
	0xFF00FF, /* F81F */
	0x009E00, /* 04E0 */
	0xFF8E20, /* FC64 */
	0xB62000, /* B100 */
	0x000086, /* 0010 */
	0x0092FF, /* 049F */
	0xFFFF00, /* FFE0 */
	0xFFFFFF, /* FFFF */
	0xE7E3E7, /* E71C */
	0xC7C3C7, /* C618 */
	0x8E8A8E, /* 8C51 */
	0x515551, /* 52AA */
	0xBEEBCF, /* BF59 */
	0x69E3FF, /* 6F1F */
	0x00FF00, /* 07E0 */
	0x00FFFF, /* 07FF */
	0x79FFF7  /* 7FFE */
};

TilemAnimation * tilem_animation_new(int display_width, int display_height,
                                     int lcdbuf_format)
{
	TilemAnimation *anim;
	TilemAnimFrame *dummy_frame;

	g_return_val_if_fail(display_width > 0, NULL);
	g_return_val_if_fail(display_height > 0, NULL);
	g_return_val_if_fail(lcdbuf_format == TILEM_LCD_BUF_BLACK_128
	                     || lcdbuf_format == TILEM_LCD_BUF_SRGB_63,
	                     NULL);

	anim = g_object_new(TILEM_TYPE_ANIMATION, NULL);
	anim->display_width = display_width;
	anim->display_height = display_height;
	anim->lcdbuf_format = lcdbuf_format;
	if (lcdbuf_format == TILEM_LCD_BUF_BLACK_128)
		anim->frame_rowstride = (display_width + 7) & ~7;
	else
		anim->frame_rowstride = (display_width * 3);
	anim->frame_size = anim->frame_rowstride * display_height;

	anim->image_width = display_width;
	anim->image_height = display_height;
	anim->speed = 1.0;
	anim->time_stretch = 1.0;

	if (lcdbuf_format != TILEM_LCD_BUF_BLACK_128) {
		anim->cq_grayscale = FALSE;
		anim->cq_rgb_fixed = FALSE;
		anim->cq_color_cube_size = 6;

		anim->cq_hist = color_histogram_new();
		color_histogram_add_fixed_colors
			(anim->cq_hist, ti84pc_system_colors,
			 G_N_ELEMENTS(ti84pc_system_colors));
	}

	anim->temp_buffer = tilem_lcd_buffer_new();
	anim->palette = tilem_color_palette_new(255, 255, 255, 0, 0, 0, GAMMA);

	dummy_frame = alloc_frame(anim->frame_size);
	dummy_frame->duration = 0;
	dummy_frame->contrast = 0;
	anim->start = anim->end = dummy_frame;

	return anim;
}

gboolean tilem_animation_append_frame(TilemAnimation *anim,
                                      const TilemLCDBuffer *buf,
                                      int duration)
{
	TilemAnimFrame *frm;

	g_return_val_if_fail(TILEM_IS_ANIMATION(anim), FALSE);
	g_return_val_if_fail(anim->end != NULL, FALSE);
	g_return_val_if_fail(buf != NULL, FALSE);
	g_return_val_if_fail(buf->data != NULL, FALSE);
	g_return_val_if_fail(buf->height == anim->display_height, FALSE);
	g_return_val_if_fail(buf->rowstride == anim->frame_rowstride, FALSE);
	g_return_val_if_fail(buf->format == anim->lcdbuf_format, FALSE);

	if (anim->out_of_memory)
		return FALSE;

	if (anim->end->contrast == buf->contrast
	    && (anim->last_stamp == buf->stamp
	        || !memcmp(anim->end->data, buf->data, anim->frame_size))) {
		anim->end->duration += duration;
	}
	else {
		if (anim->end->duration == 0) {
			frm = anim->end;
		}
		else {
			frm = alloc_frame(anim->frame_size);
			if (!frm) {
				anim->out_of_memory = TRUE;
				return FALSE;
			}
			anim->end->next = frm;
			anim->end = frm;
		}

		frm->contrast = buf->contrast;
		frm->duration = duration;
		memcpy(frm->data, buf->data, anim->frame_size);

		if (buf->format != TILEM_LCD_BUF_BLACK_128) {
			alloc_cq_image_buf(anim, anim->display_width,
			                   anim->display_height);

			set_lcdbuf_from_frame(anim, anim->temp_buffer, frm);
			tilem_draw_lcd_image_rgb(anim->temp_buffer,
			                         anim->cq_image_buf,
			                         anim->display_width,
			                         anim->display_height,
			                         3 * anim->display_width,
			                         3, anim->palette,
			                         TILEM_SCALE_FAST);
			anim->temp_buffer->data = NULL;

			color_histogram_add_pixels(anim->cq_hist,
			                           anim->cq_image_buf,
			                           anim->frame_size / 3);
		}
	}

	anim->last_stamp = buf->stamp;
	return TRUE;
}

void tilem_animation_set_size(TilemAnimation *anim, int width, int height)
{
	g_return_if_fail(TILEM_IS_ANIMATION(anim));
	anim->image_width = width;
	anim->image_height = height;

	g_free(anim->cq_image_buf);
	anim->cq_image_buf = NULL;
	anim->cq_image_buf_size = 0;
}

void tilem_animation_set_colors(TilemAnimation *anim,
                                const GdkColor *foreground,
                                const GdkColor *background)
{
	g_return_if_fail(TILEM_IS_ANIMATION(anim));
	g_return_if_fail(foreground != NULL);
	g_return_if_fail(background != NULL);

	if (anim->palette)
		tilem_free(anim->palette);

	anim->palette = tilem_color_palette_new(background->red >> 8,
	                                        background->green >> 8,
	                                        background->blue >> 8,
	                                        foreground->red >> 8,
	                                        foreground->green >> 8,
	                                        foreground->blue >> 8,
	                                        GAMMA);
}

void tilem_animation_set_quantization(TilemAnimation *anim,
                                      gboolean grayscale,
                                      gboolean rgb_fixed,
                                      int color_cube_size)
{
	g_return_if_fail(TILEM_IS_ANIMATION(anim));
	g_return_if_fail(color_cube_size >= 0);
	g_return_if_fail(color_cube_size <= 6);

	if (anim->cq_pal) {
		color_palette_free(anim->cq_pal);
		anim->cq_pal = NULL;
	}

	anim->cq_grayscale = grayscale;
	anim->cq_rgb_fixed = rgb_fixed;
	anim->cq_color_cube_size = color_cube_size;
}

void tilem_animation_set_quantize_preview(TilemAnimation *anim,
                                          gboolean enabled)
{
	g_return_if_fail(TILEM_IS_ANIMATION(anim));
	anim->quantize_preview = enabled;
}

void tilem_animation_set_speed(TilemAnimation *anim, gdouble factor)
{
	g_return_if_fail(TILEM_IS_ANIMATION(anim));
	g_return_if_fail(factor > 0.0);
	anim->speed = factor;
	anim->time_stretch = 1.0 / factor;
}

gdouble tilem_animation_get_speed(TilemAnimation *anim)
{
	g_return_val_if_fail(TILEM_IS_ANIMATION(anim), 1.0);
	return anim->speed;
}

TilemAnimFrame *tilem_animation_next_frame(TilemAnimation *anim,
                                           TilemAnimFrame *frm)
{
	g_return_val_if_fail(TILEM_IS_ANIMATION(anim), NULL);
	if (frm)
		return frm->next;
	else
		return anim->start;
}

int tilem_anim_frame_get_duration(TilemAnimFrame *frm)
{
	g_return_val_if_fail(frm != NULL, 0);
	return frm->duration;
}

void tilem_animation_get_indexed_image(TilemAnimation *anim,
                                       TilemAnimFrame *frm,
                                       byte **buffer,
                                       int *width, int *height)
{
	int w, h;

	g_return_if_fail(TILEM_IS_ANIMATION(anim));
	g_return_if_fail(frm != NULL);
	g_return_if_fail(buffer != NULL);
	g_return_if_fail(width != NULL);
	g_return_if_fail(height != NULL);

	*width = w = anim->image_width;
	*height = h = anim->image_height;
	*buffer = g_new(byte, anim->image_width * anim->image_height);

	set_lcdbuf_from_frame(anim, anim->temp_buffer, frm);

	if (anim->lcdbuf_format == TILEM_LCD_BUF_BLACK_128) {
		tilem_draw_lcd_image_indexed(anim->temp_buffer, *buffer,
		                             w, h, w, TILEM_SCALE_SMOOTH);
	}
	else {
		generate_cq_palette(anim);
		alloc_cq_image_buf(anim, w, h);
		tilem_draw_lcd_image_rgb(anim->temp_buffer,
		                         anim->cq_image_buf,
		                         w, h, w * 3, 3, anim->palette,
		                         TILEM_SCALE_SMOOTH);
		color_palette_quantize_image(anim->cq_pal, *buffer,
		                             anim->cq_image_buf, w, h, w * 3);
	}

	anim->temp_buffer->data = NULL;
}

gboolean tilem_animation_save(TilemAnimation *anim,
                              const char *fname, const char *type,
                              char **option_keys, char **option_values,
                              GError **err)
{
	FILE *fp;
	char *dname;
	int errnum;
	GdkPixbuf *pb;
	gboolean status;
	const dword *palcolors;
	byte palette[768];
	int i, palsize;

	g_return_val_if_fail(TILEM_IS_ANIMATION(anim), FALSE);
	g_return_val_if_fail(fname != NULL, FALSE);
	g_return_val_if_fail(type != NULL, FALSE);
	g_return_val_if_fail(err == NULL || *err == NULL, FALSE);

	if (strcmp(type, "gif") != 0) {
		pb = frame_to_pixbuf(anim, anim->start, FALSE);
		status = gdk_pixbuf_savev(pb, fname, type,
		                          option_keys, option_values,
		                          err);
		g_object_unref(pb);
		return status;
	}

	fp = g_fopen(fname, "wb");
	if (!fp) {
		errnum = errno;
		dname = g_filename_display_name(fname);
		g_set_error(err, G_FILE_ERROR,
		            g_file_error_from_errno(errnum),
		            _("Failed to open '%s' for writing: %s"),
		            dname, g_strerror(errnum));
		g_free(dname);
		return FALSE;
	}

	generate_cq_palette(anim);
	if (anim->cq_pal) {
		palcolors = anim->cq_pal->colors;
		palsize = 256;	/* FIXME (write_gif doesn't handle other sizes) */
	}
	else {
		palcolors = anim->palette;
		palsize = 256;
	}

	for (i = 0; i < palsize; i++) {
		palette[3 * i] = palcolors[i] >> 16;
		palette[3 * i + 1] = palcolors[i] >> 8;
		palette[3 * i + 2] = palcolors[i];
	}

	tilem_animation_write_gif(anim, palette, palsize, fp);

	if (fclose(fp)) {
		errnum = errno;
		dname = g_filename_display_name(fname);
		g_set_error(err, G_FILE_ERROR,
		            g_file_error_from_errno(errnum),
		            _("Error while closing '%s': %s"),
		            dname, g_strerror(errnum));
		g_free(dname);
		return FALSE;
	}

	return TRUE;
}
