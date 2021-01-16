AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/storage

lib_LTLIBRARIES = file.la

file_la_SOURCES = file/Factory.cpp file/MyNode.cpp
file_la_LDFLAGS =-module -avoid-version -shared

file_ladir = $(libdir)
file_la_DATA = package.json file/file.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/storage.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/storage.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/storage.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/storage.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/file.la
