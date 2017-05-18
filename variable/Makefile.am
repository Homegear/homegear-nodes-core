AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bstatic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/variable
lib_LTLIBRARIES = variable.la
variable_la_SOURCES = Factory.cpp MyNode.cpp
variable_la_LDFLAGS =-module -avoid-version -shared
variable_ladir = $(libdir)
variable_la_DATA = variable.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/variable
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/variable.la
