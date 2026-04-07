AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/mqtt

lib_LTLIBRARIES = mqtt-broker.la mqtt-in.la mqtt-out.la

mqtt_broker_la_SOURCES = mqtt-broker/Factory.cpp mqtt-broker/Mqtt.cpp mqtt-broker/MyNode.cpp
mqtt_broker_la_LDFLAGS =-module -avoid-version -shared

mqtt_in_la_SOURCES = mqtt-in/Factory.cpp mqtt-in/MyNode.cpp
mqtt_in_la_LDFLAGS =-module -avoid-version -shared

mqtt_out_la_SOURCES = mqtt-out/Factory.cpp mqtt-out/MyNode.cpp
mqtt_out_la_LDFLAGS =-module -avoid-version -shared

mqtt_ladir = $(libdir)
mqtt_la_DATA = package.json mqtt-broker/mqtt-broker.hni mqtt-broker/mqtt-broker.cred.json mqtt-in/mqtt-in.hni mqtt-out/mqtt-out.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/mqtt.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/mqtt.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/mqtt.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/mqtt.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/mqtt-broker.la
	rm -f $(DESTDIR)$(libdir)/mqtt-in.la
	rm -f $(DESTDIR)$(libdir)/mqtt-out.la
