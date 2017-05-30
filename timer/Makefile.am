AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/timer
lib_LTLIBRARIES = timer.la
timer_la_SOURCES = Factory.cpp MyNode.cpp
timer_la_LDFLAGS =-module -avoid-version -shared
timer_ladir = $(libdir)
timer_la_DATA = timer.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/timer
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/timer.la
