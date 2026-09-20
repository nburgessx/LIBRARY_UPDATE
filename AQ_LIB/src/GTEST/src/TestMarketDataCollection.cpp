// Include: Google Test Library
#include <gTest/gTest.h>


#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <boost/assign.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>


#include "TypeName.h"
#include "Variant.h"
#include "ETradingException.h"


#include "DateUtilities.h"
#include "ContainerUtilities.h"
#include "StatisticsUtilities.h"
#include "ContainerUtilities.h"
#include "UserUtilities.h"
#include "StatisticsUtilities.h"
#include "FileUtilities.h"
#include "ObjectUtilities.h"
#include "EnvironmentPool.h"
#include "CurveData.h"
#include "MarketQuote.h"

using namespace etrading;

namespace google_test
{

    TEST( TestCurveData, UNIT_CurveData )
    {
        const std::vector<std::string> terms = boost::assign::list_of( "1W" )( "1M" )( "1Y" )( "5Y" );
        const std::vector<std::string> fraTerms = boost::assign::list_of( "1x3" )( "12x15" )( "9by12" )( "3BY6" );
        const std::vector<std::string> swapTerms = boost::assign::list_of( "1Y" )( "2Y" )( "5Y" )( "10Y" );
        const std::vector<double> swapRates = boost::assign::list_of( -0.025 )( 0.0111 )( 0.0222 )( 0.0333 );
        const std::vector<double> rates = boost::assign::list_of( 0.025 )( 0.064 )( 0.01411 )( -0.0164 );

        const std::vector<std::string> basisSwapTerms = boost::assign::list_of( "1Y" )( "2Y" )( "5Y" )( "10Y" );
        const std::vector<double> basisSwapRates = boost::assign::list_of( -0.0044 )( -0.0022 )( -0.0055 )( -0.0077 );


        // TermRateQuotes genericQuotes( terms, rates, etrading::FXRATES, etrading::USD, etrading::_12M );

		// MarketQuote.cpp's FRA-tenor-format check validates via AQ_THROW (AQLCoreInvalidData
		// directly) - the commented-out `throw ETradingException(...)` right next to it in
		// MarketQuote.cpp is what this test was originally written against, before the
		// AQ_THROW/boost::format cleanup superseded it.
		EXPECT_THROW(FRAQuotes(terms, rates, etrading::_3M_FRAPERIOD, etrading::EUR), AQLCoreInvalidData );
		FRAQuotes fraQuotes(fraTerms , rates, etrading::_3M_FRAPERIOD, etrading::EUR);

		CurveData mdk("EUR_Collection",etrading::CURVE_TENOR_3M); // TODO: add checks on currency!
		mdk.setFRAQuotes(std::move(fraQuotes));
		
		SwapQuotes sqts(swapTerms, swapRates, etrading::OIS_SWAPTYPE, etrading::EUR, etrading::CURVE_TENOR_1D);
		mdk.setSwapQuotes(std::move(sqts));  // we can still set an OIS swap even though we are looking at 3M curve
		
		SwapQuotes swapQts(swapTerms, swapRates, etrading::STD_SWAPTYPE, etrading::EUR, etrading::CURVE_TENOR_3M);
		mdk.setSwapQuotes(std::move(swapQts));

        BasisSwapQuotes bswapQts(basisSwapTerms, basisSwapRates,  etrading::EUR, etrading::CURVE_TENOR_1D, etrading::CURVE_TENOR_3M);
		mdk.setSwapQuotes(std::move(bswapQts));




    };


}