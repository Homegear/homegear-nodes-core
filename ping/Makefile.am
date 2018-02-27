AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/ping
lib_LTLIBRARIES = ping.la
ping_la_SOURCES = Factory.cpp MyNode.cpp
ping_la_LDFLAGS =-module -avoid-version -shared
ping_ladir = $(libdir)
ping_la_DATA = ping.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/ping
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/ping.la
