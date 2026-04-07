AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/http

lib_LTLIBRARIES = http-server.la http-in.la http-response.la http-request.la

http_server_la_SOURCES = http-server/Factory.cpp http-server/MyNode.cpp
http_server_la_LDFLAGS =-module -avoid-version -shared

http_in_la_SOURCES = http-in/Factory.cpp http-in/MyNode.cpp
http_in_la_LDFLAGS =-module -avoid-version -shared

http_response_la_SOURCES = http-response/Factory.cpp http-response/MyNode.cpp
http_response_la_LDFLAGS =-module -avoid-version -shared

http_request_la_SOURCES = http-request/Factory.cpp http-request/MyNode.cpp
http_request_la_LDFLAGS =-module -avoid-version -shared

http_ladir = $(libdir)
http_la_DATA = package.json http-server/http-server.hni http-in/http-in.hni http-response/http-response.hni http-request/http-request.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/http.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/http.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/http.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/http.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/http-server.la
	rm -f $(DESTDIR)$(libdir)/http-in.la
	rm -f $(DESTDIR)$(libdir)/http-response.la
	rm -f $(DESTDIR)$(libdir)/http-request.la
