AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/catch
lib_LTLIBRARIES = catch.la
catch_la_SOURCES = Factory.cpp Catch.cpp
catch_la_LDFLAGS =-module -avoid-version -shared
catch_ladir = $(libdir)
catch_la_DATA = package.json catch.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/catch.json locales/en-US/catch.help.html

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/catch.json locales/en-US/catch.help.html

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/catch.la
