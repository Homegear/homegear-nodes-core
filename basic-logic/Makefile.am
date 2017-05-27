AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/basic-logic

lib_LTLIBRARIES = equal.la greater-or-equal.la greater-than.la less-or-equal.la less-than.la

equal_la_SOURCES = equal/Factory.cpp equal/MyNode.cpp
equal_la_LDFLAGS =-module -avoid-version -shared

greater_or_equal_la_SOURCES = greater-or-equal/Factory.cpp greater-or-equal/MyNode.cpp
greater_or_equal_la_LDFLAGS =-module -avoid-version -shared

greater_than_la_SOURCES = greater-than/Factory.cpp greater-than/MyNode.cpp
greater_than_la_LDFLAGS =-module -avoid-version -shared

less_or_equal_la_SOURCES = less-or-equal/Factory.cpp less-or-equal/MyNode.cpp
less_or_equal_la_LDFLAGS =-module -avoid-version -shared

less_than_la_SOURCES = less-than/Factory.cpp less-than/MyNode.cpp
less_than_la_LDFLAGS =-module -avoid-version -shared

logic_ladir = $(libdir)
logic_la_DATA = equal/equal.hni greater-or-equal/greater-or-equal.hni greater-than/greater-than.hni less-or-equal/less-or-equal.hni less-than/less-than.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = equal/locales/en-US/equal greater-or-equal/locales/en-US/greater-or-equal greater-than/locales/en-US/greater-than less-or-equal/locales/en-US/less-or-equal less-than/locales/en-US/less-than

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/equal.la
	rm -f $(DESTDIR)$(libdir)/greater-or-equal.la
	rm -f $(DESTDIR)$(libdir)/greater-than.la
	rm -f $(DESTDIR)$(libdir)/less-or-equal.la
	rm -f $(DESTDIR)$(libdir)/less-than.la
