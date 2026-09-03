////
//// @File: TestExampleObjects.cpp
//// @Description: This is a test program
//// @Created: 27 Feb 2016
//// @Author: Hans Roggeman
//// @Department: ISO Front Office Development
////
//// The copyright to the computer program(s) herein
//// is the property of AlgoQuantHub.
//
//// Include: Google Test Library
//#include <gTest/gTest.h>
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.

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
#include "LWOCurve.h"
#include "CurveBuildProperties.h"
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"
#include "InitializeAQGoogleTest.h"
#include "ExampleObjects.h"



using namespace etrading;

namespace google_test
{

    TEST( TestExampleObject, UNIT_ExampleObjects )
    {
        StandAlone sa1( "StandAlone_1" );
        moveToCache<StandAlone>( std::move( sa1 ) );

        auto& env = Environment::defaultEnv();
        EXPECT_TRUE( env.hasObject( "StandAlone_1", EXAMPLE_STAND_ALONE ) );
        EXPECT_FALSE( env.hasObject<StandAlone>( "StandAlone_2" ) );

        StandAlone sa2( "StandAlone_2" );
        copyToCache<StandAlone>( sa2 );
        EXPECT_TRUE( env.hasObject<StandAlone>( "StandAlone_1" ) );
        EXPECT_TRUE( env.hasObject( "StandAlone_2", EXAMPLE_STAND_ALONE ) );

        const std::string readWriteFileName = ( boost::format( "%s/resource/test/inputs/ETrading/LWObjects/ExampleObjects/%s.json" )
                                                % etrading::getEnvironmentVariable( "MLIBQ" ).c_str() % sa2.getRefToName().c_str() ).str();

        sa2.serialize( etrading::serialize::JSON, etrading::serialize::FILE, readWriteFileName );
        EXPECT_TRUE( fileExists( readWriteFileName ) );

        env.deleteObject<StandAlone>( sa2.getRefToName() );
        EXPECT_FALSE( env.hasObject( "StandAlone_2", EXAMPLE_STAND_ALONE ) );

        auto deserializationInfo = deSerializeFromJSON( etrading::serialize::FILE, readWriteFileName );
        EXPECT_TRUE( env.hasObject( "StandAlone_2", EXAMPLE_STAND_ALONE ) );

        const auto& objectReadFromFile = env.accessObject<StandAlone>( "StandAlone_2" );
        EXPECT_DOUBLE_EQ( objectReadFromFile.get()->d_, 10.0 );

        env.deleteAllObjects<StandAlone>();

        EXPECT_FALSE( env.hasObject<StandAlone>( "StandAlone_1" ) );

        auto ptrBase = std::make_shared<BaseObject>( "NameOfBaseObject" );
        std::shared_ptr<BaseObject> ptrDerived( new DerivedObject( "NameOfDerivedObject" ) );

        copyToCache<BaseObject>( *( ptrBase.get() ) );
        EXPECT_TRUE( env.hasObject( "NameOfBaseObject", EXAMPLE_BASE ) );
        auto ptrObject = env.accessObject<BaseObject>( "NameOfBaseObject" );
        EXPECT_TRUE( ptrObject != nullptr );
        EXPECT_STREQ( ptrObject.get()->operator std::string().c_str(), "BaseObject" );

        copyToCache<BaseObject>( *( ptrDerived.get() ) );
        EXPECT_TRUE( env.hasObject( "NameOfDerivedObject", EXAMPLE_BASE ) );
        auto ptrObject2 = env.accessObject<BaseObject>( "NameOfDerivedObject" );
        EXPECT_TRUE( ptrObject2 != nullptr );
        EXPECT_STREQ( ptrObject2.get()->operator std::string().c_str(), "BaseObject" ); //! slicing when going in

        registerToCache<BaseObject>( ptrDerived ); // this is how to register correctly through base ptr
        EXPECT_TRUE( env.hasObject( "NameOfDerivedObject", EXAMPLE_BASE ) );
        auto ptrObject3 = env.accessObject<BaseObject>( "NameOfDerivedObject" );
        EXPECT_TRUE( ptrObject3 != nullptr );
        EXPECT_STREQ( ptrObject3.get()->operator std::string().c_str(), "DerivedObject" );

        env.deleteAllObjects<BaseObject>();

    };


};
