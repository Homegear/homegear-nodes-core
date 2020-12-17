AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/tls-config
lib_LTLIBRARIES = tls-config.la
tls_config_la_SOURCES = Factory.cpp MyNode.cpp
tls_config_la_LDFLAGS =-module -avoid-version -shared
tls_config_ladir = $(libdir)
tls_config_la_DATA = package.json tls-config.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/tls-config.json
locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/tls-config.json
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/tls-config.la
