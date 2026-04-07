AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/tls-server-config
lib_LTLIBRARIES = tls-server-config.la
tls_server_config_la_SOURCES = Factory.cpp MyNode.cpp
tls_server_config_la_LDFLAGS =-module -avoid-version -shared
tls_server_config_ladir = $(libdir)
tls_server_config_la_DATA = package.json tls-server-config.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/tls-server-config.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/tls-server-config.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/tls-server-config.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/tls-server-config.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/tls-server-config.la
