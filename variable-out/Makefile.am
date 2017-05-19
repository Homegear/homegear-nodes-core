AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bstatic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/variable-out
lib_LTLIBRARIES = variable-out.la
variable_out_la_SOURCES = Factory.cpp MyNode.cpp
variable_out_la_LDFLAGS =-module -avoid-version -shared
variable_out_ladir = $(libdir)
variable_out_la_DATA = variable-out.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/variable-out
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/variable-out.la
