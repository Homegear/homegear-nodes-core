AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/tcp

lib_LTLIBRARIES = tcp-socket.la tcp-in.la tcp-out.la

tcp_socket_la_SOURCES = tcp-socket/Factory.cpp tcp-socket/MyNode.cpp
tcp_socket_la_LDFLAGS =-module -avoid-version -shared

tcp_in_la_SOURCES = tcp-in/Factory.cpp tcp-in/MyNode.cpp
tcp_in_la_LDFLAGS =-module -avoid-version -shared

tcp_out_la_SOURCES = tcp-out/Factory.cpp tcp-out/MyNode.cpp
tcp_out_la_LDFLAGS =-module -avoid-version -shared

tcp_ladir = $(libdir)
tcp_la_DATA = tcp-socket/tcp-socket.hni tcp-in/tcp-in.hni tcp-out/tcp-out.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = tcp-socket/locales/en-US/tcp-socket tcp-in/locales/en-US/tcp-in tcp-out/locales/en-US/tcp-out

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/tcp-socket.la
	rm -f $(DESTDIR)$(libdir)/tcp-in.la
	rm -f $(DESTDIR)$(libdir)/tcp-out.la
