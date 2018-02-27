AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/link

lib_LTLIBRARIES = link-in.la link-out.la

link_in_la_SOURCES = link-in/Factory.cpp link-in/MyNode.cpp
link_in_la_LDFLAGS =-module -avoid-version -shared

link_out_la_SOURCES = link-out/Factory.cpp link-out/MyNode.cpp
link_out_la_LDFLAGS =-module -avoid-version -shared

link_ladir = $(libdir)
link_la_DATA = link-in/link-in.hni link-out/link-out.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = link-in/locales/en-US/link-in link-out/locales/en-US/link-out

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/link-in.la
	rm -f $(DESTDIR)$(libdir)/link-out.la
