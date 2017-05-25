AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/mqtt-in
lib_LTLIBRARIES = mqtt-in.la
mqtt_in_la_SOURCES = Factory.cpp MyNode.cpp
mqtt_in_la_LDFLAGS =-module -avoid-version -shared
mqtt_in_ladir = $(libdir)
mqtt_in_la_DATA = mqtt-in.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/mqtt-in
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/mqtt-in.la
