/*************************************************************************
** usbparse.h for USBManager 
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
#ifndef _USBPARSE_H
#define _USBPARSE_H

#include "list.h"
#include <libusb-1.0/libusb.h>

#define MY_SYSFS_FILENAME_LEN 255
#define MY_PATH_MAX 4096
#define MAX_ENDPOINTS				32
#define MAX_INTERFACES				32
#define MAX_CONFIGS				32
#define MAX_CHILDREN				32

#define SYSFS_INTu(de,tgt, name) do { tgt->name = read_sysfs_file_int(de,#name,10); } while(0)
#define SYSFS_INTx(de,tgt, name) do { tgt->name = read_sysfs_file_int(de,#name,16); } while(0)
#define SYSFS_STR(de,tgt, name) do { read_sysfs_file_string(de, #name, tgt->name, MY_SYSFS_FILENAME_LEN); } while(0)

struct usbendpoint {
	int		address;
	unsigned int 	in;		/* TRUE if in, FALSE if out */
	int		attribute;
	char		*type;
	int		maxPacketSize;
	char		*interval;
};

struct usbinterface {
	struct list_head list;
	struct usbinterface *next;
	struct usbdevice *parent;
	struct usbendpoint *endpoint[MAX_ENDPOINTS];
	unsigned int configuration;
	unsigned int ifnum;

	unsigned int bAlternateSetting;
	unsigned int bInterfaceClass;
	unsigned int bInterfaceNumber;
	unsigned int bInterfaceProtocol;
	unsigned int bInterfaceSubClass;
	unsigned int bNumEndpoints;

	char name[MY_SYSFS_FILENAME_LEN];
	char driver[MY_SYSFS_FILENAME_LEN];
	unsigned int driverattached;
	unsigned int removable;
};
struct usbconfig {
	int		configNumber;
	int		numInterfaces;
	int		attributes;
	char		*maxPower;
	struct usbinterface	*interface[MAX_INTERFACES];
};



struct usbdevice {
	struct list_head list;	/* connect devices independant of the bus */
	struct usbdevice *next;	/* next port on this hub */
	struct usbinterface *first_interface;	/* list of interfaces */
	struct usbdevice *first_child;	/* connect devices on this port */
	struct usbdevice *parent;	/* hub this device is connected to */
	struct usbconfig *config[MAX_CONFIGS];
	unsigned int removable;

	libusb_device *dev;  /* added for libusb */
	unsigned int busnum;
	unsigned int parent_portnum;
	unsigned int portnum;

	unsigned int bConfigurationValue;
	unsigned int bDeviceClass;
	unsigned int bDeviceProtocol;
	unsigned int bDeviceSubClass;
	unsigned int bMaxPacketSize0;
	char bMaxPower[64];
	unsigned int bNumConfigurations;
	unsigned int bNumInterfaces;
	unsigned int bcdDevice;
	unsigned int bmAttributes;
	unsigned int configuration;
	unsigned int devnum;
	unsigned int idProduct;
	unsigned int idVendor;
	unsigned int maxchild;
	char manufacturer[64];
	char product[64];
	char serial[64];
	char version[64];
	char speed[5 + 1];	/* '1.5','12','480','5000' + '\n' */

	char name[MY_SYSFS_FILENAME_LEN];
	char driver[MY_SYSFS_FILENAME_LEN];
	char d_name[MY_SYSFS_FILENAME_LEN];
	unsigned int driverattached;

	GtkTreeIter *iter;
};

struct usbbusnode {
	struct usbbusnode *next;
	struct usbinterface *first_interface;	/* list of interfaces */
	struct usbdevice *first_child;	/* connect childs belonging to this bus */
	struct usbdevice *myself;	/* the roothub of this bus added by zhenguo yao */
	unsigned int busnum;

	unsigned int bDeviceClass;
	unsigned int devnum;
	unsigned int maxchild;
	char speed[5 + 1];	/* '1.5','12','480','5000' + '\n' */

	char driver[MY_SYSFS_FILENAME_LEN];

	GtkTreeIter *iter;
};
extern int build_usbtree_struct(void);

#endif
