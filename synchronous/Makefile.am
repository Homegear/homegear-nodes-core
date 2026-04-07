AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/synchronous
lib_LTLIBRARIES = synchronous.la
synchronous_la_SOURCES = Factory.cpp MyNode.cpp
synchronous_la_LDFLAGS =-module -avoid-version -shared
synchronous_ladir = $(libdir)
synchronous_la_DATA = package.json synchronous.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/synchronous.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/synchronous.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/synchronous.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/synchronous.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/synchronous.la
