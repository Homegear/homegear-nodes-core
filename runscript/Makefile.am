AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/runscript
lib_LTLIBRARIES = runscript.la
runscript_la_SOURCES = Factory.cpp RunScript.cpp
runscript_la_LDFLAGS =-module -avoid-version -shared
runscript_ladir = $(libdir)
runscript_la_DATA = runscript.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/runscript
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/runscript.la
