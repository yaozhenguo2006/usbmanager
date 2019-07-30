/*************************************************************************
** main.c for USBManager 
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
** (See the included file COPYING)
*************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <gtk/gtk.h>
#include <libusb.h>

#include "usbtree.h"
#include "usbparse.h"
#include "names.h"
#include "syserr.h"
#include "interface.h"


libusb_context *ctx;

int main (int argc, char *argv[])
{
	int err;

	gtk_set_locale ();
	gtk_init (&argc, &argv);   

	create_windowmain ();

	/* by default, print names as well as numbers */
	err = names_init(DATADIR "/usb.ids");

	if (err != 0)
		goto err_names;

	err = libusb_init(&ctx);
	if (err) 
		goto err_libusb;		

 	load_usb_tree(1);
 
	gtk_main ();

err_names:
	err_sys("usb.ids");
err_libusb:
	names_exit();
	err_quit("unable to initialize libusb: %i\n", err);
	return 0;
}

