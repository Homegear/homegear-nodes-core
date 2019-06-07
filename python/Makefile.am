AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/python
lib_LTLIBRARIES = python.la
python_la_SOURCES = Factory.cpp Python.cpp
python_la_LDFLAGS =-module -avoid-version -shared
python_ladir = $(libdir)
python_la_DATA = python.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/python
install-python-hook:
	rm -f $(DESTDIR)$(libdir)/python.la
