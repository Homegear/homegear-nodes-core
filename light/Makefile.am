AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/light
lib_LTLIBRARIES = light.la
light_la_SOURCES = Factory.cpp MyNode.cpp
light_la_LDFLAGS =-module -avoid-version -shared
light_ladir = $(libdir)
light_la_DATA = light.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/light
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/light.la
