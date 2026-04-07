AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/storage

lib_LTLIBRARIES = file.la tail.la watch.la

file_la_SOURCES = file/Factory.cpp file/MyNode.cpp
file_la_LDFLAGS =-module -avoid-version -shared

tail_la_SOURCES = tail/Factory.cpp tail/MyNode.cpp
tail_la_LDFLAGS =-module -avoid-version -shared

watch_la_SOURCES = watch/Factory.cpp watch/MyNode.cpp
watch_la_LDFLAGS =-module -avoid-version -shared

file_ladir = $(libdir)
file_la_DATA = package.json file/file.hni tail/tail.hni watch/watch.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/storage.json locales/en-US/tail.help.html locales/en-US/watch.help.html

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/storage.json locales/en-US/tail.help.html locales/en-US/watch.help.html

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/storage.json locales/de-DE/tail.help.html locales/de-DE/watch.help.html

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/storage.json locales/de-DE/tail.help.html locales/de-DE/watch.help.html

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/file.la
	rm -f $(DESTDIR)$(libdir)/tail.la
	rm -f $(DESTDIR)$(libdir)/watch.la
