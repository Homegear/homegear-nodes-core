AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/presence-light
lib_LTLIBRARIES = presence-light.la
presence_light_la_SOURCES = Factory.cpp PresenceLight.cpp
presence_light_la_LDFLAGS =-module -avoid-version -shared
presence_light_ladir = $(libdir)
presence_light_la_DATA = package.json presence-light.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/presence-light.json locales/en-US/presence-light.help.html

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/presence-light.json locales/en-US/presence-light.help.html

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/presence-light.json locales/de-DE/presence-light.help.html

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/presence-light.json locales/de-DE/presence-light.help.html

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/presence-light.la
