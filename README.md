# xDateTime

## Why another datetime library?

I built another datetime library because there were no libraries at the time of this writing which
are both compact - do not require big dependencies such as boost, and support locales and timezones
without relying on OS-specific libraries.

Because I wanted to avoid creating the N+1 problem, I have made adaptors for every other C/C++
datetime library so that you can easily convert xDateTime into other datetime classes and vice-versa,
making it very easy to integrate xDateTime in your own project without having to write elementary
adaptor code.

## Description

xDateTime is the last C++ datetime library you will need: It is compatible with all the other
major C++ datetime libraries and is a header-only file. If you want timezone and locale support,
those are also in separate header files.

## Requirements

All you need is a C++11 or more recent compiler. If you want support for the adaptor methods,
the relevant libraries must be installed and you need to define the corresponding macro.
The following are the list of adaptors implemented along with their macros:

- Boost.DateTime: `X_DATETIME_WITH_BOOST`
- Abseil Time: `X_DATETIME_WITH_ABSEIL`
- Poco DateTime: `X_DATETIME_WITH_POCO`
- GLib: `X_DATETIME_WITH_GLIB`

Only adaptors for datetime libraries present in distrs' repositories are implemented.

HowardHinnant datetime adaptor is not implemented because most of its features were merged
into C++20, and in any case are interoperable with C++11 <chrono> `time_point` and `duration`.

**Note**: All of the adaptors support UTC time only. You cannot directly convert from other
timezones.

xDateTime has no external dependencies.
                                                                                                                                                                        
## Installation

If you are on a Linux system, you can generate more recent locale and timezone data using
the bundled python scripts:

```
# In the current directory
python3 gen-locale-data.py > x_datetime_locale_data.h 
python3 gen-tz-data.py > x_datetime_timezone.h 
```

Otherwise, you can use the pregenerated files, which have been made on an Ubuntu 22.04 machine.

Being a single-header file, copy the files you want into your project, or simply copy them
to /usr/local/include if you want them available to all projects.

## License

xDateTime is Copyright (c) 2021-2023 Ali Sherief.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 


  LINKING EXCEPTION

 In addition to the permissions in the Apache Software License 2.0
 the authors give you unlimited permission to link the compiled
 version of this library into combinations with other programs,
 and to distribute those combinations without any restriction
 coming from the use of this file.  (The General Public License
 restrictions do apply in other respects; for example, they cover
 modification of the file, and distribution when not linked into
 a combined executable.)

