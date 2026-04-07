AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/python
lib_LTLIBRARIES = python.la
python_la_SOURCES = Factory.cpp Python.cpp
python_la_LDFLAGS =-module -avoid-version -shared
python_ladir = $(libdir)
python_la_DATA = package.json python.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/python.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/python.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/python.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/python.json

install-python-hook:
	rm -f $(DESTDIR)$(libdir)/python.la
