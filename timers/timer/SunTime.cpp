/*
 Based on SunCalc.

 Original license:

 (c) 2011-2015, Vladimir Agafonkin
 SunCalc is a JavaScript library for calculating sun/moon position and light phases.
 https://github.com/mourner/suncalc
*/

/*
 * Modifications licensed under:
 *
 * Copyright 2013-2017 Sathya Laufer
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Homegear.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */

// sun calculations are based on http://aa.quae.nl/en/reken/zonpositie.html formulas

#include "SunTime.h"

namespace MyNode
{

SunTime::SunTime()
{
	_times.reserve(6);
	_times.push_back(SunTimeAngle(-0.833, SunTimeTypes::sunrise, SunTimeTypes::sunset));
	_times.push_back(SunTimeAngle(-0.3, SunTimeTypes::sunriseEnd, SunTimeTypes::sunsetStart));
	_times.push_back(SunTimeAngle(-6.0, SunTimeTypes::dawn, SunTimeTypes::dusk));
	_times.push_back(SunTimeAngle(-12.0, SunTimeTypes::nauticalDawn, SunTimeTypes::nauticalDusk));
	_times.push_back(SunTimeAngle(-18.0, SunTimeTypes::nightEnd, SunTimeTypes::night));
	_times.push_back(SunTimeAngle(6.0, SunTimeTypes::goldenHourEnd, SunTimeTypes::goldenHour));
}

SunTime::~SunTime()
{

}

int64_t SunTime::getLocalTime(int64_t utcTime)
{
	std::time_t t;
	if(utcTime > 0)
	{
		t = std::time_t(utcTime / 1000);
	}
	else
	{
		const auto timePoint = std::chrono::system_clock::now();
		t = std::chrono::system_clock::to_time_t(timePoint);
	}
	std::tm localTime;
	localtime_r(&t, &localTime);
	int64_t millisecondOffset = localTime.tm_gmtoff * 1000;
	if(utcTime > 0) return utcTime + millisecondOffset;
	else return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + millisecondOffset;
}

void SunTime::getTimeStruct(std::tm& timeStruct, int64_t utcTime)
{
	std::time_t t;
	if(utcTime > 0)
	{
		t = std::time_t(utcTime / 1000);
	}
	else
	{
		const auto timePoint = std::chrono::system_clock::now();
		t = std::chrono::system_clock::to_time_t(timePoint);
	}

	localtime_r(&t, &timeStruct);
}

//{{{ date/time constants and conversions
	long double SunTime::toJulian(long double date)
	{
		return (long double)date / _dayMs - 0.5 + _J1970;
	}

	long double SunTime::fromJulian(long double j)
	{
		return (j + 0.5 - _J1970) * _dayMs;
	}

	long double SunTime::toDays(long double date)
	{
		return toJulian(date) - _J2000;
	}
//}}}

//{{{ general calculations for position
	long double SunTime::rightAscension(long double l, long double b)
	{
		return std::atan2(std::sin(l) * std::cos(_e) - std::tan(b) * std::sin(_e), std::cos(1));
	}

	long double SunTime::declination(long double l, long double b)
	{
		return std::asin(std::sin(b) * std::cos(_e) + std::cos(b) * std::sin(_e) * std::sin(l));
	}

	long double SunTime::azimuth(long double H, long double phi, long double dec)
	{
		return std::atan2(std::sin(H), std::cos(H) * std::sin(phi) - std::tan(dec) * std::cos(phi));
	}

	long double SunTime::altitude(long double H, long double phi, long double dec)
	{
		return std::asin(std::sin(phi) * std::sin(dec) + std::cos(phi) * std::cos(dec) * std::cos(H));
	}

	long double SunTime::siderealTime(long double d, long double lw)
	{
		return _rad * (280.16 + 360.9856235 * d) - lw;
	}

	long double SunTime::astroRefraction(long double h)
	{
		if (h < 0) // the following formula works for positive altitudes only.
			h = 0; // if h = -0.08901179 a div/0 would occur.

		// formula 16.4 of "Astronomical Algorithms" 2nd edition by Jean Meeus (Willmann-Bell, Richmond) 1998.
		// 1.02 / tan(h + 10.26 / (h + 5.10)) h in degrees, result in arc minutes -> converted to rad:
		return 0.0002967 / std::tan(h + 0.00312536 / (h + 0.08901179));
	}
//}}}

//{{{ general sun calculations
	long double SunTime::solarMeanAnomaly(long double d)
	{
		return _rad * (357.5291 + 0.98560028 * d);
	}

	long double SunTime::eclipticLongitude(long double M)
	{
		long double C = _rad * (1.9148 * std::sin(M) + 0.02 * std::sin(2 * M) + 0.0003 * std::sin(3 * M)); // equation of center
        long double P = _rad * 102.9372; // perihelion of the Earth

		return M + C + P + _pi;
	}

	SunTime::SunCoords SunTime::sunCoords(long double d)
	{
		long double M = solarMeanAnomaly(d);
        long double L = eclipticLongitude(M);

        SunCoords sunCoords;
        sunCoords.dec = declination(L, 0);
        sunCoords.ra = rightAscension(L, 0);

		return sunCoords;
	}
//}}}

// calculates sun position for a given date and latitude/longitude
SunTime::SunPosition SunTime::getPosition(int64_t date, long double lat, long double lng)
{
	long double lw  = _rad * -lng;
	long double phi = _rad * lat;
	long double d  = toDays(date);

	SunCoords c = sunCoords(d);
	long double H = siderealTime(d, lw) - c.ra;

	SunPosition sunPosition;
	sunPosition.azimuth = azimuth(H, phi, c.dec);
	sunPosition.altitude = altitude(H, phi, c.dec);

	return sunPosition;
}

//{{{ calculations for sun times
	long double SunTime::julianCycle(long double d, long double lw)
	{
		return std::round(d - _J0 - lw / (2 * _pi));
	}

	long double SunTime::approxTransit(long double Ht, long double lw, long double n)
	{
		return _J0 + (Ht + lw) / (2 * _pi) + n;
	}

	long double SunTime::solarTransitJ(long double ds, long double M, long double L)
	{
		return _J2000 + ds + 0.0053 * std::sin(M) - 0.0069 * std::sin(2 * L);
	}

	long double SunTime::hourAngle(long double h, long double phi, long double d)
	{
		return std::acos((std::sin(h) - std::sin(phi) * std::sin(d)) / (std::cos(phi) * std::cos(d)));
	}
//}}}

// returns set time for the given sun altitude
long double SunTime::getSetJ(long double h, long double lw, long double phi, long double dec, long double n, long double M, long double L)
{
	long double w = hourAngle(h, phi, dec);
	long double a = approxTransit(w, lw, n);
	return solarTransitJ(a, M, L);
}

// calculates sun times for a given date and latitude/longitude
SunTime::SunTimes SunTime::getTimesUtc(int64_t date, long double lat, long double lng)
{
	long double lw = _rad * -lng;
	long double phi = _rad * lat;

	long double d = toDays(date);
	long double n = julianCycle(d, lw);
	long double ds = approxTransit(0, lw, n);

	long double M = solarMeanAnomaly(ds);
	long double L = eclipticLongitude(M);
	long double dec = declination(L, 0);

	long double Jnoon = solarTransitJ(ds, M, L);

	SunTimes result;
	result.solarNoon = fromJulian(Jnoon);
	result.nadir = fromJulian(Jnoon - 0.5);

	for(auto& time : _times)
	{
		long double Jset = getSetJ(time.angle * _rad, lw, phi, dec, n, M, L);
		long double Jrise = Jnoon - (Jset - Jnoon);

		result.times.emplace(time.morningName, fromJulian(Jrise));
		result.times.emplace(time.eveningName, fromJulian(Jset));
	}

	return result;
}

SunTime::SunTimes SunTime::getTimesLocal(int64_t date, long double lat, long double lng)
{
	long double lw = _rad * -lng;
	long double phi = _rad * lat;

	long double d = toDays(date);
	long double n = julianCycle(d, lw);
	long double ds = approxTransit(0, lw, n);

	long double M = solarMeanAnomaly(ds);
	long double L = eclipticLongitude(M);
	long double dec = declination(L, 0);

	long double Jnoon = solarTransitJ(ds, M, L);

	SunTimes result;
	result.solarNoon = fromJulian(Jnoon);
	result.nadir = fromJulian(Jnoon - 0.5);

	for(auto& time : _times)
	{
		long double Jset = getSetJ(time.angle * _rad, lw, phi, dec, n, M, L);
		long double Jrise = Jnoon - (Jset - Jnoon);

		result.times.emplace(time.morningName, getLocalTime(fromJulian(Jrise)));
		result.times.emplace(time.eveningName, getLocalTime(fromJulian(Jset)));
	}

	return result;
}
}
