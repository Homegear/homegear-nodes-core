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
 * Copyright 2013-2019 Homegear GmbH
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

namespace Timer2 {

SunTime::SunTime() {
  _times.reserve(6);
  _times.push_back(SunTimeAngle(-0.833, SunTimeTypes::sunrise, SunTimeTypes::sunset));
  _times.push_back(SunTimeAngle(-0.3, SunTimeTypes::sunriseEnd, SunTimeTypes::sunsetStart));
  _times.push_back(SunTimeAngle(-6.0, SunTimeTypes::dawn, SunTimeTypes::dusk));
  _times.push_back(SunTimeAngle(-12.0, SunTimeTypes::nauticalDawn, SunTimeTypes::nauticalDusk));
  _times.push_back(SunTimeAngle(-18.0, SunTimeTypes::nightEnd, SunTimeTypes::night));
  _times.push_back(SunTimeAngle(6.0, SunTimeTypes::goldenHourEnd, SunTimeTypes::goldenHour));
}

SunTime::~SunTime() {

}

int64_t SunTime::getLocalTime(int64_t utcTime) {
  std::time_t t;
  if (utcTime > 0) {
    t = std::time_t(utcTime / 1000);
  } else {
    const auto timePoint = std::chrono::system_clock::now();
    t = std::chrono::system_clock::to_time_t(timePoint);
  }
  std::tm localTime{};
  localtime_r(&t, &localTime);
  int64_t millisecondOffset = localTime.tm_gmtoff * 1000;
  if (utcTime > 0) return utcTime + millisecondOffset;
  else return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + millisecondOffset;
}

int64_t SunTime::getUtcTime(int64_t localTime) {
  if (localTime == 0) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  }
  const auto timePoint = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(timePoint);
  std::tm localTimeStruct{};
  localtime_r(&t, &localTimeStruct);
  int64_t millisecondOffset = localTimeStruct.tm_gmtoff * 1000;
  return localTime - millisecondOffset;
}

void SunTime::getTimeStruct(std::tm &timeStruct, int64_t utcTime) {
  std::time_t t;
  if (utcTime > 0) {
    t = std::time_t(utcTime / 1000);
  } else {
    const auto timePoint = std::chrono::system_clock::now();
    t = std::chrono::system_clock::to_time_t(timePoint);
  }

  localtime_r(&t, &timeStruct);
}

//{{{ date/time constants and conversions
long double SunTime::toJulian(long double date) {
  return date / _dayMs - 0.5 + _J1970;
}

long double SunTime::fromJulian(long double j) {
  return (j + 0.5 - _J1970) * _dayMs;
}

long double SunTime::toDays(long double date) {
  return toJulian(date) - _J2000;
}
//}}}

//{{{ general calculations for position
long double SunTime::rightAscension(long double l, long double b) {
  return std::atan2(std::sin(l) * std::cos(_e) - std::tan(b) * std::sin(_e), std::cos(l));
}

long double SunTime::declination(long double l, long double b) {
  return std::asin(std::sin(b) * std::cos(_e) + std::cos(b) * std::sin(_e) * std::sin(l));
}

long double SunTime::azimuth(long double H, long double phi, long double dec) {
  return std::atan2(std::sin(H), std::cos(H) * std::sin(phi) - std::tan(dec) * std::cos(phi));
}

long double SunTime::altitude(long double H, long double phi, long double dec) {
  return std::asin(std::sin(phi) * std::sin(dec) + std::cos(phi) * std::cos(dec) * std::cos(H));
}

long double SunTime::siderealTime(long double d, long double lw) {
  return _rad * (280.16 + 360.9856235 * d) - lw;
}

long double SunTime::astroRefraction(long double h) {
  if (h < 0) // the following formula works for positive altitudes only.
    h = 0; // if h = -0.08901179 a div/0 would occur.

  // formula 16.4 of "Astronomical Algorithms" 2nd edition by Jean Meeus (Willmann-Bell, Richmond) 1998.
  // 1.02 / tan(h + 10.26 / (h + 5.10)) h in degrees, result in arc minutes -> converted to rad:
  return 0.0002967 / std::tan(h + 0.00312536 / (h + 0.08901179));
}
//}}}

//{{{ general sun calculations
long double SunTime::solarMeanAnomaly(long double d) {
  return _rad * (357.5291 + 0.98560028 * d);
}

long double SunTime::eclipticLongitude(long double M) {
  long double C = _rad * (1.9148 * std::sin(M) + 0.02 * std::sin(2 * M) + 0.0003 * std::sin(3 * M)); // equation of center
  long double P = _rad * 102.9372; // perihelion of the Earth

  return M + C + P + _pi;
}

SunTime::SunCoords SunTime::sunCoords(long double d) {
  long double M = solarMeanAnomaly(d);
  long double L = eclipticLongitude(M);

  SunCoords sunCoords;
  sunCoords.dec = declination(L, 0);
  sunCoords.ra = rightAscension(L, 0);

  return sunCoords;
}
//}}}

// calculates sun position for a given date and latitude/longitude
SunTime::SunPosition SunTime::getPosition(int64_t date, long double lat, long double lng) {
  long double lw = _rad * -lng;
  long double phi = _rad * lat;
  long double d = toDays(date);

  SunCoords c = sunCoords(d);
  long double H = siderealTime(d, lw) - c.ra;

  SunPosition sunPosition;
  sunPosition.azimuth = azimuth(H, phi, c.dec) * 180.0 / _pi + 180.0;
  sunPosition.altitude = altitude(H, phi, c.dec) * 180.0 / _pi;

  return sunPosition;
}

//{{{ calculations for sun times
long double SunTime::julianCycle(long double d, long double lw) {
  return std::round(d - _J0 - lw / (2 * _pi));
}

long double SunTime::approxTransit(long double Ht, long double lw, long double n) {
  return _J0 + (Ht + lw) / (2 * _pi) + n;
}

long double SunTime::solarTransitJ(long double ds, long double M, long double L) {
  return _J2000 + ds + 0.0053 * std::sin(M) - 0.0069 * std::sin(2 * L);
}

long double SunTime::hourAngle(long double h, long double phi, long double d) {
  return std::acos((std::sin(h) - std::sin(phi) * std::sin(d)) / (std::cos(phi) * std::cos(d)));
}
//}}}

// returns set time for the given sun altitude
long double SunTime::getSetJ(long double h, long double lw, long double phi, long double dec, long double n, long double M, long double L) {
  long double w = hourAngle(h, phi, dec);
  long double a = approxTransit(w, lw, n);
  return solarTransitJ(a, M, L);
}

// calculates sun times for a given date and latitude/longitude
SunTime::SunTimes SunTime::getTimesUtc(int64_t date, long double lat, long double lng) {
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

  for (auto &time : _times) {
    long double Jset = getSetJ(time.angle * _rad, lw, phi, dec, n, M, L);
    long double Jrise = Jnoon - (Jset - Jnoon);

    long double morning = fromJulian(Jrise);
    long double evening = fromJulian(Jset);

    result.times.emplace(time.morningName, morning == 0.0 || std::isnan(morning) ? 0.0 : morning);
    result.times.emplace(time.eveningName, evening == 0.0 || std::isnan(evening) ? 0.0 : evening);
  }

  return result;
}

SunTime::SunTimes SunTime::getTimesLocal(int64_t date, long double lat, long double lng) {
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

  for (auto &time : _times) {
    long double Jset = getSetJ(time.angle * _rad, lw, phi, dec, n, M, L);
    long double Jrise = Jnoon - (Jset - Jnoon);

    long double morning = fromJulian(Jrise);
    long double evening = fromJulian(Jset);

    result.times.emplace(time.morningName, morning == 0.0 || std::isnan(morning) ? 0.0 : getLocalTime(morning));
    result.times.emplace(time.eveningName, evening == 0.0 || std::isnan(evening) ? 0.0 : getLocalTime(evening));
  }

  return result;
}

// moon calculations, based on http://aa.quae.nl/en/reken/hemelpositie.html formulas
SunTime::MoonCoords SunTime::moonCoords(long double d) { // geocentric ecliptic coordinates of the moon

  long double L = _rad * (218.316 + 13.176396 * d); // ecliptic longitude
  long double M = _rad * (134.963 + 13.064993 * d); // mean anomaly
  long double F = _rad * (93.272 + 13.229350 * d);  // mean distance

  long double l = L + _rad * 6.289 * std::sin(M); // longitude
  long double b = _rad * 5.128 * std::sin(F);     // latitude
  long double dt = 385001 - 20905 * std::cos(M);  // distance to the moon in km

  MoonCoords moonCoords;
  moonCoords.ra = rightAscension(l, b);
  moonCoords.dec = declination(l, b);
  moonCoords.dist = dt;

  return moonCoords;
}

int64_t SunTime::hoursLater(int64_t date, int32_t h) {
  return date + (h * _dayMs) / 24;
}

SunTime::MoonPosition SunTime::getMoonPosition(int64_t date, long double lat, long double lng) {

  long double lw = _rad * -lng;
  long double phi = _rad * lat;
  long double d = toDays(date);

  MoonCoords c = moonCoords(d);
  long double H = siderealTime(d, lw) - c.ra;
  long double h = altitude(H, phi, c.dec);
  // formula 14.1 of "Astronomical Algorithms" 2nd edition by Jean Meeus (Willmann-Bell, Richmond) 1998.
  long double pa = std::atan2(std::sin(H), std::tan(phi) * std::cos(c.dec) - std::sin(c.dec) * std::cos(H));

  h = h + astroRefraction(h); // altitude correction for refraction

  MoonPosition moonPosition;
  moonPosition.azimuth = azimuth(H, phi, c.dec) * 180.0 / _pi;
  moonPosition.altitude = h * 180.0 / _pi;
  moonPosition.distance = c.dist;
  moonPosition.parallacticAngle = pa * 180.0 / _pi;
  return moonPosition;
};

// calculations for illumination parameters of the moon,
// based on http://idlastro.gsfc.nasa.gov/ftp/pro/astro/mphase.pro formulas and
// Chapter 48 of "Astronomical Algorithms" 2nd edition by Jean Meeus (Willmann-Bell, Richmond) 1998.
SunTime::MoonIllumination SunTime::getMoonIllumination(int64_t date) {
  long double d = toDays(date);
  SunCoords s = sunCoords(d);
  MoonCoords m = moonCoords(d);

  long double sdist = 149598000; // distance from Earth to Sun in km

  long double phi = std::acos(std::sin(s.dec) * std::sin(m.dec) + std::cos(s.dec) * std::cos(m.dec) * std::cos(s.ra - m.ra));
  long double inc = std::atan2(sdist * std::sin(phi), m.dist - sdist * std::cos(phi));
  long double angle = std::atan2(std::cos(s.dec) * std::sin(s.ra - m.ra), std::sin(s.dec) * std::cos(m.dec) - std::cos(s.dec) * std::sin(m.dec) * std::cos(s.ra - m.ra));

  MoonIllumination moonIllumination;
  moonIllumination.fraction = (1 + cos(inc)) / 2;
  moonIllumination.phase = 0.5 + 0.5 * inc * (angle < 0 ? -1 : 1) / _pi;
  moonIllumination.angle = angle;
  return moonIllumination;
};

// calculations for moon rise/set times are based on http://www.stargazing.net/kepler/moonrise.html article
SunTime::MoonTimes SunTime::getMoonTimesLocal(int64_t date, long double lat, long double lng) {
  date = (date / _dayMs) * _dayMs;

  long double hc = 0.133 * _rad;
  long double h0 = getMoonPosition(date, lat, lng).altitude - hc;
  long double h1 = 0;
  long double h2 = 0;
  long double rise = 0;
  long double set = 0;
  long double a = 0;
  long double b = 0;
  long double xe = 0;
  long double ye = 0;
  long double d = 0;
  long double roots = 0;
  long double x1 = 0;
  long double x2 = 0;
  long double dx = 0;

  // go in 2-hour chunks, each time seeing if a 3-point quadratic curve crosses zero (which means rise or set)
  for (int32_t i = 1; i <= 24; i += 2) {
    h1 = getMoonPosition(hoursLater(date, i), lat, lng).altitude - hc;
    h2 = getMoonPosition(hoursLater(date, i + 1), lat, lng).altitude - hc;

    a = (h0 + h2) / 2 - h1;
    b = (h2 - h0) / 2;
    xe = -b / (2 * a);
    ye = (a * xe + b) * xe + h1;
    d = b * b - 4 * a * h1;
    roots = 0;

    if (d >= 0) {
      dx = std::sqrt(d) / (std::abs(a) * 2);
      x1 = xe - dx;
      x2 = xe + dx;
      if (std::abs(x1) <= 1) roots++;
      if (std::abs(x2) <= 1) roots++;
      if (x1 < -1) x1 = x2;
    }

    if (roots == 1) {
      if (h0 < 0) rise = i + x1;
      else set = i + x1;

    } else if (roots == 2) {
      rise = i + (ye < 0 ? x2 : x1);
      set = i + (ye < 0 ? x1 : x2);
    }

    if (rise && set) break;

    h0 = h2;
  }

  MoonTimes result;

  if (rise) result.rise = getLocalTime(hoursLater(date, rise));
  if (set) result.set = getLocalTime(hoursLater(date, set));

  if (!rise && !set) {
    result.alwaysUp = ye > 0;
    result.alwaysDown = ye <= 0;
  }

  return result;
};

SunTime::MoonTimes SunTime::getMoonTimesUtc(int64_t date, long double lat, long double lng) {
  date = (date / _dayMs) * _dayMs;

  long double hc = 0.133 * _rad;
  long double h0 = getMoonPosition(date, lat, lng).altitude - hc;
  long double h1 = 0;
  long double h2 = 0;
  long double rise = 0;
  long double set = 0;
  long double a = 0;
  long double b = 0;
  long double xe = 0;
  long double ye = 0;
  long double d = 0;
  long double roots = 0;
  long double x1 = 0;
  long double x2 = 0;
  long double dx = 0;

  // go in 2-hour chunks, each time seeing if a 3-point quadratic curve crosses zero (which means rise or set)
  for (int32_t i = 1; i <= 24; i += 2) {
    h1 = getMoonPosition(hoursLater(date, i), lat, lng).altitude - hc;
    h2 = getMoonPosition(hoursLater(date, i + 1), lat, lng).altitude - hc;

    a = (h0 + h2) / 2 - h1;
    b = (h2 - h0) / 2;
    xe = -b / (2 * a);
    ye = (a * xe + b) * xe + h1;
    d = b * b - 4 * a * h1;
    roots = 0;

    if (d >= 0) {
      dx = std::sqrt(d) / (std::abs(a) * 2);
      x1 = xe - dx;
      x2 = xe + dx;
      if (std::abs(x1) <= 1) roots++;
      if (std::abs(x2) <= 1) roots++;
      if (x1 < -1) x1 = x2;
    }

    if (roots == 1) {
      if (h0 < 0) rise = i + x1;
      else set = i + x1;

    } else if (roots == 2) {
      rise = i + (ye < 0 ? x2 : x1);
      set = i + (ye < 0 ? x1 : x2);
    }

    if (rise && set) break;

    h0 = h2;
  }

  MoonTimes result;

  if (rise) result.rise = hoursLater(date, rise);
  if (set) result.set = hoursLater(date, set);

  if (!rise && !set) {
    result.alwaysUp = ye > 0;
    result.alwaysDown = ye <= 0;
  }

  return result;
};

}
