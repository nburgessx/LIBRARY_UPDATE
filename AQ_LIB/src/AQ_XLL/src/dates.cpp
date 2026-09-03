#include <main.h>
#include <ctime>
#include <cmath>

// Date Time Helper Methods
namespace
{
    // Days since 1970-01-01 from a civil date (Howard Hinnant's algorithm)
    long long daysFromCivil(long long y, unsigned m, unsigned d)
    {
        y -= m <= 2;
        const long long era = (y >= 0 ? y : y - 399) / 400;
        const unsigned yoe = static_cast<unsigned>(y - era * 400);
        const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
        const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        return era * 146097 + static_cast<long long>(doe) - 719468;
    }

    // Local wall-clock time as an Excel date serial (1900 date system)
    double excelLocalSerial()
    {
        const std::time_t t = std::time(nullptr);
        std::tm lt{};
        localtime_s(&lt, &t);
        const auto days = daysFromCivil(lt.tm_year + 1900, lt.tm_mon + 1, lt.tm_mday);
        const double frac = (lt.tm_hour * 3600.0 + lt.tm_min * 60.0 + lt.tm_sec) / 86400.0;
        return static_cast<double>(days + 25569) + frac;
    }
}


// Non-Volatile Today Method
XLO_FUNC_START(aqDatesToday())
{
    return returnValue(std::floor(excelLocalSerial()));
}
XLO_FUNC_END(aqDatesToday)
.help(L"Today's date as a non-volatile date serial. Format the cell as a date.");


// Non-Volatile Now Method
XLO_FUNC_START(aqDatesNow())
{
    return returnValue(excelLocalSerial());
}
XLO_FUNC_END(aqDatesNow)
.help(L"Current date and time, non-volatile. Format the cell as date/time.");
