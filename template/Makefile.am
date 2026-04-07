AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/template
lib_LTLIBRARIES = template.la
template_la_SOURCES = Factory.cpp MyNode.cpp
template_la_LDFLAGS =-module -avoid-version -shared
template_ladir = $(libdir)
template_la_DATA = package.json template.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/template.json locales/en-US/template.help.html

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/template.json locales/en-US/template.help.html

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/template.json locales/de-DE/template.help.html

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/template.json locales/de-DE/template.help.html

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/template.la
