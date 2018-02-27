AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/passthrough
lib_LTLIBRARIES = passthrough.la
passthrough_la_SOURCES = Factory.cpp MyNode.cpp
passthrough_la_LDFLAGS =-module -avoid-version -shared
passthrough_ladir = $(libdir)
passthrough_la_DATA = passthrough.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/passthrough
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/passthrough.la
