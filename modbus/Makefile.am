AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/modbus

lib_LTLIBRARIES = modbus-host.la modbus-in.la modbus-out.la

modbus_host_la_SOURCES = modbus-host/Factory.cpp modbus-host/Modbus.cpp modbus-host/MyNode.cpp
modbus_host_la_LDFLAGS =-module -avoid-version -shared

modbus_in_la_SOURCES = modbus-in/Factory.cpp modbus-in/MyNode.cpp
modbus_in_la_LDFLAGS =-module -avoid-version -shared

modbus_out_la_SOURCES = modbus-out/Factory.cpp modbus-out/MyNode.cpp
modbus_out_la_LDFLAGS =-module -avoid-version -shared

modbus_ladir = $(libdir)
modbus_la_DATA = package.json modbus-host/modbus-host.hni modbus-in/modbus-in.hni modbus-out/modbus-out.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/modbus.json
locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/modbus.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/modbus-host.la
	rm -f $(DESTDIR)$(libdir)/modbus-in.la
	rm -f $(DESTDIR)$(libdir)/modbus-out.la
