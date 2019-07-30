/*************************************************************************
** usbinfo.h for USBManager 
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
#ifndef _USBINFO_H
#define _USBINFO_H

void dumpdev(libusb_device *dev);

int get_string(libusb_device_handle *dev, char *buf, size_t size, u_int8_t id);
int get_class_string(char *buf, size_t size, u_int8_t cls);
int get_subclass_string(char *buf, size_t size, u_int8_t cls, u_int8_t subcls);
int get_protocol_string(char *buf, size_t size, u_int8_t cls, u_int8_t subcls, u_int8_t proto);


#endif
