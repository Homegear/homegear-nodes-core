AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/presence-light
lib_LTLIBRARIES = presence-light.la
presence_light_la_SOURCES = Factory.cpp PresenceLight.cpp
presence_light_la_LDFLAGS =-module -avoid-version -shared
presence_light_ladir = $(libdir)
presence_light_la_DATA = presence-light.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/presence-light
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/presence-light.la
