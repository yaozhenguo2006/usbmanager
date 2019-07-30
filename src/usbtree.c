/*************************************************************************
** usbtree.c for USBManager 
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "usbtree.h"
#include "usbparse.h"
#include "list.h"
#include "names.h"
#include "syserr.h"
#include "usbinfo.h"


extern struct usbbusnode *usbbuslist;
extern struct list_head interfacelist;
extern struct list_head usbdevlist;
extern struct usbdevice *currentdev;
static unsigned int last_device_num;
extern libusb_context *ctx;

libusb_device **devlist;
char *desbuf;

static const char sys_bus_usb_devices[] = "/sys/bus/usb/devices";



static struct usbdevice *find_usb_child(struct usbdevice *parent, int busnum, int devnum)
{
	struct usbdevice *d, *ret;
	
	for (d = parent->first_child; d != NULL; d = d->next) {
		if (busnum == d->busnum && devnum == d->devnum)
			return d;
		if (d->first_child)
			ret = find_usb_child(d, busnum, devnum);
		if (ret)
			return ret;
	}
	return NULL;	
	
}
static struct usbdevice *find_usb_device(int busnum, int devnum)
{
	struct usbdevice *d, *ret;
	struct usbbusnode *b;
	/* walk bus */
	for (b = usbbuslist; b != NULL; b = b->next) {
		d = b->myself;
		if (busnum == d->busnum && devnum == d->devnum)
			return d;
		/* walk root hub */
		for (d = b->first_child; d != NULL; d = d->next) {
			if (busnum == d->busnum && devnum == d->devnum)
				return d;
			ret = find_usb_child(d, busnum, devnum);
			if (ret)
				return ret;
		}
	}
	return NULL;	
}
void remove_child(struct usbdevice *dev)
{
	struct usbdevice *d;
	char command[MY_PATH_MAX + 8] = "echo 1 > ";
	if ( NULL == dev)
		return;
	for (d = dev->first_child; d != NULL; d = d->next) {
		remove_child(d);
		gtk_tree_store_remove(model, d->iter);
		snprintf(command + strlen(command), MY_PATH_MAX, "%s/%s/remove", sys_bus_usb_devices, d->d_name);
		if (system(command) < 0) {
			perror("remove:");
		}
	}	
}
void remove_device(void)
{
	struct usbdevice *d;
	char command[MY_PATH_MAX + 8] = "echo 1 > ";
	/* must root */
	if (getuid()) {
		sys_dialog_info("Permission denied !");
		return;
	}
	if (currentdev == NULL) {
		sys_dialog_info("No device has been chosen!");
		return;
	}
	if (currentdev->removable == 0) {
		sys_dialog_info("Device can not be removed!");
		return;
	}

	d = currentdev;
	if (d->first_child != NULL)
		remove_child(d);
	snprintf(command + strlen(command), MY_PATH_MAX, "%s/%s/remove", sys_bus_usb_devices, d->d_name);
	if (system(command) < 0) {
		sys_dialog_info("Error when removing device!");
		return;
	}
	gtk_tree_store_remove(model, d->iter);
	currentdev = NULL;
	
}

static void init_usb_list(void)
{
	struct usbbusnode *t, *b;
	struct list_head *ld, *li, *temp;
	struct usbdevice *d;
	struct usbinterface *e;

	b = usbbuslist;
	while (b) {
		t = b;
		b = b->next;
		free(t->myself->iter);
		free(t->myself);
		free(t);

	}
	ld = usbdevlist.next;
	while (ld != &usbdevlist) {
	        temp = ld;
		ld = ld->next;
		d = list_entry(temp, struct usbdevice, list);
		free(d->iter);	
		free(d);			
	}
	li = interfacelist.next;
	while (li != &interfacelist) {
	        temp =  li;
		li = li->next;
		e = list_entry(temp, struct usbinterface, list);	
		free(e);			
	}		
}
static int match_libusb_dev(libusb_context *ctx)
{
	ssize_t cnt;
	unsigned int i;
	struct libusb_device_descriptor desc;
	struct usbdevice *d;

	cnt = libusb_get_device_list(ctx, &devlist);
	if (cnt < 0)
		return (int) cnt;
	for (i = 0; i < cnt; ++i) {
		libusb_device *dev = devlist[i];
		uint8_t bnum = libusb_get_bus_number(dev);
		uint8_t dnum = libusb_get_device_address(dev);

		libusb_get_device_descriptor(dev, &desc);
		d = find_usb_device(bnum, dnum);
		if (d)
			d->dev = dev;
		else 
			fprintf(stdout,"ERR:Cant't match libusbdev with devicelist \n");
		
	}
	return 0;	
}
static void name_device(void)
{
	struct usbbusnode *p;
	struct list_head *ld, *li;
	struct usbdevice *d;
	struct usbinterface *i;
	char cls[128] = ""; 

	p = usbbuslist;
	while (p) {
		d = p->myself;
		get_product_string(d->name, sizeof(d->name), d->idVendor, d->idProduct);
		if (strlen(d->name) == 0) 
			snprintf(d->name, sizeof(d->name), "%s", d->product);
		p = p->next;
	}

	ld = usbdevlist.next;
	while (ld != &usbdevlist) {
		d = list_entry(ld, struct usbdevice, list);
		get_product_string(d->name, sizeof(d->name), d->idVendor, d->idProduct);
		if (strlen(d->name) == 0) 
			snprintf(d->name, sizeof(d->name), "%s", d->product);
		ld = ld->next;			
	}

	li = interfacelist.next;
	while (li != &interfacelist) {
		i = list_entry(li, struct usbinterface, list);
		get_class_string(cls, sizeof(cls), i->bInterfaceClass);

		if (strlen(cls) != 0)
			snprintf(i->name, sizeof(i->name), "%s", cls);
		else
			snprintf(i->name, sizeof(i->name), "%s", i->driver);
		li = li->next;			
	}	
}

static int usbsys_has_changed (void)
{
	DIR *sbud = opendir(sys_bus_usb_devices);
	struct dirent *de;
	unsigned int num = 0;
	if (sbud) {
		while ((de = readdir(sbud)))
			num++;
		if (num == last_device_num)
		/* no change */
			return 0;
		else {
			last_device_num = num;
			return 1;
		}
	} else { 
		perror(sys_bus_usb_devices);
		abort();
	}
}
void show_basic_info(struct usbdevice *dev)
{
	struct usbinterface *i;
	char cls[128] = "";
	char subcls[128] = ""; 
	char proto[128] = "";
	char *textbuf;

	textbuf = (char *) malloc(TEXTBUFSIZE);
	if (NULL == textbuf) {
		fprintf(stderr, "Can't get memory\n");
		return;
	}
	memset(textbuf, 0, TEXTBUFSIZE);

	get_vendor_string(dev->manufacturer, sizeof(dev->manufacturer), dev->idVendor);
	get_class_string(cls, sizeof(cls), dev->bDeviceClass);
	get_subclass_string(subcls, sizeof(subcls),
			dev->bDeviceClass, dev->bDeviceSubClass);
	get_protocol_string(proto, sizeof(proto), dev->bDeviceClass,
			dev->bDeviceSubClass, dev->bDeviceProtocol);

	snprintf(textbuf, TEXTBUFSIZE, 
               "\nDevice Basic Information:"
	       "\nName: %s"
	       "\nManufacturer: %s"
	       "\nSerial: %s"
	       "\nSpeed: %sMbyte/s"
               "\nDevice Class: %s"
               "\nDevice Subclass: %s"
               "\nProtocol: %s"
               "\nDriver: %s",
	       dev->name,
	       strlen(dev->manufacturer) ? dev->manufacturer : "Unknown",
	       strlen(dev->serial) ? dev->serial : "Unknown",
               strlen(dev->speed) ? dev->speed : "Unknown",
               strlen(cls) ? cls : "Unknown",
               strlen(subcls) ? subcls : "Unknown",
               strlen(proto) ? proto : "Unknown",
               strlen(dev->driver) ? dev->driver : "Unknown");	
	show_msg_win(textbuf);

	if ( dev->first_interface == NULL) {
		snprintf(textbuf, TEXTBUFSIZE, "\nThis is root hub");
		show_msg_win(textbuf);
	} else {
		snprintf(textbuf, TEXTBUFSIZE, "\n\nInterface Information:");
		show_msg_win(textbuf);
		for (i = dev->first_interface; i != NULL; i = i->next) {
			get_class_string(cls, sizeof(cls), i->bInterfaceClass);
			get_subclass_string(subcls, sizeof(subcls), i->bInterfaceClass, i->bInterfaceSubClass);
			get_protocol_string(proto, sizeof(proto), i->bInterfaceClass, i->bInterfaceSubClass, i->bInterfaceProtocol);
			snprintf(textbuf, TEXTBUFSIZE, 
	        	       "\n\nInterface Number %d:"
			       "\nSetting: %d"
                	       "\nInterface Class: %s"
                	       "\nInterface Subclass: %s"
                	       "\nProtocol: %s"
			       "\nEndpoint Number: %d"
	        	       "\nInterface Driver: %s",
			       i->ifnum,
			       i->bAlternateSetting,
	        	       strlen(cls) ? cls : "Unknown",
	        	       strlen(subcls) ? subcls : "Unknown",
	        	       strlen(proto) ? proto : "Unknown",
			       i->bNumEndpoints,
	        	       strlen(i->driver) ? i->driver : "Unknown");	
			show_msg_win(textbuf);
		}
	}
	free(textbuf);
	textbuf = NULL;
}

void select_item(GtkTreeView       *tree_view,
                 GtkTreePath       *path,
                 GtkTreeViewColumn *column,
                 gpointer           user_data) 
{
	GtkTreeModel *treemodel;
	GtkTreeIter iter;
	int     data;
	int     deviceNumber;
	int     busNumber;
	struct usbdevice *dev;
	libusb_device *libdev;
	GtkTextIter begin;
	GtkTextIter end;


	treemodel = gtk_tree_view_get_model (tree_view);
	gtk_tree_model_get_iter(treemodel, &iter, path);
	gtk_tree_model_get(treemodel, &iter, COL_DATA, &data, -1);
	
	deviceNumber = (data >> 8);
	busNumber = (data & 0x00ff);	
	dev = find_usb_device(busNumber, deviceNumber);
	currentdev = dev;
	if (NULL == dev) {
		fprintf(stderr, "Cant't find usb device:busnum is %d devnum is %d\n", busNumber, deviceNumber);
		return;
	}
	libdev = dev->dev;
	if (NULL == libdev) {
		printf("ERR:Device has no libdevice number\n");
		return;	
	}
	status_printf("Device：%s", dev->name);

	/* clear the textbox */
	gtk_text_buffer_get_start_iter(textbasicbuffer,&begin);
	gtk_text_buffer_get_end_iter(textbasicbuffer,&end);
	gtk_text_buffer_delete (textbasicbuffer, &begin, &end);
	/* freeze the display */
	/* this keeps the annoying scroll from happening */
	gtk_widget_freeze_child_notify(textbasicview);
	show_basic_info(dev);
	gtk_widget_thaw_child_notify(textbasicview);

	return;
}
static void insert_child(struct usbdevice *dev, GtkTreeIter *parent)
{
	struct usbdevice *d;
	GtkTreeIter *myself = NULL;

	for (d = dev->first_child; d != NULL; d = d->next) {
		myself = (GtkTreeIter *) malloc(sizeof(GtkTreeIter));
		if (NULL == myself) {
			return;
		}
		gtk_tree_store_append(model, myself, parent);
		d->iter = myself;
		if (d->first_child != NULL) {
        		gtk_tree_store_set(model, myself,
                                  COL_OPEN_PIXBUF, pixbufs->pixbuf_hubopen,
                                  COL_CLOSED_PIXBUF, pixbufs->pixbuf_hubclosed,
                                  COL_TITLE, d->name,
                                  COL_DATA,  (d->devnum<<8) | (d->busnum), -1);
			insert_child(d, myself);
		} else {
			gtk_tree_store_set(model, myself,
                                  COL_OPEN_PIXBUF, pixbufs->pixbuf_device,
                                  COL_CLOSED_PIXBUF, pixbufs->pixbuf_device,
                                  COL_TITLE, d->name,
                                  COL_DATA,  (d->devnum<<8) | (d->busnum), -1);
		}
	}
}
static GtkTreeIter *insert_usbbusnode(struct usbbusnode *b)
{
	GtkTreeIter	*myself;
	struct usbdevice *d = b->myself;

	myself = (GtkTreeIter *) malloc(sizeof(GtkTreeIter));
	if (NULL == myself) {
		return NULL;
	}

	gtk_tree_store_append(model, myself, NULL);

	gtk_tree_store_set(model, myself,
                          COL_OPEN_PIXBUF, pixbufs->pixbuf_bus,
                          COL_CLOSED_PIXBUF, pixbufs->pixbuf_bus,
                          COL_TITLE, d->name,
                          COL_DATA,  (d->devnum<<8) | (d->busnum), -1);

	d->iter = myself;

	return myself;
	
}

void load_usb_tree(unsigned int first)
{
	GtkTreeIter	*parent;
	static gboolean signal_connected = FALSE;
	struct usbbusnode *b = NULL;

	/* there is no device change in our system */
	if (!usbsys_has_changed()) {
		return;
	}
	
	if (getuid()) 
		sys_dialog_info("Permission is not enough，Part of functions can not be used.Please use sudo to run our program!");
	
	/* free memory: usbbusnode interface usbdevice from usbbuslist usbdevlist interfacelist */	
	if (usbbuslist != NULL) {
		init_usb_list();
		usbbuslist = NULL;
		INIT_LIST_HEAD(&interfacelist);
		INIT_LIST_HEAD(&usbdevlist);
	}

	build_usbtree_struct();

	if (match_libusb_dev(ctx) < 0) {
		sys_dialog_error("Match lisusb device faild");
		return;
	}

	name_device();

	b = usbbuslist;
	/* destroy usbtree tree */
	gtk_tree_store_clear(model);

	/* build usb tree */
	while (b) {
		struct usbdevice *d;
		GtkTreeIter *myself;
		parent = insert_usbbusnode(b);
		for (d = b->first_child; d != NULL; d = d->next) {
			myself = (GtkTreeIter *) malloc(sizeof(GtkTreeIter));
			if (NULL == myself) {
				return;
			}
			gtk_tree_store_append(model, myself, parent);
			d->iter = myself;
			if (d->first_child != NULL) {
        			gtk_tree_store_set(model, myself,
                                        COL_OPEN_PIXBUF, pixbufs->pixbuf_hubopen,
                                        COL_CLOSED_PIXBUF, pixbufs->pixbuf_hubclosed,
                                        COL_TITLE, d->name,
                                        COL_DATA,  (d->devnum<<8) | (d->busnum), -1);
			         insert_child(d, myself);
			} else {
				gtk_tree_store_set(model, myself,
                                       COL_OPEN_PIXBUF, pixbufs->pixbuf_device,
                                       COL_CLOSED_PIXBUF, pixbufs->pixbuf_device,
                                       COL_TITLE, d->name,
                                       COL_DATA,  (d->devnum<<8) | (d->busnum), -1);
			}	
		}
		b = b->next;
	}

	gtk_widget_show (treeview);


	/* hook up our callback function to this tree if we haven't yet */
	if (!signal_connected) {
		gtk_signal_connect (GTK_OBJECT (treeview), "row-activated", GTK_SIGNAL_FUNC (select_item), NULL);
		signal_connected = TRUE;
	}

	return;
}

void show_msg_win(char *buf)
{
	gtk_text_buffer_insert_at_cursor(textbasicbuffer, buf, strlen(buf));
	memset(buf, 0, strlen(buf) + 1);
}

void show_msg_sub(char *buf)
{
	gtk_text_buffer_insert_at_cursor(textdesbuffer, buf, strlen(buf));
	memset(buf, 0, strlen(buf) + 1);
}
void show_info_sub(char *buf)
{
	gtk_text_buffer_insert_at_cursor(textinforbuffer, buf, strlen(buf));
	memset(buf, 0, strlen(buf) + 1);
}


void win_printf(const char *fmt, ...)
{
	va_list ap;
	char *buf;

	buf = desbuf;

	va_start(ap, fmt);

	vsnprintf(buf, 4096, fmt, ap);
	show_msg_sub(buf);

	va_end(ap);
}
void info_printf(const char *fmt, ...)
{
	va_list ap;
	char *buf;

	buf = desbuf;

	va_start(ap, fmt);

	vsnprintf(buf, 4096, fmt, ap);
	show_info_sub(buf);

	va_end(ap);
}
void status_printf(const char *fmt, ...)
{
	va_list ap;
	char buf[100];

	va_start(ap, fmt);
	vsnprintf(buf, 100, fmt, ap);
	gtk_statusbar_push(GTK_STATUSBAR(statusbar), statusbar_id, buf);

	va_end(ap);
}


void show_usb_info(void)
{
	libusb_device *libdev;

	if( currentdev == NULL)
		return;	
	libdev = currentdev->dev;
	desbuf = (char *) malloc(TEXTBUFSIZE);
	if (NULL == desbuf) {
		fprintf(stderr, "Can't get memory\n");
		return;
	}
	memset(desbuf, 0, TEXTBUFSIZE);
	dumpdev(libdev);
	free(desbuf);
	desbuf = NULL;
}
void sys_dialog_info(const char *fmt, ...)
{
	va_list ap;
	GtkWidget* dialog;
	char message[100];

	va_start(ap, fmt);
	vsnprintf(message, 100, fmt, ap);
	va_end(ap);

	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, 
						GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void sys_dialog_error(const char *fmt, ...)
{
	va_list ap;
	GtkWidget* dialog;
	char message[100];

	va_start(ap, fmt);
	vsnprintf(message, 100, fmt, ap);
	va_end(ap);

	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, 
						GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", message);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}
void sys_dialog_warning(const char *fmt, ...)
{
	va_list ap;
	GtkWidget* dialog;
	char message[100];

	va_start(ap, fmt);
	vsnprintf(message, 100, fmt, ap);
	va_end(ap);

	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, 
						GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", message);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}





