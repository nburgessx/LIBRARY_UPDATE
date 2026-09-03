// TestDateIMMFuturesTickers.cpp

#include "FuturesDates.h"

// Include: Google Test Library
#include <gTest/gTest.h>


TEST( TestFuturesTickers, UNIT_ConvertFuturesDateToTicker1 )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    const AQLDate jan2019("20190101");
    const AQLDate feb2019("20190201");
    const AQLDate mar2019("20190301");

    const AQLDate apr2019("20190401");
    const AQLDate may2019("20190501");
    const AQLDate jun2019("20190601");

    const AQLDate jul2019("20190701");
    const AQLDate aug2019("20190801");
    const AQLDate sep2019("20190901");

    const AQLDate oct2019("20191001");
    const AQLDate nov2019("20191101");
    const AQLDate dec2019("20191201");

    const std::string resultF9 = etrading::convertDateToFuturesTicker( jan2019 );
    const std::string resultG9 = etrading::convertDateToFuturesTicker( feb2019 );
    const std::string resultH9 = etrading::convertDateToFuturesTicker( mar2019 );
    
    const std::string resultJ9 = etrading::convertDateToFuturesTicker( apr2019 );
    const std::string resultK9 = etrading::convertDateToFuturesTicker( may2019 );
    const std::string resultM9 = etrading::convertDateToFuturesTicker( jun2019 );

    const std::string resultN9 = etrading::convertDateToFuturesTicker( jul2019 );
    const std::string resultQ9 = etrading::convertDateToFuturesTicker( aug2019 );
    const std::string resultU9 = etrading::convertDateToFuturesTicker( sep2019 );

    const std::string resultV9 = etrading::convertDateToFuturesTicker( oct2019 );
    const std::string resultX9 = etrading::convertDateToFuturesTicker( nov2019 );
    const std::string resultZ9 = etrading::convertDateToFuturesTicker( dec2019 );

    EXPECT_EQ( "F9", resultF9 );
    EXPECT_EQ( "G9", resultG9 );
    EXPECT_EQ( "H9", resultH9 );

    EXPECT_EQ( "J9", resultJ9 );
    EXPECT_EQ( "K9", resultK9 );
    EXPECT_EQ( "M9", resultM9 );

    EXPECT_EQ( "N9", resultN9 );
    EXPECT_EQ( "Q9", resultQ9 );
    EXPECT_EQ( "U9", resultU9 );

    EXPECT_EQ( "V9", resultV9 );
    EXPECT_EQ( "X9", resultX9 );
    EXPECT_EQ( "Z9", resultZ9 );
}


TEST( TestFuturesTickers, UNIT_ConvertFuturesDateToTicker2 )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    const AQLDate jan2020("20200101");
    const AQLDate feb2020("20200201");
    const AQLDate mar2020("20200301");

    const AQLDate apr2020("20200401");
    const AQLDate may2020("20200501");
    const AQLDate jun2020("20200601");

    const AQLDate jul2020("20200701");
    const AQLDate aug2020("20200801");
    const AQLDate sep2020("20200901");

    const AQLDate oct2020("20201001");
    const AQLDate nov2020("20201101");
    const AQLDate dec2020("20201201");

    const std::string resultF0 = etrading::convertDateToFuturesTicker( jan2020 );
    const std::string resultG0 = etrading::convertDateToFuturesTicker( feb2020 );
    const std::string resultH0 = etrading::convertDateToFuturesTicker( mar2020 );
    
    const std::string resultJ0 = etrading::convertDateToFuturesTicker( apr2020 );
    const std::string resultK0 = etrading::convertDateToFuturesTicker( may2020 );
    const std::string resultM0 = etrading::convertDateToFuturesTicker( jun2020 );

    const std::string resultN0 = etrading::convertDateToFuturesTicker( jul2020 );
    const std::string resultQ0 = etrading::convertDateToFuturesTicker( aug2020 );
    const std::string resultU0 = etrading::convertDateToFuturesTicker( sep2020 );

    const std::string resultV0 = etrading::convertDateToFuturesTicker( oct2020 );
    const std::string resultX0 = etrading::convertDateToFuturesTicker( nov2020 );
    const std::string resultZ0 = etrading::convertDateToFuturesTicker( dec2020 );

    EXPECT_EQ( "F0", resultF0 );
    EXPECT_EQ( "G0", resultG0 );
    EXPECT_EQ( "H0", resultH0 );

    EXPECT_EQ( "J0", resultJ0 );
    EXPECT_EQ( "K0", resultK0 );
    EXPECT_EQ( "M0", resultM0 );

    EXPECT_EQ( "N0", resultN0 );
    EXPECT_EQ( "Q0", resultQ0 );
    EXPECT_EQ( "U0", resultU0 );

    EXPECT_EQ( "V0", resultV0 );
    EXPECT_EQ( "X0", resultX0 );
    EXPECT_EQ( "Z0", resultZ0 );
}


TEST( TestFuturesTickers, UNIT_ConvertFuturesDateToTicker3 )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    const AQLDate jan2020("20200101");
    const AQLDate jan2021("20210101");
    const AQLDate jan2022("20220101");
    const AQLDate jan2023("20230101");
    const AQLDate jan2024("20240101");
    const AQLDate jan2025("20250101");
    const AQLDate jan2026("20260101");
    const AQLDate jan2027("20270101");
    const AQLDate jan2028("20280101");
    const AQLDate jan2029("20290101");
    const AQLDate jan2030("20300101");
    
    const std::string resultF0  = etrading::convertDateToFuturesTicker( jan2020 );
    const std::string resultF1  = etrading::convertDateToFuturesTicker( jan2021 );
    const std::string resultF2  = etrading::convertDateToFuturesTicker( jan2022 );
    const std::string resultF3  = etrading::convertDateToFuturesTicker( jan2023 );
    const std::string resultF4  = etrading::convertDateToFuturesTicker( jan2024 );
    const std::string resultF5  = etrading::convertDateToFuturesTicker( jan2025 );
    const std::string resultF6  = etrading::convertDateToFuturesTicker( jan2026 );
    const std::string resultF7  = etrading::convertDateToFuturesTicker( jan2027 );
    const std::string resultF8  = etrading::convertDateToFuturesTicker( jan2028 );
    const std::string resultF9  = etrading::convertDateToFuturesTicker( jan2029 );
    const std::string resultF10 = etrading::convertDateToFuturesTicker( jan2030 );

    EXPECT_EQ( "F0", resultF0 );
    EXPECT_EQ( "F1", resultF1 );
    EXPECT_EQ( "F2", resultF2 );
    EXPECT_EQ( "F3", resultF3 );
    EXPECT_EQ( "F4", resultF4 );
    EXPECT_EQ( "F5", resultF5 );
    EXPECT_EQ( "F6", resultF6 );
    EXPECT_EQ( "F7", resultF7 );
    EXPECT_EQ( "F8", resultF8 );
    EXPECT_EQ( "F9", resultF9 );
    EXPECT_EQ( "F0", resultF10 );
}


TEST( TestFuturesTickers, UNIT_CurrentIMMFuturesTicker )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    // Current Future is the Future that is Active i.e. start date in past but yet to end.
    const AQLDate referenceDate1("20190101");
    const AQLDate referenceDate2("20190401");
    const AQLDate referenceDate3("20190701");
    const AQLDate referenceDate4("20191101");

    const std::string currentFuturesTicker1 = etrading::currentIMMFuturesTicker( referenceDate1 );
    const std::string currentFuturesTicker2 = etrading::currentIMMFuturesTicker( referenceDate2 );
    const std::string currentFuturesTicker3 = etrading::currentIMMFuturesTicker( referenceDate3 );
    const std::string currentFuturesTicker4 = etrading::currentIMMFuturesTicker( referenceDate4 );

    EXPECT_EQ( "Z8", currentFuturesTicker1 ); // Jan-19 has active contract Dec-18 (Z8)
    EXPECT_EQ( "H9", currentFuturesTicker2 ); // Apr-19 has active contract Mar-19 (H9)
    EXPECT_EQ( "M9", currentFuturesTicker3 ); // Jul-19 has active contract Jun-19 (M9)
    EXPECT_EQ( "U9", currentFuturesTicker4 ); // Nov-19 has active contract Sep-19 (U9)
}


TEST( TestFuturesTickers, UNIT_NextIMMFuturesTicker )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    // The Next Future is the first future starting after the reference date
    const AQLDate referenceDate1("20190101");
    const AQLDate referenceDate2("20190401");
    const AQLDate referenceDate3("20190701");
    const AQLDate referenceDate4("20191101");

    const std::string nextFuturesTicker1 = etrading::nextIMMFuturesTicker( referenceDate1 );
    const std::string nextFuturesTicker2 = etrading::nextIMMFuturesTicker( referenceDate2 );
    const std::string nextFuturesTicker3 = etrading::nextIMMFuturesTicker( referenceDate3 );
    const std::string nextFuturesTicker4 = etrading::nextIMMFuturesTicker( referenceDate4 );

    EXPECT_EQ( "H9", nextFuturesTicker1 ); // Jan-19 has next contract Mar-18 (H9)
    EXPECT_EQ( "M9", nextFuturesTicker2 ); // Apr-19 has next contract Jun-19 (M9)
    EXPECT_EQ( "U9", nextFuturesTicker3 ); // Jul-19 has next contract Sep-19 (U9)
    EXPECT_EQ( "Z9", nextFuturesTicker4 ); // Nov-19 has next contract Dec-19 (Z9)
}


TEST( TestFuturesTickers, UNIT_PreviousIMMFuturesTicker )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    // The Previous Future is the first future starting before the reference date
    const AQLDate referenceDate1("20190101");
    const AQLDate referenceDate2("20190401");
    const AQLDate referenceDate3("20190701");
    const AQLDate referenceDate4("20191101");

    const std::string previousFuturesTicker1 = etrading::previousIMMFuturesTicker( referenceDate1 );
    const std::string previousFuturesTicker2 = etrading::previousIMMFuturesTicker( referenceDate2 );
    const std::string previousFuturesTicker3 = etrading::previousIMMFuturesTicker( referenceDate3 );
    const std::string previousFuturesTicker4 = etrading::previousIMMFuturesTicker( referenceDate4 );

    EXPECT_EQ( "U8", previousFuturesTicker1 ); // Jan-19 has active contract Dec-18 and previous contract Sep-18 (U8)
    EXPECT_EQ( "Z8", previousFuturesTicker2 ); // Apr-19 has active contract Mar-19 and previous contract Dec-18 (Z8)
    EXPECT_EQ( "H9", previousFuturesTicker3 ); // Jul-19 has active contract Jun-19 and previous contract Mar-19 (H9)
    EXPECT_EQ( "M9", previousFuturesTicker4 ); // Nov-19 has active contract Sep-19 and previous contract Jun-19 (M9)
}

TEST( TestFuturesTickers, UNIT_NthIMMFuturesTicker )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    const AQLDate referenceDate1("20190101");
    
    const std::string futuresTicker1 = etrading::nthIMMFuturesTicker( referenceDate1, 1 );
    const std::string futuresTicker2 = etrading::nthIMMFuturesTicker( referenceDate1, 2 );
    const std::string futuresTicker3 = etrading::nthIMMFuturesTicker( referenceDate1, 3 );
    const std::string futuresTicker4 = etrading::nthIMMFuturesTicker( referenceDate1, 4 );

    EXPECT_EQ( "H9", futuresTicker1 ); // 1st contract Mar-18 (H9)
    EXPECT_EQ( "M9", futuresTicker2 ); // 2nd contract Jun-19 (M9)
    EXPECT_EQ( "U9", futuresTicker3 ); // 3rd contract Sep-19 (U9)
    EXPECT_EQ( "Z9", futuresTicker4 ); // 4th contract Dec-19 (Z9)
}

TEST( TestFuturesTickers, UNIT_ConvertFuturesDateToTicker1_TwoDigitYearFormat )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    const AQLDate jan2019("20190101");
    const AQLDate feb2019("20190201");
    const AQLDate mar2019("20190301");

    const AQLDate apr2019("20190401");
    const AQLDate may2019("20190501");
    const AQLDate jun2019("20190601");

    const AQLDate jul2019("20190701");
    const AQLDate aug2019("20190801");
    const AQLDate sep2019("20190901");

    const AQLDate oct2019("20191001");
    const AQLDate nov2019("20191101");
    const AQLDate dec2019("20191201");

    const std::string resultF9 = etrading::convertDateToFuturesTicker( jan2019, true );
    const std::string resultG9 = etrading::convertDateToFuturesTicker( feb2019, true );
    const std::string resultH9 = etrading::convertDateToFuturesTicker( mar2019, true );
    
    const std::string resultJ9 = etrading::convertDateToFuturesTicker( apr2019, true );
    const std::string resultK9 = etrading::convertDateToFuturesTicker( may2019, true );
    const std::string resultM9 = etrading::convertDateToFuturesTicker( jun2019, true );

    const std::string resultN9 = etrading::convertDateToFuturesTicker( jul2019, true );
    const std::string resultQ9 = etrading::convertDateToFuturesTicker( aug2019, true );
    const std::string resultU9 = etrading::convertDateToFuturesTicker( sep2019, true );

    const std::string resultV9 = etrading::convertDateToFuturesTicker( oct2019, true );
    const std::string resultX9 = etrading::convertDateToFuturesTicker( nov2019, true );
    const std::string resultZ9 = etrading::convertDateToFuturesTicker( dec2019, true );

    EXPECT_EQ( "F19", resultF9 );
    EXPECT_EQ( "G19", resultG9 );
    EXPECT_EQ( "H19", resultH9 );

    EXPECT_EQ( "J19", resultJ9 );
    EXPECT_EQ( "K19", resultK9 );
    EXPECT_EQ( "M19", resultM9 );

    EXPECT_EQ( "N19", resultN9 );
    EXPECT_EQ( "Q19", resultQ9 );
    EXPECT_EQ( "U19", resultU9 );

    EXPECT_EQ( "V19", resultV9 );
    EXPECT_EQ( "X19", resultX9 );
    EXPECT_EQ( "Z19", resultZ9 );
}


TEST( TestFuturesTickers, UNIT_ConvertFuturesDateToTicker2_TwoDigitYearFormat )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    const AQLDate jan2020("20200101");
    const AQLDate feb2020("20200201");
    const AQLDate mar2020("20200301");

    const AQLDate apr2020("20200401");
    const AQLDate may2020("20200501");
    const AQLDate jun2020("20200601");

    const AQLDate jul2020("20200701");
    const AQLDate aug2020("20200801");
    const AQLDate sep2020("20200901");

    const AQLDate oct2020("20201001");
    const AQLDate nov2020("20201101");
    const AQLDate dec2020("20201201");

    const std::string resultF0 = etrading::convertDateToFuturesTicker( jan2020, true );
    const std::string resultG0 = etrading::convertDateToFuturesTicker( feb2020, true );
    const std::string resultH0 = etrading::convertDateToFuturesTicker( mar2020, true );
    
    const std::string resultJ0 = etrading::convertDateToFuturesTicker( apr2020, true );
    const std::string resultK0 = etrading::convertDateToFuturesTicker( may2020, true );
    const std::string resultM0 = etrading::convertDateToFuturesTicker( jun2020, true );

    const std::string resultN0 = etrading::convertDateToFuturesTicker( jul2020, true );
    const std::string resultQ0 = etrading::convertDateToFuturesTicker( aug2020, true );
    const std::string resultU0 = etrading::convertDateToFuturesTicker( sep2020, true );

    const std::string resultV0 = etrading::convertDateToFuturesTicker( oct2020, true );
    const std::string resultX0 = etrading::convertDateToFuturesTicker( nov2020, true );
    const std::string resultZ0 = etrading::convertDateToFuturesTicker( dec2020, true );

    EXPECT_EQ( "F20", resultF0 );
    EXPECT_EQ( "G20", resultG0 );
    EXPECT_EQ( "H20", resultH0 );

    EXPECT_EQ( "J20", resultJ0 );
    EXPECT_EQ( "K20", resultK0 );
    EXPECT_EQ( "M20", resultM0 );

    EXPECT_EQ( "N20", resultN0 );
    EXPECT_EQ( "Q20", resultQ0 );
    EXPECT_EQ( "U20", resultU0 );

    EXPECT_EQ( "V20", resultV0 );
    EXPECT_EQ( "X20", resultX0 );
    EXPECT_EQ( "Z20", resultZ0 );
}


TEST( TestFuturesTickers, UNIT_ConvertFuturesDateToTicker3_TwoDigitYearFormat )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    const AQLDate jan2020("20200101");
    const AQLDate jan2021("20210101");
    const AQLDate jan2022("20220101");
    const AQLDate jan2023("20230101");
    const AQLDate jan2024("20240101");
    const AQLDate jan2025("20250101");
    const AQLDate jan2026("20260101");
    const AQLDate jan2027("20270101");
    const AQLDate jan2028("20280101");
    const AQLDate jan2029("20290101");
    const AQLDate jan2030("20300101");
    
    const std::string resultF0  = etrading::convertDateToFuturesTicker( jan2020, true );
    const std::string resultF1  = etrading::convertDateToFuturesTicker( jan2021, true );
    const std::string resultF2  = etrading::convertDateToFuturesTicker( jan2022, true );
    const std::string resultF3  = etrading::convertDateToFuturesTicker( jan2023, true );
    const std::string resultF4  = etrading::convertDateToFuturesTicker( jan2024, true );
    const std::string resultF5  = etrading::convertDateToFuturesTicker( jan2025, true );
    const std::string resultF6  = etrading::convertDateToFuturesTicker( jan2026, true );
    const std::string resultF7  = etrading::convertDateToFuturesTicker( jan2027, true );
    const std::string resultF8  = etrading::convertDateToFuturesTicker( jan2028, true );
    const std::string resultF9  = etrading::convertDateToFuturesTicker( jan2029, true );
    const std::string resultF10 = etrading::convertDateToFuturesTicker( jan2030, true );

    EXPECT_EQ( "F20", resultF0 );
    EXPECT_EQ( "F21", resultF1 );
    EXPECT_EQ( "F22", resultF2 );
    EXPECT_EQ( "F23", resultF3 );
    EXPECT_EQ( "F24", resultF4 );
    EXPECT_EQ( "F25", resultF5 );
    EXPECT_EQ( "F26", resultF6 );
    EXPECT_EQ( "F27", resultF7 );
    EXPECT_EQ( "F28", resultF8 );
    EXPECT_EQ( "F29", resultF9 );
    EXPECT_EQ( "F30", resultF10 );
}


TEST( TestFuturesTickers, UNIT_CurrentIMMFuturesTicker_TwoDigitYearFormat )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    // Current Future is the Future that is Active i.e. start date in past but yet to end.
    const AQLDate referenceDate1("20190101");
    const AQLDate referenceDate2("20190401");
    const AQLDate referenceDate3("20190701");
    const AQLDate referenceDate4("20191101");

    const std::string currentFuturesTicker1 = etrading::currentIMMFuturesTicker( referenceDate1, false, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );
    const std::string currentFuturesTicker2 = etrading::currentIMMFuturesTicker( referenceDate2, false, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/  );
    const std::string currentFuturesTicker3 = etrading::currentIMMFuturesTicker( referenceDate3, false, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/  );
    const std::string currentFuturesTicker4 = etrading::currentIMMFuturesTicker( referenceDate4, false, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/  );

    EXPECT_EQ( "Z18", currentFuturesTicker1 ); // Jan-19 has active contract Dec-18 (Z8)
    EXPECT_EQ( "H19", currentFuturesTicker2 ); // Apr-19 has active contract Mar-19 (H9)
    EXPECT_EQ( "M19", currentFuturesTicker3 ); // Jul-19 has active contract Jun-19 (M9)
    EXPECT_EQ( "U19", currentFuturesTicker4 ); // Nov-19 has active contract Sep-19 (U9)
}


TEST( TestFuturesTickers, UNIT_NextIMMFuturesTicker_TwoDigitYearFormat )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    // The Next Future is the first future starting after the reference date
    const AQLDate referenceDate1("20190101");
    const AQLDate referenceDate2("20190401");
    const AQLDate referenceDate3("20190701");
    const AQLDate referenceDate4("20191101");

    const std::string nextFuturesTicker1 = etrading::nextIMMFuturesTicker( referenceDate1, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );
    const std::string nextFuturesTicker2 = etrading::nextIMMFuturesTicker( referenceDate2, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );
    const std::string nextFuturesTicker3 = etrading::nextIMMFuturesTicker( referenceDate3, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );
    const std::string nextFuturesTicker4 = etrading::nextIMMFuturesTicker( referenceDate4, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );

    EXPECT_EQ( "H19", nextFuturesTicker1 ); // Jan-19 has next contract Mar-18 (H9)
    EXPECT_EQ( "M19", nextFuturesTicker2 ); // Apr-19 has next contract Jun-19 (M9)
    EXPECT_EQ( "U19", nextFuturesTicker3 ); // Jul-19 has next contract Sep-19 (U9)
    EXPECT_EQ( "Z19", nextFuturesTicker4 ); // Nov-19 has next contract Dec-19 (Z9)
}


TEST( TestFuturesTickers, UNIT_PreviousIMMFuturesTicker_TwoDigitYearFormat )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    // The Previous Future is the first future starting before the reference date
    const AQLDate referenceDate1("20190101");
    const AQLDate referenceDate2("20190401");
    const AQLDate referenceDate3("20190701");
    const AQLDate referenceDate4("20191101");

    const std::string previousFuturesTicker1 = etrading::previousIMMFuturesTicker( referenceDate1, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );
    const std::string previousFuturesTicker2 = etrading::previousIMMFuturesTicker( referenceDate2, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );
    const std::string previousFuturesTicker3 = etrading::previousIMMFuturesTicker( referenceDate3, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );
    const std::string previousFuturesTicker4 = etrading::previousIMMFuturesTicker( referenceDate4, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );

    EXPECT_EQ( "U18", previousFuturesTicker1 ); // Jan-19 has active contract Dec-18 and previous contract Sep-18 (U8)
    EXPECT_EQ( "Z18", previousFuturesTicker2 ); // Apr-19 has active contract Mar-19 and previous contract Dec-18 (Z8)
    EXPECT_EQ( "H19", previousFuturesTicker3 ); // Jul-19 has active contract Jun-19 and previous contract Mar-19 (H9)
    EXPECT_EQ( "M19", previousFuturesTicker4 ); // Nov-19 has active contract Sep-19 and previous contract Jun-19 (M9)
}

TEST( TestFuturesTickers, UNIT_NthIMMFuturesTicker_TwoDigitYearFormat )
{
    // Futures Month Codes:
    // In ascending orderJan - Dec
    // "F", "G", "H", "J", "K", "M", "N", "Q", "U", "V", "X", "Z"

    const AQLDate referenceDate1("20190101");
    
    const std::string futuresTicker1 = etrading::nthIMMFuturesTicker( referenceDate1, 1, false, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );
    const std::string futuresTicker2 = etrading::nthIMMFuturesTicker( referenceDate1, 2, false, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );
    const std::string futuresTicker3 = etrading::nthIMMFuturesTicker( referenceDate1, 3, false, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );
    const std::string futuresTicker4 = etrading::nthIMMFuturesTicker( referenceDate1, 4, false, "", "NO_CHANGE", true /* showYearIn2DigitFormat*/ );

    EXPECT_EQ( "H19", futuresTicker1 ); // 1st contract Mar-18 (H9)
    EXPECT_EQ( "M19", futuresTicker2 ); // 2nd contract Jun-19 (M9)
    EXPECT_EQ( "U19", futuresTicker3 ); // 3rd contract Sep-19 (U9)
    EXPECT_EQ( "Z19", futuresTicker4 ); // 4th contract Dec-19 (Z9)
}
