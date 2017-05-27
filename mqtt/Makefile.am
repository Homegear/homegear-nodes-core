AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/flows/nodes/mqtt

lib_LTLIBRARIES = mqtt-broker.la mqtt-in.la

mqtt_broker_la_SOURCES = mqtt-broker/Factory.cpp mqtt-broker/Mqtt.cpp mqtt-broker/MyNode.cpp
mqtt_broker_la_LDFLAGS =-module -avoid-version -shared

mqtt_in_la_SOURCES = mqtt-in/Factory.cpp mqtt-in/MyNode.cpp
mqtt_in_la_LDFLAGS =-module -avoid-version -shared

mqtt_ladir = $(libdir)
mqtt_la_DATA = mqtt-broker/mqtt-broker.hni mqtt-in/mqtt-in.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = mqtt-broker/locales/en-US/mqtt-broker mqtt-in/locales/en-US/mqtt-in

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/mqtt-broker.la
	rm -f $(DESTDIR)$(libdir)/mqtt-in.la
