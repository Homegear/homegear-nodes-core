AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bstatic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/greater-than
lib_LTLIBRARIES = greater-than.la
greater_than_la_SOURCES = Factory.cpp MyNode.cpp
greater_than_la_LDFLAGS =-module -avoid-version -shared
greater_than_ladir = $(libdir)
greater_than_la_DATA = greater-than.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/greater-than
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/greater-than.la
