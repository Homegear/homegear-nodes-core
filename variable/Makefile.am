AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/variable

lib_LTLIBRARIES = constant.la inject.la round-robin-shift.la toggle.la variable-in.la variable-loop-prevention-group.la variable-out.la

constant_la_SOURCES = constant/Factory.cpp constant/MyNode.cpp
constant_la_LDFLAGS =-module -avoid-version -shared

inject_la_SOURCES = inject/Factory.cpp inject/Inject.cpp
inject_la_LDFLAGS =-module -avoid-version -shared

round_robin_shift_la_SOURCES = round-robin-shift/Factory.cpp round-robin-shift/MyNode.cpp
round_robin_shift_la_LDFLAGS =-module -avoid-version -shared

toggle_la_SOURCES = toggle/Factory.cpp toggle/MyNode.cpp
toggle_la_LDFLAGS =-module -avoid-version -shared

variable_in_la_SOURCES = variable-in/Factory.cpp variable-in/VariableIn.cpp
variable_in_la_LDFLAGS =-module -avoid-version -shared

variable_out_la_SOURCES = variable-out/Factory.cpp variable-out/MyNode.cpp
variable_out_la_LDFLAGS =-module -avoid-version -shared

variable_loop_prevention_group_la_SOURCES = variable-loop-prevention-group/Factory.cpp variable-loop-prevention-group/MyNode.cpp
variable_loop_prevention_group_la_LDFLAGS =-module -avoid-version -shared

variable_ladir = $(libdir)
variable_la_DATA = package.json constant/constant.hni inject/inject.hni round-robin-shift/round-robin-shift.hni toggle/toggle.hni variable-in/variable-in.hni variable-loop-prevention-group/variable-loop-prevention-group.hni variable-out/variable-out.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/variable.json locales/en-US/inject.help.html

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/variable.json locales/en-US/inject.help.html

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/variable.json locales/de-DE/inject.help.html

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/variable.json locales/de-DE/inject.help.html

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/constant.la
	rm -f $(DESTDIR)$(libdir)/inject.la
	rm -f $(DESTDIR)$(libdir)/round-robin-shift.la
	rm -f $(DESTDIR)$(libdir)/toggle.la
	rm -f $(DESTDIR)$(libdir)/variable-in.la
	rm -f $(DESTDIR)$(libdir)/variable-loop-prevention-group.la
	rm -f $(DESTDIR)$(libdir)/variable-out.la
