AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/status
lib_LTLIBRARIES = status.la
status_la_SOURCES = Factory.cpp Status.cpp
status_la_LDFLAGS =-module -avoid-version -shared
status_ladir = $(libdir)
status_la_DATA = package.json status.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/status.json locales/en-US/status.help.html

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/status.json locales/en-US/status.help.html

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/status.la
