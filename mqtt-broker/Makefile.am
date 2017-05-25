AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/mqtt-broker
lib_LTLIBRARIES = mqtt-broker.la
mqtt_broker_la_SOURCES = Factory.cpp MyNode.cpp
mqtt_broker_la_LDFLAGS =-module -avoid-version -shared
mqtt_broker_ladir = $(libdir)
mqtt_broker_la_DATA = mqtt-broker.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/mqtt-broker
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/mqtt-broker.la
