AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/basic-logic

lib_LTLIBRARIES = and.la fallingedge.la not.la or.la risingedge.la srflipflop.la switch.la variable-switch.la

and_la_SOURCES = and/Factory.cpp and/MyNode.cpp
and_la_LDFLAGS =-module -avoid-version -shared

fallingedge_la_SOURCES = fallingedge/Factory.cpp fallingedge/MyNode.cpp
fallingedge_la_LDFLAGS =-module -avoid-version -shared

not_la_SOURCES = not/Factory.cpp not/MyNode.cpp
not_la_LDFLAGS =-module -avoid-version -shared

or_la_SOURCES = or/Factory.cpp or/MyNode.cpp
or_la_LDFLAGS =-module -avoid-version -shared

risingedge_la_SOURCES = risingedge/Factory.cpp risingedge/MyNode.cpp
risingedge_la_LDFLAGS =-module -avoid-version -shared

srflipflop_la_SOURCES = srflipflop/Factory.cpp srflipflop/MyNode.cpp
srflipflop_la_LDFLAGS =-module -avoid-version -shared

switch_la_SOURCES = switch/Factory.cpp switch/MyNode.cpp
switch_la_LDFLAGS =-module -avoid-version -shared

variable_switch_la_SOURCES = variable-switch/Factory.cpp variable-switch/MyNode.cpp
variable_switch_la_LDFLAGS =-module -avoid-version -shared

logic_ladir = $(libdir)
logic_la_DATA = and/and.hni fallingedge/fallingedge.hni not/not.hni or/or.hni risingedge/risingedge.hni srflipflop/srflipflop.hni switch/switch.hni variable-switch/variable-switch.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = and/locales/en-US/and fallingedge/locales/en-US/fallingedge not/locales/en-US/not or/locales/en-US/or risingedge/locales/en-US/risingedge srflipflop/locales/en-US/srflipflop switch/locales/en-US/switch variable-switch/locales/en-US/variable-switch

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/and.la
	rm -f $(DESTDIR)$(libdir)/fallingedge.la
	rm -f $(DESTDIR)$(libdir)/not.la
	rm -f $(DESTDIR)$(libdir)/or.la
	rm -f $(DESTDIR)$(libdir)/risingedge.la
	rm -f $(DESTDIR)$(libdir)/srflipflop.la
	rm -f $(DESTDIR)$(libdir)/switch.la
	rm -f $(DESTDIR)$(libdir)/variable-switch.la
