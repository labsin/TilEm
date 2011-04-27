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

/* Set size hints for the toplevel window */
static void set_size_hints(GtkWidget *widget, gpointer data)
{
	GLOBAL_SKIN_INFOS *gsi = data;

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
		                              &gsi->emu->geomhints,
		                              gsi->emu->geomhintmask);
}

static void skin_size_allocate(GtkWidget *widget, GtkAllocation *alloc,
                               gpointer data)
{
	GLOBAL_SKIN_INFOS *gsi = data;
	int rawwidth, rawheight;
	int lcdleft, lcdright, lcdtop, lcdbottom;
	double rx, ry;

	g_return_if_fail(gsi->si != NULL);

	if (gsi->si->width == alloc->width && gsi->si->height == alloc->height)
		return;

	gsi->si->width = alloc->width;
	gsi->si->height = alloc->height;

	rawwidth = gdk_pixbuf_get_width(gsi->si->raw);
	rawheight = gdk_pixbuf_get_height(gsi->si->raw);

	rx = (double) alloc->width / rawwidth;
	ry = (double) alloc->height / rawheight;
	gsi->si->sx = (double) rawwidth / alloc->width;
	gsi->si->sy = (double) rawheight / alloc->height;

	if (gsi->si->image)
		g_object_unref(gsi->si->image);
	gsi->si->image = gdk_pixbuf_scale_simple(gsi->si->raw,
	                                         alloc->width, alloc->height,
	                                         GDK_INTERP_BILINEAR);

	gtk_image_set_from_pixbuf(GTK_IMAGE(gsi->emu->background),
	                          gsi->si->image);

	lcdleft = gsi->si->lcd_pos.left * rx + 0.5;
	lcdright = gsi->si->lcd_pos.right * rx + 0.5;
	lcdtop = gsi->si->lcd_pos.top * ry + 0.5;
	lcdbottom = gsi->si->lcd_pos.bottom * ry + 0.5;

	gtk_widget_set_size_request(gsi->emu->lcdwin,
	                            MAX(lcdright - lcdleft, 1),
	                            MAX(lcdbottom - lcdtop, 1));

	gtk_layout_move(GTK_LAYOUT(widget), gsi->emu->lcdwin,
	                lcdleft, lcdtop);
}

/* Used when you load another skin */
void redraw_screen(GLOBAL_SKIN_INFOS *gsi)
{
	GtkWidget *pImage;
	GtkWidget *emuwin;
	int lcdwidth, lcdheight;
	int screenwidth, screenheight;
	int minwidth, minheight, defwidth, defheight;
	double sx, sy, s, a1, a2;

	if (gsi->si) {
		skin_unload(gsi->si);
		g_free(gsi->si);
	}

	gsi->si = g_new0(SKIN_INFOS, 1);
	skin_load(gsi->si, gsi->emu->cmdline->SkinFileName);

	lcdwidth = gsi->emu->calc->hw.lcdwidth;
	lcdheight = gsi->emu->calc->hw.lcdheight;

	if (gsi->emu->lcdwin)
		gtk_widget_destroy(gsi->emu->lcdwin);
	if (gsi->emu->background)
		gtk_widget_destroy(gsi->emu->background);
	if (gsi->pLayout)
		gtk_widget_destroy(gsi->pLayout);

	/* create LCD widget */
	gsi->emu->lcdwin = gtk_drawing_area_new();

	/* create background image and layout */
	if (gsi->view == 0) {
		gsi->pLayout = gtk_layout_new(NULL, NULL);

		pImage = gtk_image_new_from_pixbuf(gsi->si->image);
		gtk_layout_put(GTK_LAYOUT(gsi->pLayout), pImage, 0, 0);
		gsi->emu->background = pImage;

		screenwidth = gsi->si->lcd_pos.right - gsi->si->lcd_pos.left;
		screenheight = gsi->si->lcd_pos.bottom - gsi->si->lcd_pos.top;

		gtk_widget_set_size_request(gsi->emu->lcdwin,
		                            screenwidth, screenheight);
		gtk_layout_put(GTK_LAYOUT(gsi->pLayout), gsi->emu->lcdwin,
		               gsi->si->lcd_pos.left,
		               gsi->si->lcd_pos.top);

		g_signal_connect(gsi->pLayout, "size-allocate",
		                 G_CALLBACK(skin_size_allocate), gsi);

		emuwin = gsi->pLayout;

		defwidth = tilem_config_universal_getter_int("settings", "width");	
		defheight = tilem_config_universal_getter_int("settings", "width");	

		if(defwidth == 0)
			defwidth = gsi->si->width;
		if(defheight == 0)
			defheight = gsi->si->height;

		sx = (double) lcdwidth / screenwidth;
		sy = (double) lcdheight / screenheight;
		s = MAX(sx, sy);
		minwidth = defwidth * s + 0.5;
		minheight = defheight * s + 0.5;
	}
	else {
		gsi->pLayout = NULL;
		gsi->emu->background = NULL;

		emuwin = gsi->emu->lcdwin;

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

	g_signal_connect(gsi->emu->lcdwin, "expose-event",
	                 G_CALLBACK(screen_repaint), gsi);
	g_signal_connect(gsi->emu->lcdwin, "style-set",
	                 G_CALLBACK(screen_restyle), gsi);

	g_signal_connect(emuwin, "button-press-event",
	                 G_CALLBACK(mouse_press_event), gsi);
	g_signal_connect(emuwin, "motion-notify-event",
	                 G_CALLBACK(pointer_motion_event), gsi);
	g_signal_connect(emuwin, "button-release-event",
	                 G_CALLBACK(mouse_release_event), gsi);

	static GtkTargetEntry targetentries[] =
    	{
      		{ "STRING", GTK_TARGET_OTHER_APP, 0 },
    	};

	
	gtk_drag_dest_set(emuwin, GTK_DEST_DEFAULT_ALL, targetentries, 1, GDK_ACTION_MOVE );
	g_signal_connect(emuwin, "drag-data-received",
	                 G_CALLBACK(on_drag_and_drop), gsi);


	/* Hint calculation assumes the emulator is the only thing in
	   the window; if other widgets are added, this will have to
	   change accordingly
	*/
	gsi->emu->geomhints.min_width = minwidth;
	gsi->emu->geomhints.min_height = minheight;
	a1 = (double) minwidth / minheight;
	a2 = (double) defwidth / defheight;
	gsi->emu->geomhints.min_aspect = MIN(a1, a2) - 0.0001;
	gsi->emu->geomhints.max_aspect = MAX(a1, a2) + 0.0001;
	gsi->emu->geomhintmask = (GDK_HINT_MIN_SIZE | GDK_HINT_ASPECT);

	/* If the window is already realized, set the hints now, so
	   that the WM will see the new hints before we try to resize
	   the window */
	set_size_hints(gsi->pWindow, gsi);

	gtk_widget_set_size_request(emuwin, minwidth, minheight);
	gtk_container_add(GTK_CONTAINER(gsi->pWindow), emuwin);
	gtk_window_resize(GTK_WINDOW(gsi->pWindow), defwidth, defheight);

	gtk_widget_show_all(gsi->pWindow);
}

/* Switch between skin and LCD-only mode */
void switch_view(GLOBAL_SKIN_INFOS * gsi)
{
	gsi->view = !gsi->view;
	redraw_screen(gsi);
}

/* Display the lcd image into the terminal */
void display_lcdimage_into_terminal(GLOBAL_SKIN_INFOS* gsi)  /* Absolument necessaire */
{
	
	int width, height;
	guchar* lcddata;
	int x, y;
	char c;
	width = gsi->emu->calc->hw.lcdwidth;
	height = gsi->emu->calc->hw.lcdheight;
	FILE* lcd_content_file;
	/* Alloc mmem */
	lcddata = g_new(guchar, (width / 8) * height);
		
	/* Get the lcd content using the function 's pointer from Benjamin's core */
	(*gsi->emu->calc->hw.get_lcd)(gsi->emu->calc, lcddata);
		
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

/* Set the color palette for drawing the emulated LCD. */
void screen_restyle(GtkWidget* w, GtkStyle* oldstyle G_GNUC_UNUSED,
		    GLOBAL_SKIN_INFOS* gsi)
{
	TilemCalcEmulator* emu = gsi->emu;
	dword* palette;
	GtkStyle *style;
	int r_dark, g_dark, b_dark;
	int r_light, g_light, b_light;
	double gamma = 2.2;

	if (gsi->view == 1 || !gsi->si) {
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

		r_dark = ((gsi->si->lcd_black >> 16) & 0xff);
		g_dark = ((gsi->si->lcd_black >> 8) & 0xff);
		b_dark = (gsi->si->lcd_black & 0xff);

		r_light = ((gsi->si->lcd_white >> 16) & 0xff);
		g_light = ((gsi->si->lcd_white >> 8) & 0xff);
		b_light = (gsi->si->lcd_white & 0xff);
	}

	/* Generate a new palette, and convert it into GDK format */

	if (emu->lcd_cmap)
		gdk_rgb_cmap_free(emu->lcd_cmap);

	palette = tilem_color_palette_new(r_light, g_light, b_light,
					  r_dark, g_dark, b_dark, gamma);
	emu->lcd_cmap = gdk_rgb_cmap_new(palette, 256);
	tilem_free(palette);

	gtk_widget_queue_draw(emu->lcdwin);
}

gboolean screen_repaint(GtkWidget *w, GdkEventExpose *ev G_GNUC_UNUSED,
			GLOBAL_SKIN_INFOS *gsi)
{
	TilemCalcEmulator* emu = gsi->emu;
	GtkAllocation alloc;
	GdkWindow *win;
	GtkStyle *style;

	gtk_widget_get_allocation(w, &alloc);

	/* If image buffer is not the correct size, allocate a new one */

	if (!emu->lcd_image_buf
	    || alloc.width != emu->lcd_image_width
	    || alloc.height != emu->lcd_image_height) {
		emu->lcd_image_width = alloc.width;
		emu->lcd_image_height = alloc.height;
		g_free(emu->lcd_image_buf);
		emu->lcd_image_buf = g_new(byte, alloc.width * alloc.height);
	}

	/* Draw LCD contents into the image buffer */

	g_mutex_lock(emu->lcd_mutex);
	tilem_gray_lcd_draw_image_indexed(emu->glcd, emu->lcd_image_buf,
	                                  alloc.width, alloc.height, alloc.width,
					  TILEM_SCALE_SMOOTH);
	g_mutex_unlock(emu->lcd_mutex);

	/* Render buffer to the screen */

	win = gtk_widget_get_window(w);
	style = gtk_widget_get_style(w);
	gdk_draw_indexed_image(win, style->fg_gc[GTK_STATE_NORMAL], 0, 0,
	                       alloc.width, alloc.height, GDK_RGB_DITHER_NONE,
	                       emu->lcd_image_buf, alloc.width, emu->lcd_cmap);
	return TRUE;
}

void create_menus(GtkWidget *window,GdkEvent *event, GtkWidget * menu_items)
{
	
	DLCD_L2_A0("Entering : create_menus...\n");
	GtkAccelGroup *accel_group;
	GdkEventButton *bevent = (GdkEventButton *) event;


	accel_group = gtk_accel_group_new();
	//factory = gtk_item_factory_new(GTK_TYPE_MENU, menuname, accel_group);
	/* translatefunc */
	//gtk_item_factory_create_items(factory, thisitems, menu_items, gsi);
	//menu = factory->widget;

	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
	gtk_menu_popup(GTK_MENU(menu_items), NULL, NULL, NULL, NULL, bevent->button, bevent->time);
	gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	DLCD_L2_A0("Exiting create_menus...\n");

}

GtkWidget* draw_screen(GLOBAL_SKIN_INFOS *gsi)  
{
	gsi->view = 0;

	/* Create the window */
	gsi->pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	g_signal_connect(gsi->pWindow, "destroy", G_CALLBACK(on_destroy), gsi);

	g_signal_connect_after(gsi->pWindow, "check-resize",
	                       G_CALLBACK(set_size_hints), gsi);

	gtk_widget_add_events(gsi->pWindow, (GDK_KEY_PRESS_MASK
	                                | GDK_KEY_RELEASE_MASK));

	g_signal_connect(gsi->pWindow, "key-press-event",
	                 G_CALLBACK(key_press_event), gsi);
	g_signal_connect(gsi->pWindow, "key-release-event",
	                 G_CALLBACK(key_release_event), gsi);

	/* Create emulator widget */
	redraw_screen(gsi);

	g_timeout_add(100, tilem_animation_record,  gsi);

	return gsi->pWindow;
}
