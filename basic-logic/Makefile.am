AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/basic-logic

lib_LTLIBRARIES = and.la change.la fallingedge.la not.la or.la risingedge.la srflipflop.la switch.la variable-switch.la

and_la_SOURCES = and/Factory.cpp and/MyNode.cpp
and_la_LDFLAGS =-module -avoid-version -shared

change_la_SOURCES = change/Factory.cpp change/Change.cpp
change_la_LDFLAGS =-module -avoid-version -shared

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

switch_la_SOURCES = switch/Factory.cpp switch/Switch.cpp
switch_la_LDFLAGS =-module -avoid-version -shared

variable_switch_la_SOURCES = variable-switch/Factory.cpp variable-switch/MyNode.cpp
variable_switch_la_LDFLAGS =-module -avoid-version -shared

logic_ladir = $(libdir)
logic_la_DATA = package.json and/and.hni change/change.hni fallingedge/fallingedge.hni not/not.hni or/or.hni risingedge/risingedge.hni srflipflop/srflipflop.hni switch/switch.hni variable-switch/variable-switch.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/basic-logic.json locales/en-US/and.help.html

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/basic-logic.json locales/en-US/and.help.html

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/basic-logic.json locales/de-DE/and.help.html

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/basic-logic.json locales/de-DE/and.help.html

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/and.la
	rm -f $(DESTDIR)$(libdir)/change.la
	rm -f $(DESTDIR)$(libdir)/fallingedge.la
	rm -f $(DESTDIR)$(libdir)/not.la
	rm -f $(DESTDIR)$(libdir)/or.la
	rm -f $(DESTDIR)$(libdir)/risingedge.la
	rm -f $(DESTDIR)$(libdir)/srflipflop.la
	rm -f $(DESTDIR)$(libdir)/switch.la
	rm -f $(DESTDIR)$(libdir)/variable-switch.la
