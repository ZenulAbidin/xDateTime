// xDateTime is Copyright (c) 2021-2023 Ali Sherief.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License. 

// xDateTime: Powerful date and time classes using only C++11 chrono.
#ifndef X_DATETIME_H
#define X_DATETIME_H

#if !defined(__cplusplus) || __cplusplus < 201103L
#error "xDateTime requires C++11 or later."
#endif

// Limitations:
// - Currently only supports Gregorian calendar.
// - We do not support Ethiopian or Chinese alternate digits due to complex addition rules.
// - We also do not support Japanese eras as they are unknown ahead of time.
// All of these will be addressed in a later revision.


// C and POSIX locales are already defined by default.
// You should not define them yourself.
#define X_DATETIME_WITH_LOCALE_C
#define X_DATETIME_WITH_LOCALE_POSIX

#ifdef X_DATETIME_WITH_BOOST
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#endif /* X_DATETIME_WITH_BOOST */

#ifdef X_DATETIME_WITH_ABSEIL
#include "absl/time/time.h"
#include "absl/time/clock.h"
#include "absl/time/civil_time.h"
#endif /* X_DATETIME_WITH_ABSEIL */

#ifdef X_DATETIME_WITH_POCO
#include <Poco/DateTime.h>
#endif /* X_DATETIME_WITH_POCO */

#ifdef X_DATETIME_WITH_HHDATE
#include <date/date.h>
#endif /* X_DATETIME_WITH_HHDATE */

#ifdef X_DATETIME_WITH_GLIB
#include <glib.h>
#endif /* X_DATETIME_WITH_GLIB */

#include <cctype>
#include <iostream> //ios::failure thown by DateTime constructor
#include <locale>
#include <sstream>
#include <string>
#include <ctime>
#include <climits>

#include <tuple>
#include <chrono>

#include <algorithm>
#include <cctype>
#include <cstring>


// In C++20 and later, the u8 literal makes a char8_t which is incompatible
// with char strings unless you cast it.
#if __cplusplus >= 202002L
#define X_DATETIME_UTF8_STR(s)  reinterpret_cast<char *>(u8##s)
#else
#define X_DATETIME_UTF8_STR(s)  u8##s
#endif

// Indicates the year that 2-digit years start on
// Nobody should use 2-digit years for anything anymore, they should be left for legacy
// applications which are stuck with 2-digit 20th century dates.
// You can redefine this at will, but be aware that the default value is 1920 and as a result will
// not read values from present-day dates. (This was deliberately chosen to make it easier for
// applications with legacy formats to use xDateTime).
#ifndef X_DATETIME_2_YEAR_START
#define X_DATETIME_2_YEAR_START 1920
#endif

// FIXME Broken
// #if (sizeof(time_t) == 4)
// #warning "The system time_t is 4 bytes - dates past 2038 or before 1900 will cause undefined behavior."
// #warning "See https://en.wikipedia.org/wiki/Year_2038_problem for details."
// #endif

// 
// // I hope this works....
// template <typename StreamT>
// void set_utf8_locale(StreamT& stream) {
//     std::locale loc(std::locale(), new std::codecvt_utf8<typename StreamT::char_type>());
//     stream.imbue(loc);
// }
//

namespace xDateTime {

class MalformedDateTime {
public:
    MalformedDateTime() {}
    MalformedDateTime(const char *msg_) { msg = std::string(msg_); }
    MalformedDateTime(const std::string &msg_) { msg = msg_; }
    std::string what() const { return msg; }

private:
    std::string msg;
};
}

#ifndef X_DATETIME_NO_TIMEZONES
#include "x_datetime_timezone.h"
#else
namespace xDateTime {
struct Timezone {
    std::string name;
    std::vector<time_t> utc;
    std::vector<time_t> local;
    std::vector<std::string> tzname;
    std::vector<int> dst;
    std::vector<int> gmtoff; // seconds
    static std::string CalcOffset(int hour, int minute) {
        std::string n = std::to_string(hour) + ((minute > 0) ? ":" + std::to_string(minute) : "");
        n.insert(0, hour < 0 ? "UTC-" : "UTC+");
        return n;
    }
    Timezone() {}
    Timezone(int hour, int minute) {
        name = Timezone::CalcOffset(hour, minute);
        gmtoff.push_back(minute * 60 + hour * 60 * 60);
    }
    Timezone(const std::string& name_, int hour, int minute) {
        name = name_;
        gmtoff.push_back(minute * 60 + hour * 60 * 60);
    }
};

static inline std::map<std::string, Timezone> InitializeTimezones();
static inline Timezone TZ(const std::string& name) { return InitializeTimezones()[name]; }
struct BaseTimezone {
    Timezone tz;
    BaseTimezone() { tz = TZ("UTC"); }
};

static inline std::map<std::string, Timezone> InitializeTimezones() {
static std::map<std::string, Timezone> timezones;
    bool initialized = false;
    if (initialized) return timezones;
    timezones["UTC"] = Timezone("UTC", 0, 0);
    initialized = true;
    return timezones;
}
struct UTC_Timezone: public BaseTimezone {
    UTC_Timezone(): BaseTimezone() {
        tz = TZ("UTC");
    }
};
}
#endif /* X_DATETIME_NO_TIMEZONES */

#ifndef X_DATETIME_NO_LOCALES
#include "x_datetime_locale_data.h"
#else
namespace xDateTime {

struct _LocaleData {
    std::map<std::string, std::string> am;
    std::map<std::string, std::string> pm;
    std::map<std::string, std::string> date1_format;
    std::map<std::string, std::string> date_time_format;
    std::map<std::string, std::string> date_format;
    std::map<std::string, std::string> time24_format;
    std::map<std::string, std::string> time12_format;
    std::map<std::string, int> days_in_week;
    std::map<std::string, int> first_weekday_ref;
    std::map<std::string, int> first_weekday;
    std::map<std::string, int> first_week_year_min_days;
    std::map<std::string, std::map<int, std::string>> long_months;
    std::map<std::string, std::map<int, std::string>> short_months;
    std::map<std::string, std::map<int, std::string>> long_weekdays;
    std::map<std::string, std::map<int, std::string>> short_weekdays;
    std::map<std::string, std::map<int, std::string>> alt_digits;
};

static inline void InitializeLocaleData(_LocaleData& data) {
    data.am["C"] = reinterpret_cast<const char*>(u8"AM");
    data.pm["C"] = reinterpret_cast<const char*>(u8"PM");
    data.date1_format["C"] = reinterpret_cast<const char*>(u8"%a %b %e %H:%M:%S %Z %Y");
    data.date_time_format["C"] = reinterpret_cast<const char*>(u8"%a %b %e %H:%M:%S %Y");
    data.date_format["C"] = reinterpret_cast<const char*>(u8"%m/%d/%y");
    data.time24_format["C"] = reinterpret_cast<const char*>(u8"%H:%M:%S");
    data.time12_format["C"] = reinterpret_cast<const char*>(u8"%I:%M:%S %p");
    data.days_in_week["C"] = 7;
    data.first_weekday_ref["C"] = 19971130;
    data.first_week_year_min_days["C"] = 4;
    data.first_weekday["C"] = 1;
    data.long_months["C"][0] = reinterpret_cast<const char*>(u8"January");
    data.long_months["C"][1] = reinterpret_cast<const char*>(u8"February");
    data.long_months["C"][2] = reinterpret_cast<const char*>(u8"March");
    data.long_months["C"][3] = reinterpret_cast<const char*>(u8"April");
    data.long_months["C"][4] = reinterpret_cast<const char*>(u8"May");
    data.long_months["C"][5] = reinterpret_cast<const char*>(u8"June");
    data.long_months["C"][6] = reinterpret_cast<const char*>(u8"July");
    data.long_months["C"][7] = reinterpret_cast<const char*>(u8"August");
    data.long_months["C"][8] = reinterpret_cast<const char*>(u8"September");
    data.long_months["C"][9] = reinterpret_cast<const char*>(u8"October");
    data.long_months["C"][10] = reinterpret_cast<const char*>(u8"November");
    data.long_months["C"][11] = reinterpret_cast<const char*>(u8"December");

    data.short_months["C"][0] = reinterpret_cast<const char*>(u8"Jan");
    data.short_months["C"][1] = reinterpret_cast<const char*>(u8"Feb");
    data.short_months["C"][2] = reinterpret_cast<const char*>(u8"Mar");
    data.short_months["C"][3] = reinterpret_cast<const char*>(u8"Apr");
    data.short_months["C"][4] = reinterpret_cast<const char*>(u8"May");
    data.short_months["C"][5] = reinterpret_cast<const char*>(u8"Jun");
    data.short_months["C"][6] = reinterpret_cast<const char*>(u8"Jul");
    data.short_months["C"][7] = reinterpret_cast<const char*>(u8"Aug");
    data.short_months["C"][8] = reinterpret_cast<const char*>(u8"Sep");
    data.short_months["C"][9] = reinterpret_cast<const char*>(u8"Oct");
    data.short_months["C"][10] = reinterpret_cast<const char*>(u8"Nov");
    data.short_months["C"][11] = reinterpret_cast<const char*>(u8"Dec");

    data.long_weekdays["C"][0] = reinterpret_cast<const char*>(u8"Sunday");
    data.long_weekdays["C"][1] = reinterpret_cast<const char*>(u8"Monday");
    data.long_weekdays["C"][2] = reinterpret_cast<const char*>(u8"Tuesday");
    data.long_weekdays["C"][3] = reinterpret_cast<const char*>(u8"Wednesday");
    data.long_weekdays["C"][4] = reinterpret_cast<const char*>(u8"Thursday");
    data.long_weekdays["C"][5] = reinterpret_cast<const char*>(u8"Friday");
    data.long_weekdays["C"][6] = reinterpret_cast<const char*>(u8"Saturday");

    data.short_weekdays["C"][0] = reinterpret_cast<const char*>(u8"Sun");
    data.short_weekdays["C"][1] = reinterpret_cast<const char*>(u8"Mon");
    data.short_weekdays["C"][2] = reinterpret_cast<const char*>(u8"Tue");
    data.short_weekdays["C"][3] = reinterpret_cast<const char*>(u8"Wed");
    data.short_weekdays["C"][4] = reinterpret_cast<const char*>(u8"Thu");
    data.short_weekdays["C"][5] = reinterpret_cast<const char*>(u8"Fri");
    data.short_weekdays["C"][6] = reinterpret_cast<const char*>(u8"Sat");
    data.alt_digits["C"][0] = "0";
    data.alt_digits["C"][1] = "1";
    data.alt_digits["C"][2] = "2";
    data.alt_digits["C"][3] = "3";
    data.alt_digits["C"][4] = "4";
    data.alt_digits["C"][5] = "5";
    data.alt_digits["C"][6] = "6";
    data.alt_digits["C"][7] = "7";
    data.alt_digits["C"][8] = "8";
    data.alt_digits["C"][9] = "9";
}

}
#endif

namespace xDateTime {

class LocaleData
{
    public:
        static LocaleData& getInstance()
        {
            static LocaleData instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
            return instance;
        }

        static std::string GetLongMonth(std::string locale, int key) {
            LocaleData& d = LocaleData::getInstance();
            std::map<int, std::string> a;
            try {
                a = d.data.long_months[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
            try {
                return a[key];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Month not found");
            }
        }

        static std::string GetShortMonth(std::string locale, int key) {
            LocaleData& d = LocaleData::getInstance();
            std::map<int, std::string> a;
            try {
                a = d.data.short_months[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
            try {
                return a[key];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Month not found");
            }
        }

        static std::string GetLongWeekday(std::string locale, int key) {
            LocaleData& d = LocaleData::getInstance();
            std::map<int, std::string> a;
            try {
                a = d.data.long_weekdays[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
            try {
                return a[key];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Weekday not found");
            }
        }

        static std::string GetShortWeekday(std::string locale, int key) {
            LocaleData& d = LocaleData::getInstance();
            std::map<int, std::string> a;
            try {
                a = d.data.short_weekdays[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
            try {
                return a[key];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Weekday not found");
            }
        }

        static std::string GetAM(std::string locale) {
            LocaleData& d = LocaleData::getInstance();
            try {
                return d.data.am[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
        }

        static std::string GetPM(std::string locale) {
            LocaleData& d = LocaleData::getInstance();
            try {
                return d.data.pm[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
        }

        static std::string GetDate1Format(std::string locale) {
            LocaleData& d = LocaleData::getInstance();
            try {
                return d.data.date1_format[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
        }

        static std::string GetDateTimeFormat(std::string locale) {
            LocaleData& d = LocaleData::getInstance();
            try {
                return d.data.date_time_format[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
        }

        static std::string GetDateFormat(std::string locale) {
            LocaleData& d = LocaleData::getInstance();
            try {
                return d.data.date_format[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
        }

        static std::string GetTime24Format(std::string locale) {
            LocaleData& d = LocaleData::getInstance();
            try {
                return d.data.time24_format[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
        }

        static std::string GetTime12Format(std::string locale) {
            LocaleData& d = LocaleData::getInstance();
            try {
                return d.data.time12_format[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
        }

        static int GetDaysInWeeks(std::string locale) {
            LocaleData& d = LocaleData::getInstance();
            try {
                return d.data.days_in_week[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
        }

        static int GetFirstWeekdayReference(std::string locale) {
            LocaleData& d = LocaleData::getInstance();
            try {
                return d.data.first_weekday_ref[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
        }

        static int GetFirstWeekOfYearMinDays(std::string locale) {
            LocaleData& d = LocaleData::getInstance();
            try {
                return d.data.first_week_year_min_days[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
        }

        static int GetFirstWeekday(std::string locale) {
            LocaleData& d = LocaleData::getInstance();
            try {
                return d.data.first_weekday[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
        }

        static bool IsValidLocaleAltDigit(std::string locale, std::string num) {
            LocaleData& d = LocaleData::getInstance();
            std::map<int, std::string> a;
            try {
                a = d.data.short_weekdays[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
            for (int i = 0; i < a.size(); ++i) {
                if (a[i] == num) {
                    return true;
                }
            }
            return false;
        }

        static std::string GetAltDigit(std::string locale, int num) {
            LocaleData& d = LocaleData::getInstance();
            std::map<int, std::string> a;
            try {
                a = d.data.alt_digits[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
            try {
                return a[num];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Alternate digit not found");
            }
        }

        static int GetReverseAltDigit(std::string locale, std::string num) {
            LocaleData& d = LocaleData::getInstance();
            std::map<int, std::string> a;
            try {
                a = d.data.alt_digits[locale];
            }
            catch (std::out_of_range& e) {
                throw MalformedDateTime("Locale not found");
            }
            for (int i = 0; i < a.size(); ++i) {
                if (d.data.alt_digits[locale][i] == num) {
                    return i;
                }
            }
            throw MalformedDateTime("Alternate digit not found");
        }

        template <typename Length>
        static std::string GetNumber(std::string locale, Length num) {
            if (num == 0) return LocaleData::GetAltDigit(locale, 0);
            std::string s;
            if (num < 0) { s += '-'; num = -num; }
            while (num > 0) {
                s += LocaleData::GetAltDigit(locale, num % 10);
                num /= 10;
            }
            return s;
        }

    private:
	_LocaleData data;
        LocaleData() {                    // Constructor? (the {} brackets) are needed here.
            InitializeLocaleData(this->data);
        }


        // We can use the better technique of deleting the methods
        // we don't want.
    public:
        LocaleData(LocaleData const&)               = delete;
        void operator=(LocaleData const&)  = delete;

        // Note: Scott Meyers mentions in his Effective Modern
        //       C++ book, that deleted functions should generally
        //       be public as it results in better error messages
        //       due to the compilers behavior to check accessibility
        //       before deleted status
};

class GregorianCalendar {
    //TODO this is a stub.
};

#ifndef X_DATETIME_NO_LOCALES
#define UTF8_CHAR_LEN( byte ) (( 0xE5000000 >> (( byte >> 3 ) & 0x1e )) & 3 ) + 1
#endif

template <typename Calendar, typename Clock, typename Period> class DateTime;
template <typename Ratio> class TimeDelta;
template <typename Calendar, typename Clock, typename Period> class DateTimePeriod;

template <typename Char, typename Traits, typename Ratio>
static inline std::basic_ostream<Char, Traits> &
operator<<(std::basic_ostream<Char, Traits> &os, const TimeDelta<Ratio> &obj);


template <typename Char, typename Traits, typename Calendar, typename Clock, typename Period>
static inline std::basic_ostream<Char, Traits> &
operator<<(std::basic_ostream<Char, Traits> &os, const DateTime<Calendar, Clock, Period> &obj);


template <typename Char, typename Traits, typename Calendar, typename Clock, typename Period>
static inline std::basic_ostream<Char, Traits> &
operator<<(std::basic_ostream<Char, Traits> &os, const DateTimePeriod<Calendar, Clock, Period> &obj);

template <typename Ratio>
static inline TimeDelta<Ratio> operator+(const TimeDelta<Ratio> &a, const TimeDelta<Ratio> &b);

template <typename Ratio>
static inline TimeDelta<Ratio> operator-(const TimeDelta<Ratio> &a, const TimeDelta<Ratio> &b);

template <typename Calendar, typename Clock, typename Period>
static inline DateTime<Calendar, Clock, Period> operator+(const DateTime<Calendar, Clock, Period> &a, const TimeDelta<Period> &b);

template <typename Calendar, typename Clock, typename Period>
static inline DateTime<Calendar, Clock, Period> operator+(const TimeDelta<Period> &a, const DateTime<Calendar, Clock, Period> &b);

template <typename Calendar, typename Clock, typename Period>
static inline TimeDelta<Period> operator-(const DateTime<Calendar, Clock, Period> &a, const DateTime<Calendar, Clock, Period> &b);

template <typename Calendar, typename Clock, typename Period>
static inline DateTime<Calendar, Clock, Period> operator-(const DateTime<Calendar, Clock, Period> &a, const TimeDelta<Period> &b);

template<typename Ratio = std::nano> class TimeDelta {
private:
    std::chrono::duration<long long, Ratio> d;

    template <typename Calendar, typename Clock, typename Period>
    friend class DateTime;

public:
    TimeDelta() { this->d = std::chrono::hours(0); }

    template <typename OldRatio = Ratio>
    TimeDelta(const TimeDelta<OldRatio> &a) : TimeDelta(a.d) {}

    template <typename OldRatio = Ratio, typename Length>
    TimeDelta(const std::chrono::duration<Length, OldRatio>& d) {
        long long count = static_cast<long long>(d.count());
        long long q = count / OldRatio::den;
        long long r = count % OldRatio::den;
        q *= Ratio::den;
        if (Ratio::den > OldRatio::den) {
            r *= Ratio::den / OldRatio::den;
        }
        else {
            r /= OldRatio::den / Ratio::den;
        }
        q += r;
        this->d = std::chrono::duration<long long, Ratio>(q);
    }

#ifdef X_DATETIME_WITH_BOOST
    TimeDelta(const boost::posix_time::time_duration& td) {
        int FS = 0;
#ifdef BOOST_DATE_TIME_HAS_NANOSECONDS
        if (Ratio::den > 1000000) {
            FS = static_cast<int>(static_cast<long long>(
                        (td.total_nanoseconds() % 1000000000) /
                        (Ratio::den / 1000000000.0)) % 1000000000);
        }
#else
        if (false) {}
#endif
        else if (Ratio::den > 1000) {
            FS = static_cast<int>(static_cast<long long>(
                        (td.total_microseconds() % 1000000) /
                        (Ratio::den / 1000000.0)) % 1000000);
        }
        else if (Ratio::den > 1) {
            FS = static_cast<int>(static_cast<long long>(
                        (td.total_milliseconds() % 1000) /
                        (Ratio::den / 1000.0)) % 1000);
        }
        *this = TimeDelta(0, td.hours(), td.minutes(), td.seconds(), FS);
    }
#endif

#ifdef X_DATETIME_WITH_ABSEIL
    TimeDelta(const absl::Duration& t) {
        if (Ratio::den > 1000000) {
            *this = TimeDelta(absl::ToChronoNanoseconds(t));
        }
        else if (Ratio::den > 1000) {
            *this = TimeDelta(absl::ToChronoMicroseconds(t));
        }
        else if (Ratio::den > 1) {
            *this = TimeDelta(absl::ToChronoMilliseconds(t));
        }
        else {
            *this = TimeDelta(absl::ToChronoSeconds(t));
        }
    }
#endif

    TimeDelta(long long _days) {
        this->d = std::chrono::duration<long long, Ratio>(Ratio::den * 60 * 60 * 24 * _days);
    }

    TimeDelta(long long _days, long long _hours) {
        this->d = std::chrono::duration<long long, Ratio>(Ratio::den * 60 * 60 *
                        (_hours + 24 * _days));
    }

    TimeDelta(long long _days, long long _hours, long long _minutes) {
        this->d = std::chrono::duration<long long, Ratio>(Ratio::den * 60 * (_minutes + 60 *
                        (_hours + 24 * _days)));
    }

    TimeDelta(long long _days, long long _hours, long long _minutes, long long _seconds) {
        this->d = std::chrono::duration<long long, Ratio>(Ratio::den * (_seconds + 60 * (_minutes + 60 *
                        (_hours + 24 * _days))));
    }

    // Note: the precision of _fracseconds depends on the Ratio template.
    // By default, it is nanoseconds.
    // However, if you change it to microseconds, or milliseconds, or something else, that
    // will be its precision.
    TimeDelta(long long _days, long long _hours, long long _minutes, long long _seconds, long long _fracseconds) {
        this->d = std::chrono::duration<long long, Ratio>(_fracseconds + Ratio::den * (_seconds + 60 * (_minutes + 60 *
                        (_hours + 24 * _days))));
    }

    TimeDelta Abs() const {
        if (d.count() < 0)
            return TimeDelta(-d);
        else return *this;
    }

    std::chrono::duration<long long, Ratio> SinceEpoch() const {
        return TimeDelta<Ratio>(std::chrono::system_clock::now().time_since_epoch());
    }

    TimeDelta &operator=(const TimeDelta &other) {
        // Guard self assignment
        if (this == &other)
            return *this;
        this->d = other.d;
        return *this;
    }

    template <typename Ratio_>
    friend TimeDelta<Ratio_> operator+(const TimeDelta<Ratio_> &a,
            const TimeDelta<Ratio_> &b);

    template <typename Ratio_>
    friend TimeDelta<Ratio_> operator-(const TimeDelta<Ratio_> &a,
            const TimeDelta<Ratio_> &b);
    TimeDelta<Ratio> &operator+=(const TimeDelta<Ratio> &rhs) {
        d += rhs.d;
        return *this;
    }
    TimeDelta<Ratio> &operator-=(const TimeDelta<Ratio> &rhs) {
        d -= rhs.d;
        return *this;
    }

    bool operator==(const TimeDelta<Ratio> &rhs) const { return d == rhs.d; }
    bool operator!=(const TimeDelta<Ratio> &rhs) const { return d != rhs.d; }
    bool operator<(const TimeDelta<Ratio> &rhs) const { return d < rhs.d; }
    bool operator>(const TimeDelta<Ratio> &rhs) const { return d > rhs.d; }
    bool operator<=(const TimeDelta<Ratio> &rhs) const { return d <= rhs.d; }
    bool operator>=(const TimeDelta<Ratio> &rhs) const { return d >= rhs.d; }

    bool operator!() const { return d.count() == 0; }
    operator bool() const { return d.count() != 0; }

    int Days() const {
        return static_cast<int>(d.count() / Ratio::den / 86400);
    }

    int Hours() const {
        return static_cast<int>(d.count() / Ratio::den / 3600 % 24);
    }

    int Minutes() const {
        return static_cast<int>(d.count() / Ratio::den / 60 % 60);
    }

    int Seconds() const {
        return static_cast<int>(d.count() / Ratio::den % 60);
    }

    int Milliseconds() const {
        if (Ratio::den < 1000) {
            return 0;
        }
        return static_cast<int>(static_cast<long long>(d.count() / (Ratio::den / 1000.0)) % 1000);
    }

    int Microseconds() const {
        if (Ratio::den <= 1000) {
            return 0;
        }
        return static_cast<int>(static_cast<long long>(d.count() / (Ratio::den / 1000000.0)) % 1000000);
    }

    int Nanoseconds() const {
        if (Ratio::den <= 1000000) {
            return 0;
        }
        return static_cast<int>(static_cast<long long>(d.count() / (Ratio::den / 1000000000.0)) % 1000000000);
    }

    long long DaysW() const {
        return d.count() / Ratio::den / 86400;
    }

    long long HoursW() const {
        return d.count() / Ratio::den / 3600 % 24;
    }

    long long MinutesW() const {
        return d.count() / Ratio::den / 60 % 60;
    }

    long long SecondsW() const {
        return d.count() / Ratio::den % 60;
    }

    long long MillisecondsW() const {
        if (Ratio::den < 1000) {
            return 0;
        }
        return static_cast<long long>(d.count() / (Ratio::den / 1000.0)) % 1000;
    }

    long long MicrosecondsW() const {
        if (Ratio::den <= 1000) {
            return 0;
        }
        return static_cast<long long>(d.count() / (Ratio::den / 1000000.0)) % 1000000;
    }

    long long NanosecondsW() const {
        if (Ratio::den <= 1000000) {
            return 0;
        }
        return static_cast<long long>(d.count() / (Ratio::den / 1000000000.0)) % 1000000000;
    }

    template <typename Calendar_, typename Clock_, typename Period_>
    friend DateTime<Calendar_, Clock_, Period_> operator+(const DateTime<Calendar_, Clock_, Period_> &a, const TimeDelta<Period_> &b);

    template <typename Calendar_, typename Clock_, typename Period_>
    friend DateTime<Calendar_, Clock_, Period_> operator+(const TimeDelta<Period_> &a, const DateTime<Calendar_, Clock_, Period_> &b);

    template <typename Calendar_, typename Clock_, typename Period_>
    friend DateTime<Calendar_, Clock_, Period_> operator-(const DateTime<Calendar_, Clock_, Period_> &a, const TimeDelta<Period_> &b);

    template <typename Calendar_, typename Clock_, typename Period_>
    friend TimeDelta<Period_> operator-(const DateTime<Calendar_, Clock_, Period_> &a, const DateTime<Calendar_, Clock_, Period_> &b);

private:

    struct TimeEncapsulation {
        long long ns, us, ms, s, min, h, d;
    };

    TimeEncapsulation Encapsulate() const {
        TimeEncapsulation e;
        e.ns = 0;
        e.us = 0;
        e.ms = 0;
        long long count = d.count();
        if (Ratio::den > 1000000) {
            e.ns = static_cast<long long>(d.count() / (Ratio::den / 1000000000.0)) % 1000000000;
        }
        else if (Ratio::den > 1000) {
            e.us = static_cast<long long>(d.count() / (Ratio::den / 1000000.0)) % 1000000;
        }
        else if (Ratio::den > 1) {
            e.ms = static_cast<long long>(d.count() / (Ratio::den / 1000.0)) % 1000;
        }
        count /= Ratio::den;
        e.s = count % 60;
        count /= 60;
        e.min = count % 60;
        count /= 60;
        e.h = count % 24;
        count /= 24;
        e.d = count;
        return e;
    }

public:

    std::string ToString() const {
        TimeEncapsulation e = Encapsulate();
        std::string str;
        str += std::to_string(e.d) + "d ";
        str += std::to_string(e.h) + "h ";
        str += std::to_string(e.min) + "min ";
        str += std::to_string(e.s) + "s";

        // Depending on the range we use, print fractional
        // seconds in the highest supported resolution.
        // This won't print anything for any precision coarser
        // than milliseconds.
        if (Ratio::den > 1000000) {
            str += " " + std::to_string(e.ns) + "ns";
        }
        else if (Ratio::den > 1000) {
            str += " " + std::to_string(e.us) + X_DATETIME_UTF8_STR("μs");
        }
        else if (Ratio::den > 1) {
            str += " " + std::to_string(e.ms) + "ms";
        }

        return str;
    }


    int TotalDays() const {
        return static_cast<int>(d.count() / Ratio::den / 86400);
    }

    int TotalHours() const {
        return static_cast<int>(d.count() / Ratio::den / 3600);
    }

    int TotalMinutes() const {
        return static_cast<int>(d.count() / Ratio::den / 60);
    }

    int TotalSeconds() const {
        return static_cast<int>(d.count() / Ratio::den);
    }

    int TotalMilliseconds() const {
        if (Ratio::den < 1000) {
            return 0;
        }
        return static_cast<int>(static_cast<long long>(d.count() / (Ratio::den / 1000.0)));
    }

    int TotalMicroseconds() const {
        if (Ratio::den <= 1000) {
            return 0;
        }
        return static_cast<int>(static_cast<long long>(d.count() / (Ratio::den / 1000000.0)));
    }

    int TotalNanoseconds() const {
        if (Ratio::den <= 1000000) {
            return 0;
        }
        return static_cast<int>(static_cast<long long>(d.count() / (Ratio::den / 1000000000.0)));
    }

    long long TotalDaysW() const {
        return d.count() / Ratio::den / 86400;
    }

    long long TotalHoursW() const {
        return d.count() / Ratio::den / 3600;
    }

    long long TotalMinutesW() const {
        return d.count() / Ratio::den / 60;
    }

    long long TotalSecondsW() const {
        return d.count() / Ratio::den;
    }

    long long TotalMillisecondsW() const {
        if (Ratio::den < 1000) {
            return 0;
        }
        return static_cast<long long>(d.count() / (Ratio::den / 1000.0));
    }

    long long TotalMicrosecondsW() const {
        if (Ratio::den <= 1000) {
            return 0;
        }
        return static_cast<long long>(d.count() / (Ratio::den / 1000000.0));
    }

    long long TotalNanosecondsW() const {
        if (Ratio::den <= 1000000) {
            return 0;
        }
        return static_cast<long long>(d.count() / (Ratio::den / 1000000000.0));
    }

    void AddDays(long long _days) {
        long long a = _days * 86400 * Ratio::den;
        auto count = d.count();
        if (a > 0 &&
                (count > 0 && count > LLONG_MAX - a || count < 0 && count < LLONG_MIN + a)
                || a < 0 &&
                (count > 0 && count > LLONG_MAX + a || count < 0 && count < LLONG_MIN - a)
                ) {
            throw MalformedDateTime("Overflow during add/sub days");
        }
        d += std::chrono::duration<long long, Ratio>(a);
    }

    void AddHours(long long _hours) {
        long long a = _hours * 3600 * Ratio::den;
        auto count = d.count();
        if (a > 0 &&
                (count > 0 && count > LLONG_MAX - a || count < 0 && count < LLONG_MIN + a)
                || a < 0 &&
                (count > 0 && count > LLONG_MAX + a || count < 0 && count < LLONG_MIN - a)
                ) {
            throw MalformedDateTime("Overflow during add/sub days");
        }
        d += std::chrono::duration<long long, Ratio>(a);
    }

    void AddMinutes(long long _minutes) {
        long long a = _minutes * 60 * Ratio::den;
        auto count = d.count();
        if (a > 0 &&
                (count > 0 && count > LLONG_MAX - a || count < 0 && count < LLONG_MIN + a)
                || a < 0 &&
                (count > 0 && count > LLONG_MAX + a || count < 0 && count < LLONG_MIN - a)
                ) {
            throw MalformedDateTime("Overflow during add/sub days");
        }
        d += std::chrono::duration<long long, Ratio>(a);
    }

    void AddSeconds(long long _seconds) {
        long long a = _seconds * Ratio::den;
        auto count = d.count();
        if (a > 0 &&
                (count > 0 && count > LLONG_MAX - a || count < 0 && count < LLONG_MIN + a)
                || a < 0 &&
                (count > 0 && count > LLONG_MAX + a || count < 0 && count < LLONG_MIN - a)
                ) {
            throw MalformedDateTime("Overflow during add/sub days");
        }
        d += std::chrono::duration<long long, Ratio>(a);
    }

    void AddMilliseconds(long long _milliseconds) {
        if (Ratio::den <= 1) return;
        long long a = static_cast<long long>(_milliseconds * (Ratio::den / 1000));
        auto count = d.count();
        if (a > 0 &&
                (count > 0 && count > LLONG_MAX - a || count < 0 && count < LLONG_MIN + a)
                || a < 0 &&
                (count > 0 && count > LLONG_MAX + a || count < 0 && count < LLONG_MIN - a)
                ) {
            throw MalformedDateTime("Overflow during add/sub days");
        }
        d += std::chrono::duration<long long, Ratio>(a);
    }

    void AddMicroseconds(long long _microseconds) {
        if (Ratio::den <= 1000) return;
        long long a = static_cast<long long>(_microseconds * (Ratio::den / 1000000));
        auto count = d.count();
        if (a > 0 &&
                (count > 0 && count > LLONG_MAX - a || count < 0 && count < LLONG_MIN + a)
                || a < 0 &&
                (count > 0 && count > LLONG_MAX + a || count < 0 && count < LLONG_MIN - a)
                ) {
            throw MalformedDateTime("Overflow during add/sub days");
        }
        d += std::chrono::duration<long long, Ratio>(a);
    }

    void AddNanoseconds(long long _nanoseconds) {
        if (Ratio::den <= 1000000) return;
        long long a = static_cast<long long>(_nanoseconds * (Ratio::den / 1000000000));
        auto count = d.count();
        if (a > 0 &&
                (count > 0 && count > LLONG_MAX - a || count < 0 && count < LLONG_MIN + a)
                || a < 0 &&
                (count > 0 && count > LLONG_MAX + a || count < 0 && count < LLONG_MIN - a)
                ) {
            throw MalformedDateTime("Overflow during add/sub days");
        }
        d += std::chrono::duration<long long, Ratio>(a);
    }

    void SubDays(long long _days) { AddDays(-_days); }

    void SubHours(long long _hours) { AddHours(-_hours); }

    void SubMinutes(long long _minutes) { AddMinutes(-_minutes); }

    void SubSeconds(long long _seconds) { AddSeconds(-_seconds); }

    void SubMilliseconds(long long _milliseconds) { AddMilliseconds(-_milliseconds); }

    void SubMicroseconds(long long _microseconds) { AddMicroseconds(-_microseconds); }

    void SubNanoseconds(long long _nanoseconds) { AddNanoseconds(-_nanoseconds); }

    template <typename NewRatio = Ratio>
    std::chrono::duration<long long, NewRatio> ToChrono() const {
        long long count = d.count();
        long long q = count / Ratio::den;
        long long r = count % Ratio::den;
        if (q > LLONG_MAX/NewRatio::den || q < LLONG_MIN/NewRatio::den) {
            throw MalformedDateTime("Conversion to std::chrono::duration is not possible without overflow.");
        }
        q *= Ratio::den;
        if (Ratio::den > NewRatio::den) {
            r /= Ratio::den / NewRatio::den;
        }
        else {
            r *= NewRatio::den / Ratio::den;
        }
        if (q > LLONG_MAX - r || q < LLONG_MIN + r) {
            throw MalformedDateTime("Conversion to std::chrono::time_point is not possible without overflow.");
        }
        q += r;
        return std::chrono::duration<long long, NewRatio>(q);
    }

#ifdef X_DATETIME_WITH_BOOST
    boost::posix_time::time_duration ToBoostTimeDuration() const {
        TimeEncapsulation e = Encapsulate();
        boost::posix_time::time_duration td;
        td += boost::posix_time::hours(e.h + 24 * e.d);
        td += boost::posix_time::minutes(e.min);
        td += boost::posix_time::seconds(e.s);

#ifdef BOOST_DATE_TIME_HAS_NANOSECONDS
        if (e.ns != 0) {
            td += boost::posix_time::nanoseconds(e.ns);
        }
#endif
        if (e.us != 0) {
            td += boost::posix_time::microseconds(e.us);
        }
        if (e.ms != 0) {
            td += boost::posix_time::milliseconds(e.ms);
        }

        return td;
    }
#endif

#ifdef X_DATETIME_WITH_ABSEIL
    absl::Duration ToAbseilDuration() const {
        return absl::FromChrono(ToChrono());
    }
#endif

    template<typename Char, typename Traits, typename Ratio_>
    friend std::basic_ostream<Char, Traits> &operator<<(std::basic_ostream<Char, Traits> &os, const TimeDelta<Ratio_> &obj);

};


template<typename Ratio = std::nano>
inline static TimeDelta<Ratio> Days(long long a) {
    return TimeDelta<Ratio>(a, 0, 0, 0, 0);
}

template<typename Ratio = std::nano>
inline static TimeDelta<Ratio> Hours(long long a) {
    return TimeDelta<Ratio>(0, a, 0, 0, 0);
}

template<typename Ratio = std::nano>
inline static TimeDelta<Ratio> Minutes(long long a) {
    return TimeDelta<Ratio>(0, 0, a, 0, 0);
}

template<typename Ratio = std::nano>
inline static TimeDelta<Ratio> Seconds(long long a) {
    return TimeDelta<Ratio>(0, 0, 0, a, 0);
}

template<typename Ratio = std::nano>
inline static TimeDelta<Ratio> Milliseconds(long long a) {
    return TimeDelta<Ratio>(0, 0, 0, 0, a*Ratio::den/1000);
}

template<typename Ratio = std::nano>
inline static TimeDelta<Ratio> Microseconds(long long a) {
    return TimeDelta<Ratio>(0, 0, 0, 0, a*Ratio::den/1000000);
}

template<typename Ratio = std::nano>
inline static TimeDelta<Ratio> Nanoseconds(long long a) {
    return TimeDelta<Ratio>(0, 0, 0, 0, a*Ratio::den/1000000000);
}

// Calculates the offset of Clock2 relative to Clock1, in seconds.
// Assumes the fractional seconds part of the epoches for Clock1 and Clock2 are both zero.
template <typename Clock1, typename Clock2>
long long ClockOffset() {
        auto now1clock = Clock1::now();
        auto now2clock = Clock2::now();

        // We have no idea at what second the template clock starts on. But this method works on an important
        // assumption: that the fractional seconds part of both clocks' epoch are zero.
        std::chrono::duration<long long, std::milli> now1 = std::chrono::duration_cast<std::chrono::milliseconds>(now1clock.time_since_epoch());
        std::chrono::duration<long long, std::milli> now2 = std::chrono::duration_cast<std::chrono::milliseconds>(now2clock.time_since_epoch());

        long long dur1 = static_cast<long long>(now1.count());
        long long dur2 = static_cast<long long>(now2.count());

        if (dur1 % 1000 > dur2 % 1000) {
            // The clock's second hand changed in between libc calls. 
            // Increase the second hand.
            dur1 += 1000;

            // Convert to seconds (truncating division)
            dur1 /= 1000;
            dur2 /= 1000;
        }

        // Calculate the number of seconds we must add to the time_point to align with "dur2"'s epoch.
        long long offset = dur2 - dur1;
        return offset;
}


template <typename Calendar = GregorianCalendar, typename Clock = std::chrono::system_clock,
         typename Period = typename std::chrono::time_point<Clock>::period>
class DateTime {
private:
    std::chrono::time_point<Clock> tp;
    const long long unix_offset; // number of seconds this clock differs from Unix time. (0 for system_clock)
    Timezone tz; // initialize like this: UTC_Timezone().tz
    using time_point = typename std::chrono::time_point<Clock>;
    using clock_duration = typename time_point::duration;
    typedef typename time_point::period ClockPeriod;

    void assertValidPeriod() {
        if (ClockPeriod::den < Period::den) {
            throw MalformedDateTime("Specified duration must not be more granular than clock's duration)");
        }
    }

public:
    DateTime(Timezone zone = UTC_Timezone().tz)
        : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()), tz(zone) {
        assertValidPeriod();
        tp = time_point(clock_duration(unix_offset * (ClockPeriod::den)));
    }
    DateTime(const DateTime<Calendar, Clock, Period> &a) : tp(a.tp), unix_offset(a.unix_offset), tz(a.tz) {}

    explicit DateTime(time_t t, Timezone zone = UTC_Timezone().tz)
        : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()), tz(zone) {
        assertValidPeriod();
        tp = time_point(clock_duration((t + unix_offset) * Period::den));
    }
    DateTime(struct tm *tm): DateTime(mktime(tm)) {}

    // 00:00 UTC+0
    DateTime(int y, int m, int d, Timezone zone = UTC_Timezone().tz)
        : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()), tz(zone) {
        assertValidPeriod();
        struct tm t;
        memset(&t, 0, sizeof(struct tm));
        t.tm_year = y - 1900;
        t.tm_mon = m - 1;
        t.tm_mday = d;
        tp = time_point(clock_duration((mktime(&t) + unix_offset) * Period::den));
    }

    DateTime(int y, int m, int d, int H, int M, int S, Timezone zone = UTC_Timezone().tz)
    : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()), tz(zone){
        assertValidPeriod();
        struct tm t;
        memset(&t, 0, sizeof(struct tm));
        t.tm_year = y - 1900;
        t.tm_mon = m - 1;
        t.tm_mday = d;
        t.tm_hour = H;
        t.tm_min = M;
        t.tm_sec = S;
        tp = time_point(clock_duration((mktime(&t) + unix_offset) * Period::den));
    }

    DateTime(int y, int m, int d, int H, int M, int S, int FS, Timezone zone = UTC_Timezone().tz)
    : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()), tz(zone){
        assertValidPeriod();
        struct tm t;
        memset(&t, 0, sizeof(struct tm));
        t.tm_year = y - 1900;
        t.tm_mon = m - 1;
        t.tm_mday = d;
        t.tm_hour = H;
        t.tm_min = M;
        t.tm_sec = S;

        tp = time_point(clock_duration((mktime(&t) + unix_offset) * Period::den + FS));
    }

    explicit DateTime(const std::string &s, const std::string &locale = "C")
        : unix_offset(ClockOffset<std::chrono::system_clock, Clock>())
    {
        assertValidPeriod();
        //Use the local representation first and then ISO 8601 and other common formats.

        std::vector<std::string> datetime_strs;
        datetime_strs.push_back(LocaleData::GetDateTimeFormat(locale.c_str()));
        datetime_strs.push_back(LocaleData::GetDate1Format(locale.c_str()));
        datetime_strs.push_back(LocaleData::GetDateFormat(locale.c_str()));
        datetime_strs.push_back(LocaleData::GetTime24Format(locale.c_str()));
        datetime_strs.push_back(LocaleData::GetTime12Format(locale.c_str()));

        datetime_strs.push_back("%Y-%m-%dT%H:%M:%S.%3Z%z");
        datetime_strs.push_back("%Y-%m-%dT%H:%M:%S.%3Z");
        datetime_strs.push_back("%Y-%m-%d %H:%M:%S.%3");
        datetime_strs.push_back("%Y-%m-%dT%H:%M:%S.%2Z%z");
        datetime_strs.push_back("%Y-%m-%dT%H:%M:%S.%2Z");
        datetime_strs.push_back("%Y-%m-%d %H:%M:%S.%2");
        datetime_strs.push_back("%Y-%m-%dT%H:%M:%S.%1Z%z");
        datetime_strs.push_back("%Y-%m-%dT%H:%M:%S.%1Z");
        datetime_strs.push_back("%Y-%m-%d %H:%M:%S.%1");
        datetime_strs.push_back("%Y-%m-%dT%H:%M:%SZ%z");
        datetime_strs.push_back("%Y-%m-%dT%H:%M:%SZ");
        datetime_strs.push_back("%Y-%m-%d %H:%M:%S");
        datetime_strs.push_back("%Y-%m-%d %H:%M");
        datetime_strs.push_back("%Y-%m-%d");

        /***************  BEWARE ******************
         *
         * struct tm's tm_year field stores years
         * since 1900. It does NOT store the exact
         * year. tm_month is also between 0 and 11.
         *
         */
        bool was_imported = false;
        for (auto fmt: datetime_strs) {
            try {
                FromString(s, fmt);
                was_imported = true;
                break;
            }
            catch (const MalformedDateTime&) {}
        }
        if (!was_imported) {
            throw MalformedDateTime("None of the ISO8601 or supplied locale formats could interpret the string.");
        }
    }

    DateTime(const std::chrono::time_point<Clock> &_tp, Timezone zone = UTC_Timezone().tz) :
    unix_offset(ClockOffset<std::chrono::system_clock, Clock>()), tz(zone) {
        assertValidPeriod();
        long long count = static_cast<long long>(_tp.time_since_epoch().count());
        long long q = count / ClockPeriod::den;
        long long r = count % ClockPeriod::den;
        q += unix_offset;
        q *= Period::den;
        if (Period::den > ClockPeriod::den) {
            r *= Period::den / ClockPeriod::den;
        }
        else {
            r /= ClockPeriod::den / Period::den;
        }
        q += r;
        tp = time_point(clock_duration(q));
        

    }
    
    DateTime(const TimeDelta<Period> &td, Timezone zone = UTC_Timezone().tz)
    : unix_offset(0 /* i.e. TimeDelta uses system_clock */), tz(zone) {
        assertValidPeriod();
        tp = std::chrono::system_clock::now() + td.ToChrono();
    }

#ifdef X_DATETIME_WITH_BOOST
    DateTime(const boost::posix_time::ptime& p)
    : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()) {
        assertValidPeriod();
        auto date = p.date();
        auto tod = p.time_of_day();
        int FS = 0;
#ifdef BOOST_DATE_TIME_HAS_NANOSECONDS
        if (Ratio::den > 1000000) {
            FS = static_cast<int>(static_cast<long long>(
                        (tod.total_nanoseconds() % 1000000000) /
                        (Period::den / 1000000000.0)) % 1000000000);
        }
#else
        if (false) {}
#endif
        else if (Period::den > 1000) {
            FS = static_cast<int>(static_cast<long long>(
                        (tod.total_microseconds() % 1000000) /
                        (Period::den / 1000000.0)) % 1000000);
        }
        else if (Period::den > 1) {
            FS = static_cast<int>(static_cast<long long>(
                        (tod.total_milliseconds() % 1000) /
                        (Period::den / 1000.0)) % 1000);
        }
        *this = DateTime(static_cast<int>(date.year()),
                static_cast<int>(date.month()),
                static_cast<int>(date.day()),
                static_cast<int>(tod.hours()),
                static_cast<int>(tod.minutes()),
                static_cast<int>(tod.seconds()), FS);
    }
#endif /* X_DATETIME_WITH_BOOST */

#ifdef X_DATETIME_WITH_ABSEIL
    DateTime(const absl::CivilSecond& civ)
    : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()) {
        assertValidPeriod();
        *this = DateTime(civ.year(), civ.month(), civ.day(),
                civ.hour(), civ.minute(), civ.second(), 0);
    }

    DateTime(const absl::CivilMinute& civ)
    : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()) {
        assertValidPeriod();
        *this = DateTime(civ.year(), civ.month(), civ.day(),
                civ.hour(), civ.minute(), 0, 0);
    }

    DateTime(const absl::CivilHour& civ)
    : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()) {
        assertValidPeriod();
        *this = DateTime(civ.year(), civ.month(), civ.day(),
                civ.hour(), 0, 0, 0);
    }

    DateTime(const absl::CivilDay& civ)
    : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()) {
        assertValidPeriod();
        *this = DateTime(civ.year(), civ.month(), civ.day(),
                0, 0, 0, 0);
    }

    DateTime(const absl::Time& t)
    : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()) {
        assertValidPeriod();
        *this = DateTime(absl::ToChronoTime(t));
    }
#endif /* X_DATETIME_WITH_ABSEIL */

#ifdef X_DATETIME_WITH_POCO
    DateTime(const Poco::DateTime& p)
    : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()) {
        assertValidPeriod();
        int FS = 0;
        if (Period::den > 1000) {
            FS = static_cast<int>(static_cast<long long>(
                        (p.microsecond() % 1000000) /
                        (Period::den / 1000000.0)) % 1000000);
        }
        else if (Period::den > 1) {
            FS = static_cast<int>(static_cast<long long>(
                        (p.millisecond() % 1000) /
                        (Period::den / 1000.0)) % 1000);
        }
        *this = DateTime(p.year(), p.month(), p.day(),
                p.hour(), p.minute(), p.second(), FS);
    }
#endif /* X_DATETIME_WITH_POCO */

#ifdef X_DATETIME_WITH_GLIB
    DateTime(GDateTime* datetime)
    : unix_offset(ClockOffset<std::chrono::system_clock, Clock>()) {
        *this = DateTime(static_cast<time_t>(g_date_time_to_unix(datetime)));
        AddMicroseconds(g_date_time_get_microsecond(datetime));
    }
#endif /* X_DATETIME_WITH_GLIB */

    void FromString(const std::string& input, const std::string &fmt, const std::string& locale = "C") {
        int i = 0;
        int ms = 0;
        int us = 0;
        int ns = 0;
        int d = 0;
        int e = 0;
        int G = 0;
        int g = 0;
        int m = 0;
        int Y = 0;
        int H = 0;
        int I = 0;
        int M = 0;
        long long ssepoch = true;
        int S = 0;
        int u = 0;
        int U = 0;
        int V = 0;
        int W = 0;
        int y = 0;
        int C = 0;
        int doy = 0;
        int k = 0;
        int l = 0;
        int zhh = 0;
        int zmm = 0;
        std::string zname;
        bool have_am = false;
        bool have_pm = false;
        bool have_ms = false;
        bool have_us = false;
        bool have_ns = false;
        bool have_d = false;
        bool have_e = false;
        bool have_G = false;
        bool have_g = false;
        bool have_m = false;
        bool have_Y = false;
        bool have_H = false;
        bool have_I = false;
        bool have_M = false;
        bool have_ssepoch = false;
        bool have_S = false;
        bool have_u = false;
        bool have_U = false;
        bool have_V = false;
        bool have_W = false;
        bool have_y = false;
        bool have_C = false;
        bool have_doy = false;
        bool have_k = false;
        bool have_l = false;
        bool have_z = false;
        bool have_Z = false;

        bool format = false; // Encountered %?
        bool era = false; // Encountered %E?
        bool alt = false; // Encountered %O?
                          
        std::string s = input;
        std::string this_format = fmt;
        std::string old_format;
        int old_ci = -1;
        int ci = 0;

        if (s.empty()) return;

nested_junction:
        for (; ci < this_format.size(); ++ci) {
            char c = this_format[ci];
            switch (c) {
                case '%':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    // ignore literal characters
                    format = false;
                }
                else {
                    format = true;
                    break;
                }
                if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                ++i;
                break;

                case '1':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxms = 3;
                    std::string tmp;
                    for (int j = 0; j < maxms; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    ms = std::stoi(tmp);
                    have_ms = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxms = 3;
                    std::string tmp;
                    for (int j = 0; j < maxms; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    ms = std::stoi(tmp);
                    have_ms = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case '2':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxus = 6;
                    std::string tmp;
                    for (int j = 0; j < maxus; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    us = std::stoi(tmp);
                    have_us = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxus = 6;
                    std::string tmp;
                    for (int j = 0; j < maxus; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    us = std::stoi(tmp);
                    have_us = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case '3':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxns = 9;
                    std::string tmp;
                    for (int j = 0; j < maxns; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    us = std::stoi(tmp);
                    have_ns = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxns = 9;
                    std::string tmp;
                    for (int j = 0; j < maxns; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    ns = std::stoi(tmp);
                    have_ns = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'a':
                // We should make a rule that says all locales are required to have exactly 7 days in a week.
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    for (int j = 0; j < 7; ++j) {
                        std::string _a = LocaleData::GetShortWeekday(locale.c_str(), j);
                        if (!strncmp(s.data()+i, _a.data(), _a.length())) {
                            // They match
                            u = j;
                            have_u = true;
                            i += strlen(_a.data());
                            break;
                        }
                    }
                    if (!have_u) { throw MalformedDateTime("Bad format string and/or input values"); }
                    format = false;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;


                case 'A':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    for (int j = 0; j < 7; ++j) {
                        std::string _a = LocaleData::GetLongWeekday(locale.c_str(), j);
                        if (!strncmp(s.data()+i, _a.data(), _a.length())) {
                            // They match
                            u = j;
                            have_u = true;
                            i += strlen(_a.data());
                            break;
                        }
                    }
                    if (!have_u) { throw MalformedDateTime("Bad format string and/or input values"); }
                    format = false;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;


                case 'b':
                case 'h':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    for (int j = 0; j < 12; ++j) {
                        std::string _a = LocaleData::GetShortMonth(locale.c_str(), j);
                        if (!strncmp(s.data()+i, _a.data(), _a.length())) {
                            // They match
                            m = j+1;
                            have_m = true;
                            i += strlen(_a.data());
                            break;
                        }
                    }
                    if (!have_m) { throw MalformedDateTime("Bad format string and/or input values"); }
                    format = false;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;


                case 'B':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    for (int j = 0; j < 12; ++j) {
                        std::string _a = LocaleData::GetLongMonth(locale.c_str(), j);
                        if (!strncmp(s.data()+i, _a.data(), _a.length())) {
                            // They match
                            m = j;
                            have_m = true;
                            i += strlen(_a.data());
                            break;
                        }
                    }
                    if (!have_m) { throw MalformedDateTime("Bad format string and/or input values"); }
                    format = false;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;
                

                case 'c':
                if (alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format || era) {
                    old_format = this_format;
                    old_ci = ci + 1; // also advance old ptr to next char
                    this_format = LocaleData::GetDateTimeFormat(locale.c_str());
                    ci = 0;
                    format = false;
                    era = false;
                    goto nested_junction;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'C':
                if (alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || era
#endif
                        ) {
                    std::string tmp;
                    for (int j = 0 ; /* true */ ; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    C = std::stoi(tmp) * 100;
                    have_C = true;
                    format = false;
                    era = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (era) {
                    std::string tmp;
                    for (int j = 0; /* true */; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    C = std::stoi(tmp);
                    have_C = true;
                    era = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'd':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxd = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxd; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    d = std::stoi(tmp);
                    have_d = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxd = 2;
                    std::string tmp;
                    for (int j = 0; j < maxd; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    d = std::stoi(tmp);
                    have_d = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'D':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    old_format = this_format;
                    old_ci = ci + 1; // also advance old ptr to next char
                    this_format = "%m/%d/%y";
                    ci = 0;
                    format = false;
                    era = false;
                    goto nested_junction;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'e':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxe = 2;
                    std::string tmp;
                    int spacer = 0;
                    while (isspace(s[i])) ++i, ++spacer;
                    for (int j = 0 ; j < maxe - spacer; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    e = std::stoi(tmp);
                    have_e = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxe = 2;
                    std::string tmp;
                    int spacer = 0;
                    while (isspace(s[i])) ++i, ++spacer;
                    for (int j = 0; j < maxe - spacer; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    e = std::stoi(tmp);
                    have_e = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;


                case 'E':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                   era = true;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'F':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    old_format = this_format;
                    old_ci = ci + 1; // also advance old ptr to next char
                    this_format = "%Y-%m-%d";
                    ci = 0;
                    format = false;
                    era = false;
                    goto nested_junction;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'G':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    std::string tmp;
                    for (int j = 0 ; /* true */; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    G = std::stoi(tmp);
                    have_G = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    std::string tmp;
                    for (int j = 0; /* true */; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    G = std::stoi(tmp);
                    have_G = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'g':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxg = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxg; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    g = std::stoi(tmp);
                    have_g = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxg = 2;
                    std::string tmp;
                    for (int j = 0; j < maxg; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    g = std::stoi(tmp);
                    have_g = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'H':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxH = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxH; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    H = std::stoi(tmp);
                    have_H = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxH = 2;
                    std::string tmp;
                    for (int j = 0; j < maxH; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    g = std::stoi(tmp);
                    have_g = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;
                
                case 'I':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxI = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxI; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    I = std::stoi(tmp);
                    have_I = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxI = 2;
                    std::string tmp;
                    for (int j = 0; j < maxI; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    I = std::stoi(tmp);
                    have_I = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'j':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxdoy = 3;
                    std::string tmp;
                    for (int j = 0 ; j < maxdoy; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    doy = std::stoi(tmp);
                    have_doy = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxdoy = 3;
                    std::string tmp;
                    for (int j = 0; j < maxdoy; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    doy = std::stoi(tmp);
                    have_doy = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'k':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxk = 2;
                    std::string tmp;
                    int spacer = 0;
                    while (isspace(s[i])) ++i, ++spacer;
                    for (int j = 0 ; j < maxk - spacer; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    k = std::stoi(tmp);
                    have_k = true;
                    format = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxk = 2;
                    std::string tmp;
                    int spacer = 0;
                    while (isspace(s[i])) ++i, ++spacer;
                    for (int j = 0 ; j < maxk - spacer; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                        if (altdig != -1) tmp += (char) (altdig + '0');
                        else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    k = std::stoi(tmp);
                    have_k = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'l':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxl = 2;
                    std::string tmp;
                    int spacer = 0;
                    while (isspace(s[i])) ++i, ++spacer;
                    for (int j = 0 ; j < maxl - spacer; ++j, ++i) {
                        if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                        else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    l = std::stoi(tmp);
                    have_l = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxl = 2;
                    std::string tmp;
                    int spacer = 0;
                    while (isspace(s[i])) ++i, ++spacer;
                    for (int j = 0 ; j < maxl - spacer; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                        if (altdig != -1) tmp += (char) (altdig + '0');
                        else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    l = std::stoi(tmp);
                    have_l = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'm':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxm = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxm; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    m = std::stoi(tmp);
                    have_m = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxm = 2;
                    std::string tmp;
                    for (int j = 0; j < maxm; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    m = std::stoi(tmp);
                    have_m = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'M':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxM = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxM; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    M = std::stoi(tmp);
                    have_M = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxM = 2;
                    std::string tmp;
                    for (int j = 0; j < maxM; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    M = std::stoi(tmp);
                    have_M = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'n':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    if (s[i] != '\n') { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                    format = false;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;
                
                case 'O':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    alt = true;
                    format = false;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'p':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    std::string _a = LocaleData::GetAM(locale.c_str());
                    if (!strncmp(s.data()+i, _a.data(), _a.length())) {
                        // They match
                        have_am = true;
                        i += _a.length();
                    }
                    else {
                        _a = LocaleData::GetPM(locale.c_str());
                        if (!strncmp(s.data()+i, _a.data(), _a.length())) {
                            // They match
                            have_pm = true;
                            i += _a.length();
                        }
                        else {
                            { throw MalformedDateTime("Bad format string and/or input values"); }
                        }
                    }
                    format = false;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'P':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    std::string _a = LocaleData::GetAM(locale.c_str());
                    std::transform(_a.begin(), _a.end(), _a.begin(),
                        [](unsigned char c){ return std::tolower(c); });

                    if (!strncmp(s.data()+i, _a.data(), _a.length())) {
                        // They match
                        have_am = true;
                        i += _a.length();
                    }
                    else {
                        _a = LocaleData::GetPM(locale.c_str());
                    std::transform(_a.begin(), _a.end(), _a.begin(),
                        [](unsigned char c){ return std::tolower(c); });

                        if (!strncmp(s.data()+i, _a.data(), _a.length())) {
                            // They match
                            have_pm = true;
                            i += _a.length();
                        }
                        else {
                            { throw MalformedDateTime("Bad format string and/or input values"); }
                        }
                    }
                    format = false;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'r':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    old_format = this_format;
                    old_ci = ci + 1; // also advance old ptr to next char
                    this_format = LocaleData::GetTime12Format(locale.c_str());
                    ci = 0;
                    format = false;
                    goto nested_junction;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'R':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    old_format = this_format;
                    old_ci = ci + 1; // also advance old ptr to next char
                    this_format = LocaleData::GetTime24Format(locale.c_str());
                    ci = 0;
                    format = false;
                    goto nested_junction;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 's':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    std::string tmp;
                    for (int j = 0 ; /* true */; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    ssepoch = std::stoll(tmp);
                    have_ssepoch = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxS = 2;
                    std::string tmp;
                    for (int j = 0; j < maxS; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    ssepoch = std::stoll(tmp);
                    have_ssepoch = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;


                case 'S':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxS = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxS; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    S = std::stoi(tmp);
                    have_S = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxS = 2;
                    std::string tmp;
                    for (int j = 0; j < maxS; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    S = std::stoi(tmp);
                    have_S = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 't':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    if (s[i] != '\t') { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                    format = false;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'T':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    old_format = this_format;
                    old_ci = ci + 1; // also advance old ptr to next char
                    this_format = "%H:%M:%S";
                    ci = 0;
                    format = false;
                    era = false;
                    goto nested_junction;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'u':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxu = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxu; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    u = std::stoi(tmp);
                    have_u = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxu = 2;
                    std::string tmp;
                    for (int j = 0; j < maxu; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    u = std::stoi(tmp);
                    if (u == 0)
                        u = 7;
                    have_u = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'U':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxU = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxU; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    U = std::stoi(tmp);
                    have_U = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxU = 2;
                    std::string tmp;
                    for (int j = 0; j < maxU; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    U = std::stoi(tmp);
                    have_U = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'V':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxV = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxV; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    V = std::stoi(tmp);
                    have_V = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxV = 2;
                    std::string tmp;
                    for (int j = 0; j < maxV; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    V = std::stoi(tmp);
                    have_V = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'w':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxu = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxu; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    u = std::stoi(tmp);
                    have_u = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxu = 2;
                    std::string tmp;
                    for (int j = 0; j < maxu; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    u = std::stoi(tmp);
                    have_u = true;
                    alt = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;


                case 'W':
                if (era) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int maxW = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxW; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    W = std::stoi(tmp);
                    have_W = true;
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int maxW = 2;
                    std::string tmp;
                    for (int j = 0; j < maxW; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    W = std::stoi(tmp);
                    have_W = true;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'x':
                if (alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format || era) {
                    old_format = this_format;
                    old_ci = ci + 1; // also advance old ptr to next char
                    this_format = LocaleData::GetDateFormat(locale.c_str());
                    ci = 0;
                    format = false;
                    era = false;
                    goto nested_junction;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'X':
                if (alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format || era) {
                    old_format = this_format;
                    old_ci = ci + 1; // also advance old ptr to next char
                    this_format = LocaleData::GetTime24Format(locale.c_str());
                    ci = 0;
                    format = false;
                    era = false;
                    goto nested_junction;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'y':
                if (alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || era
#endif
                        ) {
                    int maxy = 2;
                    std::string tmp;
                    for (int j = 0 ; j < maxy; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    y = std::stoi(tmp);
                    have_y = true;
                    format = false;
                    era = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (era) {
                    int maxy = 2;
                    std::string tmp;
                    for (int j = 0; j < maxy; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    y = std::stoi(tmp);
                    have_y = true;
                    era = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'Y':
                if (alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || era
#endif
                        ) {
                    std::string tmp;
                    for (int j = 0 ; /* true */; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmp += s[i];
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    Y = std::stoi(tmp);
                    have_Y = true;
                    format = false;
                    era = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (era) {
                    std::string tmp;
                    for (int j = 0; /* true */; ++j, ++i) {
                        int altdig = LocaleData::GetReverseAltDigit(locale.c_str(), &s[i]);
                       if (altdig != -1) tmp += (char) (altdig + '0');
                       else break;
                    }
                    if (tmp.empty()) { throw MalformedDateTime("Bad input value"); }
                    Y = std::stoi(tmp);
                    have_Y = true;
                    era = false;
                }
#endif
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'z':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    int maxzpart = 2;
                    std::string tmphh, tmpmm;
                    int sign = 1;
                    if (s[i] == '+') sign = 1;
                    else if (s[i] == '-') sign = -1;
                    else { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                    for (int j = 0 ; j < maxzpart; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmphh += s[i];
                       else break;
                    }
                    for (int j = 0 ; j < maxzpart; ++j, ++i) {
                       if (s[i] >= '0' && s[i] <= '9') tmpmm += s[i];
                       else break;
                    }
                    if (tmphh.empty() || tmpmm.empty()) { throw MalformedDateTime("Bad input value"); }
                    zhh = std::stoi(tmphh) * sign;
                    zmm = std::stoi(tmpmm);
                    have_z = true;
                    format = false;
                    alt = false;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case 'Z':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    for (zname = ""; isupper(s[i]); ++i) {
                        zname += s[i];
                    }
                    have_Z = true;
                    format = false;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                case '+':
                if (era || alt) { throw MalformedDateTime("Bad format string and/or input values"); }
                if (format) {
                    old_format = this_format;
                    old_ci = ci + 1; // also advance old ptr to next char
                    this_format = LocaleData::GetDate1Format(locale.c_str());
                    ci = 0;
                    format = false;
                    era = false;
                    goto nested_junction;
                }
                else {
                    if (s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                    ++i;
                }
                break;

                default:
                if (format || s[i] != c) { throw MalformedDateTime("Bad format string and/or input values"); }
                ++i;

            }
        }

        if (old_ci != -1) {
            ci = old_ci;
            this_format = old_format;
            old_ci = -1;
            old_format = "";
            goto nested_junction;
        }

        if (i != s.length()) {
            throw MalformedDateTime("Trailing characters are not allowed");
        }

        // PRECEDENCE RULES:
        // Z before z
        // s before anything else - if s is present, then check for ms/us/ns and
        // return immediately.
        // Y, m, d, H, M, S, ns before anything else
        // if H not available then use I, k, l (in that order)
        // if ns not available use us
        // if us not available use ms
        // if d not available use e
        // if both m and d/e are not available, use u and V <-- ISO week
        // if V is not available use U <-- first sunday
        // if U is not available use W <-- first monday
        // if none of m/d or u/(U,V,W) are available, use j
        // if Y is not available use y + X_DATETIME_2_YEAR_START
        // if y is not available use G
        // if G is not available use g + X_DATETIME_2_YEAR_START
        // We will not use century (C) unless y or g is also specified (it can be
        // used to override the 2-digit start year).
        // If no time zone is specified default to UTC
        // fail if no year type was specified or if none of m/d or u/(U,V,W)
        // or j were specified.
        // For the time components of the date, default to zero if not present.
        
        Timezone zone;
        if (have_z) {
            zone = Timezone(Timezone::CalcOffset(zhh, zmm), zhh, zmm);
        }
        else if (have_Z) {
            zone = TZ(zname);
        }
        else {
            zone = UTC_Timezone().tz;
        }

        // all datetimes are initialized to the clock's epoch.
        DateTime<Calendar, Clock, Period> dt(zone);

        if (have_ssepoch) {
            dt.AddSeconds(ssepoch);

            if (have_ms) {
                dt.AddMilliseconds(ms);
            }

            else if (have_us) {
                dt.AddMicroseconds(us);
            }

            else if (have_ns) {
                dt.AddNanoseconds(us);
            }

            *this = dt;
            return;
        }

        if (have_Y) {
            dt.AddYears(Y - 1970);
        }
        else if (have_y) {
            if (have_C) {
                dt.AddYears(C + y);
            }
            else {
                if (X_DATETIME_2_YEAR_START % 100 > y) {
                    y += 100;
                }
                y += X_DATETIME_2_YEAR_START / 100 * 100;
                dt.AddYears(y - 1970);
            }
        }
        else if (have_G) {
            dt.AddYears(G);
        }
        else if (have_g) {
            if (have_C) {
                dt.AddYears(C + g);
            }
            else {
                if (X_DATETIME_2_YEAR_START % 100 > g) {
                    g += 100;
                }
                g += X_DATETIME_2_YEAR_START / 100 * 100;
                dt.AddYears(g - 1970);
            }
            dt.AddYears(g);
        }
        // Else just leave it at epoch

        if (have_m) {
            dt.AddMonths(m-1);
            if (have_d) {
                dt.AddDays(d-1);
            }
            else if (have_e) {
                dt.AddDays(e-1);
            }
            // Else just start the day at 1
        }
        else if (have_V) {
            // find the day of the first ISO week.
            // This means we look for the first sunday.
            int _d = 1;
            for (; DateTime::DayOfWeek(Y, 1, _d) != 0 && _d < 7; ++_d) {}
            dt.AddDays(_d-1 + V*7 + (have_u) ? u-1 : 0 );
        }
        else if (have_U) {
            // find the day of the first Sunday week.
            int _d = 1;
            for (; DateTime::DayOfWeek(Y, 1, _d) != 0 && _d < 7; ++_d) {}
            dt.AddDays(_d-1 + U*7 + (have_u) ? u-1 : 0 );
        }
        else if (have_W) {
            // find the day of the first Monday week.
            int _d = 1;
            for (; DateTime::DayOfWeek(Y, 1, _d) != 1 && _d < 7; ++_d) {}
            int _u = u-1;
            if (_u < 0) u = 6;
            dt.AddDays(_d-1 + U*7 + (have_u) ? u-1 : 0 );
        }
        else if (have_doy) {
            // Day is already at 1 and not 0 so we should not try to add the 1-based day
            // as it is, take 1 away from that amount (applies to above logic for weekdays
            // as well, and months, and month-days).
            dt.AddDays(doy-1);
        }

        // Time-based units like hour, minute, etc. start at zero.
        if (have_H) {
            dt.AddHours(H);
        }
        else if (have_I) {
            // Just assume AM if neither am or pm is set.
            // no need to test for have_am in this context
            // - it's implied in the 'else' condition.
            if (I == 12) I = 0;
            dt.AddHours(I + (have_pm) ? 12 : 0);
        }
        else if (have_k) {
            dt.AddHours(k);
        }
        else if (have_l) {
            dt.AddHours(l + (have_pm) ? 12 : 0);
        }

        if (have_M) {
            dt.AddMinutes(M);
        }
        
        if (have_S) {
            dt.AddSeconds(S);
        }
        
        if (have_ms) {
            dt.AddMilliseconds(ms);
        }

        else if (have_us) {
            dt.AddMicroseconds(us);
        }

        else if (have_ns) {
            dt.AddNanoseconds(us);
        }

        *this = dt;
    }

    DateTime &operator=(const DateTime<Calendar, Clock, Period> &other) {
        // Guard self assignment
        if (this == &other)
            return *this;
        this->tp = other.tp;
        this->tz = other.tz;
        return *this;
    }

    static DateTime Now() {
        std::chrono::time_point<Clock> now = Clock::now();
        return DateTime(now);
    }

    long long TimeSinceEpoch() const {
        return static_cast<long long>(tp.time_since_epoch) - unix_offset;
    };

    // Our epoch is always the same as Unix epoch.
    static DateTime Epoch() { return DateTime(1970, 1, 1); }

    // Assumes the week always starts on monday.
    static int DayOfWeekISO(int y, int m, int d) {
        int ret = (d += m < 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4- y/100 + y/400)%7;
        if (ret == 0) // sunday
            ret = 7;
        return ret;
    }

    // Assumes the week always starts on Sunday (as in C and POSIX)
    static int DayOfWeek(int y, int m, int d) {
        int ret = (d += m < 3 ? y-- : y - 2, 23*m/9 + d + 4 + y/4- y/100 + y/400)%7;
        return ret;
    }

    static int GetDaysInMonth(int year, int month) {
        int numDays;

        switch (month) {
            case 1:
            case 3:
            case 5:
            case 7:
            case 8:
            case 10:
            case 12:
                numDays = 31;
                break;
            case 4:
            case 6:
            case 9:
            case 11:
                numDays = 30;
                break;
            case 2:
                numDays = IsLeap(year) ? 29 : 28;
                break;
            default:
                numDays = 0;
                break;
        }
        return numDays;
    }

    static bool IsLeap(int year) {
        return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    }

    static int GetDayOfYear(int year, int month, int day) {
        int i = 0;
        int doy = 0;
        for (i = 0; i < month-1; i++) {
            doy += DateTime::GetDaysInMonth(year, i+1);
        }
        return doy + day;
    }

    static int WeeksInYear(int y) {
        int py = (y + y/4 - y/100 + y/400) % 7;
        int py1 = ((y-1) + (y-1)/4 - (y-1)/100 + (y-1)/400) % 7;
        if (py == 4) return 53;
        if (py1 == 3) return 53;
        else return 52;
    }

    // Calculates the week number but assumes the first week has at least 4 days in it.
    static int ISOWeek(int y, int m, int d) {
        int w = (10 + DateTime::GetDayOfYear(y, m, d)  - DateTime::DayOfWeekISO(y, m, d))/7;
        int weeks = DateTime::WeeksInYear(y);
        if (w < 1) return DateTime::WeeksInYear(y-1);
        else if (w > weeks) return 1;
        else return w;
    }

    // Calculates the week number but assumes weeks start on a Sunday.
    static int SundayWeek(int y, int m, int d) {
        int i;
        for (i = 1; i <= 7; i++) {
            if (DateTime::DayOfWeek(y, 1, i) == 0) break;
        }
        int offset = i-1;
        int doy = DateTime::GetDayOfYear(y, m, d);
        return (doy+offset)/7 + 1;
    }

    // Calculates the week number but assumes weeks start on a Monday.
    static int MondayWeek(int y, int m, int d) {
        int i;
        for (i = 1; i <= 7; i++) {
            if (DateTime::DayOfWeek(y, 1, i) == 1) break;
        }
        int offset = i-1;
        int doy = DateTime::GetDayOfYear(y, m, d);
        return (doy+offset)/7 + 1;
    }

    Timezone TimeZone() const { return tz; }

    int Year() const {
        // time_t uses a 64-bit type on modern Windows (compiled with VS >= 2005),
        // modern MacOS, modern 64-bit Linux and Unix flavors[1], and 32-bit Linux since 5.x.
        // Some 32-bit BSD flavors are still using 32-bit time-t though, but this is the most
        // portable solution.
        //
        // In other words, this should be safe from the Year 2038 problem.
        //
        // We only use time_t for date primitives (e.g. year, month, day, week), not time primitives
        // such as hour, minute, second.
        //
        // [0] https://learn.microsoft.com/en-us/cpp/c-runtime-library/time-management?view=msvc-170
        // [1] https://en.wikipedia.org/wiki/Year_2038_problem
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return tm_.tm_year + 1900;
        //TODO ideally we should be checking that the year is not zero
        //because that is invalid in the Gregorian calendar, but people will
        //only encounter this problem with very ancient dates, and also, such
        //a check should be designated to a future Gregorian calendar class.
    }

    int Month() const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return tm_.tm_mon + 1; // month is zero-based
    }

    std::string MonthString(const std::string& locale = "C") const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return LocaleData::GetLongMonth(locale.c_str(), tm_.tm_mon);
    }

    std::string MonthShortString(const std::string& locale = "C") const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return LocaleData::GetShortMonth(locale.c_str(), tm_.tm_mon);
    }

    int EndOfMonthDay() const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return GetDaysInMonth(tm_.tm_mon, tm_.tm_year);
    }

    int Week() const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return DateTime::ISOWeek(tm_.tm_year + 1900, tm_.tm_mon + 1, tm_.tm_mday);
    }

    int Day() const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return tm_.tm_mday; // day is one-based
    }

    int DayOfWeek() const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return tm_.tm_wday + 1; // week day is zero-based
    }

    std::string DayOfWeekString(const std::string& locale = "C") const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        struct tm tm_;
        gmtime_r(&t, &tm_);
        int first_weekday = LocaleData::GetFirstWeekday(locale.c_str());
        return LocaleData::GetLongWeekday(locale.c_str(), (tm_.tm_wday + first_weekday - 1) % 7);
    }

    std::string DayOfWeekShortString(const std::string& locale = "C") const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        struct tm tm_;
        gmtime_r(&t, &tm_);
        int first_weekday = LocaleData::GetFirstWeekday(locale.c_str());
        return LocaleData::GetShortWeekday(locale.c_str(), (tm_.tm_wday + first_weekday - 1) % 7);
    }

    int DayOfYear() const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return static_cast<int>(tm_.tm_yday + 1); // week day is zero-based
    }

    int Hour() const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return tm_.tm_hour;
    }

    int Minute() const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return tm_.tm_min;
    }

    int Second() const {
        time_t t = static_cast<time_t>(static_cast<long long>(
                    tp.time_since_epoch().count())/Period::den - unix_offset);
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return tm_.tm_sec;
    }

    int Millisecond() const {
        if (Period::den < 1000) return 0;
        return static_cast<int>(tp.time_since_epoch().count() / (Period::den/1000) % 1000);
    }

    int Microsecond() const {
        if (Period::den < 1000000) return 0;
        return static_cast<int>(tp.time_since_epoch().count() / (Period::den/1000000) % 1000000);
    }

    int Nanosecond() const {
        if (Period::den < 1000000000) return 0;
        return static_cast<int>(tp.time_since_epoch().count() / (Period::den/1000000000) % 1000000000);
    }

    DateTime<Calendar, Clock, Period> ToUTC() const {
        int gmtoff = tz.gmtoff[0]; // in seconds
        DateTime d = *this;
        d.tz = UTC_Timezone().tz;
        d.SubSeconds(gmtoff);
        return d;
    }

    DateTime<Calendar, Clock, Period> ToTimeZone(Timezone newtz) const {
        int gmtoff = tz.gmtoff[0]; // in seconds
        int newgmtoff = newtz.gmtoff[0]; // in seconds
        DateTime d = *this;
        d.tz = newtz;
        // d.SubSeconds(gmtoff).AddSeconds(newgmtoff);
        d.AddSeconds(newgmtoff - gmtoff); // Should be equivalent
        return d;
    }

    std::string ToDateTime(const std::string& locale = "C") const {
        std::string _format = LocaleData::GetDateTimeFormat(locale.c_str());
        return ToString(_format);
    }


    // Prints the string in date(1) format.
    std::string ToDate1(const std::string& locale = "C") const {
        std::string _format = LocaleData::GetDate1Format(locale.c_str());
        return ToString(_format);
    }

    // C locale uses %y, which is unsafe for old&new dates.
    // If you want 4-digit years should use en_US or another locale.
    std::string ToDate(const std::string& locale = "C") const {
        std::string _format = LocaleData::GetDateFormat(locale.c_str());
        return ToString(_format);
    }

    std::string ToTime24(const std::string& locale = "C") const {
        std::string _format = LocaleData::GetTime24Format(locale.c_str());
        return ToString(_format);
    }

    std::string ToTime12(const std::string& locale = "C") const {
        std::string _format = LocaleData::GetTime12Format(locale.c_str());
        return ToString(_format);
    }
    
    // Defaults to ISO8601
    std::string ToISO8601String() const { return ToString("%Y-%m-%dT%H:%M:%SZ%z"); }
    std::string ToString() const { return ToISO8601String(); }

    std::string toDate() const { return ToString("%Y-%m-%d"); }

    // This function will throw away fractional seconds.
    time_t ToTimeT() const {
        time_t t = static_cast<time_t>(static_cast<long long>(tp.time_since_epoch().count()) - unix_offset);
        return t;
    }

    // This function will throw away fractional seconds.
    struct tm ToTimeTM() const {
        time_t t = static_cast<time_t>(static_cast<long long>(tp.time_since_epoch()) - unix_offset);
        struct tm tm_;
        gmtime_r(&t, &tm_);
        return tm_;
    }

#ifdef X_DATETIME_WITH_BOOST
    boost::posix_time::ptime ToBoostPTime() const {
        boost::posix_time::ptime p = boost::posix_time::from_time_t(ToTimeT());
#ifdef BOOST_DATE_TIME_HAS_NANOSECONDS
        p += boost::posix_time::nanoseconds(Nanosecond());
#else
        p += boost::posix_time::microseconds(Microsecond());
#endif /* BOOOST_DATE_TIME_HAS_NANOSECONDS */
        return p;
    }
#endif /* X_DATETIME_WITH_BOOST */

#ifdef X_DATETIME_WITH_ABSEIL
    absl::CivilSecond ToAbseilCivilSecond() const {
        absl::CivilSecond civ(Year(), Month(), Day(),
                Hour(), Minute(), Second());
        return civ;
    }

    absl::CivilMinute ToAbseilCivilMinute() const {
        absl::CivilMinute civ(Year(), Month(), Day(),
                Hour(), Minute());
        return civ;
    }

    absl::CivilHour ToAbseilCivilHour() const {
        absl::CivilHour civ(Year(), Month(), Day(), Hour());
        return civ;
    }

    absl::CivilDay ToAbseilCivilDay() const {
        absl::CivilDay civ(Year(), Month(), Day());
        return civ;
    }

    absl::Time ToAbseilTime() const {
        return absl::FromChrono(ToChrono());
    }
#endif /* X_DATETIME_WITH_ABSEIL */

#ifdef X_DATETIME_WITH_POCO
    Poco::DateTime ToPoco() const {
        Poco::DateTime p(Year(), Month(), Day(),
                Hour(), Minute(), Second(),
                Microsecond()/1000, Microsecond() % 1000);
        return p;
    }
#endif /* X_DATETIME_WITH_POCO */


#ifdef X_DATETIME_WITH_GLIB
    GDateTime* ToGLib() const {
        return g_date_time_new_utc(Year(), Month(), Day(),
                Hour(), Minute(), Second());
    }
#endif

    std::chrono::time_point<Clock> ToChrono() const {
        auto tp_ = tp;
        long long count = static_cast<long long>(tp.time_since_epoch().count());
        long long q = count / Period::den;
        long long r = count % Period::den;
        q -= unix_offset;
        if (q > LLONG_MAX/ClockPeriod::den || q < LLONG_MIN/ClockPeriod::den) {
            throw MalformedDateTime("Conversion to std::chrono::time_point is not possible without overflow.");
        }
        q *= ClockPeriod::den;
        if (Period::den > ClockPeriod::den) {
            r /= Period::den / ClockPeriod::den;
        }
        else {
            r *= ClockPeriod::den / Period::den;
        }
        if (q > LLONG_MAX - r || q < LLONG_MIN + r) {
            throw MalformedDateTime("Conversion to std::chrono::time_point is not possible without overflow.");
        }
        q += r;
        return time_point(clock_duration(q));
    }

    // Convert a date and time to string, strictly following strftime(3) format conventions.
    // This string is encoded at UTF8 which means that characters could take more than 1 byte.
    // However, it is guarenteed that only the ASCII characters will use bytes 0x00 to 0x7F.
    std::string ToString(const std::string &fmt, const std::string& locale = "C") const {
        std::string s;


        bool format = false; // Encountered %?
        bool era = false; // Encountered %E?
        bool alt = false; // Encountered %O?
        for (char c: fmt) {
            switch (c) {
                case '%':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += c;
                    format = false;
                }
                else {
                    format = true;
                }
                break;

                case '1':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int ms = Millisecond();
                    if (ms < 100) s += "0";
                    if (ms < 10) s += "0";
                    s += std::to_string(ms);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int ms = Millisecond();
                    if (ms < 100) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (ms < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), ms);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case '2':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int us = Microsecond();
                    if (us < 100000) s += "0";
                    if (us < 10000) s += "0";
                    if (us < 1000) s += "0";
                    if (us < 100) s += "0";
                    if (us < 10) s += "0";
                    s += std::to_string(us);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int us = Microsecond();
                    if (us < 100000) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (us < 10000) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (us < 1000) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (us < 100) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (us < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), us);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case '3':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int ns = Nanosecond();
                    if (ns < 100000000) s += "0";
                    if (ns < 10000000) s += "0";
                    if (ns < 1000000) s += "0";
                    if (ns < 100000) s += "0";
                    if (ns < 10000) s += "0";
                    if (ns < 1000) s += "0";
                    if (ns < 100) s += "0";
                    if (ns < 10) s += "0";
                    s += std::to_string(ns);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int ns = Nanosecond();
                    if (ns < 100000000) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (ns < 10000000) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (ns < 1000000) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (ns < 100000) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (ns < 10000) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (ns < 1000) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (ns < 100) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (ns < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), ns);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'a':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += DayOfWeekShortString();
                    format = false;
                }
                else {
                    s += c;
                }
                break;


                case 'A':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += DayOfWeekString();
                    format = false;
                }
                else {
                    s += c;
                }
                break;


                case 'b':
                case 'h':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += MonthShortString();
                    format = false;
                }
                else {
                    s += c;
                }
                break;


                case 'B':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += MonthString();
                    format = false;
                }
                else {
                    s += c;
                }
                break;
                

                case 'c':
                if (alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format || era) {
                    s += ToString(LocaleData::GetDateTimeFormat(locale.c_str()), locale);
                    format = false;
                    era = false;
                }
                else {
                    s += c;
                }
                break;

                case 'C':
                if (alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || era
#endif
                        ) {
                    int year = Year() / 100;
                    if (year < 10) s += "0";
                    s += std::to_string(year);
                    format = false;
                    era = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (era) {
                    int year = Year() / 100;
                    if (year < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), year);
                    era = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'd':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int day = Day();
                    if (day < 10) s += "0";
                    s += std::to_string(day);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int day = Day();
                    if (day < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), day);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'D':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += ToString("%m/%d/%y", locale);
                    format = false;
                }
                else {
                    s += c;
                }
                break;

                case 'e':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int day = Day();
                    if (day < 10) s += " ";
                    s += std::to_string(day);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    std::string tmp = LocaleData::GetNumber(locale.c_str(), Day());
                    std::string cmp = LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (!strncmp(tmp.data(), cmp.data(), cmp.length())) tmp[0] = ' ';
                    s += tmp;
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;


                case 'E':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                   era = true;
                }
                else {
                    s += c;
                }
                break;

                case 'F':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += ToString("%Y-%m-%d", locale);
                    format = false;
                }
                else {
                    s += c;
                }
                break;

                case 'G':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int day = Day();
                    int year = Year();
                    int week = DateTime::ISOWeek(year, Month(), day);
                    if (week >= 52 && day < 7) {
                        // This is January of the next year, %G wants us to
                        // make it the previous year.
                        --year;
                    }
                    s += std::to_string(year);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int day = Day();
                    int year = Year();
                    int week = DateTime::ISOWeek(year, Month(), day);
                    if (week >= 52 && day < 7) {
                        // This is January of the next year, %G wants us to
                        // make it the previous year.
                        --year;
                    }
                    if (year < 0) { s += "-"; year = -year; }
                    if (year < 1000) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (year < 100) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (year < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), year);
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'g':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int day = Day();
                    int year = Year();
                    int week = DateTime::ISOWeek(year, Month(), day);
                    if (week >= 52 && day < 7) {
                        // This is January of the next year, %G wants us to
                        // make it the previous year.
                        --year;
                    }
                    year =  year % 100;
                    if (year < 10) s += "0";
                    s += std::to_string(year);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int day = Day();
                    int year = Year();
                    int week = DateTime::ISOWeek(year, Month(), day);
                    if (week >= 52 && day < 7) {
                        // This is January of the next year, %G wants us to
                        // make it the previous year.
                        --year;
                    }
                    year =  year % 100;
                    if (year < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), year);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'H':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int hour = Hour();
                    if (hour < 10) s += "0";
                    s += std::to_string(hour);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int hour = Hour();
                    if (hour < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), hour);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;
                
                case 'I':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int hour = Hour() % 12;
                    if (hour == 0) hour = 12;
                    if (hour < 10) s += "0";
                    s += std::to_string(hour);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int hour = Hour() % 12;
                    if (hour == 0) hour = 12;
                    if (hour < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), hour);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'j':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int doy = DayOfYear();
                    if (doy < 100) s += "0";
                    if (doy < 10) s += "0";
                    s += std::to_string(doy);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int doy = DayOfYear();
                    if (doy < 100) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (doy < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), doy);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'k':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int hour = Hour();
                    if (hour < 10) s += " ";
                    s += std::to_string(hour);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int hour = Hour();
                    if (hour < 10) s += " ";
                    s += LocaleData::GetNumber(locale.c_str(), hour);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'l':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int hour = Hour() % 12;
                    if (hour == 0) hour = 12;
                    if (hour < 10) s += " ";
                    s += std::to_string(hour);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int hour = Hour() % 12;
                    if (hour == 0) hour = 12;
                    if (hour < 10) s += " ";
                    s += LocaleData::GetNumber(locale.c_str(), hour);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'm':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int month = Month();
                    if (month < 10) s += "0";
                    s += std::to_string(month);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int month = Month();
                    if (month < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), month);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'M':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int minute = Minute();
                    if (minute < 10) s += "0";
                    s += std::to_string(minute);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int minute = Minute();
                    if (minute < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), minute);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'n':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += "\n";
                    format = false;
                }
                else {
                    s += c;
                }
                break;
                
                case 'O':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    alt = true;
                    format = false;
                }
                else {
                    s += c;
                }
                break;

                case 'p':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    // first get the 24-hour time
                    int hour = Hour();
                    if (hour < 12) {
                        s += LocaleData::GetAM(locale.c_str());
                    }
                    else {
                        s += LocaleData::GetPM(locale.c_str());
                    }
                    format = false;
                }
                else {
                    s += c;
                }
                break;

                case 'P':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    // first get the 24-hour time
                    int hour = Hour();
                    if (hour < 12) {
                        std::string data = LocaleData::GetAM(locale.c_str());
                        std::transform(data.begin(), data.end(), data.begin(),
                            [](unsigned char c){ return std::tolower(c); });
                        s += data;
                    }
                    else {
                        std::string data = LocaleData::GetPM(locale.c_str());
                        std::transform(data.begin(), data.end(), data.begin(),
                            [](unsigned char c){ return std::tolower(c); });
                        s += data;
                    }
                    format = false;
                }
                else {
                    s += c;
                }
                break;

                case 'r':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += ToString(LocaleData::GetTime12Format(locale.c_str()), locale);
                    format = false;
                }
                else {
                    s += c;
                }
                break;

                case 'R':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += ToString(LocaleData::GetTime24Format(locale.c_str()), locale);
                    format = false;
                }
                else {
                    s += c;
                }
                break;

                case 's':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    time_t ssepoch = ToTimeT();
                    s += std::to_string(ssepoch);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    time_t ssepoch;
                    s += LocaleData::GetNumber(locale.c_str(), ssepoch);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;


                case 'S':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int second = Second();
                    if (second < 10) s += "0";
                    s += std::to_string(second);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int second = Second();
                    if (second < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), second);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 't':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += "\t";
                    format = false;
                }
                else {
                    s += c;
                }
                break;

                case 'T':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += ToString("%H:%M:%S", locale);
                    format = false;
                }
                else {
                    s += c;
                }
                break;

                case 'u':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    // 'u' must account that Sunday is 7 and other weekdays are the same as 'w'.
                    int dow = DayOfWeek();
                    if (dow == 0) dow == 7;
                    s += std::to_string(dow);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    s += LocaleData::GetNumber(locale.c_str(), DayOfWeek());
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'U':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int day = Day();
                    int year = Year();
                    int week = DateTime::SundayWeek(year, Month(), day)-1;
                    if (week < 10) s += "0";
                    s += std::to_string(week);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int day = Day();
                    int year = Year();
                    int week = DateTime::SundayWeek(year, Month(), day)-1;
                    if (week < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), week);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'V':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int day = Day();
                    int year = Year();
                    int week = DateTime::ISOWeek(year, Month(), day)-1;
                    if (week < 10) s += "0";
                    s += std::to_string(week);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int day = Day();
                    int year = Year();
                    int week = DateTime::ISOWeek(year, Month(), day)-1;
                    if (week < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), week);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'w':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    s += std::to_string(DayOfWeek());
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    s += LocaleData::GetNumber(locale.c_str(), DayOfWeek());
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'W':
                if (era) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || alt
#endif
                        ) {
                    int day = Day();
                    int year = Year();
                    int week = DateTime::MondayWeek(year, Month(), day)-1;
                    if (week < 10) s += "0";
                    s += std::to_string(week);
                    format = false;
                    alt = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (alt) {
                    int day = Day();
                    int year = Year();
                    int week = DateTime::MondayWeek(year, Month(), day)-1;
                    if (week < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), week);
                    alt = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'x':
                if (alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format || era) {
                    s += ToString(LocaleData::GetDateFormat(locale.c_str()), locale);
                    format = false;
                    era = false;
                }
                else {
                    s += c;
                }
                break;

                case 'X':
                if (alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format || era) {
                    s += ToString(LocaleData::GetTime24Format(locale.c_str()), locale);
                    format = false;
                    era = false;
                }
                else {
                    s += c;
                }
                break;

                case 'y':
                if (alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || era
#endif
                        ) {
                    int year = Year() % 100;
                    if (year < 0) { s += "-"; year = -year; }
                    if (year < 10) s += "0";
                    s += std::to_string(year);
                    format = false;
                    era = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (era) {
                    int year = Year() % 100;
                    if (year < 0) { s += "-"; year = -year; }
                    if (year < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), year);
                    era = false;
                }
#endif
                else {
                    s += c;
                }
                break;

                case 'Y':
                if (alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format
#ifdef X_DATETIME_NO_LOCALES
                    || era
#endif
                        ) {
                    int year = Year();
                    if (year < 0) { s += "-"; year = -year; }
                    if (year < 1000) s += "0";
                    if (year < 100) s += "0";
                    if (year < 10) s += "0";
                    s += std::to_string(year);
                    format = false;
                    era = false;
                }
#ifndef X_DATETIME_NO_LOCALES
                else if (era) {
                    int year = Year();
                    if (year < 0) { s += "-"; year = -year; }
                    if (year < 1000) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (year < 100) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    if (year < 10) s += LocaleData::GetAltDigit(locale.c_str(), 0);
                    s += LocaleData::GetNumber(locale.c_str(), year);
                    era = false;
                }
#endif
                else {
                    s += c;
                }
                break;


                case 'z':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    int gmtoff = tz.gmtoff[0] / 60;
                    int mm = gmtoff % 60;
                    mm = (mm >= 0) ? mm : -mm;
                    int hh = gmtoff % 60;
                    if (hh < 0) { s += "-"; hh = -hh; }
                    else { s += "+"; }
                    if (hh < 10) s += "0";
                    s += std::to_string(hh);
                    if (mm < 10) s += "0";
                    s += std::to_string(mm);
                    format = false;
                }
                else {
                    s += c;
                }
                break;

                case 'Z':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += tz.name;
                    format = false;
                }
                else {
                    s += c;
                }
                break;

                case '+':
                if (era || alt) { throw MalformedDateTime("Bad format specifier"); }
                if (format) {
                    s += ToString(LocaleData::GetDate1Format(locale.c_str()), locale);
                    format = false;
                }
                else {
                    s += c;
                }
                break;

                default:
                if (format) { throw MalformedDateTime("Bad format specifier"); }
                s += c;

            }
        }

         return s;
    }

    void AddYears(long long _years) {
        if (_years == 0) return;
        // We want to land on the same month and day as we are on right now.
        // But if today is Feb 29 then we should land on March 1
        long long year1 = Year();
        long long year2 = year1 + _years;
        
        long long leapyears = _years / 4;

        // If today is a leap year and the dest year is also a leap year,
        // and we are after Feb 28, do not account for this year's leap
        // year because we have already passed it.
        if (IsLeap(year1) && IsLeap(year2) && DayOfYear() > 59) {
            leapyears = static_cast<long long>(leapyears) - 1;
        }
        else {
            leapyears = static_cast<long long>(leapyears);
        }

        long long days = (leapyears) * 366 + (_years - leapyears) * 365;
        *this += Days<Period>(days);
    }

    // This will simply add the number of days in each long longermediate month,
    // beginning with the current month but not including the
    // number of days in the `_months`th month.
    // Day will remain the same.
    // If we are subtracting, take the number of days in the PREVIOUS months.
    // do not count the CURRENT month.
    void AddMonths(long long _months) {
        if (_months == 0) return;
        AddYears(_months/12);
        long long leftover_months = _months % 12;
        if (leftover_months < 0) leftover_months = -leftover_months;
        long long month = Month();
        long long days = 0;
        long long year = Year();
        
        // There are less than 12 months we have to add so we can use iteration now.
        long long counter = month;
        if (_months > 0) {
            for (long long i = 0; i < leftover_months; ++i) {
                if (counter == 0) {
                    counter = 12;
                }
                else if (counter == 1 && i != 0) {
                    ++year;
                }
                days += GetDaysInMonth(year, counter);
                ++counter;
            }
        }
        else {
            for (long long i = 0; i < leftover_months; ++i) {
                if (counter == 0) {
                    counter = 12;
                    if (i != 0) --year;
                }
                days -= GetDaysInMonth((counter == 1) ? year-1 : year, counter-1);
                --counter;
            }
        }
        AddDays(days);
    }

    void AddWeeks(long long _weeks) {
        *this += Days<Period>(_weeks * 7);
    }

    void AddDays(long long _days) {
        *this += Days<Period>(_days);
    }

    void AddHours(long long _hours) {
        *this += Hours<Period>(_hours);
    }

    void AddMinutes(long long _minutes) {
        *this += Minutes<Period>(_minutes);
    }

    void AddSeconds(long long _seconds) {
        *this += Seconds<Period>(_seconds);
    }

    void AddMilliseconds(long long _milliseconds) {
        *this += Milliseconds<Period>(_milliseconds);
    }

    void AddMicroseconds(long long _microseconds) {
        *this += Microseconds<Period>(_microseconds);
    }

    void AddNanoseconds(long long _nanoseconds) {
        *this += Nanoseconds<Period>(_nanoseconds);
    }

    void SubYears(long long _years) { AddYears(-_years); }

    void SubMonths(long long _months) { AddMonths(-_months); }

    void SubWeeks(long long _weeks) { AddWeeks(-_weeks); }

    void SubDays(long long _days) { AddDays(-_days); }

    void SubHours(long long _hours) { AddHours(-_hours); }

    void SubMinutes(long long _minutes) { AddMinutes(-_minutes); }

    void SubSeconds(long long _seconds) { AddSeconds(-_seconds); }

    void SubMilliseconds(long long _milliseconds) { AddMilliseconds(-_milliseconds); }

    void SubMicroseconds(long long _microseconds) { AddMicroseconds(-_microseconds); }

    void SubNanoseconds(long long _nanoseconds) { AddNanoseconds(-_nanoseconds); }

    template <typename Calendar_, typename Clock_, typename Period_>
    friend DateTime<Calendar_, Clock_, Period_> operator+(const DateTime<Calendar_, Clock_, Period_> &a, const TimeDelta<Period_> &b);

    template <typename Calendar_, typename Clock_, typename Period_>
    friend DateTime<Calendar_, Clock_, Period_> operator+(const TimeDelta<Period_> &a, const DateTime<Calendar_, Clock_, Period_> &b);

    template <typename Calendar_, typename Clock_, typename Period_>
    friend TimeDelta<Period_> operator-(const DateTime<Calendar_, Clock_, Period_> &a, const DateTime<Calendar_, Clock_, Period_> &b);

    template <typename Calendar_, typename Clock_, typename Period_>
    friend DateTime<Calendar_, Clock_, Period_> operator-(const DateTime<Calendar_, Clock_, Period_> &a, const TimeDelta<Period_> &b);

    DateTime &operator+=(const TimeDelta<Period> &rhs) {
        tp += rhs.d;
        return *this;
    }

    DateTime &operator-=(const TimeDelta<Period> &rhs) {
        tp -= rhs.d;
        return *this;
    }

    // We ignore the offset because that is clock dependent, and these
    // functions only allow comparison with datetimes having the same
    // clock anyway.
    // However, we do NOT ignore the timezone. We always subtract the timezone
    // offsets from both times before doing the comparison.
    bool operator==(const DateTime<Calendar, Clock, Period> &rhs) const {
        return tp - clock_duration(tz.gmtoff[0] * Period::den) ==
            rhs.tp - clock_duration(rhs.tz.gmtoff[0] * Period::den);
    }
    bool operator!=(const DateTime<Calendar, Clock, Period> &rhs) const {
        return tp - clock_duration(tz.gmtoff[0] * Period::den) !=
            rhs.tp - clock_duration(rhs.tz.gmtoff[0] * Period::den);
    }
    bool operator<(const DateTime<Calendar, Clock, Period> &rhs) const {
        return tp - clock_duration(tz.gmtoff[0] * Period::den) <
            rhs.tp - clock_duration(rhs.tz.gmtoff[0] * Period::den);
    }
    bool operator>(const DateTime<Calendar, Clock, Period> &rhs) const {
        return tp - clock_duration(tz.gmtoff[0] * Period::den) >
            rhs.tp - clock_duration(rhs.tz.gmtoff[0] * Period::den);
    }
    bool operator<=(const DateTime<Calendar, Clock, Period> &rhs) const {
        return tp - clock_duration(tz.gmtoff[0] * Period::den) <=
            rhs.tp - clock_duration(rhs.tz.gmtoff[0] * Period::den);
    }
    bool operator>=(const DateTime<Calendar, Clock, Period> &rhs) const {
        return tp - clock_duration(tz.gmtoff[0] * Period::den) >=
            rhs.tp - clock_duration(rhs.tz.gmtoff[0] * Period::den);
    }

    template <typename Char, typename Traits, typename Calendar_, typename Clock_, typename Period_>
    friend std::basic_ostream<Char, Traits> &operator<<(std::basic_ostream<Char, Traits> &os,
            const DateTime<Calendar_, Clock_, Period_> &obj);
};

// Extended version of date_duration which also supports the factional
// part of the date, hence can be used for e.g. hourly events as well.
// It also supports durations with endpoints of different timezones,
// by virtue of DateTime storing the UTC times internally.
template <typename Calendar = GregorianCalendar, typename Clock = std::chrono::system_clock,
         typename Period = typename std::chrono::time_point<Clock>::period>
class DateTimePeriod {
private:
    DateTime<Calendar, Clock, Period> d1, d2;

public:
    DateTimePeriod() {}
    DateTimePeriod(const DateTime<Calendar, Clock, Period> &a, const DateTime<Calendar, Clock, Period> &b) {
        if (b <= a) {
            throw MalformedDateTime(
                    "End time must be strictly greater than start time");
        }
        d1 = a;
        d2 = b;
    }
    DateTimePeriod(const DateTime<Calendar, Clock, Period> &a, const TimeDelta<Period> &duration) {
        d1 = a;
        d2 = a + duration;
    }
    DateTimePeriod(const DateTimePeriod<Calendar, Clock, Period> &other) {
        d1 = other.d1;
        d2 = other.d2;
    }

    DateTimePeriod<Calendar, Clock, Period> &operator=(const DateTimePeriod<Calendar, Clock, Period> &other) {
        // Guard self assignment
        if (this == &other)
            return *this;
        this->d1 = other.d1;
        this->d2 = other.d2;
        return *this;
    }

    DateTime<Calendar, Clock, Period> First() const { return d1; }
    DateTime<Calendar, Clock, Period> Last() const { return d2; }
    
    TimeDelta<Period> Duration() const { return d2 - d1; }

    void Shift(const TimeDelta<Period> &duration) {
        d1 += duration;
        d2 += duration;
    }

    void Expand(const TimeDelta<Period> &duration) {
        d1 -= duration;
        d2 += duration;
    }

    DateTimePeriod<Calendar, Clock, Period> Shifted(const TimeDelta<Period> &duration) const {
        DateTimePeriod<Calendar, Clock, Period> d = *this;
        d.d1 += duration;
        d.d2 += duration;
        return d;
    }

    DateTimePeriod<Calendar, Clock, Period> Expanded(const TimeDelta<Period> &duration) const {
        DateTimePeriod<Calendar, Clock, Period> d = *this;
        d.d1 -= duration;
        d.d2 += duration;
        return d;
    }

    bool Contains(const DateTime<Calendar, Clock, Period> &other) const {
        return d1 <= other && other <= d2;
    }

    bool Contains(const DateTimePeriod<Calendar, Clock, Period> &other) const {
        return d1 <= other.d1 && other.d2 <= d2;
    }

    bool Overlaps(const DateTimePeriod<Calendar, Clock, Period> &other) const {
        return (other.d1 <= d2 && other.d1 >= d1 ) || (d1 <= other.d2 && d1 >= other.d1);
    }

    bool Consecutive(const DateTimePeriod<Calendar, Clock, Period> &other) const {
        return other.d2 == d1 || d2 == other.d1;
    }

    bool operator==(const DateTimePeriod<Calendar, Clock, Period> &rhs) const { return d1 == rhs.d1 && d2 == rhs.d2; }
    bool operator!=(const DateTimePeriod<Calendar, Clock, Period> &rhs) const { return d1 != rhs.d1 || d2 != rhs.d2; }

    // Strictly less, Strictly greater, etc., won't return equal for overlapping durations.
    bool operator<(const DateTimePeriod<Calendar, Clock, Period> &rhs) const { return d1 < rhs.d1 && d2 < rhs.d1; }
    bool operator>(const DateTimePeriod<Calendar, Clock, Period> &rhs) const { return d1 > rhs.d2 || d2 > rhs.d2; }
    bool operator<=(const DateTimePeriod<Calendar, Clock, Period> &rhs) const { return d1 <= rhs.d1 && d2 <= rhs.d1; }
    bool operator>=(const DateTimePeriod<Calendar, Clock, Period> &rhs) const { return d1 >= rhs.d2 || d2 >= rhs.d2; }

    bool operator<(const DateTime<Calendar, Clock, Period> &rhs) const { return d1 < rhs && d2 < rhs; }
    bool operator>(const DateTime<Calendar, Clock, Period> &rhs) const { return d1 > rhs || d2 > rhs; }
    bool operator<=(const DateTime<Calendar, Clock, Period> &rhs) const { return d1 <= rhs && d2 <= rhs; }
    bool operator>=(const DateTime<Calendar, Clock, Period> &rhs) const { return d1 >= rhs || d2 >= rhs; }



    bool ComesBefore(const DateTime<Calendar, Clock, Period> &a) const { return a > d1; }

    bool ComesAfter(const DateTime<Calendar, Clock, Period> &a) const { return a < d2; }

    bool ComesDuring(const DateTime<Calendar, Clock, Period> &a) const {
        // Alias for Contains()
        return Contains(a);
    }

    // Equal to operator
    bool ComesBefore(const DateTimePeriod<Calendar, Clock, Period> &a) const { return a < d1; }

    bool ComesAfter(const DateTimePeriod<Calendar, Clock, Period> &a) const { return a > d2; }

    bool ComesDuring(const DateTimePeriod<Calendar, Clock, Period> &a) const {
        // Alias for Contains()
        return Contains(a);
    }

    void Expand(const DateTimePeriod<Calendar, Clock, Period> &other) {
        if (other.d1 < d1)
            d1 = other.d1;
        if (other.d2 > d2)
            d2 = other.d2;
    }

    void Contract(const DateTimePeriod<Calendar, Clock, Period> &other) {
        if (d1 < other.d1)
            d1 = other.d1;
        if (d2 > other.d2)
            d2 = other.d2;
    }

    DateTimePeriod<Calendar, Clock, Period> Expanded(const DateTimePeriod<Calendar, Clock, Period> &other) const {
        DateTimePeriod<Calendar, Clock, Period> d(d1, d2);
        if (other.d1 < d1)
            d.d1 = other.d1;
        if (other.d2 > d2)
            d.d2 = other.d2;
        return d;
    }

    DateTimePeriod<Calendar, Clock, Period> Contracted(const DateTimePeriod<Calendar, Clock, Period> &other) const {
        DateTimePeriod<Calendar, Clock, Period> d(d1, d2);
        if (d1 < other.d1)
            d.d1 = other.d1;
        if (d2 > other.d2)
            d.d2 = other.d2;
        return d;
    }

    DateTimePeriod<Calendar, Clock, Period> Union(const DateTimePeriod<Calendar, Clock, Period> &other) const {
        if (!Overlaps(other)) {
            throw MalformedDateTime("Periods do not overlap");
        }

        return Expanded(other);
    }

    DateTimePeriod<Calendar, Clock, Period> Intersection(const DateTimePeriod<Calendar, Clock, Period> &other) const {
        if (!Overlaps(other)) {
            throw MalformedDateTime("Periods do not overlap");
        }
        return Contracted(other);
    }

    template <typename Char, typename Traits, typename Calendar_, typename Clock_, typename Period_>
    friend std::basic_ostream<Char, Traits> &operator<<(std::basic_ostream<Char, Traits> &os, const DateTimePeriod<Calendar_, Clock_, Period_> &obj);

};


template <typename Char, typename Traits, typename Ratio>
static inline std::basic_ostream<Char, Traits> &
operator<<(std::basic_ostream<Char, Traits> &os, const TimeDelta<Ratio> &obj) {
    os << obj.ToString();
    return os;
}


template <typename Char, typename Traits, typename Calendar, typename Clock, typename Period>
static inline std::basic_ostream<Char, Traits> &
operator<<(std::basic_ostream<Char, Traits> &os, const DateTime<Calendar, Clock, Period> &obj) {
    os << obj.ToString();
    return os;
}

template <typename Char, typename Traits, typename Calendar, typename Clock, typename Period>
static inline std::basic_ostream<Char, Traits> &
operator<<(std::basic_ostream<Char, Traits> &os, const DateTimePeriod<Calendar, Clock, Period> &obj) {
    os << obj.d1 << " - " << obj.d2;
    return os;
}

template <typename Ratio>
static inline TimeDelta<Ratio> operator+(const TimeDelta<Ratio> &a, const TimeDelta<Ratio> &b) {
    return TimeDelta<Ratio>(a.d + b.d);
}

template <typename Ratio>
static inline TimeDelta<Ratio> operator-(const TimeDelta<Ratio> &a, const TimeDelta<Ratio> &b) {
    return TimeDelta<Ratio>(a.d - b.d);
}

template <typename Calendar, typename Clock, typename Period>
static inline DateTime<Calendar, Clock, Period> operator+(const DateTime<Calendar, Clock, Period> &a, const TimeDelta<Period> &b) {
    return DateTime<Calendar, Clock, Period>(a.tp + b.d);
}

template <typename Calendar, typename Clock, typename Period>
static inline DateTime<Calendar, Clock, Period> operator+(const TimeDelta<Period> &a, const DateTime<Calendar, Clock, Period> &b) {
    return DateTime<Calendar, Clock, Period>(b.tp + a.d);
}

template <typename Calendar, typename Clock, typename Period>
static inline TimeDelta<Period> operator-(const DateTime<Calendar, Clock, Period> &a, const DateTime<Calendar, Clock, Period> &b) {
    TimeDelta<Period> td;
    td.d = a.tp - b.tp;
    return td;
}

template <typename Calendar, typename Clock, typename Period>
static inline DateTime<Calendar, Clock, Period> operator-(const DateTime<Calendar, Clock, Period> &a, const TimeDelta<Period> &b) {
    return DateTime<Calendar, Clock, Period>(a.tp - b.d);
}

// The "D" stands for default, as declaring a templated class object without <>
// was illegal until C++17.
typedef TimeDelta<> TimeDeltaD;
typedef DateTime<> DateTimeD;
typedef DateTimePeriod<> DateTimePeriodD;

typedef TimeDelta<std::micro> TimeDeltaW;
typedef DateTime<GregorianCalendar, std::chrono::system_clock, std::micro> DateTimeW;
typedef DateTimePeriod<GregorianCalendar, std::chrono::system_clock, std::micro> DateTimePeriodW;


}
#endif /* X_DATETIME_H */
