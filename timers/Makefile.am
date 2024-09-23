AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++17 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic -lhomegear-node

libdir = $(localstatedir)/lib/homegear/node-blue/nodes/timers

lib_LTLIBRARIES = clock.la delay.la impulse.la interval.la off-delay.la on-delay.la ramp-to.la rate-limiter.la timer.la timer2.la slow-pwm.la sun-position.la weekly-program.la

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

ramp_to_la_SOURCES = ramp-to/Factory.cpp ramp-to/RampTo.cpp
ramp_to_la_LDFLAGS =-module -avoid-version -shared

rate_limiter_la_SOURCES = rate-limiter/Factory.cpp rate-limiter/RateLimiter.cpp
rate_limiter_la_LDFLAGS =-module -avoid-version -shared

timer_la_SOURCES = timer/Factory.cpp timer/MyNode.cpp timer/SunTime.cpp
timer_la_LDFLAGS =-module -avoid-version -shared

timer2_la_SOURCES = timer2/Factory.cpp timer2/MyNode.cpp timer2/SunTime.cpp
timer2_la_LDFLAGS =-module -avoid-version -shared

slow_pwm_la_SOURCES = slow-pwm/Factory.cpp slow-pwm/MyNode.cpp
slow_pwm_la_LDFLAGS =-module -avoid-version -shared

sun_position_la_SOURCES = sun-position/Factory.cpp sun-position/MyNode.cpp sun-position/SunTime.cpp
sun_position_la_LDFLAGS =-module -avoid-version -shared

weekly_program_la_SOURCES = weekly-program/Factory.cpp weekly-program/MyNode.cpp weekly-program/SunTime.cpp
weekly_program_la_LDFLAGS =-module -avoid-version -shared

timers_ladir = $(libdir)
timers_la_DATA = package.json clock/clock.hni delay/delay.hni impulse/impulse.hni interval/interval.hni off-delay/off-delay.hni on-delay/on-delay.hni ramp-to/ramp-to.hni rate-limiter/rate-limiter.hni timer/timer.hni timer2/timer2.hni slow-pwm/slow-pwm.hni sun-position/sun-position.hni weekly-program/weekly-program.hni

locale_endir = $(libdir)/locales/en
locale_en_DATA = locales/en-US/timers.json

locale_en_usdir = $(libdir)/locales/en-US
locale_en_us_DATA = locales/en-US/timers.json

locale_dedir = $(libdir)/locales/de
locale_de_DATA = locales/de-DE/timers.json

locale_de_dedir = $(libdir)/locales/de-DE
locale_de_de_DATA = locales/de-DE/timers.json

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/clock.la
	rm -f $(DESTDIR)$(libdir)/delay.la
	rm -f $(DESTDIR)$(libdir)/impulse.la
	rm -f $(DESTDIR)$(libdir)/interval.la
	rm -f $(DESTDIR)$(libdir)/off-delay.la
	rm -f $(DESTDIR)$(libdir)/on-delay.la
	rm -f $(DESTDIR)$(libdir)/ramp-to.la
	rm -f $(DESTDIR)$(libdir)/rate-limiter.la
	rm -f $(DESTDIR)$(libdir)/timer.la
	rm -f $(DESTDIR)$(libdir)/timer2.la
	rm -f $(DESTDIR)$(libdir)/slow-pwm.la
	rm -f $(DESTDIR)$(libdir)/sun-position.la
	rm -f $(DESTDIR)$(libdir)/weekly-program.la
