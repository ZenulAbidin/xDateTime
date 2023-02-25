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

#define X_DATETIME_WITH_LOCALE_EN_US
#include "x_datetime.h"
#include <gtest/gtest.h>

using namespace xDateTime;

TEST(xDateTime, TimeDelta) {
  ASSERT_EQ((TimeDelta<std::ratio<1>>(1).ToString()), "1d 0h 0min 0s");
  ASSERT_EQ((TimeDelta<std::ratio<1>>(0, 1).ToString()), "0d 1h 0min 0s");
  ASSERT_EQ((TimeDelta<std::ratio<1>>(0, 0, 1).ToString()), "0d 0h 1min 0s");
  ASSERT_EQ((TimeDelta<std::ratio<1>>(0, 0, 0, 1).ToString()), "0d 0h 0min 1s");
  ASSERT_EQ((TimeDelta<std::nano>(0, 0, 0, 0, 1).ToString()), "0d 0h 0min 0s 1ns");
  ASSERT_EQ((TimeDelta<std::micro>(0, 0, 0, 0, 1).ToString()), "0d 0h 0min 0s 1μs");

  ASSERT_EQ(Nanoseconds(1).ToString(), "0d 0h 0min 0s 1ns");
  ASSERT_EQ(Microseconds<std::micro>(1).ToString(), "0d 0h 0min 0s 1μs");
  ASSERT_EQ(Milliseconds<std::milli>(1).ToString(), "0d 0h 0min 0s 1ms");
  ASSERT_EQ(Seconds<std::ratio<1>>(1).ToString(), "0d 0h 0min 1s");
  ASSERT_EQ(Minutes<std::ratio<1>>(1).ToString(), "0d 0h 1min 0s");
  ASSERT_EQ(Hours<std::ratio<1>>(1).ToString(), "0d 1h 0min 0s");
  ASSERT_EQ(Days<std::ratio<1>>(1).ToString(), "1d 0h 0min 0s");

  TimeDeltaD t(0);
  ASSERT_EQ(t.ToString(), "0d 0h 0min 0s 0ns");

  t.AddNanoseconds(1);
  ASSERT_EQ(t.ToString(), "0d 0h 0min 0s 1ns");

  t.AddMicroseconds(1);
  ASSERT_EQ(t.ToString(), "0d 0h 0min 0s 1001ns");

  t.AddMilliseconds(1);
  ASSERT_EQ(t.ToString(), "0d 0h 0min 0s 1001001ns");

  t.AddSeconds(1);
  ASSERT_EQ(t.ToString(), "0d 0h 0min 1s 1001001ns");
  
  t.AddMinutes(1);
  ASSERT_EQ(t.ToString(), "0d 0h 1min 1s 1001001ns");

  t.AddHours(1);
  ASSERT_EQ(t.ToString(), "0d 1h 1min 1s 1001001ns");

  t.AddDays(1);
  ASSERT_EQ(t.ToString(), "1d 1h 1min 1s 1001001ns");

  t.SubDays(1);
  ASSERT_EQ(t.ToString(), "0d 1h 1min 1s 1001001ns");

  t.SubHours(1);
  ASSERT_EQ(t.ToString(), "0d 0h 1min 1s 1001001ns");

  t.SubMinutes(1);
  ASSERT_EQ(t.ToString(), "0d 0h 0min 1s 1001001ns");

  t.SubSeconds(1);
  ASSERT_EQ(t.ToString(), "0d 0h 0min 0s 1001001ns");

  t.SubMilliseconds(1);
  ASSERT_EQ(t.ToString(), "0d 0h 0min 0s 1001ns");

  t.SubMicroseconds(1);
  ASSERT_EQ(t.ToString(), "0d 0h 0min 0s 1ns");

  t.SubNanoseconds(1);
  ASSERT_EQ(t.ToString(), "0d 0h 0min 0s 0ns");

  t.SubNanoseconds(1);
  ASSERT_EQ(t.ToString(), "0d 0h 0min 0s -1ns");

  t.SubDays(1);
  ASSERT_EQ(t.ToString(), "-1d 0h 0min 0s -1ns");

  t = t.Abs();
  ASSERT_EQ(t.ToString(), "1d 0h 0min 0s 1ns");

  t = TimeDeltaD(1, 2, 3, 4, 1234567);
  ASSERT_EQ(t.Days(), 1);
  ASSERT_EQ(t.Hours(), 2);
  ASSERT_EQ(t.Minutes(), 3);
  ASSERT_EQ(t.Seconds(), 4);
  ASSERT_EQ(t.Milliseconds(), 1);
  ASSERT_EQ(t.Microseconds(), 1234);
  ASSERT_EQ(t.Nanoseconds(), 1234567);

  ASSERT_EQ(t.TotalDaysW(), 1LL);
  ASSERT_EQ(t.TotalHoursW(), 1LL*24 + 2);
  ASSERT_EQ(t.TotalMinutesW(), (1LL*24 + 2) * 60 + 3);
  ASSERT_EQ(t.TotalSecondsW(), ((1LL*24 + 2) * 60 + 3) * 60 + 4);
  ASSERT_EQ(t.TotalMillisecondsW(), (((1LL*24 + 2) * 60 + 3) * 60 + 4) * 1000 + 1);
  ASSERT_EQ(t.TotalMicrosecondsW(), (((1LL*24 + 2) * 60 + 3) * 60 + 4) * 1000000 + 1234);
  ASSERT_EQ(t.TotalNanosecondsW(), (((1LL*24 + 2) * 60 + 3) * 60 + 4) * 1000000000 + 1234567);
  auto c = std::chrono::nanoseconds((((1LL*24 + 2) * 60 + 3) * 60 + 4) * 1000000000 + 1234567);
  ASSERT_EQ(t.ToChrono(), c);
  ASSERT_EQ(TimeDeltaD(c).ToChrono(), c);
}                                                                                                                                                                       
TEST(xDateTime, DateTime) {
  auto test = std::chrono::system_clock::now();

  DateTime<GregorianCalendar, std::chrono::high_resolution_clock> d =
      DateTime<GregorianCalendar, std::chrono::high_resolution_clock>::Epoch();
  ASSERT_EQ(d.Year(), 1970);
  ASSERT_EQ(d.Month(), 1);
  ASSERT_EQ(d.Day(), 1);
  ASSERT_EQ(d.Hour(), 0);
  ASSERT_EQ(d.Minute(), 0);
  ASSERT_EQ(d.Second(), 0);
  ASSERT_EQ(d.Millisecond(), 0);
  ASSERT_EQ(d.Microsecond(), 0);
  ASSERT_EQ(d.Nanosecond(), 0);

  d.AddNanoseconds(1);
  ASSERT_EQ(d.Nanosecond(), 1);

  d.AddMicroseconds(1);
  ASSERT_EQ(d.Microsecond(), 1);

  d.AddMilliseconds(1);
  ASSERT_EQ(d.Millisecond(), 1);

  d.AddSeconds(1);
  ASSERT_EQ(d.Second(), 1);

  d.AddMinutes(1);
  ASSERT_EQ(d.Minute(), 1);

  d.AddHours(1);
  ASSERT_EQ(d.Hour(), 1);

  d.AddDays(1);
  ASSERT_EQ(d.Day(), 2);

  d.AddWeeks(1);
  ASSERT_EQ(d.Day(), 9);

  d.AddMonths(1);
  ASSERT_EQ(d.Month(), 2);

  d.AddYears(4);
  ASSERT_EQ(d.Year(), 1974);
  ASSERT_EQ(d.Month(), 2);
  ASSERT_EQ(d.Day(), 9);

  d.AddYears(1);
  ASSERT_EQ(d.Year(), 1975);
  ASSERT_EQ(d.Month(), 2);
  ASSERT_EQ(d.Day(), 9);

  d.AddYears(1);
  ASSERT_EQ(d.Year(), 1976);
  ASSERT_EQ(d.Month(), 2);
  ASSERT_EQ(d.MonthString(), "February");
  ASSERT_EQ(d.MonthShortString(), "Feb");
  ASSERT_EQ(d.Day(), 9);
  ASSERT_EQ(d.DayOfYear(), 40);
  ASSERT_EQ(d.DayOfWeek(), 2);
  ASSERT_EQ(d.DayOfWeekString(), "Monday");
  ASSERT_EQ(d.DayOfWeekShortString(), "Mon");

  d.SubYears(1);
  ASSERT_EQ(d.Year(), 1975);
  ASSERT_EQ(d.Month(), 2);
  ASSERT_EQ(d.Day(), 9);

  d.SubYears(1);
  ASSERT_EQ(d.Year(), 1974);
  ASSERT_EQ(d.Month(), 2);
  ASSERT_EQ(d.Day(), 9);

  d.SubYears(4);
  ASSERT_EQ(d.Year(), 1970);
  ASSERT_EQ(d.Month(), 2);
  ASSERT_EQ(d.Day(), 9);

  d.SubMonths(1);
  ASSERT_EQ(d.Month(), 1);

  d.SubWeeks(1);
  ASSERT_EQ(d.Day(), 2);

  d.SubDays(1);
  ASSERT_EQ(d.Day(), 1);

  d.SubHours(1);
  ASSERT_EQ(d.Hour(), 0);

  d.SubMinutes(1);
  ASSERT_EQ(d.Minute(), 0);

  d.SubSeconds(1);
  ASSERT_EQ(d.Second(), 0);

  d.SubMilliseconds(1);
  ASSERT_EQ(d.Millisecond(), 0);

  d.SubMicroseconds(1);
  ASSERT_EQ(d.Microsecond(), 0);

  d.SubNanoseconds(1);
  ASSERT_EQ(d.Nanosecond(), 0);
  ASSERT_EQ(d, DateTimeD::Epoch());


  ASSERT_EQ(d.ToDate1(), "Thu Jan  1 00:00:00 UTC 1970");
  ASSERT_EQ(d.ToDateTime(), "Thu Jan  1 00:00:00 1970");
  ASSERT_EQ(d.ToDate(), "01/01/70");
  ASSERT_EQ(d.ToTime24(), "00:00:00");
  ASSERT_EQ(d.ToTime12(), "12:00:00 AM");
  ASSERT_EQ(d.ToISO8601String(), "1970-01-01T00:00:00Z+0000");

  ASSERT_EQ(DateTimeD("Thu Jan  1 00:00:00 UTC 1970"), DateTimeD::Epoch());
  ASSERT_EQ(DateTimeD("Thu Jan  1 00:00:00 1970"), DateTimeD::Epoch());
  ASSERT_EQ(DateTimeD("01/01/70"), DateTimeD::Epoch());
  ASSERT_EQ(DateTimeD("00:00:00"), DateTimeD::Epoch());
  ASSERT_EQ(DateTimeD("12:00:00 AM"), DateTimeD::Epoch());
  ASSERT_EQ(DateTimeD("1970-01-01T00:00:00Z+0000"), DateTimeD::Epoch());

  ASSERT_EQ(d.ToString("%a %A %b %h %B %c %C %d %D %e %F %g %G %H %I %j %k %l %m %M %n %p %P %r %R %s %S %t %T %u %U %V %w %W %x %X %y %Y %z %Z %+ %%"),
          "Thu Thursday Jan Jan January Thu Jan  1 00:00:00 1970 19 01 01/01/70  1 1970-01-01 70 1970 00 12 001  0 12 01 00 \n"
          " AM am 12:00:00 AM 00:00:00 0 00 \t 00:00:00 5 00 00 5 00 01/01/70 00:00:00 70 1970 +0000 UTC Thu Jan  1 00:00:00 UTC 1970 %");

  d.FromString("Thu Thursday Jan Jan January Thu Jan  1 00:00:00 1970 19 01 01/01/70  1 1970-01-01 70 1970 00 12 001  0 12 01 00 \n"
               " AM am 12:00:00 AM 00:00:00 3 00 \t 00:00:00 5 00 00 5 00 01/01/70 00:00:00 70 1970 +0000 UTC Thu Jan  1 00:00:00 UTC 1970 %",
               "%a %A %b %h %B %c %C %d %D %e %F %g %G %H %I %j %k %l %m %M %n %p %P %r %R %s %S %t %T %u %U %V %w %W %x %X %y %Y %z %Z %+ %%");
  ASSERT_EQ(d, DateTimeD::Epoch() + Seconds(3));
  d.FromString("71", "%y");
  ASSERT_EQ(d.Year(), 1971);
  d.FromString("71 +0100", "%y %z");
  ASSERT_EQ(d.TimeZone().name, "UTC+1");

  DateTimeD e = d.ToUTC();
  ASSERT_EQ(e.TimeZone().name, "UTC");
  ASSERT_EQ(e.Hour(), 23);

  ASSERT_LT(DateTimeD(2020, 1, 1, 10, 0, 0, PST_Timezone().tz), DateTimeD(2020, 1, 1, 9, 0, 0, EST_Timezone().tz));


}

TEST(xDateTime, DateTimePeriod) {
    auto dt = DateTimeD(2021, 1, 1, 1, 1, 1);
    auto dt2 = DateTimeD(2022, 1, 1, 1, 1, 1);
    auto dp = DateTimePeriodD(dt, dt2);
    auto dt3 = DateTimeD(2023, 1, 1, 1, 1, 1);
    auto dp2 = DateTimePeriodD(dt, dt3);
    auto dp3 = DateTimePeriodD(dt2, dt3);
    auto dt4 = DateTimeD(2021, 6, 1, 1, 1, 1);
    auto dp4 = DateTimePeriodD(dt4, dt2);

    ASSERT_EQ(dp.First(), dt);
    ASSERT_EQ(dp.Last(), dt2);
    ASSERT_EQ(dp.Duration(), Days(365));

    ASSERT_TRUE(dp2.ComesBefore(DateTimeD(2024, 1, 1, 1, 1, 1)));
    ASSERT_TRUE(dp2.ComesAfter(DateTimeD(2019, 1, 1, 1, 1, 1)));
    ASSERT_TRUE(dp.Consecutive(dp3));
    ASSERT_TRUE(dp2.Contains(dp));
    ASSERT_TRUE(dp.Overlaps(dp4));
    ASSERT_TRUE(dp2.ComesDuring(dt4));
    ASSERT_TRUE(dp2.Contains(dt4)); // ALIAS
    // DateTimePeriods should contian themselves
    ASSERT_TRUE(dp2.Contains(dp2));

    auto dpexpand = dp.Expanded(dp3);
    ASSERT_EQ(dpexpand, dp2);
    auto dpcontract = dp2.Contracted(dp3);
    ASSERT_EQ(dpcontract, dp3);

    auto dpshift = dp.Shifted(Seconds(1));
    ASSERT_EQ(dpshift.First(), DateTimeD(2021, 1, 1, 1, 1, 2));
    ASSERT_EQ(dpshift.Last(), DateTimeD(2022, 1, 1, 1, 1, 2));

    auto dpexpand2 = dp.Expanded(Seconds(2));
    ASSERT_EQ(dpexpand2.First(), DateTimeD(2021, 1, 1, 1, 0, 59));
    ASSERT_EQ(dpexpand2.Last(), DateTimeD(2022, 1, 1, 1, 1, 3));

    auto dpunion = dp.Union(dp3);
    ASSERT_EQ(dpunion, dp2);
    auto dpintersection = dp2.Intersection(dp3);
    ASSERT_EQ(dpintersection, dp3);

    auto dpfar =
        DateTimePeriodD(DateTimeD::Epoch(), DateTimeD::Epoch() + Days(365));
    ASSERT_THROW(dp2.Union(dpfar), MalformedDateTime);
    ASSERT_THROW(dp2.Intersection(dpfar), MalformedDateTime);
}
