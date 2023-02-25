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

#define X_DATETIME_WITH_ABSEIL
#include "x_datetime.h"
#include <gtest/gtest.h>

using namespace xDateTime;

TEST(xDateTime, Abseil) {
    TimeDeltaD t = Seconds(10);
    DateTimeD d = DateTimeD::Epoch();

    ASSERT_EQ(TimeDeltaD(t.ToAbseilDuration()), t);
    ASSERT_EQ(DateTimeD(d.ToAbseilCivilSecond()), d);
    ASSERT_EQ(DateTimeD(d.ToAbseilCivilMinute()), d);
    ASSERT_EQ(DateTimeD(d.ToAbseilCivilHour()), d);
    ASSERT_EQ(DateTimeD(d.ToAbseilCivilDay()), d);
    ASSERT_EQ(DateTimeD(d.ToAbseilTime()), d);

}

