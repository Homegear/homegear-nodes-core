AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/homegear-event
lib_LTLIBRARIES = homegear-event.la
homegear_event_la_SOURCES = Factory.cpp MyNode.cpp
homegear_event_la_LDFLAGS =-module -avoid-version -shared
homegear_event_ladir = $(libdir)
homegear_event_la_DATA = package.json homegear-event.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/homegear-event.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/homegear-event.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/homegear-event.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/homegear-event.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/homegear-event.la
