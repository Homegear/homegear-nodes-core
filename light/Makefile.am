AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/light
lib_LTLIBRARIES = light.la
light_la_SOURCES = Factory.cpp Light.cpp
light_la_LDFLAGS =-module -avoid-version -shared
light_ladir = $(libdir)
light_la_DATA = package.json light.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/light.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/light.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/light.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/light.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/light.la
