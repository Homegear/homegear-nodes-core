AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/tcp

lib_LTLIBRARIES = tcp-socket.la tcp-in.la tcp-out.la

tcp_socket_la_SOURCES = tcp-socket/Factory.cpp tcp-socket/TcpSocket.cpp
tcp_socket_la_LDFLAGS =-module -avoid-version -shared

tcp_in_la_SOURCES = tcp-in/Factory.cpp tcp-in/TcpIn.cpp
tcp_in_la_LDFLAGS =-module -avoid-version -shared

tcp_out_la_SOURCES = tcp-out/Factory.cpp tcp-out/TcpOut.cpp
tcp_out_la_LDFLAGS =-module -avoid-version -shared

tcp_ladir = $(libdir)
tcp_la_DATA = package.json tcp-socket/tcp-socket.hni tcp-in/tcp-in.hni tcp-out/tcp-out.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/tcp.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/tcp.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/tcp-socket.la
	rm -f $(DESTDIR)$(libdir)/tcp-in.la
	rm -f $(DESTDIR)$(libdir)/tcp-out.la
