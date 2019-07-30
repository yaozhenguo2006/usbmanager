/*************************************************************************
** interface.h for USBManager 
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
#ifndef _INTERFACE_H
#define _INTERFACE_H

#include "../config.h"

enum {
        COL_OPEN_PIXBUF,
        COL_CLOSED_PIXBUF,
        COL_TITLE,
        COL_DATA,
        N_COLUMNS
};

typedef struct {
        GdkPixbuf *pixbuf_hubopen;
        GdkPixbuf *pixbuf_hubclosed;
        GdkPixbuf *pixbuf_bus;
	GdkPixbuf *pixbuf_device;
} TocPixbufs;


extern GtkWidget 	*windowsub;
extern GtkWidget	*treeview;
extern GtkTreeStore	*model;
extern TocPixbufs       *pixbufs;

extern GtkTextBuffer *textbasicbuffer;
extern GtkTextBuffer *textdesbuffer;
extern GtkTextBuffer *textinforbuffer;

extern GtkWidget	*textbasicview;
extern GtkWidget	*textdesview;
extern GtkWidget	*textinforview;

extern GtkWidget *statusbar;
extern int statusbar_id;

#define DATADIR	 PACKAGE_DATA_DIR

void create_windowmain(void);
void create_windowsub (void);
void create_windowabout (void);

#endif
