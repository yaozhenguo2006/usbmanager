## Process this file with automake to produce Makefile.in

INCLUDES = \
	$(GTK_CFLAGS)	\
	$(LIBUSB_CFLAGS)	\
	$(GLIB_CFLAGS)	\
	@CPPFLAGS@

VERSION = 1.0

bin_PROGRAMS = usbmanager


usbmanager_SOURCES =		\
	main.c			\
	interface.c		\
	interface.h		\
	callbacks.c		\
	callbacks.h		\
	usbinfo.c		\
	usbinfo.h		\
	usbparse.c		\
	usbparse.h		\
	syserr.c		\
	syserr.h		\
	usbtree.c		\
	usbtree.h		\
	names.c			\
	names.h			\
	usbmisc.h		\
	list.h			

usbmanager_LDADD = \
	$(GTK_LIBS)	\
	$(GLIB_LIBS) \
	$(LIBUSB_LIBS)

