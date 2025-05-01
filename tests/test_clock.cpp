
extern "C" {
#include "clock.h"
}
#include "catch.hpp"

// This is our reference library to test with
// #include "date.h"
#include <chrono>
#include <iostream>

using namespace std::chrono;

// Quick simple tests to checks that the clock is working in nominal way
// Not full test
void SanityCheck()
{
    using namespace std::chrono;
    constexpr std::chrono::year_month_weekday_last ymwl{ 2015y / March / Sunday[last] };

    // Convertir en sys_days pour obtenir la date exacte
    sys_days sd = sys_days{ymwl};
    year_month_day ymd = year_month_day{sd};

    int year  = ymd.year().operator int();
    unsigned int month  = ymd.month().operator unsigned int();
    unsigned int day  = unsigned(ymd.day());

    // Check that the date is correct
    REQUIRE(year == 2015);
    REQUIRE(month == 3);
    REQUIRE(day == 29);

    // Check day number
    uint32_t last_sun = clk_last_dow(year, month, SUNDAY);

    REQUIRE(last_sun == day);

    // Check DOW
    uint32_t dow = clk_dow(year, month, day);

    // Check that dow is sunday
    REQUIRE(dow == SUNDAY);

}


TEST_CASE( "Clock", "[clock_test]" )
{
    SanityCheck();

}



