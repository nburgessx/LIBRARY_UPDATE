////
//// @Description: This is a test program
////
//
//// Include: Google Test Library
//#include <gTest/gTest.h>

// Include: Google Test Library
#include <gTest/gTest.h>

#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <memory>
#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include "DataSchema.h"
#include "CoreEnumerations.h"
#include "FreeObject.h"
#include "TypeName.h"
#include "Variant.h"
#include "DateUtilities.h"
#include "ContainerUtilities.h"
#include "ETradingException.h"
#include "TableDateDouble.h"
#include "ContainerUtilities.h"
#include "ConcurrentMap.h"
#include "EnvironmentAttacker.h"
#include "UserUtilities.h"
#include "SerializeContainedData.h"
#include "StatisticsUtilities.h"
#include "FileUtilities.h"
#include "EnvironmentPool.h"
#include "RuleInterface.h"
#include "AQObjCurve.h"
#include "CurveBuildProperties.h"
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"
#include "InitializeGoogleTest.h"
#include "ExampleObjects.h"
#include "Dependency.h"

using namespace etrading;

namespace google_test
{

    // set the data structure / /etrading::ContainedTypeEnum::
#define SK2(NAME,C1_TYPE,C2_TYPE,C1_NAME,C2_NAME) \
    #NAME, 2, boost::assign::list_of(etrading::##C1_TYPE)(etrading::##C2_TYPE), boost::assign::list_of(#C1_NAME)(#C2_NAME)  \

	// Declare Test will Set-Up the Test Fixture Class which inherits the AlgoQuantLib tear-down / clean-up class
	DECLARE_TEST_FIXTURE(TestAQObjCurve);

    TEST_F( TestAQObjCurve, UNIT_AQObjCurve )
    {
		// Creating an instance of InitializeGoogleTest loads in the calendars.
		// When this object goes out of scope AlgoQuantLib is shut down gracefully.
        // InitializeGoogleTest instance; // Resolved using the DECLARE_TEST_FIXTURE macro above

        auto& env = Environment::defaultEnv();
        auto& curveOutputsInCache = env.getCache<AQObjCurve>();

        EXPECT_THROW( CurveBuildProperties( etrading::SWAP_CURVETYPE, "CBP_for_MyCurveOutput", "EURYC", "OIS",
                                            etrading::EUR, boost::gregorian::date( 2016, 5, 3 ),
                                            etrading::LINEAR_INTERPOLATION, etrading::SEMI_ANNUAL_COMPOUNDING ,
                                            etrading::CURVE_TENOR_6M,  etrading::ARITHMETIC_COMPOUNDING_METHOD, etrading::MOD_FOLLOWING, "TGT", {}, false ),
                      ETradingException );

        CurveBuildProperties conventionUsed(	etrading::OIS_CURVETYPE, "CBP_for_MyCurveOutput", "EURYC", "OIS", etrading::EUR,
                                                boost::gregorian::date( 2016, 5, 3 ), etrading::LINEAR_INTERPOLATION,
                                                etrading::SIMPLE_COMPOUNDING, etrading::CURVE_TENOR_1D,  etrading::ARITHMETIC_COMPOUNDING_METHOD,
                                                etrading::MOD_FOLLOWING, "TGT", {}, false );

        std::vector<boost::gregorian::date> dates = boost::assign::list_of( boost::gregorian::date( 2016, 12, 21 ) )
                ( boost::gregorian::date( 2017, 4, 20 ) )
                ( boost::gregorian::date( 2018, 6, 4 ) )
                ( boost::gregorian::date( 2019, 8, 12 ) )
                ( boost::gregorian::date( 2024, 11, 4 ) );
        std::vector<double> discountFactors = boost::assign::list_of( 0.995 )( 0.98 )( 0.7 )( 0.75 )( 0.65 );
        std::vector<double> forwardRates = boost::assign::list_of( 0.00568 )( 0.00912 )( 0.01444 )( 0.02685 )( 0.01799 );
        AQObjCurve curveOutput( "MyCurveOutput", dates, discountFactors, conventionUsed, forwardRates );

        // Say I wish to keep my local copy and cache a independent copy:
        etrading::copyToCache( curveOutput );
        AQObjCurve myCurveOutputTest = env.copyObject<AQObjCurve>( "MyCurveOutput" );

        // I want to check that the correct object is indeed there
        env.hasObject<AQObjCurve>( "MyCurveOutput" );
        curveOutputsInCache.has( "MyCurveOutput" );
        EXPECT_TRUE( env.hasObject<AQObjCurve>( "MyCurveOutput" ) );

        // I want to remove the object from the cache
        env.deleteObject<AQObjCurve>( "MyCurveOutput" );
        curveOutputsInCache.erase( "MyCurveOutput" );
        EXPECT_FALSE( env.hasObject<AQObjCurve>( "MyCurveOutput" ) );

        // I no longer wish to use my local object and want to move the memory to the cache
        etrading::moveToCache( std::move( curveOutput ) );
        // any use of curveOutput here will give UB

        EXPECT_TRUE( env.hasObject<AQObjCurve>( "MyCurveOutput" ) );

        // now I want to change or do a calculation using the object on the cache
        auto ptrToObj = env.accessObject<AQObjCurve>( "MyCurveOutput" );
        // or
        // auto ptrToObj = curveOutputsInCache.get("MyCurveOutput");
        double my_df = ptrToObj->calculateDiscountFactor( "1Y" );
        std::string nameOfCurveOutput = ptrToObj->getName();
        CurveBuildProperties newConventionUsed(
            etrading::OIS_CURVETYPE, "CBP_for_MyCurveOutput", "EURYC", "OIS", etrading::EUR,
            boost::gregorian::date( 2016, 5, 19 ), etrading::LINEAR_INTERPOLATION,
			etrading::SEMI_ANNUAL_COMPOUNDING, etrading::CURVE_TENOR_1D, etrading::ARITHMETIC_COMPOUNDING_METHOD, etrading::MOD_FOLLOWING,  "TGT", {}, false );
        ptrToObj->setCurveBuildStaticDataObject( newConventionUsed );

        // I have an object on the cache I want to get a local copy of it - the object must have a copy CTOR defined
        AQObjCurve myCurveOutput = env.copyObject<AQObjCurve>( "MyCurveOutput" );

        // I want to create a new object straight on the store and calculate on it
        auto ptrToCurveOutput = CreateObjectOnStore<AQObjCurve>::create( Environment::DEFAULT_ENV_NAME, "AnotherCurveOutput" );
        ptrToCurveOutput->setData( dates, discountFactors, forwardRates );
        ptrToCurveOutput->setCurveBuildStaticDataObject( newConventionUsed );
        double my_df2 = ptrToCurveOutput->calculateDiscountFactor( "3M" );

        const std::string readWriteFileName = ( boost::format( "%s/resource/test/inputs/ETrading/AQObjects/AQObjCurve/%s.json" )
                                                % etrading::getEnvironmentVariable( "AQ" ).c_str() % ptrToCurveOutput->getName().c_str() ).str();

        // I want to serialize an object to a file (local or pointing to an object in the cache)
        ptrToCurveOutput->serialize( etrading::serialize::JSON, etrading::serialize::FILE, readWriteFileName );
        EXPECT_TRUE( fileExists( readWriteFileName ) );

        // or I could have used the interface on the Environment
        auto ptrToCurveOutput2 = env.createObject<AQObjCurve, AQObjCurve>( "YetAnotherCurveOutput" );

        EXPECT_TRUE( env.hasObject<AQObjCurve>( "AnotherCurveOutput" ) );
        curveOutputsInCache.erase( "AnotherCurveOutput" );
        EXPECT_FALSE( env.hasObject<AQObjCurve>( "AnotherCurveOutput" ) );
        // I want to de-serliaze from a file into an enviroment

        auto deserializationInfo = deSerializeFromJSON(  etrading::serialize::FILE, readWriteFileName );
        // whatever we deserialized has now overrwritten any object with the same name in the cache
        EXPECT_TRUE( env.hasObject<AQObjCurve>( "AnotherCurveOutput" ) );
        auto ptrToCheck = env.accessObject<AQObjCurve>( "AnotherCurveOutput" );

        auto info = ptrToCheck->getData( 0 );
        auto date0 = std::get<0>( info );
        auto df0 = std::get<1>( info );
        auto fwdr0 = std::get<2>( info );

        auto info3 = ptrToCheck->getData( 3 );
        auto date3 = std::get<0>( info3 );
        auto df3 = std::get<1>( info3 );
        auto fwdr3 = std::get<2>( info3 );

        std::cout << ptrToCheck->calculateDiscountFactor( "2Y4M12D" ) << std::endl;
        std::cout << ptrToCheck->calculateDiscountFactor( 1.02 ) << std::endl;
        std::cout << ptrToCheck->calculateDiscountFactor( date0 ) << std::endl;
        std::cout << ptrToCheck->calculateDiscountFactor( date3 ) << std::endl;
        std::cout << ptrToCheck->calculateDiscountFactor( date3, date3 ) << std::endl;

        EXPECT_DOUBLE_EQ( 0.995, df0 );
        EXPECT_DOUBLE_EQ( 0.75, df3 );
        EXPECT_DOUBLE_EQ( ptrToCheck->calculateDiscountFactor( date0 ), df0 );
        EXPECT_DOUBLE_EQ( ptrToCheck->calculateDiscountFactor( date3 ), df3 );

        std::cout << ptrToCheck->calculateForwardRate( "2Y4M12D" ) << std::endl;
        std::cout << ptrToCheck->calculateForwardRate( 1.02 ) << std::endl;
        std::cout << ptrToCheck->calculateForwardRate( date0 ) << std::endl;
        std::cout << ptrToCheck->calculateForwardRate( date3 ) << std::endl;
        std::cout << ptrToCheck->calculateForwardRateUsingDiscountFactors( date0, date3, etrading::ACT_ACT_DAYCOUNT ) << std::endl;
        EXPECT_THROW( ptrToCheck->calculateForwardRateUsingDiscountFactors( date3, date3, etrading::ACT_ACT_DAYCOUNT ), ETradingException );

        // ( 0.00568 )( 0.00912 )( 0.01444 )( 0.02685 )( 0.01799 );
        EXPECT_DOUBLE_EQ( 0.00568, fwdr0 );
        EXPECT_DOUBLE_EQ( 0.02685, fwdr3 );
        EXPECT_DOUBLE_EQ( ptrToCheck->calculateForwardRate( date0 ), fwdr0 );
        EXPECT_DOUBLE_EQ( ptrToCheck->calculateForwardRate( date3 ), fwdr3 );

        env.hasObject<AQObjCurve>( "AnotherCurveOutput" );

        // I want to delete a specific object
        env.deleteObject<AQObjCurve>( "YetAnotherCurveOutput" );
        env.hasObject<AQObjCurve>( "YetAnotherCurveOutput" );

        // or I could have accessed the cache directly
        curveOutputsInCache.erase( "MoreCurveOutput" );
        curveOutputsInCache.has( "MoreCurveOutput" );

        // I want to delete all objects of this type
        env.deleteAllObjects<AQObjCurve>();

        // or I could have accessed the cache directly
        curveOutputsInCache.clear();
        // std::tuple<boost::gregorian::date,double,double> getData(const unsigned int idx) const;

    };




}
