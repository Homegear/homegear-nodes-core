AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/flows/nodes/timers

lib_LTLIBRARIES = timer.la delay.la

timer_la_SOURCES = timer/Factory.cpp timer/MyNode.cpp
timer_la_LDFLAGS =-module -avoid-version -shared

delay_la_SOURCES = delay/Factory.cpp delay/MyNode.cpp
delay_la_LDFLAGS =-module -avoid-version -shared

timers_ladir = $(libdir)
timers_la_DATA = timer/timer.hni delay/delay.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = timer/locales/en-US/timer delay/locales/en-US/delay

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/timer.la
	rm -f $(DESTDIR)$(libdir)/delay.la
