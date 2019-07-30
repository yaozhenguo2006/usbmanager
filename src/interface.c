/*************************************************************************
** interface.c for USBManager 
** Copyright (c) 2012, 2013 by YAO Zhen-Guo <yaozhenguo2006@gmail.com>
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
*************************************************************************/

#include <gtk/gtk.h>
#include <unistd.h>
#include <string.h>

#include "interface.h"
#include "callbacks.h"


GtkWidget        *treeview;
GtkTreeStore     *model;

GtkTextBuffer *textbasicbuffer;
GtkTextBuffer *textdesbuffer;
GtkTextBuffer *textinforbuffer;

GtkWidget	*textbasicview;
GtkWidget	*textdesview;
GtkWidget	*textinforview;

TocPixbufs       *pixbufs;
GtkWidget 	*statusbar;
int statusbar_id;
int timer;


static TocPixbufs *create_pixbufs(void)
{
        TocPixbufs *pixbufs;

        pixbufs = g_slice_new(TocPixbufs);

        pixbufs->pixbuf_hubopen = gdk_pixbuf_new_from_file("./data/hub_open.png", NULL);
        pixbufs->pixbuf_hubclosed = gdk_pixbuf_new_from_file("./data/hub_close.png", NULL);
        pixbufs->pixbuf_bus    = gdk_pixbuf_new_from_file("./data/usbbus.png", NULL);
	pixbufs->pixbuf_device    = gdk_pixbuf_new_from_file("./data/device.png", NULL);
        return pixbufs;
}

static GtkTreeViewColumn *create_columns(void)
{
        GtkTreeViewColumn *column = gtk_tree_view_column_new();
        GtkCellRenderer *cell = gtk_cell_renderer_pixbuf_new();

        gtk_tree_view_column_pack_start(column, cell, FALSE);
        gtk_tree_view_column_set_attributes(
                column,
                cell,
                "pixbuf", COL_OPEN_PIXBUF,
                "pixbuf-expander-open", COL_OPEN_PIXBUF,
                "pixbuf-expander-closed", COL_CLOSED_PIXBUF,
                NULL);

        cell = gtk_cell_renderer_text_new();
        g_object_set(cell,
                     "ellipsize", PANGO_ELLIPSIZE_END,
                     NULL);
        gtk_tree_view_column_pack_start(column, cell, TRUE);
        gtk_tree_view_column_set_attributes(column,
                                            cell,
                                            "text", COL_TITLE,
                                            NULL);

        return column;
}

void create_windowmain()
{
	GtkBuilder	*builder;
	GtkWidget       *windowMain;

	GtkWidget *menuitem;
	GtkWidget *toolbutton;
	GtkWidget *scrolledwindow;
 
	builder = gtk_builder_new ();
	gtk_builder_add_from_file (builder, DATADIR "/USBManager.glade", NULL);

	windowMain = GTK_WIDGET (gtk_builder_get_object (builder, "windowMain"));
	g_signal_connect(GTK_WINDOW(windowMain), "destroy", G_CALLBACK(on_windowmain_delete_event), NULL);
	/* menu signal */
	menuitem = GTK_WIDGET (gtk_builder_get_object (builder, "imagemenuitemquit"));
	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_buttonClose_clicked), NULL);
	menuitem = GTK_WIDGET (gtk_builder_get_object (builder, "imagemenuitemedit"));
	//g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(gtk_main_quit), NULL);
	menuitem = GTK_WIDGET (gtk_builder_get_object (builder, "imagemenuitemabout"));
	g_signal_connect(G_OBJECT(menuitem),"activate",G_CALLBACK(on_buttonAbout_clicked), NULL);
	/* toobar signal */ 
	toolbutton = GTK_WIDGET (gtk_builder_get_object (builder, "toolbuttoninfo"));
	g_signal_connect(G_OBJECT(toolbutton),"clicked",G_CALLBACK(on_buttoninfo_clicked), NULL);
	toolbutton = GTK_WIDGET (gtk_builder_get_object (builder, "toolbuttonremove"));
	g_signal_connect(G_OBJECT(toolbutton),"clicked",G_CALLBACK(on_buttonDelete_clicked), NULL);
	toolbutton = GTK_WIDGET (gtk_builder_get_object (builder, "toolbuttonabout"));
	g_signal_connect(G_OBJECT(toolbutton),"clicked",G_CALLBACK(on_buttonAbout_clicked), NULL);
	toolbutton = GTK_WIDGET (gtk_builder_get_object (builder, "toolbuttonquit"));
	g_signal_connect(G_OBJECT(toolbutton),"clicked",G_CALLBACK(on_buttonClose_clicked), NULL);
	/* create treeview */
	scrolledwindow = GTK_WIDGET (gtk_builder_get_object (builder, "scrolledwindow1"));
	pixbufs = create_pixbufs();
	treeview = gtk_tree_view_new();
        model = gtk_tree_store_new(N_COLUMNS,
                                         GDK_TYPE_PIXBUF,
                                         GDK_TYPE_PIXBUF,
                                         G_TYPE_STRING,
                                         G_TYPE_INT);

        gtk_tree_view_set_model(GTK_TREE_VIEW (treeview),
                                GTK_TREE_MODEL (model));
        gtk_tree_view_set_headers_visible(GTK_TREE_VIEW (treeview), FALSE);
        gtk_tree_view_append_column(GTK_TREE_VIEW (treeview), create_columns());
	gtk_container_add(GTK_CONTAINER(scrolledwindow), treeview);

	/* get basic text buffer */ 
	textbasicview =  GTK_WIDGET (gtk_builder_get_object (builder, "textview1"));
	textbasicbuffer = GTK_TEXT_BUFFER(gtk_builder_get_object (builder, "textbuffer1"));
	statusbar = GTK_WIDGET (gtk_builder_get_object (builder, "statusbar1"));
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), statusbar_id, "No device has been chosen!");

	/* create our timer */
	timer = gtk_timeout_add (1000, on_timer_timeout, 0);	       
	gtk_widget_show_all(windowMain);                
 
}

void create_windowsub ()
{
	GtkBuilder	*builder;
	GtkWidget *windowsub;

	builder = gtk_builder_new ();
	gtk_builder_add_from_file (builder, DATADIR "/USBManager.glade", NULL);
	windowsub = GTK_WIDGET (gtk_builder_get_object (builder, "windowsub"));

	/* get descraptor text buffer */
	textdesview =  GTK_WIDGET (gtk_builder_get_object (builder, "textview2"));
	textdesbuffer = GTK_TEXT_BUFFER(gtk_builder_get_object (builder, "textbuffer2"));
	/* get information text buffer */ 
	textinforview =  GTK_WIDGET (gtk_builder_get_object (builder, "textview3"));
	textinforbuffer = GTK_TEXT_BUFFER(gtk_builder_get_object (builder, "textbuffer3"));

	gtk_widget_show_all(windowsub);
}
void create_windowabout ()
{
	GtkBuilder	*builder;
	GtkWidget  *about;
	builder = gtk_builder_new ();
	gtk_builder_add_from_file (builder, DATADIR "/USBManager.glade", NULL);
	about = GTK_WIDGET (gtk_builder_get_object (builder, "aboutdialog1"));
	gtk_widget_show(about); 
}

