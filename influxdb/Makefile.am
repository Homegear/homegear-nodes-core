AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/influxdb
lib_LTLIBRARIES = influxdb.la
influxdb_la_SOURCES = Factory.cpp MyNode.cpp
influxdb_la_LDFLAGS =-module -avoid-version -shared
influxdb_ladir = $(libdir)
influxdb_la_DATA = influxdb.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/influxdb
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/influxdb.la
