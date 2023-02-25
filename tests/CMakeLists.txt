set(CMAKE_CXX_STANDARD 11)

enable_testing()
find_package(GTest REQUIRED)

set(XDATETIME_SOURCES test_xdatetime.cpp)
set(HEADERS x_datetime.h x_datetime_locale_data.h x_datetime_timezone.h)
list(TRANSFORM HEADERS PREPEND ${CMAKE_SOURCE_DIR}/)
add_compile_options(-g -I${CMAKE_SOURCE_DIR}/)

add_executable(test_xDateTime ${XDATETIME_SOURCES} ${HEADERS})
target_link_libraries(test_xDateTime GTest::gtest GTest::gtest_main)

if (DEFINED WITH_BOOST)
    find_package(Boost REQUIRED COMPONENTS headers)
    include_directories(${Boost_INCLUDE_DIRS})
    set(XDATETIME_BOOST_SOURCES test_xdatetime_boost.cpp)
    add_executable(test_xDateTime_Boost ${XDATETIME_BOOST_SOURCES} ${HEADERS})
    target_link_libraries(test_xDateTime_Boost GTest::gtest GTest::gtest_main)
endif()

if (DEFINED WITH_ABSEIL)
    # The preferred way to build Abseil is by cloning the master branch and then
    # adding it as a subdirectory just below the root folder. It is really not suitable
    # for single-header file distributions such as ours so you will have to create the
    # abseil-cpp folder yourself in order to build this test.
    # We have to do this in the top-level CMake file, otherwise CMake will complain.
    set(XDATETIME_ABSEIL_SOURCES test_xdatetime_abseil.cpp)
    add_executable(test_xDateTime_Abseil ${XDATETIME_ABSEIL_SOURCES} ${HEADERS})
    target_link_libraries(test_xDateTime_Abseil GTest::gtest GTest::gtest_main absl::time absl::time_zone)
endif()

if (DEFINED WITH_POCO)
    find_package(Poco REQUIRED COMPONENTS Foundation)
    include_directories(${Poco_INCLUDE_DIRS})
    set(XDATETIME_POCO_SOURCES test_xdatetime_poco.cpp)
    add_executable(test_xDateTime_Poco ${XDATETIME_POCO_SOURCES} ${HEADERS})
    target_link_libraries(test_xDateTime_Poco GTest::gtest GTest::gtest_main Poco::Foundation)
endif()

if (DEFINED WITH_GLIB)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)
    include_directories(${glib_INCLUDE_DIRS})
    set(XDATETIME_GLIB_SOURCES test_xdatetime_glib.cpp)
    add_executable(test_xDateTime_GLib ${XDATETIME_GLIB_SOURCES} ${HEADERS})
    target_link_libraries(test_xDateTime_GLib GTest::gtest GTest::gtest_main PkgConfig::deps)
endif()
