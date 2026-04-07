AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -D_FORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node -lhomegear-base

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/python-wrapper
lib_LTLIBRARIES = python-wrapper.la
python_wrapper_la_SOURCES = Factory.cpp Python.cpp
python_wrapper_la_LDFLAGS =-module -avoid-version -shared
python_wrapper_ladir = $(libdir)
install-python-hook:
	rm -f $(DESTDIR)$(libdir)/python-wrapper.la
