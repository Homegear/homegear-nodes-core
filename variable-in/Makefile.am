AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bstatic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/variable-in
lib_LTLIBRARIES = variable-in.la
variable_in_la_SOURCES = Factory.cpp MyNode.cpp
variable_in_la_LDFLAGS =-module -avoid-version -shared
variable_in_ladir = $(libdir)
variable_in_la_DATA = variable-in.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/variable-in
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/variable-in.la
