#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <gui.h>
#include <signal.h>

/* Used when you load another skin */
GLOBAL_SKIN_INFOS* redraw_screen(GtkWidget *pWindow,GLOBAL_SKIN_INFOS * gsi) 
{
	if(gsi->view==1) 
	{
		DEBUGGINGGLOBAL_L2_A0("Use >>Switch view<< before !\n");
	} else {
	DEBUGGINGLCD_L2_A0("Entering : redraw_screen...\n");
	GtkWidget *pImage;
	GtkWidget *pAf;
	GtkWidget * pLayout;

	gsi->view=0;
	DEBUGGINGLCD_L2_A1("REDRAW_SCREEN name : %s\n",gsi->si->name);
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
	g_signal_connect(gsi->emu->lcdwin, "size-allocate",G_CALLBACK(screen_resize), gsi);
	g_signal_connect(gsi->emu->lcdwin, "style-set", G_CALLBACK(screen_restyle), gsi); 
	gtk_widget_add_events(pLayout, GDK_KEY_RELEASE_MASK); /* Get the event on the window (leftclick, rightclick) */
	gtk_signal_connect(GTK_OBJECT(pLayout), "key_press_event", G_CALLBACK(keyboard_event), NULL);
	gtk_widget_add_events(pLayout,GDK_BUTTON_PRESS_MASK );
	gtk_signal_connect(GTK_OBJECT(pLayout), "button-press-event", G_CALLBACK(mouse_press_event),gsi);
	gtk_widget_add_events(pLayout, GDK_BUTTON_RELEASE_MASK);	
	gtk_signal_connect(GTK_OBJECT(pLayout), "button-release-event", G_CALLBACK(mouse_release_event), gsi); 
	g_signal_connect(GTK_OBJECT(gsi->emu->lcdwin), "expose-event",G_CALLBACK(screen_repaint), gsi);
	gtk_widget_show_all(pWindow);	/*display the window and all that it contains.*/
	g_timeout_add(50, screen_update, gsi->emu);
	
	DEBUGGINGLCD_L2_A0("Exiting : redraw_screen...\n");
	}
	return gsi;

}

void switch_view(GLOBAL_SKIN_INFOS * gsi) 
{
	if(gsi->view==1) 
	{
		gsi->view=0;
		DEBUGGINGLCD_L2_A0("Entering : draw_not_only_lcd...\n");
		GtkWidget *pImage;
		GtkWidget *pAf;
		GtkWidget * pLayout;

		DEBUGGINGLCD_L2_A1("draw_not_only_lcd name : %s\n",gsi->si->name);
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
		gtk_container_add(GTK_CONTAINER(gsi->pWindow),pLayout);
		gtk_window_resize(GTK_WINDOW(gsi->pWindow),gsi->si->width,gsi->si->height);
		g_signal_connect(G_OBJECT(gsi->pWindow),"destroy",G_CALLBACK(on_destroy),NULL); 
		
		/* Connection signal keyboard key press */
		g_signal_connect(gsi->emu->lcdwin, "size-allocate",G_CALLBACK(screen_resize), gsi);
		g_signal_connect(gsi->emu->lcdwin, "style-set", G_CALLBACK(screen_restyle), gsi); 
		gtk_widget_add_events(pLayout, GDK_KEY_RELEASE_MASK); 
		gtk_signal_connect(GTK_OBJECT(pLayout), "key_press_event", G_CALLBACK(keyboard_event), NULL);
		gtk_widget_add_events(pLayout,GDK_BUTTON_PRESS_MASK );
		gtk_signal_connect(GTK_OBJECT(pLayout), "button-press-event", G_CALLBACK(mouse_press_event),gsi);
		gtk_widget_add_events(pLayout, GDK_BUTTON_RELEASE_MASK);	
		gtk_signal_connect(GTK_OBJECT(pLayout), "button-release-event", G_CALLBACK(mouse_release_event), gsi); 
		g_signal_connect(GTK_OBJECT(gsi->emu->lcdwin), "expose-event",G_CALLBACK(screen_repaint), gsi);
		gtk_widget_show_all(gsi->pWindow);	/*display the window and all that it contains.*/
		//g_timeout_add(50, screen_update, gsi->emu);

		DEBUGGINGLCD_L2_A0("Exiting : draw_not_only_lcd...\n");
	} else {
		/* Draw ONLY the lcd area */
		gsi->view=1; /* keep in memory we just print lcd */
		DEBUGGINGLCD_L2_A0("Entering : draw_only_lcd...\n");
		GtkWidget *pAf, *pLayout;

		
		/* Remove the pImage from the pWindow */ 
		gtk_container_remove(GTK_CONTAINER(gsi->pWindow),gsi->pLayout);
		
		int screenwidth=gsi->si->lcd_pos.right-gsi->si->lcd_pos.left;	
		int screenheight=gsi->si->lcd_pos.bottom-gsi->si->lcd_pos.top;
		printf("%d %d", screenwidth, screenheight);
		gtk_window_resize(GTK_WINDOW(gsi->pWindow),screenwidth, screenheight);
		pAf=create_draw_area(gsi);
		gsi->pAf=pAf;
		
		pLayout=gtk_layout_new(NULL,NULL);
		gsi->pLayout=pLayout;
		
		//gtk_container_add(GTK_CONTAINER(gsi->pWindow),pAf);
		gtk_container_add(GTK_CONTAINER(gsi->pLayout),pAf);
		gtk_widget_show(pAf);
		g_signal_connect(G_OBJECT(gsi->pWindow),"destroy",G_CALLBACK(on_destroy),NULL); 
		
		/* Connection signal keyboard key press */
		g_signal_connect(gsi->emu->lcdwin, "size-allocate",G_CALLBACK(screen_resize), gsi);
		g_signal_connect(gsi->emu->lcdwin, "style-set", G_CALLBACK(screen_restyle), gsi); 
		gtk_widget_add_events(gsi->pWindow, GDK_KEY_RELEASE_MASK); // Get the event on the window (leftclick, rightclick)
		gtk_signal_connect(GTK_OBJECT(gsi->pLayout), "key_press_event", G_CALLBACK(keyboard_event), NULL);
		gtk_widget_add_events(gsi->pLayout,GDK_BUTTON_PRESS_MASK );
		gtk_signal_connect(GTK_OBJECT(gsi->pLayout), "button-press-event", G_CALLBACK(mouse_press_event),gsi);
		gtk_widget_add_events(gsi->pLayout, GDK_BUTTON_RELEASE_MASK);	
		gtk_signal_connect(GTK_OBJECT(gsi->pLayout), "button-release-event", G_CALLBACK(mouse_release_event), gsi); 
		g_signal_connect(GTK_OBJECT(gsi->emu->lcdwin), "expose-event",G_CALLBACK(screen_repaint), gsi);
		gtk_container_add(GTK_CONTAINER(gsi->pWindow),pLayout);
		gtk_widget_show_all(gsi->pWindow);	/*display the window and all that it contains.*/
		//g_timeout_add(50, screen_update, gsi->emu);
		
		DEBUGGINGLCD_L2_A0("Exiting : draw_only_lcd...\n");
	}

}





/* update_lcdimage */
/* La moindre modif entraine un bug graphique sur une partie de l'écran */
void update_lcdimage(TilemCalcEmulator* emu)  /* Absolument necessaire */
{
	DEBUGGINGLCD_L2_A0(">update_lcdimage\n");
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
	DEBUGGINGLCD_L2_A0("<update_lcdimage\n");
}






static gpointer core_thread(gpointer data)
{
	TilemCalcEmulator* emu = data;
	int width, height;
	guchar* lcddata;
	int x, y;
	int low, high, v, old;

	width = emu->calc->hw.lcdwidth;
	height = emu->calc->hw.lcdheight;
	lcddata = g_new(guchar, (width / 8) * height);


	
	
	while (1) {
		
		g_mutex_lock(emu->calc_mutex);
		tilem_z80_run_time(emu->calc, 10000, NULL);
		
		/* Get the lcd_content */
		(*emu->calc->hw.get_lcd)(emu->calc, lcddata);
		
		/* Ne pas enlever ça sinon l'écran fonctionne mais il est de toutes les couleurs (entre blanc et noir)*/
		if (emu->calc->lcd.contrast < 32) {
			low = 0;
			high = emu->calc->lcd.contrast * 4;
		}
		else {
			low = (emu->calc->lcd.contrast - 32) * 4;
			high = 128;
		}

		
		g_mutex_unlock(emu->calc_mutex);
		g_mutex_lock(emu->lcd_mutex);
		
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				if (lcddata[(y * width + x) / 8]
				    & (0x80 >> (x % 8)))
					v = high;
				else
					v = low;

				old = emu->lcdlevel[y * width + x];
				emu->lcdlevel[y * width + x]
					= v + ((old - v) * 3) / 4;
			}
		}
		
		g_mutex_unlock(emu->lcd_mutex);
		/* sans ça, le carré se met à clignoter très vite ! */
		g_usleep(10000);
	}
}


void screen_resize(GtkWidget* w G_GNUC_UNUSED,GtkAllocation* alloc, GLOBAL_SKIN_INFOS * gsi) /* Absolument necessaire */
{
	DEBUGGINGLCD_L2_A0(">screen_resize\n");
	TilemCalcEmulator* emu = gsi->emu;
	g_object_unref(emu->lcdscaledpb);
	emu->lcdscaledpb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,alloc->width, alloc->height);
	DEBUGGINGLCD_L2_A0("<screen_resize\n");
}

void screen_restyle(GtkWidget* w, GtkStyle* oldstyle G_GNUC_UNUSED,GLOBAL_SKIN_INFOS * gsi)
{
	DEBUGGINGLCD_L2_A0(">screen_restyle\n");
	TilemCalcEmulator* emu = gsi->emu;
	emu->lcdfg = w->style->fg[GTK_STATE_NORMAL];
	emu->lcdbg = w->style->bg[GTK_STATE_NORMAL];
	gtk_widget_queue_draw(emu->lcdwin);
	DEBUGGINGLCD_L2_A0("<screen_restyle\n");
}

gboolean screen_repaint(GtkWidget* w G_GNUC_UNUSED,GdkEventExpose* ev G_GNUC_UNUSED,GLOBAL_SKIN_INFOS * gsi)
{
	DEBUGGINGLCD_L2_A0(">screen_repaint\n");
	TilemCalcEmulator* emu = gsi->emu;
	double fx, fy;

	fx = ((double) w->allocation.width/ (emu->calc->hw.lcdwidth + 2 * X_FRINGE));
	fy = ((double) w->allocation.height/ (emu->calc->hw.lcdheight + 2 * Y_FRINGE));

	g_mutex_lock(emu->lcd_mutex);
	update_lcdimage(emu);
	gdk_pixbuf_scale(emu->lcdpb, emu->lcdscaledpb,0, 0, w->allocation.width, w->allocation.height,0.0, 0.0, fx, fy, GDK_INTERP_TILES);
	g_mutex_unlock(emu->lcd_mutex);

	gdk_draw_pixbuf(w->window, w->style->fg_gc[w->state],emu->lcdscaledpb, 0, 0, 0, 0,w->allocation.width, w->allocation.height,GDK_RGB_DITHER_NONE, 0, 0);
	DEBUGGINGLCD_L2_A0("<screen_repaint\n");
	return TRUE;
}

gboolean screen_update(gpointer data)
{
	DEBUGGINGLCD_L2_A0(">screen_update\n");
	TilemCalcEmulator* emu = data;
	gtk_widget_queue_draw(emu->lcdwin);
	DEBUGGINGLCD_L2_A0("<screen_update\n");
	return TRUE;
}



void create_menus(GtkWidget *window,GdkEvent *event, GtkItemFactoryEntry * menu_items, int thisitems, const char *menuname,gpointer* gsi)
{
	
	DEBUGGINGLCD_L2_A0("Entering : create_menus...\n");
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
	DEBUGGINGLCD_L2_A0("Exiting create_menus...\n");

}



GtkWidget* draw_screen(GLOBAL_SKIN_INFOS *gsi)  
{
	//g_thread_init(NULL);
	
	DEBUGGINGLCD_L0_A0("**************** fct : draw_screen *********************\n");
	DEBUGGINGLCD_L0_A0("*  - load skin                                         *\n");
	DEBUGGINGLCD_L0_A0("*  - create GtkLayout                                  *\n");
	DEBUGGINGLCD_L0_A0("*  - add skin, add lcd area                            *\n");
	DEBUGGINGLCD_L0_A0("*  - connect events (callback)                         *\n");
	DEBUGGINGLCD_L0_A0("*  - print top level window                            *\n");
	DEBUGGINGLCD_L0_A0("*  - launch thread                                     *\n");
	DEBUGGINGLCD_L0_A0("********************************************************\n");
	int screenwidth ;
	int screenheight;
	GtkWidget *pAf;
	GThread *th;

	gsi->view=0;
	/* LOAD A SKIN */
	SKIN_INFOS *si;
	si=malloc(sizeof(SKIN_INFOS));
	gsi->si=(SKIN_INFOS*)si;
	skin_load(gsi->si,gsi->SkinFileName);
	
	/* Create the window */
	GtkWidget *pWindow,  *pImage, *pLayout;
	pWindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);	// GTK_WINDOW_LEVEL : define how is the window 
	gtk_window_set_title(GTK_WINDOW(pWindow),"tilem");	// define title of the window 
	gtk_window_set_position(GTK_WINDOW(pWindow),GTK_WIN_POS_CENTER); // GTK_WIN_POS_CENTER : define how the window is displayed 
	gtk_window_set_default_size(GTK_WINDOW(pWindow),gsi->si->width,gsi->si->height);	// define size of the window
	pImage=gtk_image_new_from_pixbuf(gsi->si->image);
	
	g_signal_connect(G_OBJECT(pWindow),"destroy",G_CALLBACK(on_destroy),NULL); 
	gtk_widget_add_events(pWindow, GDK_KEY_RELEASE_MASK); 		/* Get the event on the window (leftclick, rightclick) */
	gtk_signal_connect(GTK_OBJECT(pWindow), "key_press_event", G_CALLBACK(keyboard_event), NULL);
	

	
	/* Create the draw area */
	pAf=create_draw_area(gsi);
	
	/* Add the lcd to the pWindow using a GtkLayout */
	pLayout=gtk_layout_new(NULL,NULL);
	gsi->pLayout=pLayout;
	gtk_widget_show(pAf);
	gtk_layout_put(GTK_LAYOUT(pLayout),pImage,0,0);
	gtk_layout_put(GTK_LAYOUT(pLayout),pAf,gsi->si->lcd_pos.left,gsi->si->lcd_pos.top);
	
	g_signal_connect(gsi->emu->lcdwin, "size-allocate",G_CALLBACK(screen_resize), gsi);
	g_signal_connect(gsi->emu->lcdwin, "style-set", G_CALLBACK(screen_restyle), gsi); 
	gtk_widget_add_events(pLayout, GDK_BUTTON_PRESS_MASK);	
	gtk_signal_connect(GTK_OBJECT(pLayout), "button-press-event", G_CALLBACK(mouse_press_event),gsi);
	gtk_widget_add_events(pLayout, GDK_BUTTON_RELEASE_MASK);	
	gtk_signal_connect(GTK_OBJECT(pLayout), "button-release-event", G_CALLBACK(mouse_release_event), gsi); 
	g_signal_connect(GTK_OBJECT(gsi->emu->lcdwin), "expose-event",G_CALLBACK(screen_repaint), gsi);
	gtk_container_add(GTK_CONTAINER(pWindow),pLayout);
	
	
	
	gsi->emu->lcdlevel = g_new0(guchar, (gsi->emu->calc->hw.lcdwidth * gsi->emu->calc->hw.lcdheight));
	screenwidth = gsi->emu->calc->hw.lcdwidth + 2 * X_FRINGE;
	screenheight = gsi->emu->calc->hw.lcdheight + 2 * Y_FRINGE ;
	gsi->emu->lcdimage = g_new0(guchar, 3 * screenwidth * screenheight);
	
	gsi->emu->lcdpb = gdk_pixbuf_new_from_data(gsi->emu->lcdimage,GDK_COLORSPACE_RGB, FALSE, 8,screenwidth, screenheight,screenwidth * 3,NULL, NULL);
	gsi->emu->lcdscaledpb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,screenwidth, screenheight);
	

	gtk_widget_show_all(pWindow);	/* display the window and all that it contains. */
	
	/* THREAD */
	th = g_thread_create(&core_thread, gsi->emu, TRUE, NULL);
	g_timeout_add(50, screen_update, gsi->emu);
	


	return pWindow;
}



GtkWidget * create_draw_area(GLOBAL_SKIN_INFOS * gsi) 
{
	

	GtkWidget *pAf;
	/* Get the size of the lcd area in the SKIN_INFOS struct */
	int screenwidth=gsi->si->lcd_pos.right-gsi->si->lcd_pos.left;	
	int screenheight=gsi->si->lcd_pos.bottom-gsi->si->lcd_pos.top; 
	
	/*int screenwidth = gsi->emu->calc->hw.lcdwidth + 2 * X_FRINGE;
	int screenheight = gsi->emu->calc->hw.lcdheight + 2 * Y_FRINGE;*/
	
	DEBUGGINGLCD_L0_A0("**************** fct : create_draw_area ****************\n");
	DEBUGGINGLCD_L0_A1("*  screenwidth = %d                                   *\n",screenwidth);
	DEBUGGINGLCD_L0_A1("*  screenheight = %d                                  *\n",screenheight);
	DEBUGGINGLCD_L0_A0("********************************************************\n");
	pAf = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1.0, TRUE);	
                         gtk_frame_set_shadow_type(GTK_FRAME(pAf),GTK_SHADOW_NONE);
                         {
                                 gsi->emu->lcdwin = gtk_drawing_area_new();
                                 gtk_widget_set_name(gsi->emu->lcdwin, "tilem-lcd");
                                 gtk_widget_set_size_request(gsi->emu->lcdwin,screenwidth,screenheight);
                                 gtk_container_add(GTK_CONTAINER(pAf),gsi->emu->lcdwin);
                                 gtk_widget_show(gsi->emu->lcdwin);
                       }
	g_signal_connect(gsi->emu->lcdwin, "expose-event",G_CALLBACK(screen_repaint), (gpointer)gsi);

	return pAf;
}






