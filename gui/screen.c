#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>


void update_lcdimage(TilemCalcEmulator *emu)
{
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

	for (i = 0; i < Y_FRINGE * (emu->calc->hw.lcdwidth + 2 * X_FRINGE);
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


gboolean screen_repaint(GtkWidget* w G_GNUC_UNUSED,gpointer data)
{
	GLOBAL_SKIN_INFOS *gsi = data;
	double fx, fy;

	fx = ((double) w->allocation.width
	      / (gsi->emu->calc->hw.lcdwidth + 2 * X_FRINGE));
	fy = ((double) w->allocation.height
	      / (gsi->emu->calc->hw.lcdheight + 2 * Y_FRINGE));

	g_mutex_lock(gsi->emu->lcd_mutex);
	update_lcdimage(gsi->emu);
	gdk_pixbuf_scale(gsi->emu->lcdpb, gsi->emu->lcdscaledpb,
			 0, 0, 0, 0,
			 0.0, 0.0, 100, 100, GDK_INTERP_TILES);
	
	
	g_mutex_unlock(gsi->emu->lcd_mutex);

	int i,j;
	for(i=0;i<300;i++) {
		for(j=0;j<300;j++) {
	 gdk_draw_point(w->window,w->style->fg_gc[w->state],i,j);
	gdk_draw_line(w->window,w->style->fg_gc[w->state],60,60,100,100);
		}
	} 
	//getchar();
	/*gdk_draw_pixbuf(gsi->emu->lcdwin, w->style->fg_gc[w->state],
			gsi->emu->lcdscaledpb, 0, 0, 0, 0,
			w->allocation.width, w->allocation.height,
			GDK_RGB_DITHER_NONE, 0, 0); */
			gdk_draw_pixbuf(w->window, w->style->fg_gc[w->state],
			gsi->emu->lcdscaledpb, 0, 0, 0, 0,
			30, 30,
			GDK_RGB_DITHER_NONE, 0, 0); 
	return TRUE;
}

gboolean screen_update(gpointer data)
{
	TilemCalcEmulator* emu = data;
	gtk_widget_queue_draw(emu->lcdwin);
	return TRUE;
}


static gpointer core_thread(gpointer data)
{
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
	lcddata = g_new(guchar, (width / 8) * height);

	//signal(SIGINT, &catchint);

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

		/*if (debugmode) {
			if ((cmd = readline("> "))) {
				if (!*cmd && prev) {
					free(cmd);
					cmd = prev;
				}
				else {
					if (prev)
						free(prev);
					prev = cmd;
				}

				switch (cmd[0]) {
				case 'c':
					debugmode = 0;
					break;

				case 'r':
					g_mutex_lock(gsi->emu->calc_mutex);
					tilem_calc_reset(gsi->emu->calc);
					g_mutex_unlock(gsi->emu->calc_mutex);
					debugmode = 0;
					break;

				case 's':
					g_mutex_lock(gsi->emu->calc_mutex);
					tilem_z80_run(gsi->emu->calc, 1, NULL);
					printstate(gsi->emu);
					g_mutex_unlock(gsi->emu->calc_mutex);
					break;

				case 'b':
					addr = strtol(cmd + 1, &end, 16);
					g_mutex_lock(gsi->emu->calc_mutex);
					tilem_z80_add_breakpoint(gsi->emu->calc, TILEM_BREAK_MEM_EXEC, addr, addr, 0xFFFF, NULL, 0);
					g_mutex_unlock(gsi->emu->calc_mutex);
					printf("Added breakpoint (exec at %04X)\n", addr);
					break;

				case 'w':
					addr = strtol(cmd + 1, &end, 16);
					g_mutex_lock(gsi->emu->calc_mutex);
					tilem_z80_add_breakpoint(gsi->emu->calc, TILEM_BREAK_MEM_WRITE, addr, addr, 0xFFFF, NULL, 0);
					g_mutex_unlock(gsi->emu->calc_mutex);
					printf("Added breakpoint (write at %04X)\n", addr);
					break;

				case 't':
					g_mutex_lock(gsi->emu->calc_mutex);
					printf("*** Stack ***\n");
					for (addr = gsi->emu->calc->z80.r.sp.w.l;
					     addr < 0x10000;
					     addr += 2) {
						printf(" %04X: ", addr);
						printf("%02X", (*gsi->emu->calc->hw.z80_rdmem)(gsi->emu->calc, addr + 1));
						printf("%02X\n", (*gsi->emu->calc->hw.z80_rdmem)(gsi->emu->calc, addr));
					}
					g_mutex_unlock(gsi->emu->calc_mutex);
					break;

				default:
					printf("Continue, Reset, Step, sTack, Brkpt, Write-brkpt?\n");
					break;
				}
			}

			continue;
		}*/

		g_mutex_lock(gsi->emu->calc_mutex);

		/*{
			TilemZ80Timer* tmr;
			if (emu->calc->z80.timers_cpu) {
				printf("C:");
				for (tmr = emu->calc->z80.timers_cpu; tmr; tmr = tmr->next) {
					printf(" %X", tmr->count - emu->calc->z80.clock);
				}
				printf("\n");
			}
			if (emu->calc->z80.timers_rt) {
				printf("R:");
				for (tmr = emu->calc->z80.timers_rt; tmr; tmr = tmr->next) {
					printf(" %X", tmr->count - emu->calc->z80.clock);
				}
				printf("\n");
			}
		}*/

		if (tilem_z80_run_time(gsi->emu->calc, 10000, NULL)) {
			bp = &gsi->emu->calc->z80.breakpoints[gsi->emu->calc->z80.stop_breakpoint];
			printf("Breakpoint (%s at %X) at time %04X\n",
			       bpstring[bp->type], bp->start,
			       gsi->emu->calc->z80.clock);
			printstate(gsi->emu);
			debugmode = 1;
		}

		if (!gsi->emu->calc->lcd.active
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
}




void create_menus(GtkWidget *window,GdkEvent *event, GtkItemFactoryEntry * menu_items, int thisitems, const char *menuname,gpointer* gsi)
{
	GtkAccelGroup *accel_group;
	GtkItemFactory *factory;
	GtkWidget *menu;
	GdkEventButton *bevent = (GdkEventButton *) event;

	/*gtk_image_factory_parse_rc()*/

	accel_group = gtk_accel_group_new();
	factory = gtk_item_factory_new(GTK_TYPE_MENU, menuname, accel_group);
	/* translatefunc */
	gtk_item_factory_create_items(factory, thisitems, menu_items, gsi);
	menu = factory->widget;

	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, bevent->button, bevent->time);
	gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

}

GtkWidget * create_draw_area(GLOBAL_SKIN_INFOS * gsi) {
	
	GtkWidget *pAf;
	int screenwidth=gsi->si->lcd_pos.right-gsi->si->lcd_pos.left;	// fixed for the test
	int screenheight=gsi->si->lcd_pos.bottom-gsi->si->lcd_pos.top;	// idem
	pAf = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1.0, TRUE);		// et pAf ! ça fait des Chocapic !! 
                         gtk_frame_set_shadow_type(GTK_FRAME(pAf),
                                                   GTK_SHADOW_NONE);
                         {
                                 gsi->emu->lcdwin = gtk_drawing_area_new();
                                 gtk_widget_set_name(gsi->emu->lcdwin, "tilem-lcd");
 
                                 gtk_widget_set_size_request(gsi->emu->lcdwin,
                                                             screenwidth,
                                                             screenheight);
                                 gtk_container_add(GTK_CONTAINER(pAf),
                                                   gsi->emu->lcdwin);
                               gtk_widget_show(gsi->emu->lcdwin);
                       }
	return pAf;
}


GtkWidget* draw_screen(GLOBAL_SKIN_INFOS *gsi)  {
	
	GtkWidget *pAf;
	GThread *th;
	gsi->emu->run_mutex = g_mutex_new();
	gsi->emu->calc_mutex = g_mutex_new();
	gsi->emu->lcd_mutex = g_mutex_new();
	gsi->emu->exiting = FALSE;
	gsi->emu->forcebreak = FALSE;

	SKIN_INFOS *si;
	si=malloc(sizeof(SKIN_INFOS));
	gsi->si=(SKIN_INFOS*)si;
	
		skin_load(gsi->si,gsi->SkinFileName);
	
	/* Create the window */
	GtkWidget *pImage;
	GtkWidget *pWindow;
	GtkWidget *pLayout;

	GtkWidget *gtk_window_new(GtkWindowType type);
	pWindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);				// GTK_WINDOW_LEVEL : define how is the window 
		gtk_window_set_title(GTK_WINDOW(pWindow),"tilem");					// define title of the window 
		gtk_window_set_position(GTK_WINDOW(pWindow),GTK_WIN_POS_CENTER); // GTK_WIN_POS_CENTER : define how the window is displayed 
		gtk_window_set_default_size(GTK_WINDOW(pWindow),gsi->si->width,gsi->si->height);	// define size of the window

	
	pImage=gtk_image_new_from_pixbuf(gsi->si->image);
	
	/* Create the draw area */
	pAf=create_draw_area(gsi);
	
	
	//gtk_container_add(GTK_CONTAINER(pImage),pAf);
	pLayout=gtk_layout_new(NULL,NULL);
	gsi->pLayout=pLayout;
	gtk_widget_show(pAf);
	gtk_layout_put(GTK_LAYOUT(pLayout),pImage,0,0);
	gtk_layout_put(GTK_LAYOUT(pLayout),pAf,gsi->si->lcd_pos.left,gsi->si->lcd_pos.top);
	gtk_container_add(GTK_CONTAINER(pWindow),pLayout);
	
	th = g_thread_create(&core_thread, gsi, TRUE, NULL);

	gsi->emu->calc->lcd.emuflags = TILEM_LCD_REQUIRE_DELAY;
	gsi->emu->calc->flash.emuflags = TILEM_FLASH_REQUIRE_DELAY;
	int screenwidth ;
	int screenheight;
	
	
	 screenwidth = gsi->emu->calc->hw.lcdwidth;
	 screenheight = gsi->emu->calc->hw.lcdheight;
	 
	gsi->emu->lcdscaledpb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,screenwidth, screenheight);
	gsi->emu->lcdlevel = g_new0(guchar, (screenheight*screenwidth));

	screenwidth = gsi->emu->calc->hw.lcdwidth + 2 * X_FRINGE;
	screenheight = gsi->emu->calc->hw.lcdheight + 2 * Y_FRINGE;
	gsi->emu->lcdimage = g_new0(guchar, 3 * screenwidth * screenheight);

	gsi->emu->lcdpb = gdk_pixbuf_new_from_data(gsi->emu->lcdimage,
					     GDK_COLORSPACE_RGB, FALSE, 8,
					     screenwidth, screenheight,
					     screenwidth * 3,
					     NULL, NULL);
					     
	//ticalcs_library_init();
					     
					     
					     
					     
	/*g_mutex_lock(gsi->emu->run_mutex);
	gsi->emu->exiting = TRUE;
	g_mutex_unlock(gsi->emu->run_mutex);
	g_thread_join(th); */
	
	
	g_signal_connect(G_OBJECT(pWindow),"destroy",G_CALLBACK(on_destroy),NULL); 
	/* Connection signal keyboard key press */
	gtk_widget_add_events(pWindow, GDK_KEY_RELEASE_MASK); // Get the event on the window (leftclick, rightclick)
	/*gtk_signal_connect(GTK_OBJECT(pWindow), "key_press_event", GTK_SIGNAL_FUNC(keyboard_event), NULL); */
	gtk_signal_connect(GTK_OBJECT(pWindow), "key_press_event", G_CALLBACK(keyboard_event), NULL);
	gtk_widget_add_events(pWindow, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_signal_connect(GTK_OBJECT(pWindow), "button_press_event", G_CALLBACK(mouse_event),(gpointer)gsi);
	gtk_widget_show_all(pWindow);	// display the window and all that it contains.
	

	return pWindow;
}

GLOBAL_SKIN_INFOS* redraw_screen(GtkWidget *pWindow,GLOBAL_SKIN_INFOS * gsi) {
	
	GtkWidget *pImage;
	GtkWidget *pAf;
	GtkWidget * pLayout;
	/*gtk_stop();*/
	printf("REDRAW_SCREEN name : %s\n",gsi->si->name);
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
	

	return gsi;

}



