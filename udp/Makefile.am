AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/udp

lib_LTLIBRARIES = udp-in.la udp-out.la

udp_in_la_SOURCES = udp-in/Factory.cpp udp-in/UdpIn.cpp
udp_in_la_LDFLAGS =-module -avoid-version -shared

udp_out_la_SOURCES = udp-out/Factory.cpp udp-out/UdpOut.cpp
udp_out_la_LDFLAGS =-module -avoid-version -shared

udp_ladir = $(libdir)
udp_la_DATA = udp-in/udp-in.hni udp-out/udp-out.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = udp-in/locales/en-US/udp-in udp-in/locales/en-US/udp-in.help.html udp-out/locales/en-US/udp-out udp-out/locales/en-US/udp-out.help.html
locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = udp-in/locales/de-DE/udp-in udp-in/locales/de-DE/udp-in.help.html udp-out/locales/de-DE/udp-out udp-out/locales/de-DE/udp-out.help.html

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/udp-in.la
	rm -f $(DESTDIR)$(libdir)/udp-out.la
