AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/average

lib_LTLIBRARIES = average.la

average_la_SOURCES = Factory.cpp MyNode.cpp
average_la_LDFLAGS =-module -avoid-version -shared

average_ladir = $(libdir)
average_la_DATA = package.json average.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/average.json locales/en-US/average.help.html

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/average.json locales/en-US/average.help.html

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/average.json locales/de-DE/average.help.html

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/average.json locales/de-DE/average.help.html

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/average.la
