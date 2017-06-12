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

#ifndef SUNCALC_H_
#define SUNCALC_H_

#include <homegear-node/HelperFunctions.h>
#include <unordered_map>
#include <vector>
#include <cmath>

namespace MyNode
{

class SunTime
{
public:
	enum class SunTimeTypes
	{
		sunrise,
		sunset,
		sunriseEnd,
		sunsetStart,
		dawn,
		dusk,
		nauticalDawn,
		nauticalDusk,
		nightEnd,
		night,
		goldenHourEnd,
		goldenHour
	};

	struct EnumClassHash
	{
		template <typename T>
		std::size_t operator()(T t) const
		{
			return static_cast<std::size_t>(t);
		}
	};

	struct SunTimeAngle
	{
		SunTimeAngle();
		SunTimeAngle(long double angle, SunTimeTypes morningName, SunTimeTypes eveningName)
		{
			this->angle = angle;
			this->morningName = morningName;
			this->eveningName = eveningName;
		}

		long double angle;
		SunTimeTypes morningName;
		SunTimeTypes eveningName;
	};

	struct SunCoords
	{
		long double dec;
		long double ra;
	};

	struct SunPosition
	{
		long double azimuth;
		long double altitude;
	};

	struct SunTimes
	{
		int64_t solarNoon;
		int64_t nadir;
		std::unordered_map<SunTimeTypes, int64_t, EnumClassHash> times;
	};

	struct MoonCoords
	{
		long double ra;
		long double dec;
		long double dist;
	};

	struct MoonPosition
	{
		long double azimuth;
		long double altitude;
		long double distance;
		long double parallacticAngle;
	};

	struct MoonIllumination
	{
		long double fraction;
		long double phase;
		long double angle;
	};

	struct MoonTimes
	{
		int64_t rise;
		int64_t set;
		bool alwaysUp = false;
		bool alwaysDown = false;
	};

	SunTime();
	virtual ~SunTime();

	int64_t getLocalTime(int64_t utcTime = 0);
	void getTimeStruct(std::tm& timeStruct, int64_t utcTime = 0);
	SunPosition getPosition(int64_t date, long double lat, long double lng);
	SunTimes getTimesLocal(int64_t date, long double lat, long double lng);
	SunTimes getTimesUtc(int64_t date, long double lat, long double lng);
	MoonPosition getMoonPosition(int64_t date, long double lat, long double lng);
	MoonIllumination getMoonIllumination(int64_t date);
	MoonTimes getMoonTimesLocal(int64_t date, long double lat, long double lng);
	MoonTimes getMoonTimesUtc(int64_t date, long double lat, long double lng);
private:
	typedef long double Angle;

	std::vector<SunTimeAngle> _times;

	static constexpr long double _dayMs = 1000 * 60 * 60 * 24;
	static constexpr long double _J1970 = 2440588;
	static constexpr long double _J2000 = 2451545;

	static constexpr long double _pi = 3.141592653589793;
	static constexpr long double _rad = _pi / 180.0;

	static constexpr long double _e = _rad * 23.4397;

	static constexpr long double _J0 = 0.0009;

	long double toJulian(long double date);
	long double fromJulian(long double j);
	long double toDays(long double date);

	long double rightAscension(long double l, long double b);
	long double declination(long double l, long double b);
	long double azimuth(long double H, long double phi, long double dec);
	long double altitude(long double H, long double phi, long double dec);
	long double siderealTime(long double d, long double lw);
	long double astroRefraction(long double h);

	long double solarMeanAnomaly(long double d);
	long double eclipticLongitude(long double M);
	SunCoords sunCoords(long double d);

	long double julianCycle(long double d, long double lw);
	long double approxTransit(long double Ht, long double lw, long double n);
	long double solarTransitJ(long double ds, long double M, long double L);
	long double hourAngle(long double h, long double phi, long double d);

	long double getSetJ(long double h, long double lw, long double phi, long double dec, long double n, long double M, long double L);

	MoonCoords moonCoords(long double d);
	int64_t hoursLater(int64_t date, int32_t h);
};

}
#endif
