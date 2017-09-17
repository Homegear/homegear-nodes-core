AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/storage

lib_LTLIBRARIES = file.la

file_la_SOURCES = file/Factory.cpp file/MyNode.cpp
file_la_LDFLAGS =-module -avoid-version -shared

file_ladir = $(libdir)
file_la_DATA = file/file.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = file/locales/en-US/file

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/file.la
