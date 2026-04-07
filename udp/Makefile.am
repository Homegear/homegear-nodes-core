AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/udp

lib_LTLIBRARIES = udp-in.la udp-out.la

udp_in_la_SOURCES = udp-in/Factory.cpp udp-in/UdpIn.cpp
udp_in_la_LDFLAGS =-module -avoid-version -shared

udp_out_la_SOURCES = udp-out/Factory.cpp udp-out/UdpOut.cpp
udp_out_la_LDFLAGS =-module -avoid-version -shared

udp_ladir = $(libdir)
udp_la_DATA = package.json udp-in/udp-in.hni udp-out/udp-out.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/udp.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/udp.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/udp.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/udp.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/udp-in.la
	rm -f $(DESTDIR)$(libdir)/udp-out.la
