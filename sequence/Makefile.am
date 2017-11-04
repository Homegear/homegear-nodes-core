AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/sequence
lib_LTLIBRARIES = sequence.la
sequence_la_SOURCES = Factory.cpp MyNode.cpp
sequence_la_LDFLAGS =-module -avoid-version -shared
sequence_ladir = $(libdir)
sequence_la_DATA = sequence.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/sequence
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/sequence.la
