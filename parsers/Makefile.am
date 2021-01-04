AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/parsers

lib_LTLIBRARIES = json.la strip.la xml.la

json_la_SOURCES = json/Factory.cpp json/MyNode.cpp
json_la_LDFLAGS =-module -avoid-version -shared
strip_la_SOURCES = strip/Factory.cpp strip/MyNode.cpp
strip_la_LDFLAGS =-module -avoid-version -shared
xml_la_SOURCES = xml/Factory.cpp xml/Xml.cpp
xml_la_LDFLAGS =-module -avoid-version -shared

parsers_ladir = $(libdir)
parsers_la_DATA = package.json json/json.hni strip/strip.hni xml/xml.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/parsers.json
locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/parsers.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/json.la $(DESTDIR)$(libdir)/strip.la $(DESTDIR)$(libdir)/xml.la
