// Include: Google Test Library
#include <gTest/gTest.h>

#include "Dependency.h"
#include "CurveValidation.h"

namespace google_test
{
	DECLARE_TEST_FIXTURE(TestCurveDates);

    //
    // Call Test Fixture
    //

    TEST_F( TestCurveDates, UNIT_ConvertCurveTermsToDates )
    {
        LADate asOfDate("20190301");

        // Initialize the terms and payment date vectors
        DoubleVector terms(50);
        DateVector paymentDates(50);

        // Curve Calibration Terms nodes from USDYC:USDOIS on 25-May-2018
        terms[0]  = 0.000000000000000000;
        terms[1]  = 0.008219178082191780;
        terms[2]  = 0.010958904109589000;
        terms[3]  = 0.052054794520547900;
        terms[4]  = 0.263013698630137000;
        terms[5]  = 0.301369863013699000;
        terms[6]  = 0.515068493150685000;
        terms[7]  = 0.550684931506849000;
        terms[8]  = 0.764383561643836000;
        terms[9]  = 0.800000000000000000;
        terms[10] = 1.013698630136990000;
        terms[11] = 1.049315068493150000;
        terms[12] = 1.265753424657530000;
        terms[13] = 1.298630136986300000;
        terms[14] = 1.526027397260270000;
        terms[15] = 1.547945205479450000;
        terms[16] = 1.772602739726030000;
        terms[17] = 1.797260273972600000;
        terms[18] = 2.013698630136990000;
        terms[19] = 2.046575342465750000;
        terms[20] = 2.271232876712330000;
        terms[21] = 2.523287671232880000;
        terms[22] = 2.769863013698630000;
        terms[23] = 3.019178082191780000;
        terms[24] = 3.268493150684930000;
        terms[25] = 3.520547945205480000;
        terms[26] = 3.767123287671230000;
        terms[27] = 4.016438356164380000;
        terms[28] = 4.265753424657530000;
        terms[29] = 4.517808219178080000;
        terms[30] = 4.767123287671230000;
        terms[31] = 5.016438356164380000;
        terms[32] = 5.268493150684930000;
        terms[33] = 5.520547945205480000;
        terms[34] = 5.769863013698630000;
        terms[35] = 6.016438356164380000;
        terms[36] = 6.268493150684930000;
        terms[37] = 6.520547945205480000;
        terms[38] = 6.769863013698630000;
        terms[39] = 7.016438356164380000;
        terms[40] = 7.268493150684930000;
        terms[41] = 7.528767123287670000;
        terms[42] = 7.775342465753420000;
        terms[43] = 8.016438356164380000;
        terms[44] = 8.273972602739730000;
        terms[45] = 8.526027397260270000;
        terms[46] = 8.772602739726030000;
        terms[47] = 9.021917808219180000;
        terms[48] = 9.271232876712330000;
        terms[49] = 9.523287671232880000;
        
        // Corresponding calibration dates from USDYC:USDOIS on 25-May-2018
        paymentDates[0]  = LADate("20190301");
        paymentDates[1]  = LADate("20190304");
        paymentDates[2]  = LADate("20190305");
        paymentDates[3]  = LADate("20190320");
        paymentDates[4]  = LADate("20190605");
        paymentDates[5]  = LADate("20190619");
        paymentDates[6]  = LADate("20190905");
        paymentDates[7]  = LADate("20190918");
        paymentDates[8]  = LADate("20191205");
        paymentDates[9]  = LADate("20191218");
        paymentDates[10] = LADate("20200305");
        paymentDates[11] = LADate("20200318");
        paymentDates[12] = LADate("20200605");
        paymentDates[13] = LADate("20200617");
        paymentDates[14] = LADate("20200908");
        paymentDates[15] = LADate("20200916");
        paymentDates[16] = LADate("20201207");
        paymentDates[17] = LADate("20201216");
        paymentDates[18] = LADate("20210305");
        paymentDates[19] = LADate("20210317");
        paymentDates[20] = LADate("20210607");
        paymentDates[21] = LADate("20210907");
        paymentDates[22] = LADate("20211206");
        paymentDates[23] = LADate("20220307");
        paymentDates[24] = LADate("20220606");
        paymentDates[25] = LADate("20220906");
        paymentDates[26] = LADate("20221205");
        paymentDates[27] = LADate("20230306");
        paymentDates[28] = LADate("20230605");
        paymentDates[29] = LADate("20230905");
        paymentDates[30] = LADate("20231205");
        paymentDates[31] = LADate("20240305");
        paymentDates[32] = LADate("20240605");
        paymentDates[33] = LADate("20240905");
        paymentDates[34] = LADate("20241205");
        paymentDates[35] = LADate("20250305");
        paymentDates[36] = LADate("20250605");
        paymentDates[37] = LADate("20250905");
        paymentDates[38] = LADate("20251205");
        paymentDates[39] = LADate("20260305");
        paymentDates[40] = LADate("20260605");
        paymentDates[41] = LADate("20260908");
        paymentDates[42] = LADate("20261207");
        paymentDates[43] = LADate("20270305");
        paymentDates[44] = LADate("20270607");
        paymentDates[45] = LADate("20270907");
        paymentDates[46] = LADate("20271206");
        paymentDates[47] = LADate("20280306");
        paymentDates[48] = LADate("20280605");
        paymentDates[49] = LADate("20280905");

        // Get results
        DateVector paymentDateResults = etrading::convertCurveTermsToDates( asOfDate, terms );

        // Dimensionality Checks
        ASSERT_EQ( terms.size(), paymentDates.size() );
        ASSERT_EQ( paymentDateResults.size(), terms.size() );
        
        // Run Test
        for( size_t i = 0; i < paymentDateResults.size(); ++i )
        {
            EXPECT_EQ( paymentDates[i].stringWithFormat("YYYYMMDD"),
                       paymentDateResults[i].stringWithFormat("YYYYMMDD") );
        }
    }

    TEST_F( TestCurveDates, UNIT_ConvertCurveDatesToTerms )
    {
        LADate asOfDate("20190301");

        // Initialize the terms and payment date vectors
        DoubleVector terms(50);
        DateVector paymentDates(50);

        // Curve Calibration Terms nodes from USDYC:USDOIS on 25-May-2018
        terms[0]  = 0.000000000000000000;
        terms[1] = 0.008219178082191780;
        terms[2] = 0.010958904109589000;
        terms[3] = 0.052054794520547900;
        terms[4] = 0.263013698630137000;
        terms[5] = 0.301369863013699000;
        terms[6] = 0.515068493150685000;
        terms[7] = 0.550684931506849000;
        terms[8] = 0.764383561643836000;
        terms[9] = 0.800000000000000000;
        terms[10] = 1.013698630136990000;
        terms[11] = 1.049315068493150000;
        terms[12] = 1.265753424657530000;
        terms[13] = 1.298630136986300000;
        terms[14] = 1.526027397260270000;
        terms[15] = 1.547945205479450000;
        terms[16] = 1.772602739726030000;
        terms[17] = 1.797260273972600000;
        terms[18] = 2.013698630136990000;
        terms[19] = 2.046575342465750000;
        terms[20] = 2.271232876712330000;
        terms[21] = 2.523287671232880000;
        terms[22] = 2.769863013698630000;
        terms[23] = 3.019178082191780000;
        terms[24] = 3.268493150684930000;
        terms[25] = 3.520547945205480000;
        terms[26] = 3.767123287671230000;
        terms[27] = 4.016438356164380000;
        terms[28] = 4.265753424657530000;
        terms[29] = 4.517808219178080000;
        terms[30] = 4.767123287671230000;
        terms[31] = 5.016438356164380000;
        terms[32] = 5.268493150684930000;
        terms[33] = 5.520547945205480000;
        terms[34] = 5.769863013698630000;
        terms[35] = 6.016438356164380000;
        terms[36] = 6.268493150684930000;
        terms[37] = 6.520547945205480000;
        terms[38] = 6.769863013698630000;
        terms[39] = 7.016438356164380000;
        terms[40] = 7.268493150684930000;
        terms[41] = 7.528767123287670000;
        terms[42] = 7.775342465753420000;
        terms[43] = 8.016438356164380000;
        terms[44] = 8.273972602739730000;
        terms[45] = 8.526027397260270000;
        terms[46] = 8.772602739726030000;
        terms[47] = 9.021917808219180000;
        terms[48] = 9.271232876712330000;
        terms[49] = 9.523287671232880000;
        
        // Corresponding calibration dates from USDYC:USDOIS on 25-May-2018
        paymentDates[0]  = LADate("20190301");
        paymentDates[1]  = LADate("20190304");
        paymentDates[2]  = LADate("20190305");
        paymentDates[3]  = LADate("20190320");
        paymentDates[4]  = LADate("20190605");
        paymentDates[5]  = LADate("20190619");
        paymentDates[6]  = LADate("20190905");
        paymentDates[7]  = LADate("20190918");
        paymentDates[8]  = LADate("20191205");
        paymentDates[9]  = LADate("20191218");
        paymentDates[10] = LADate("20200305");
        paymentDates[11] = LADate("20200318");
        paymentDates[12] = LADate("20200605");
        paymentDates[13] = LADate("20200617");
        paymentDates[14] = LADate("20200908");
        paymentDates[15] = LADate("20200916");
        paymentDates[16] = LADate("20201207");
        paymentDates[17] = LADate("20201216");
        paymentDates[18] = LADate("20210305");
        paymentDates[19] = LADate("20210317");
        paymentDates[20] = LADate("20210607");
        paymentDates[21] = LADate("20210907");
        paymentDates[22] = LADate("20211206");
        paymentDates[23] = LADate("20220307");
        paymentDates[24] = LADate("20220606");
        paymentDates[25] = LADate("20220906");
        paymentDates[26] = LADate("20221205");
        paymentDates[27] = LADate("20230306");
        paymentDates[28] = LADate("20230605");
        paymentDates[29] = LADate("20230905");
        paymentDates[30] = LADate("20231205");
        paymentDates[31] = LADate("20240305");
        paymentDates[32] = LADate("20240605");
        paymentDates[33] = LADate("20240905");
        paymentDates[34] = LADate("20241205");
        paymentDates[35] = LADate("20250305");
        paymentDates[36] = LADate("20250605");
        paymentDates[37] = LADate("20250905");
        paymentDates[38] = LADate("20251205");
        paymentDates[39] = LADate("20260305");
        paymentDates[40] = LADate("20260605");
        paymentDates[41] = LADate("20260908");
        paymentDates[42] = LADate("20261207");
        paymentDates[43] = LADate("20270305");
        paymentDates[44] = LADate("20270607");
        paymentDates[45] = LADate("20270907");
        paymentDates[46] = LADate("20271206");
        paymentDates[47] = LADate("20280306");
        paymentDates[48] = LADate("20280605");
        paymentDates[49] = LADate("20280905");

        // Get results
        DoubleVector termResults = etrading::convertCurveDatesToTerms( asOfDate, paymentDates );

        // Dimensionality Checks
        ASSERT_EQ( terms.size(), paymentDates.size() );
        ASSERT_EQ( termResults.size(), terms.size() );
        
        // Run Test
        const double testPrecision = 1e-12;
        for( size_t i = 0; i < termResults.size(); ++i )
        {
            EXPECT_NEAR( terms[i], termResults[i], testPrecision );
        }
    }
    
    TEST_F( TestCurveDates, CONSISTENCY_ConvertCurveDatesToTerms_RoundTrip )
    {
        LADate asOfDate("20180525");

        // Initialize the terms and payment date vectors
        DoubleVector terms(50);
        DateVector paymentDates(50);

        // Curve Calibration Terms nodes from USDYC:USDOIS on 25-May-2018
        terms[0]  = 0.00000000000000000;
        terms[1]  = 0.01095890410958900;
        terms[2]  = 0.01369863013698640;
        terms[3]  = 0.03287671232876720;
        terms[4]  = 0.05205479452054780;
        terms[5]  = 0.07123287671232870;
        terms[6]  = 0.09589041095890400;
        terms[7]  = 0.18082191780821900;
        terms[8]  = 0.26575342465753400;
        terms[9]  = 0.34520547945205500;
        terms[10] = 0.43287671232876700;
        terms[11] = 0.51780821917808200;
        terms[12] = 0.76438356164383600;
        terms[13] = 1.01369863013699000;
        terms[14] = 1.26575342465753000;
        terms[15] = 1.51506849315069000;
        terms[16] = 1.76394191181975000;
        terms[17] = 2.01257579160117000;
        terms[18] = 2.26120967138259000;
        terms[19] = 2.51804027247548000;
        terms[20] = 2.75890410958904000;
        terms[21] = 3.00821917808219000;
        terms[22] = 3.26849315068493000;
        terms[23] = 3.51780821917808000;
        terms[24] = 3.76438356164384000;
        terms[25] = 4.01643835616438000;
        terms[26] = 4.26575342465753000;
        terms[27] = 4.51780821917808000;
        terms[28] = 4.76438356164384000;
        terms[29] = 5.01369863013699000;
        terms[30] = 5.26575342465753000;
        terms[31] = 5.51780821917808000;
        terms[32] = 5.76667415225691000;
        terms[33] = 6.01530803203833000;
        terms[34] = 6.26667415225691000;
        terms[35] = 6.51530803203833000;
        terms[36] = 6.76438356164384000;
        terms[37] = 7.01369863013699000;
        terms[38] = 7.26301369863014000;
        terms[39] = 7.51232876712329000;
        terms[40] = 7.76164383561644000;
        terms[41] = 8.01095890410959000;
        terms[42] = 8.26027397260274000;
        terms[43] = 8.51780821917808000;
        terms[44] = 8.75890410958904000;
        terms[45] = 9.00821917808219000;
        terms[46] = 9.26849315068493000;
        terms[47] = 9.51780821917808000;
        terms[48] = 9.76667415225691000;
        terms[49] = 10.0153080320383000;
        
        // Corresponding calibration dates from USDYC:USDOIS on 25-May-2018
        paymentDates[0]  = LADate("20180525");
        paymentDates[1]  = LADate("20180529");
        paymentDates[2]  = LADate("20180530");
        paymentDates[3]  = LADate("20180606");
        paymentDates[4]  = LADate("20180613");
        paymentDates[5]  = LADate("20180620");
        paymentDates[6]  = LADate("20180629");
        paymentDates[7]  = LADate("20180730");
        paymentDates[8]  = LADate("20180830");
        paymentDates[9]  = LADate("20180928");
        paymentDates[10] = LADate("20181030");
        paymentDates[11] = LADate("20181130");
        paymentDates[12] = LADate("20190228");
        paymentDates[13] = LADate("20190530");
        paymentDates[14] = LADate("20190830");
        paymentDates[15] = LADate("20191129");
        paymentDates[16] = LADate("20200228");
        paymentDates[17] = LADate("20200529");
        paymentDates[18] = LADate("20200828");
        paymentDates[19] = LADate("20201130");
        paymentDates[20] = LADate("20210226");
        paymentDates[21] = LADate("20210528");
        paymentDates[22] = LADate("20210831");
        paymentDates[23] = LADate("20211130");
        paymentDates[24] = LADate("20220228");
        paymentDates[25] = LADate("20220531");
        paymentDates[26] = LADate("20220830");
        paymentDates[27] = LADate("20221130");
        paymentDates[28] = LADate("20230228");
        paymentDates[29] = LADate("20230530");
        paymentDates[30] = LADate("20230830");
        paymentDates[31] = LADate("20231130");
        paymentDates[32] = LADate("20240229");
        paymentDates[33] = LADate("20240530");
        paymentDates[34] = LADate("20240830");
        paymentDates[35] = LADate("20241129");
        paymentDates[36] = LADate("20250228");
        paymentDates[37] = LADate("20250530");
        paymentDates[38] = LADate("20250829");
        paymentDates[39] = LADate("20251128");
        paymentDates[40] = LADate("20260227");
        paymentDates[41] = LADate("20260529");
        paymentDates[42] = LADate("20260828");
        paymentDates[43] = LADate("20261130");
        paymentDates[44] = LADate("20270226");
        paymentDates[45] = LADate("20270528");
        paymentDates[46] = LADate("20270831");
        paymentDates[47] = LADate("20271130");
        paymentDates[48] = LADate("20280229");
        paymentDates[49] = LADate("20280530");

        // 1. Convert Dates to Terms
        DoubleVector termResults = etrading::convertCurveDatesToTerms( asOfDate, paymentDates );

        // Dimensionality Checks
        ASSERT_EQ( terms.size(), paymentDates.size() );
        ASSERT_EQ( termResults.size(), terms.size() );
        
        // 2. Convert Terms back to Payment Dates
        DateVector paymentResults = etrading::convertCurveTermsToDates( asOfDate, termResults );

        // Dimensionality Checks
        ASSERT_EQ( terms.size(), paymentDates.size() );
        ASSERT_EQ( paymentResults.size(), terms.size() );

        // Run Test
        for( size_t i = 0; i < paymentResults.size(); ++i )
        {
            EXPECT_EQ( paymentDates[i].stringWithFormat("YYYYMMDD"),
                       paymentResults[i].stringWithFormat("YYYYMMDD") );
        }
    }
}
