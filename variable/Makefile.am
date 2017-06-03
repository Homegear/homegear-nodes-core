AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/flows/nodes/variable

lib_LTLIBRARIES = constant.la variable-in.la variable-out.la

constant_la_SOURCES = constant/Factory.cpp constant/MyNode.cpp
constant_la_LDFLAGS =-module -avoid-version -shared

variable_in_la_SOURCES = variable-in/Factory.cpp variable-in/MyNode.cpp
variable_in_la_LDFLAGS =-module -avoid-version -shared

variable_out_la_SOURCES = variable-out/Factory.cpp variable-out/MyNode.cpp
variable_out_la_LDFLAGS =-module -avoid-version -shared

variable_ladir = $(libdir)
variable_la_DATA = constant/constant.hni variable-in/variable-in.hni variable-in/variable-in.png variable-out/variable-out.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = constant/locales/en-US/constant variable-in/locales/en-US/variable-in variable-out/locales/en-US/variable-out

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/constant.la
	rm -f $(DESTDIR)$(libdir)/variable-in.la
	rm -f $(DESTDIR)$(libdir)/variable-out.la
