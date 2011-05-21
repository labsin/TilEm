/*
 * TilEm II
 *
 * Copyright (c) 2010-2011 Thibault Duponchelle
 * Copyright (c) 2010-2011 Benjamin Moody
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

#include <stdio.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <ticalcs.h>
#include <tilem.h>

#include "gui.h"
#include "files.h"

/* Set size hints for the toplevel window */
static void set_size_hints(GtkWidget *widget, gpointer data)
{
	TilemEmulatorWindow *ewin = data;

	/* Don't use gtk_window_set_geometry_hints() (which would
	   appear to do what we want) because, in addition to setting
	   the hints we want, that function causes GTK+ to argue with
	   the window manager.

	   Instead, we call this function after the check-resize
	   signal (which is when GTK+ itself would normally set the
	   hints.)

	   FIXME: check that this works as desired on Win32/Quartz. */

	if (gtk_widget_get_window(widget))
		gdk_window_set_geometry_hints(gtk_widget_get_window(widget),
		                              &ewin->geomhints,
		                              ewin->geomhintmask);
}

static gboolean screen_repaint(GtkWidget *w, GdkEventExpose *ev G_GNUC_UNUSED,
                               TilemEmulatorWindow *ewin)
{
	GtkAllocation alloc;
	GdkWindow *win;
	GtkStyle *style;

	gtk_widget_get_allocation(w, &alloc);

	/* If image buffer is not the correct size, allocate a new one */

	if (!ewin->lcd_image_buf
	    || alloc.width != ewin->lcd_image_width
	    || alloc.height != ewin->lcd_image_height) {
		ewin->lcd_image_width = alloc.width;
		ewin->lcd_image_height = alloc.height;
		g_free(ewin->lcd_image_buf);
		ewin->lcd_image_buf = g_new(byte, alloc.width * alloc.height);
	}

	/* Draw LCD contents into the image buffer */

	g_mutex_lock(ewin->emu->lcd_mutex);
	tilem_draw_lcd_image_indexed(ewin->emu->lcd_buffer,
	                             ewin->lcd_image_buf,
	                             alloc.width, alloc.height, alloc.width,
	                             TILEM_SCALE_SMOOTH);
	g_mutex_unlock(ewin->emu->lcd_mutex);

	/* Render buffer to the screen */

	win = gtk_widget_get_window(w);
	style = gtk_widget_get_style(w);
	gdk_draw_indexed_image(win, style->fg_gc[GTK_STATE_NORMAL], 0, 0,
	                       alloc.width, alloc.height, GDK_RGB_DITHER_NONE,
	                       ewin->lcd_image_buf, alloc.width,
	                       ewin->lcd_cmap);
	return TRUE;
}

/* Set the color palette for drawing the emulated LCD. */
static void screen_restyle(GtkWidget* w, GtkStyle* oldstyle G_GNUC_UNUSED,
                           TilemEmulatorWindow* ewin)
{
	dword* palette;
	GtkStyle *style;
	int r_dark, g_dark, b_dark;
	int r_light, g_light, b_light;
	double gamma = 2.2;

	if (ewin->skin_disabled || !ewin->skin) {
		/* no skin -> use standard GTK colors */

		style = gtk_widget_get_style(w);

		r_dark = style->text[GTK_STATE_NORMAL].red / 257;
		g_dark = style->text[GTK_STATE_NORMAL].green / 257;
		b_dark = style->text[GTK_STATE_NORMAL].blue / 257;

		r_light = style->base[GTK_STATE_NORMAL].red / 257;
		g_light = style->base[GTK_STATE_NORMAL].green / 257;
		b_light = style->base[GTK_STATE_NORMAL].blue / 257;
	}
	else {
		/* use skin colors */

		r_dark = ((ewin->skin->lcd_black >> 16) & 0xff);
		g_dark = ((ewin->skin->lcd_black >> 8) & 0xff);
		b_dark = (ewin->skin->lcd_black & 0xff);

		r_light = ((ewin->skin->lcd_white >> 16) & 0xff);
		g_light = ((ewin->skin->lcd_white >> 8) & 0xff);
		b_light = (ewin->skin->lcd_white & 0xff);
	}

	/* Generate a new palette, and convert it into GDK format */

	if (ewin->lcd_cmap)
		gdk_rgb_cmap_free(ewin->lcd_cmap);

	palette = tilem_color_palette_new(r_light, g_light, b_light,
					  r_dark, g_dark, b_dark, gamma);
	ewin->lcd_cmap = gdk_rgb_cmap_new(palette, 256);
	tilem_free(palette);

	gtk_widget_queue_draw(ewin->lcd);
}

static void skin_size_allocate(GtkWidget *widget, GtkAllocation *alloc,
                               gpointer data)
{
	TilemEmulatorWindow *ewin = data;
	int rawwidth, rawheight;
	int lcdleft, lcdright, lcdtop, lcdbottom;
	double rx, ry;

	g_return_if_fail(ewin->skin != NULL);

	if (ewin->skin->width == alloc->width
	    && ewin->skin->height == alloc->height)
		return;

	ewin->skin->width = alloc->width;
	ewin->skin->height = alloc->height;

	rawwidth = gdk_pixbuf_get_width(ewin->skin->raw);
	rawheight = gdk_pixbuf_get_height(ewin->skin->raw);

	rx = (double) alloc->width / rawwidth;
	ry = (double) alloc->height / rawheight;
	ewin->skin->sx = (double) rawwidth / alloc->width;
	ewin->skin->sy = (double) rawheight / alloc->height;

	if (ewin->skin->image)
		g_object_unref(ewin->skin->image);
	ewin->skin->image = gdk_pixbuf_scale_simple(ewin->skin->raw,
	                                            alloc->width,
	                                            alloc->height,
	                                            GDK_INTERP_BILINEAR);

	gtk_image_set_from_pixbuf(GTK_IMAGE(ewin->background),
	                          ewin->skin->image);

	lcdleft = ewin->skin->lcd_pos.left * rx + 0.5;
	lcdright = ewin->skin->lcd_pos.right * rx + 0.5;
	lcdtop = ewin->skin->lcd_pos.top * ry + 0.5;
	lcdbottom = ewin->skin->lcd_pos.bottom * ry + 0.5;

	gtk_widget_set_size_request(ewin->lcd,
	                            MAX(lcdright - lcdleft, 1),
	                            MAX(lcdbottom - lcdtop, 1));

	gtk_layout_move(GTK_LAYOUT(widget), ewin->lcd,
	                lcdleft, lcdtop);
}

/* Used when you load another skin */
static void redraw_screen(TilemEmulatorWindow *ewin)
{
	GtkWidget *pImage;
	GtkWidget *emuwin;
	int lcdwidth, lcdheight;
	int screenwidth, screenheight;
	int minwidth, minheight, defwidth, defheight;
	double sx, sy, s, a1, a2;

	if (ewin->skin) {
		skin_unload(ewin->skin);
		g_free(ewin->skin);
	}

	ewin->skin = g_new0(SKIN_INFOS, 1);

	if (!ewin->skin_disabled) {
		if (!ewin->skin_file_name
		    || skin_load(ewin->skin, ewin->skin_file_name))
			ewin->skin_disabled = 1;
	}

	if (ewin->emu->calc) {
		lcdwidth = ewin->emu->calc->hw.lcdwidth;
		lcdheight = ewin->emu->calc->hw.lcdheight;
	}
	else {
		lcdwidth = lcdheight = 1;
	}

	if (ewin->lcd)
		gtk_widget_destroy(ewin->lcd);
	if (ewin->background)
		gtk_widget_destroy(ewin->background);
	if (ewin->layout)
		gtk_widget_destroy(ewin->layout);

	/* create LCD widget */
	ewin->lcd = gtk_drawing_area_new();

	/* create background image and layout */
	if (!ewin->skin_disabled) {
		ewin->layout = gtk_layout_new(NULL, NULL);

		pImage = gtk_image_new_from_pixbuf(ewin->skin->image);
		gtk_layout_put(GTK_LAYOUT(ewin->layout), pImage, 0, 0);
		ewin->background = pImage;

		screenwidth = (ewin->skin->lcd_pos.right
		               - ewin->skin->lcd_pos.left);
		screenheight = (ewin->skin->lcd_pos.bottom
		                - ewin->skin->lcd_pos.top);

		gtk_widget_set_size_request(ewin->lcd,
		                            screenwidth, screenheight);
		gtk_layout_put(GTK_LAYOUT(ewin->layout), ewin->lcd,
		               ewin->skin->lcd_pos.left,
		               ewin->skin->lcd_pos.top);

		g_signal_connect(ewin->layout, "size-allocate",
		                 G_CALLBACK(skin_size_allocate), ewin);

		emuwin = ewin->layout;

		tilem_config_get("settings",
		                 "width/i", &defwidth,
		                 "height/i", &defheight,
		                 NULL);

		if(defwidth == 0)
			defwidth = ewin->skin->width;
		if(defheight == 0)
			defheight = ewin->skin->height;

		sx = (double) lcdwidth / screenwidth;
		sy = (double) lcdheight / screenheight;
		s = MAX(sx, sy);
		minwidth = defwidth * s + 0.5;
		minheight = defheight * s + 0.5;
	}
	else {
		ewin->layout = NULL;
		ewin->background = NULL;

		emuwin = ewin->lcd;

		minwidth = lcdwidth;
		minheight = lcdheight;
		defwidth = lcdwidth * 2;
		defheight = lcdheight * 2;
	}

	gtk_widget_add_events(emuwin, (GDK_BUTTON_PRESS_MASK
	                               | GDK_BUTTON_RELEASE_MASK
	                               | GDK_BUTTON1_MOTION_MASK
	                               | GDK_POINTER_MOTION_HINT_MASK 
				       | GDK_DROP_FINISHED
				       | GDK_DRAG_MOTION));

	g_signal_connect(ewin->lcd, "expose-event",
	                 G_CALLBACK(screen_repaint), ewin);
	g_signal_connect(ewin->lcd, "style-set",
	                 G_CALLBACK(screen_restyle), ewin);

	g_signal_connect(emuwin, "button-press-event",
	                 G_CALLBACK(mouse_press_event), ewin);
	g_signal_connect(emuwin, "motion-notify-event",
	                 G_CALLBACK(pointer_motion_event), ewin);
	g_signal_connect(emuwin, "button-release-event",
	                 G_CALLBACK(mouse_release_event), ewin);

	gtk_drag_dest_set(emuwin, GTK_DEST_DEFAULT_ALL,
	                  NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_text_targets(emuwin);
	g_signal_connect(emuwin, "drag-data-received",
	                 G_CALLBACK(on_drag_and_drop), ewin);


	/* Hint calculation assumes the emulator is the only thing in
	   the window; if other widgets are added, this will have to
	   change accordingly
	*/
	ewin->geomhints.min_width = minwidth;
	ewin->geomhints.min_height = minheight;
	a1 = (double) minwidth / minheight;
	a2 = (double) defwidth / defheight;
	ewin->geomhints.min_aspect = MIN(a1, a2) - 0.0001;
	ewin->geomhints.max_aspect = MAX(a1, a2) + 0.0001;
	ewin->geomhintmask = (GDK_HINT_MIN_SIZE | GDK_HINT_ASPECT);

	/* If the window is already realized, set the hints now, so
	   that the WM will see the new hints before we try to resize
	   the window */
	set_size_hints(ewin->window, ewin);

	gtk_widget_set_size_request(emuwin, minwidth, minheight);
	gtk_container_add(GTK_CONTAINER(ewin->window), emuwin);
	gtk_window_resize(GTK_WINDOW(ewin->window), defwidth, defheight);

	gtk_widget_show_all(emuwin);
}

TilemEmulatorWindow *tilem_emulator_window_new(TilemCalcEmulator *emu)
{
	TilemEmulatorWindow *ewin;

	g_return_val_if_fail(emu != NULL, NULL);

	ewin = g_slice_new0(TilemEmulatorWindow);
	ewin->emu = emu;

	tilem_config_get("settings",
	                 "skin_disabled/b", &ewin->skin_disabled,
	                 NULL);

	/* Create the window */
	ewin->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	g_signal_connect_swapped(ewin->window, "destroy", G_CALLBACK(on_destroy), ewin);

	g_signal_connect_after(ewin->window, "check-resize",
	                       G_CALLBACK(set_size_hints), ewin);

	gtk_widget_add_events(ewin->window, (GDK_KEY_PRESS_MASK
	                                      | GDK_KEY_RELEASE_MASK));

	g_signal_connect(ewin->window, "key-press-event",
	                 G_CALLBACK(key_press_event), ewin);
	g_signal_connect(ewin->window, "key-release-event",
	                 G_CALLBACK(key_release_event), ewin);

	tilem_emulator_window_calc_changed(ewin);
	redraw_screen(ewin);

	return ewin;
}

void tilem_emulator_window_free(TilemEmulatorWindow *ewin)
{
	g_return_if_fail(ewin != NULL);

	if (ewin->lcd)
		gtk_widget_destroy(ewin->lcd);
	if (ewin->background)
		gtk_widget_destroy(ewin->background);
	if (ewin->layout)
		gtk_widget_destroy(ewin->layout);
	if (ewin->window)
		gtk_widget_destroy(ewin->window);

	g_free(ewin->lcd_image_buf);

	g_free(ewin->skin_file_name);
	if (ewin->skin) {
		skin_unload(ewin->skin);
		g_free(ewin->skin);
	}

	if (ewin->lcd_cmap)
		gdk_rgb_cmap_free(ewin->lcd_cmap);
}

void tilem_emulator_window_set_skin(TilemEmulatorWindow *ewin,
                                    const char *filename)
{
	g_return_if_fail(ewin != NULL);

	g_free(ewin->skin_file_name);
	if (filename)
		ewin->skin_file_name = g_strdup(filename);
	else
		ewin->skin_file_name = NULL;
	redraw_screen(ewin);
}

/* Switch between skin and LCD-only mode */
void tilem_emulator_window_set_skin_disabled(TilemEmulatorWindow *ewin,
                                             gboolean disabled)
{
	g_return_if_fail(ewin != NULL);

	if (ewin->skin_disabled != !!disabled) {
		ewin->skin_disabled = !!disabled;
		redraw_screen(ewin);
	}
}

void tilem_emulator_window_calc_changed(TilemEmulatorWindow *ewin)
{
	const char *model;
	char *name = NULL, *path;

	g_return_if_fail(ewin != NULL);
	g_return_if_fail(ewin->emu != NULL);

	g_free(ewin->skin_file_name);
	ewin->skin_file_name = NULL;

	if (!ewin->emu->calc)
		return;

	model = ewin->emu->calc->hw.name;

	tilem_config_get(model,
	                 "skin/f", &name,
	                 NULL);

	if (!name)
		name = g_strdup_printf("%s.skn", model);

	if (!g_path_is_absolute(name)) {
		path = get_shared_file_path("skins", name, NULL);
		tilem_emulator_window_set_skin(ewin, path);
		g_free(path);
	}
	else {
		tilem_emulator_window_set_skin(ewin, name);
	}

	g_free(name);
}

void tilem_emulator_window_refresh_lcd(TilemEmulatorWindow *ewin)
{
	g_return_if_fail(ewin != NULL);
	gtk_widget_queue_draw(ewin->lcd);
}




/* Display the lcd image into the terminal */
void display_lcdimage_into_terminal(TilemEmulatorWindow *ewin)  /* Absolument necessaire */
{
	
	int width, height;
	guchar* lcddata;
	int x, y;
	char c;
	width = ewin->emu->calc->hw.lcdwidth;
	height = ewin->emu->calc->hw.lcdheight;
	FILE* lcd_content_file;
	/* Alloc mmem */
	lcddata = g_new(guchar, (width / 8) * height);
		
	/* Get the lcd content using the function 's pointer from Benjamin's core */
	(*ewin->emu->calc->hw.get_lcd)(ewin->emu->calc, lcddata);
		
	/* Print a little demo just for fun;) */
	printf("\n\n\n");	
	printf("	 r     rr    r  rrrrr  rrr  r     rrrrr r   r  rr    r    rr     r                      \n");
	printf("  r     r     r     r     r     r   r     r     rr rr    r    r     r     r     r     r     r   \n");
	printf("   r   r      r    r      r     r   r     r     r r r   r      r    r      r     r     r     r  \n");
	printf("rrrrr r      r     r      r     r   r     rrrr  r r r  r       r     r      r rrrrr rrrrr rrrrr \n");
	printf("   r   r      r    r      r     r   r     r     r   r  rrr     r    r      r     r     r     r  \n");
	printf("  r     r     r     r     r     r   r     r     r   r         r     r     r     r     r     r   \n");
	printf("	 r     rr    r    r    rrr  rrrrr rrrrr r   r        r    rr     r                      \n");
	printf("\n(Here is just a sample...)\n\n");	
	
	/* Request user to know which char user wants */	
	
	printf("Which char to display FOR BLACK?\n");
	scanf("%c", &c); /* Choose wich char for the black */	
	
	//printf("Which char to display FOR GRAY ?\n");
	//scanf("%c", &b); /* Choose wich char for the black */	
	
	lcd_content_file = g_fopen("lcd_content.txt", "w");

	printf("\n\n\n### LCD CONTENT ###\n\n\n\n");
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			/*printf("%d ", lcddata[y * width + x]); */	
			if (lcddata[(y * width + x) / 8] & (0x80 >> (x % 8))) {
				printf("%c", c);
				if(lcd_content_file != NULL)	
					fprintf(lcd_content_file,"%c", c);
			} else {
				printf("%c", ' ');
				if(lcd_content_file != NULL)	
					fprintf(lcd_content_file,"%c", ' ');
			}
		}
		printf("\n");
		if(lcd_content_file != NULL)	
			fprintf(lcd_content_file,"%c", '\n');
	}	
	if(lcd_content_file != NULL) {	
		fclose(lcd_content_file);
		printf("\n### END ###\n\nSaved into lcd_content.txt (You're really geek!)");
	}	

}

void create_menus(GtkWidget *window,GdkEvent *event, GtkWidget * menu_items)
{
	
	DLCD_L2_A0("Entering : create_menus...\n");
	GtkAccelGroup *accel_group;
	GdkEventButton *bevent = (GdkEventButton *) event;


	accel_group = gtk_accel_group_new();
	//factory = gtk_item_factory_new(GTK_TYPE_MENU, menuname, accel_group);
	/* translatefunc */
	//gtk_item_factory_create_items(factory, thisitems, menu_items, emu);
	//menu = factory->widget;

	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
	gtk_menu_popup(GTK_MENU(menu_items), NULL, NULL, NULL, NULL, bevent->button, bevent->time);
	gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	DLCD_L2_A0("Exiting create_menus...\n");

}
