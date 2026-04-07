AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/unit-test-helper
lib_LTLIBRARIES = unit-test-helper.la
unit_test_helper_la_SOURCES = Factory.cpp UnitTestHelper.cpp
unit_test_helper_la_LDFLAGS =-module -avoid-version -shared
unit_test_helper_ladir = $(libdir)
unit_test_helper_la_DATA = package.json unit-test-helper.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/unit-test-helper.json locales/en-US/unit-test-helper.help.html

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/unit-test-helper.json locales/en-US/unit-test-helper.help.html

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/unit-test-helper.la
