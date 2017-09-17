AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/flows/nodes/http

lib_LTLIBRARIES = http-server.la http-in.la http-response.la

http_server_la_SOURCES = http-server/Factory.cpp http-server/MyNode.cpp
http_server_la_LDFLAGS =-module -avoid-version -shared

http_in_la_SOURCES = http-in/Factory.cpp http-in/MyNode.cpp
http_in_la_LDFLAGS =-module -avoid-version -shared

http_response_la_SOURCES = http-response/Factory.cpp http-response/MyNode.cpp
http_response_la_LDFLAGS =-module -avoid-version -shared

http_ladir = $(libdir)
http_la_DATA = http-server/http-server.hni http-in/http-in.hni http-response/http-response.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = http-server/locales/en-US/http-server http-in/locales/en-US/http-in http-response/locales/en-US/http-response

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/http-server.la
	rm -f $(DESTDIR)$(libdir)/http-in.la
	rm -f $(DESTDIR)$(libdir)/http-response.la
