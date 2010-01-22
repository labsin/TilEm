#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>
#include <signal.h>

static void catchint(int sig G_GNUC_UNUSED)
{
	sforcebreak = 1;
}

gboolean screen_update(gpointer data)
{
	DEBUGGING ("Entering screen_update...\n");
	TilemCalcEmulator* emu = data;
	gtk_widget_queue_draw(emu->lcdwin);
	DEBUGGING ("Exiting screen_update...\n");
	return TRUE;
}


void create_menus(GtkWidget *window,GdkEvent *event, GtkItemFactoryEntry * menu_items, int thisitems, const char *menuname,gpointer* gsi)
{
	
	DEBUGGING ("Entering : create_menus...\n");
	GtkAccelGroup *accel_group;
	GtkItemFactory *factory;
	GtkWidget *menu;
	GdkEventButton *bevent = (GdkEventButton *) event;

	accel_group = gtk_accel_group_new();
	factory = gtk_item_factory_new(GTK_TYPE_MENU, menuname, accel_group);
	/* translatefunc */
	gtk_item_factory_create_items(factory, thisitems, menu_items, gsi);
	menu = factory->widget;

	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, bevent->button, bevent->time);
	gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	DEBUGGING ("Exiting create_menus...\n");

}

static gpointer core_thread(gpointer data)
{
	DEBUGGING ("Entering core_thread...\n");
	int debugmode = 0;
	TilemZ80Breakpoint* bp;
	char *cmd, *prev = NULL, *end;
	dword addr;
	int width, height;
	guchar* lcddata;
	int x, y;
	int low, high, v, old;
	

	GLOBAL_SKIN_INFOS *gsi = data;
	width = gsi->emu->calc->hw.lcdwidth;
	height = gsi->emu->calc->hw.lcdheight;
	//printf("\ncore_thread : gsi->emu->calc->hw.lcdwidth : %d",gsi->emu->calc->hw.lcdwidth);
	//printf("\ncore_thread : gsi->emu->calc->hw.lcdheight : %d\n",gsi->emu->calc->hw.lcdheight);
	lcddata = g_new(guchar, (width / 8) * height);

	signal(SIGINT, &catchint);

	while (1) {
		g_mutex_lock(gsi->emu->run_mutex);
		if (gsi->emu->exiting) {
			g_mutex_unlock(gsi->emu->run_mutex);
			g_free(lcddata);
			return NULL;
		}
		if (gsi->emu->forcebreak || sforcebreak) {
			printf("Interrupted at %04X\n", gsi->emu->calc->z80.clock);
			printstate(gsi->emu);
			debugmode = 1;
			gsi->emu->forcebreak = FALSE;
			sforcebreak = 0;
		}
		g_mutex_unlock(gsi->emu->run_mutex);

		

		g_mutex_lock(gsi->emu->calc_mutex);


		if (tilem_z80_run_time(gsi->emu->calc, 10000, NULL)) {
			bp = &gsi->emu->calc->z80.breakpoints[gsi->emu->calc->z80.stop_breakpoint];
			printf("Breakpoint (%s at %X) at time %04X\n",
			       bpstring[bp->type], bp->start,
			       gsi->emu->calc->z80.clock);
			printstate(gsi->emu);
			debugmode = 1;
		}

		if (!gsi->act==0
		    || (gsi->emu->calc->z80.halted && !gsi->emu->calc->poweronhalt)) {
			low = high = 0;
		}
		else { 
			(*gsi->emu->calc->hw.get_lcd)(gsi->emu->calc, lcddata);
			if (gsi->emu->calc->lcd.contrast < 32) {
				low = 0;
				high = gsi->emu->calc->lcd.contrast * 4;
			}
			else {
				low = (gsi->emu->calc->lcd.contrast - 32) * 4;
				high = 128;
			}
		}

		g_mutex_unlock(gsi->emu->calc_mutex);

		g_mutex_lock(gsi->emu->lcd_mutex);
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (lcddata[(y * width + x) / 8]
				    & (0x80 >> (x % 8)))
					v = high;
				else
					v = low;

				old = gsi->emu->lcdlevel[y * width + x];
				gsi->emu->lcdlevel[y * width + x]
					= v + ((old - v) * 3) / 4;
			}
		}
		g_mutex_unlock(gsi->emu->lcd_mutex);

		g_usleep(10000);
	}
	DEBUGGING ("Exiting core_thread...\n");
}


void update_lcdimage(TilemCalcEmulator *emu)
{
	DEBUGGING("Entering update_lcdimage\n");
	int x, y, i, level;
	int br, bg, bb, dr, dg, db;
	guchar* p;

	br = emu->lcdbg.red;
	bg = emu->lcdbg.green;
	bb = emu->lcdbg.blue;
	dr = emu->lcdfg.red - br;
	dg = emu->lcdfg.green - bg;
	db = emu->lcdfg.blue - bb;

	p = emu->lcdimage;

	for (i = 0; i < Y_FRINGE *(emu->calc->hw.lcdwidth + 2 * X_FRINGE);
	     i++) {
		p[0] = br / 256;
		p[1] = bg / 256;
		p[2] = bb / 256;
		p += 3;
	}

	for (y = 0; y < emu->calc->hw.lcdheight; y++) {
		for (i = 0; i < X_FRINGE; i++) {
			p[0] = br / 256;
			p[1] = bg / 256;
			p[2] = bb / 256;
			p += 3;
		}

		for (x = 0; x < emu->calc->hw.lcdwidth; x++) {
			level = emu->lcdlevel[y * emu->calc->hw.lcdwidth + x];
			p[0] = (br * 128 + dr * level + 16384) / 32768;
			p[1] = (bg * 128 + dg * level + 16384) / 32768;
			p[2] = (bb * 128 + db * level + 16384) / 32768;
			p += 3;
		}

		for (i = 0; i < X_FRINGE; i++) {
			p[0] = br / 256;
			p[1] = bg / 256;
			p[2] = bb / 256;
			p += 3;
		}
	}

	for (i = 0; i < Y_FRINGE * (emu->calc->hw.lcdwidth + 2 * X_FRINGE);
	     i++) {
		p[0] = br / 256;
		p[1] = bg / 256;
		p[2] = bb / 256;
		p += 3;
	}
	DEBUGGING("Exiting update_lcdimage...\n");
	
}




void printstate(TilemCalcEmulator* emu)
{
	printf("  PC=%02X:%04X AF=%04X BC=%04X DE=%04X"
	       " HL=%04X IX=%04X IY=%04X SP=%04X\n",
	       emu->calc->mempagemap[emu->calc->z80.r.pc.b.h >> 6],
	       emu->calc->z80.r.pc.w.l,
	       emu->calc->z80.r.af.w.l,
	       emu->calc->z80.r.bc.w.l,
	       emu->calc->z80.r.de.w.l,
	       emu->calc->z80.r.hl.w.l,
	       emu->calc->z80.r.ix.w.l,
	       emu->calc->z80.r.iy.w.l,
	       emu->calc->z80.r.sp.w.l);
}


gboolean screen_repaint(GtkWidget* w G_GNUC_UNUSED,GdkEvent *event,GLOBAL_SKIN_INFOS * gsi)
{
	DEBUGGING("Entering screen_repaint...\n");
	double fx, fy;

	//DEBUGGING2("screen_repaint : w->allocation.width = %d\n", w->allocation.width);
	//DEBUGGING2("screen_repaint : w->allocation.height = %d\n", w->allocation.height);
	fx = ((double) w->allocation.width/ (gsi->emu->calc->hw.lcdwidth + 2 * X_FRINGE));
	fy = ((double) w->allocation.height/ (gsi->emu->calc->hw.lcdheight + 2 * Y_FRINGE));

	g_mutex_lock(gsi->emu->lcd_mutex);
	update_lcdimage(gsi->emu);
	
	/*gdk_pixbuf_scale: assertion `dest_x >= 0 && dest_x + dest_width <= dest->width' failed */

	//                            GdkPixbuf *src,   GdkPixbuf *dest,       dest_x,  dest_y, dest_width,      dest_height, offset_x, offset_y,double scale_x,double scale_y,GdkInterpType interp_type);
	gdk_pixbuf_scale(gsi->emu->lcdpb, gsi->emu->lcdscaledpb, 0,       0,w->allocation.width/2, w->allocation.height/2,0.0, 0.0, fx, fy, GDK_INTERP_TILES);
	//	dest_x=0, dest_y=0,dest_width=158 dest_height=100 
	g_mutex_unlock(gsi->emu->lcd_mutex);

	gdk_draw_pixbuf(w->window, w->style->fg_gc[w->state],gsi->emu->lcdscaledpb, 1, 2, 4, 7,70,60,GDK_RGB_DITHER_NONE, 0, 0);
	DEBUGGING("Exiting screen_repaint...\n");
	return TRUE;
}


GtkWidget* draw_screen(GLOBAL_SKIN_INFOS *gsi)  
{
	//g_thread_init(NULL);
	
	DEBUGGING ("Entering : draw_screen...\n");
	int screenwidth ;
	int screenheight;
	GtkWidget *pAf;
	GThread *th;
	gsi->act=0; // no button is clicked

	/* LOAD A SKIN */
	SKIN_INFOS *si;
	si=malloc(sizeof(SKIN_INFOS));
	gsi->si=(SKIN_INFOS*)si;
	skin_load(gsi->si,gsi->SkinFileName);
	
	/* Create the window */
	GtkWidget *pImage;
	GtkWidget *pWindow;
	GtkWidget *pLayout;
	pWindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);	// GTK_WINDOW_LEVEL : define how is the window 
	gtk_window_set_title(GTK_WINDOW(pWindow),"tilem");	// define title of the window 
	gtk_window_set_position(GTK_WINDOW(pWindow),GTK_WIN_POS_CENTER); // GTK_WIN_POS_CENTER : define how the window is displayed 
	gtk_window_set_default_size(GTK_WINDOW(pWindow),gsi->si->width,gsi->si->height);	// define size of the window
	pImage=gtk_image_new_from_pixbuf(gsi->si->image);
	
	
	/* Create the draw area */
	pAf=create_draw_area(gsi);
	
	/* Add the lcd to the pWindow using a GtkLayout */
	pLayout=gtk_layout_new(NULL,NULL);
	gsi->pLayout=pLayout;
	gtk_widget_show(pAf);
	gtk_layout_put(GTK_LAYOUT(pLayout),pImage,0,0);
	gtk_layout_put(GTK_LAYOUT(pLayout),pAf,gsi->si->lcd_pos.left,gsi->si->lcd_pos.top);
	gtk_container_add(GTK_CONTAINER(pWindow),pLayout);
	
	
	gsi->emu->lcdlevel = g_new0(guchar, (gsi->emu->calc->hw.lcdwidth * gsi->emu->calc->hw.lcdheight));
	screenwidth = gsi->emu->calc->hw.lcdwidth + 2 * X_FRINGE;
	screenheight = gsi->emu->calc->hw.lcdheight + 2 * Y_FRINGE ;
	gsi->emu->lcdimage = g_new0(guchar, 3 * screenwidth * screenheight);
	
	gsi->emu->lcdpb = gdk_pixbuf_new_from_data(gsi->emu->lcdimage,GDK_COLORSPACE_RGB, FALSE, 8,screenwidth, screenheight,screenwidth * 3,NULL, NULL);
	gsi->emu->lcdscaledpb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,screenwidth, screenheight);
	
	
	g_signal_connect(gsi->emu->lcdwin, "size-allocate",G_CALLBACK(screen_resize), gsi->emu);
	g_signal_connect(gsi->emu->lcdwin, "style-set", G_CALLBACK(screen_restyle), gsi->emu); 
	
	g_signal_connect(G_OBJECT(pWindow),"destroy",G_CALLBACK(on_destroy),NULL); 
	gtk_widget_add_events(pWindow, GDK_KEY_RELEASE_MASK); 		// Get the event on the window (leftclick, rightclick)
	/*gtk_signal_connect(GTK_OBJECT(pWindow), "key_press_event", GTK_SIGNAL_FUNC(keyboard_event), NULL); */
	gtk_signal_connect(GTK_OBJECT(pWindow), "key_press_event", G_CALLBACK(keyboard_event), NULL);
	gtk_widget_add_events(pWindow, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_signal_connect(GTK_OBJECT(pWindow), "button_press_event", G_CALLBACK(mouse_event),gsi);
	
	
	gtk_signal_connect(GTK_OBJECT(gsi->emu->lcdwin), "expose-event",G_CALLBACK(screen_repaint),gsi);
	
	
	//screen_repaint(gsi->emu->lcdwin,gsi);
	gtk_widget_show_all(pWindow);	// display the window and all that it contains.
	
	/* THREAD */
	th = g_thread_create(&core_thread, gsi, TRUE, NULL);
	g_timeout_add(100, screen_update, gsi->emu);
	signal(SIGINT, &catchint);
	
	DEBUGGING ("Exiting draw_screen...\n");
	return pWindow;
}


GLOBAL_SKIN_INFOS* redraw_screen(GtkWidget *pWindow,GLOBAL_SKIN_INFOS * gsi) 
{
	
	DEBUGGING ("Entering : redraw_screen...\n");
	GtkWidget *pImage;
	GtkWidget *pAf;
	GtkWidget * pLayout;
	/*gtk_stop();*/
	DEBUGGING2("REDRAW_SCREEN name : %s\n",gsi->si->name);
		skin_unload(gsi->si);
		skin_load(gsi->si,gsi->SkinFileName);
	
	/* Remove the pImage from the pWindow */ 
	gtk_container_remove(GTK_CONTAINER(gsi->pWindow),gsi->pLayout);
	
	pImage=gtk_image_new_from_pixbuf(gsi->si->image);
	pAf=create_draw_area(gsi);
		       
	
	pLayout=gtk_layout_new(NULL,NULL);
	gsi->pLayout=pLayout;
	gtk_widget_show(pAf);
	gtk_layout_put(GTK_LAYOUT(pLayout),pImage,0,0);
	gtk_layout_put(GTK_LAYOUT(pLayout),pAf,gsi->si->lcd_pos.left,gsi->si->lcd_pos.top);
	gtk_container_add(GTK_CONTAINER(pWindow),pLayout);
	gtk_window_resize(GTK_WINDOW(pWindow),gsi->si->width,gsi->si->height);
	g_signal_connect(G_OBJECT(pWindow),"destroy",G_CALLBACK(on_destroy),NULL); 
	
	/* Connection signal keyboard key press */
	gtk_widget_add_events(pWindow, GDK_KEY_RELEASE_MASK); // Get the event on the window (leftclick, rightclick)
	/*gtk_signal_connect(GTK_OBJECT(pWindow), "key_press_event", GTK_SIGNAL_FUNC(keyboard_event), NULL); */
	gtk_signal_connect(GTK_OBJECT(pWindow), "key_press_event", G_CALLBACK(keyboard_event), NULL);
	gtk_widget_add_events(pWindow, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_signal_connect(GTK_OBJECT(pWindow), "button_press_event", G_CALLBACK(mouse_event),gsi);
	gtk_widget_show_all(pWindow);	/*display the window and all that it contains.*/
	
	DEBUGGING ("Exiting : redraw_screen...\n");
	return gsi;

}

GtkWidget * create_draw_area(GLOBAL_SKIN_INFOS * gsi) 
{
	
	DEBUGGING ("Entering : create_draw_area...\n");
	GtkWidget *pAf;
	int screenwidth=gsi->si->lcd_pos.right-gsi->si->lcd_pos.left;	
	int screenheight=gsi->si->lcd_pos.bottom-gsi->si->lcd_pos.top;	// idem
	DEBUGGING("****************create_draw_area****************");
	DEBUGGING2("\n*create_draw_area : screenwidth = %d\n",screenwidth);
	DEBUGGING2("*create_draw_area : screenheight = %d\n",screenheight);
	pAf = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1.0, TRUE);		// et pAf ! ça fait des Chocapic !! 
                         gtk_frame_set_shadow_type(GTK_FRAME(pAf),GTK_SHADOW_NONE);
                         {
                                 gsi->emu->lcdwin = gtk_drawing_area_new();
                                 gtk_widget_set_name(gsi->emu->lcdwin, "tilem-lcd");
                                 gtk_widget_set_size_request(gsi->emu->lcdwin,screenwidth,screenheight);
                                 gtk_container_add(GTK_CONTAINER(pAf),gsi->emu->lcdwin);
                               gtk_widget_show(gsi->emu->lcdwin);
                       }
	//g_signal_connect(gsi->emu->lcdwin, "expose-event",G_CALLBACK(screen_repaint), (gpointer)gsi);
	DEBUGGING ("Exiting create_draw_area...\n");
	return pAf;
}

void screen_restyle(GtkWidget* w, GtkStyle* oldstyle G_GNUC_UNUSED,gpointer data)
{
	TilemCalcEmulator* emu = data;
	emu->lcdfg = w->style->fg[GTK_STATE_NORMAL];
	emu->lcdbg = w->style->bg[GTK_STATE_NORMAL];
	gtk_widget_queue_draw(emu->lcdwin);
}

void screen_resize(GtkWidget* w G_GNUC_UNUSED,
		  GtkAllocation* alloc, gpointer data)
{
	TilemCalcEmulator* emu = data;

	g_object_unref(emu->lcdscaledpb);
	emu->lcdscaledpb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,
					  alloc->width, alloc->height);
}


