AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/basic-logic

lib_LTLIBRARIES = and.la not.la or.la switch.la

and_la_SOURCES = and/Factory.cpp and/MyNode.cpp
and_la_LDFLAGS =-module -avoid-version -shared

not_la_SOURCES = not/Factory.cpp not/MyNode.cpp
not_la_LDFLAGS =-module -avoid-version -shared

or_la_SOURCES = or/Factory.cpp or/MyNode.cpp
or_la_LDFLAGS =-module -avoid-version -shared

switch_la_SOURCES = switch/Factory.cpp switch/MyNode.cpp
switch_la_LDFLAGS =-module -avoid-version -shared

logic_ladir = $(libdir)
logic_la_DATA = and/and.hni not/not.hni or/or.hni switch/switch.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = and/locales/en-US/and not/locales/en-US/not or/locales/en-US/or switch/locales/en-US/switch

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/and.la
	rm -f $(DESTDIR)$(libdir)/not.la
	rm -f $(DESTDIR)$(libdir)/or.la
	rm -f $(DESTDIR)$(libdir)/switch.la
