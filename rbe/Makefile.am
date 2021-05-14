AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/rbe

lib_LTLIBRARIES = rbe.la

rbe_la_SOURCES = Factory.cpp MyNode.cpp
rbe_la_LDFLAGS =-module -avoid-version -shared

rbe_ladir = $(libdir)
rbe_la_DATA = package.json rbe.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/rbe.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/rbe.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/rbe.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/rbe.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/rbe.la
