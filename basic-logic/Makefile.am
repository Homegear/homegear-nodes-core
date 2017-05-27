AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/basic-logic

lib_LTLIBRARIES = greater-than.la less-than.la

greater_than_la_SOURCES = greater-than/Factory.cpp greater-than/MyNode.cpp
greater_than_la_LDFLAGS =-module -avoid-version -shared

less_than_la_SOURCES = less-than/Factory.cpp less-than/MyNode.cpp
less_than_la_LDFLAGS =-module -avoid-version -shared

logic_ladir = $(libdir)
logic_la_DATA = greater-than/greater-than.hni less-than/less-than.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = greater-than/locales/en-US/greater-than less-than/locales/en-US/less-than

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/greater-than.la
	rm -f $(DESTDIR)$(libdir)/less-than.la
