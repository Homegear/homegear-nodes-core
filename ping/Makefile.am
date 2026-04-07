AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/ping
lib_LTLIBRARIES = ping.la
ping_la_SOURCES = Factory.cpp MyNode.cpp
ping_la_LDFLAGS =-module -avoid-version -shared
ping_ladir = $(libdir)
ping_la_DATA = package.json ping.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/ping.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/ping.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/ping.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/ping.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/ping.la
