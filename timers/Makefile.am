AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/timers

lib_LTLIBRARIES = clock.la interval.la delay.la timer.la

clock_la_SOURCES = clock/Factory.cpp clock/MyNode.cpp
clock_la_LDFLAGS =-module -avoid-version -shared

interval_la_SOURCES = interval/Factory.cpp interval/MyNode.cpp
interval_la_LDFLAGS =-module -avoid-version -shared

delay_la_SOURCES = delay/Factory.cpp delay/MyNode.cpp
delay_la_LDFLAGS =-module -avoid-version -shared

timer_la_SOURCES = timer/Factory.cpp timer/MyNode.cpp timer/SunTime.cpp
timer_la_LDFLAGS =-module -avoid-version -shared

timers_ladir = $(libdir)
timers_la_DATA = clock/clock.hni interval/interval.hni delay/delay.hni timer/timer.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = clock/locales/en-US/clock interval/locales/en-US/interval delay/locales/en-US/delay timer/locales/en-US/timer

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/clock.la
	rm -f $(DESTDIR)$(libdir)/interval.la
	rm -f $(DESTDIR)$(libdir)/delay.la
	rm -f $(DESTDIR)$(libdir)/timer.la
