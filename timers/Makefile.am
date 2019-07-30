AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/timers

lib_LTLIBRARIES = clock.la delay.la impulse.la interval.la off-delay.la on-delay.la rate-limiter.la timer.la slow-pwm.la sun-position.la

clock_la_SOURCES = clock/Factory.cpp clock/MyNode.cpp
clock_la_LDFLAGS =-module -avoid-version -shared

delay_la_SOURCES = delay/Factory.cpp delay/MyNode.cpp
delay_la_LDFLAGS =-module -avoid-version -shared

impulse_la_SOURCES = impulse/Factory.cpp impulse/MyNode.cpp
impulse_la_LDFLAGS =-module -avoid-version -shared

interval_la_SOURCES = interval/Factory.cpp interval/MyNode.cpp
interval_la_LDFLAGS =-module -avoid-version -shared

off_delay_la_SOURCES = off-delay/Factory.cpp off-delay/MyNode.cpp
off_delay_la_LDFLAGS =-module -avoid-version -shared

on_delay_la_SOURCES = on-delay/Factory.cpp on-delay/MyNode.cpp
on_delay_la_LDFLAGS =-module -avoid-version -shared

rate_limiter_la_SOURCES = rate-limiter/Factory.cpp rate-limiter/MyNode.cpp
rate_limiter_la_LDFLAGS =-module -avoid-version -shared

timer_la_SOURCES = timer/Factory.cpp timer/MyNode.cpp timer/SunTime.cpp
timer_la_LDFLAGS =-module -avoid-version -shared

slow_pwm_la_SOURCES = slow-pwm/Factory.cpp slow-pwm/MyNode.cpp
slow_pwm_la_LDFLAGS =-module -avoid-version -shared

sun_position_la_SOURCES = sun-position/Factory.cpp sun-position/MyNode.cpp sun-position/SunTime.cpp
sun_position_la_LDFLAGS =-module -avoid-version -shared

timers_ladir = $(libdir)
timers_la_DATA = clock/clock.hni delay/delay.hni impulse/impulse.hni interval/interval.hni off-delay/off-delay.hni on-delay/on-delay.hni rate-limiter/rate-limiter.hni timer/timer.hni slow-pwm/slow-pwm.hni sun-position/sun-position.hni
locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = clock/locales/en-US/clock delay/locales/en-US/delay impulse/locales/en-US/impulse interval/locales/en-US/interval off-delay/locales/en-US/off-delay on-delay/locales/en-US/on-delay rate-limiter/locales/en-US/rate-limiter timer/locales/en-US/timer slow-pwm/locales/en-US/slow-pwm sun-position/locales/en-US/sun-position

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/clock.la
	rm -f $(DESTDIR)$(libdir)/delay.la
	rm -f $(DESTDIR)$(libdir)/impulse.la
	rm -f $(DESTDIR)$(libdir)/interval.la
	rm -f $(DESTDIR)$(libdir)/off-delay.la
	rm -f $(DESTDIR)$(libdir)/on-delay.la
	rm -f $(DESTDIR)$(libdir)/rate-limiter.la
	rm -f $(DESTDIR)$(libdir)/timer.la
	rm -f $(DESTDIR)$(libdir)/slow-pwm.la
	rm -f $(DESTDIR)$(libdir)/sun-position.la
