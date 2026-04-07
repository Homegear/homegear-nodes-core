AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/link

lib_LTLIBRARIES = link-in.la link-out.la

link_in_la_SOURCES = link-in/Factory.cpp link-in/LinkIn.cpp
link_in_la_LDFLAGS =-module -avoid-version -shared

link_out_la_SOURCES = link-out/Factory.cpp link-out/LinkOut.cpp
link_out_la_LDFLAGS =-module -avoid-version -shared

link_ladir = $(libdir)
link_la_DATA = package.json link.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/link.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/link.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/link.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/link.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/link-in.la
	rm -f $(DESTDIR)$(libdir)/link-out.la
