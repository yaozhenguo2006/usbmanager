/*************************************************************************
** usbtree.h for USBManager 
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

#ifndef __USB_TREE_H
#define __USB_TREE_H

#include <gtk/gtk.h>
#include "usbparse.h"

void load_usb_tree(unsigned int first);
void show_msg_win(char *buf);
void show_msg_sub(char *buf);
void show_info_sub(char *buf);
void show_usb_info(void);
void win_printf(const char *fmt, ...);
void info_printf(const char *fmt, ...);
void status_printf(const char *fmt, ...);

void sys_dialog_info(const char *fmt, ...);
void sys_dialog_error(const char *fmt, ...);
void sys_dialog_warning(const char *fmt, ...);

void remove_child(struct usbdevice *dev);
void remove_device(void);

#define TEXTBUFSIZE 1000

#endif	/* __USB_TREE_H */
