
// Include: Google Test Library
#include <gTest/gTest.h>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <boost/assign.hpp>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>

#include "Dependency.h"
#include "InitializeGoogleTest.h"

#include "DataSchema.h"
#include "CoreEnumerations.h"
#include "FreeObject.h"
#include "TypeName.h"
#include "Variant.h"
#include "DateUtilities.h"
#include "ContainerUtilities.h"
#include "ETradingException.h"
#include "TableDateDouble.h"
#include "StatisticsUtilities.h"
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
#include "ExposedInterface.h"
#include "EnvironmentPool.h"
#include "Environment.h"
#include "EnvironmentImplementation.h"
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"

using namespace etrading;
using namespace etrading::environment_implementation;
using namespace std;

namespace google_test
{

#define SK2(NAME,C1_TYPE,C2_TYPE,C1_NAME,C2_NAME) \
    #NAME, 2, boost::assign::list_of(etrading::C1_TYPE)(etrading::C2_TYPE), boost::assign::list_of(#C1_NAME)(#C2_NAME)  \


	DECLARE_TEST_FIXTURE(TestAQObjInfrastructure);

    TEST_F( TestAQObjInfrastructure, UNIT_Variant )
    {

        // boost::gregorian::date testDate = boost::lexical_cast<boost::gregorian::date>(std::string("20010101"));  // bad_cast!!!
        // std::cout << toYYYYMMDDFromGregorianDate(testDate) << std::endl;

        std::vector<int> test_vec = boost::assign::list_of( 43 )( 23 )( 11 )( 98 );
        EXPECT_STREQ( containerAsString( test_vec ).c_str(), "43,23,11,98" );

        Variant cv( "What is this?" );

        EXPECT_TRUE( cv.getType() == etrading::STRING_VALUE );
        EXPECT_STREQ( cv.getValue<std::string>().c_str(), "What is this?" );
        EXPECT_ANY_THROW( cv.getValue<double>() ); // Cast String to Double -> This Should Fail random text to double
        
        Variant cv2( 342.2342 );
        EXPECT_TRUE( cv2.getType() == etrading::DOUBLE_VALUE );
        EXPECT_DOUBLE_EQ( cv2.getValue<double>(), 342.2342 );
        EXPECT_EQ( cv2.getValue<std::string>(), "342.23419999999998709" ); // Cast Double to String -> This Should Pass from 342.2342 to "342.23419999999998709", defaults to 20 significant places

        cv = cv2;

        EXPECT_TRUE( cv.getType() == etrading::DOUBLE_VALUE );
        EXPECT_DOUBLE_EQ( cv.getValue<double>(), 342.2342 );
        EXPECT_EQ( cv.getValue<int>(), 342 ); // Cast Double to Int -> This Should Pass from 342.2342 to 342

        Variant cv3( boost::gregorian::date( 2011, 11, 11 ) );
        EXPECT_EQ( cv3.getValue<boost::gregorian::date>(), boost::gregorian::from_undelimited_string( "20111111" ) );

        Variant cv4( "20111111" );
        //EXPECT_EQ( cv4.getValue<boost::gregorian::date>(), boost::gregorian::from_undelimited_string( "20111111" ) );
        EXPECT_EQ(toGregorianDateFromREGEX( cv4.toString()), boost::gregorian::from_undelimited_string( "20111111" ) );

        Variant cv5( "2011-11-11" );
        //EXPECT_EQ( cv5.getValue<boost::gregorian::date>(), boost::gregorian::from_undelimited_string( "20111111" ) );
        EXPECT_EQ(toGregorianDateFromREGEX( cv5.toString()), boost::gregorian::from_undelimited_string( "20111111" ) );

        Variant cv6( "2011\\11\\11" ); // C++ and backslashes
        //EXPECT_EQ( cv6.getValue<boost::gregorian::date>(), boost::gregorian::from_undelimited_string( "20111111" ) );
        EXPECT_EQ(toGregorianDateFromREGEX( cv6.toString()), boost::gregorian::from_undelimited_string( "20111111" ) );

        Variant cv7( "2011/11/11" );
        //EXPECT_EQ( cv7.getValue<boost::gregorian::date>(), boost::gregorian::from_undelimited_string( "20111111" ) );
        EXPECT_EQ(toGregorianDateFromREGEX( cv7.toString()), boost::gregorian::from_undelimited_string( "20111111" ) );

        Variant cv8( "11-11-2011" );
        //EXPECT_EQ( cv8.getValue<boost::gregorian::date>(), boost::gregorian::from_undelimited_string( "20111111" ) );
        EXPECT_EQ(toGregorianDateFromREGEX( cv8.toString()), boost::gregorian::from_undelimited_string( "20111111" ) );

        Variant cv9( "11/11/2011" );
        //EXPECT_EQ( cv9.getValue<boost::gregorian::date>(), boost::gregorian::from_undelimited_string( "20111111" ) );
        EXPECT_EQ(toGregorianDateFromREGEX( cv9.toString()), boost::gregorian::from_undelimited_string( "20111111" ) );

        std::vector<int> inputValues_i = boost::assign::list_of( 32 )( 41 )( 50 )( 60 );
        std::vector<std::string> inputValues_s = boost::assign::list_of( "AA" )( "bbb" )( "CCCC" )( "dddd" );

        etrading::VariantMatrix variantMatrix;
        variantMatrix.push_back( Variant::createVariantVector( inputValues_i, etrading::INTEGER_VALUE ) );
        variantMatrix.push_back( Variant::createVariantVector( inputValues_s, etrading::STRING_VALUE ) );

        EXPECT_EQ( variantMatrix.at( 0 ).at( 1 ).getValue<int>(), 41 );
        EXPECT_STREQ( variantMatrix.at( 1 ).at( 2 ).toString().c_str(), "CCCC" );

        auto newVariantMatrix = etrading::transpose( variantMatrix );

        EXPECT_EQ( newVariantMatrix.at( 1 ).at( 0 ).getValue<int>(), 41 );
        EXPECT_STREQ( newVariantMatrix.at( 2 ).at( 1 ).toString().c_str(), "CCCC" );

        EXPECT_STREQ( newVariantMatrix.at( 0 ).at( 1 ).toString().c_str(), "AA" );
        EXPECT_STREQ( newVariantMatrix.at( 1 ).at( 1 ).toString().c_str(), "bbb" );

    }

    TEST_F( TestAQObjInfrastructure, UNIT_CreateVariantVector )
    {
        std::vector<int> inputValues_i = boost::assign::list_of( 32 )( 41 )( 50 )( 60 );
        std::vector<Variant> outputValues_i =
            Variant::createVariantVector<int>( inputValues_i, etrading::INTEGER_VALUE );
        // etrading::displayContainer(outputValues);
        EXPECT_EQ( outputValues_i.at( 0 ).getValue<int>(), 32 );
        EXPECT_EQ( outputValues_i.at( 1 ).getValue<int>(), 41 );
        EXPECT_EQ( outputValues_i.at( 2 ).getValue<int>(), 50 );
        EXPECT_EQ( outputValues_i.at( 3 ).getValue<int>(), 60 );
        EXPECT_NE( outputValues_i.at( 3 ).getValue<int>(), 32 );
        EXPECT_TRUE( outputValues_i.size() == 4 );

        std::vector<double> inputValues_d = boost::assign::list_of( 32.33 )( 416.22 )( 50.65 )( 60.56 );
        std::vector<Variant> outputValues_d =
            Variant::createVariantVector<double>( inputValues_d, etrading::DOUBLE_VALUE );
        // etrading::displayContainer(outputValues);
        EXPECT_DOUBLE_EQ( outputValues_d.at( 0 ).getValue<double>(), 32.33 );
        EXPECT_DOUBLE_EQ( outputValues_d.at( 1 ).getValue<double>(), 416.22 );
        EXPECT_DOUBLE_EQ( outputValues_d.at( 2 ).getValue<double>(), 50.65 );
        EXPECT_DOUBLE_EQ( outputValues_d.at( 3 ).getValue<double>(), 60.56 );
        EXPECT_NE( outputValues_d.at( 3 ).getValue<double>(), 32.33 );
        EXPECT_TRUE( outputValues_d.size() == 4 );

        std::vector<std::string> inputValues_s = boost::assign::list_of( "AA" )( "bbb" )( "CCCC" )( "dddd" );
        std::vector<Variant> outputValues_s =
            Variant::createVariantVector<std::string>( inputValues_s, etrading::STRING_VALUE );
        // etrading::displayContainer(outputValues);
        EXPECT_STREQ( outputValues_s.at( 0 ).getValue<std::string>().c_str(), "AA" );
        EXPECT_STREQ( outputValues_s.at( 1 ).getValue<std::string>().c_str(), "bbb" );
        EXPECT_STREQ( outputValues_s.at( 2 ).getValue<std::string>().c_str(), "CCCC" );
        EXPECT_STREQ( outputValues_s.at( 3 ).getValue<std::string>().c_str(), "dddd" );
        EXPECT_NE( outputValues_s.at( 3 ).getValue<std::string>().c_str(), "CCCC" );
        EXPECT_TRUE( outputValues_s.size() == 4 );

        std::vector<boost::gregorian::date> inputValues_dt =
            boost::assign::list_of	( toGregorianDateFromYYYYMMDD( "20110101" ) )
            ( toGregorianDateFromYYYYMMDD( "20130303" ) )
            ( toGregorianDateFromYYYYMMDD( "20140404" ) )
            ( toGregorianDateFromYYYYMMDD( "20990909" ) );
        std::vector<Variant> outputValues_dt =
            Variant::createVariantVector<boost::gregorian::date>( inputValues_dt, etrading::DATE_VALUE );
        // etrading::displayContainer(outputValues);

        EXPECT_STREQ( toYYYYMMDDFromGregorianDate( outputValues_dt.at( 0 ).getValue<boost::gregorian::date>() ).c_str(), "20110101" );
        EXPECT_STREQ( toYYYYMMDDFromGregorianDate( outputValues_dt.at( 1 ).getValue<boost::gregorian::date>() ).c_str(), "20130303" );
        EXPECT_STREQ( toYYYYMMDDFromGregorianDate( outputValues_dt.at( 2 ).getValue<boost::gregorian::date>() ).c_str(), "20140404" );
        EXPECT_STREQ( toYYYYMMDDFromGregorianDate( outputValues_dt.at( 3 ).getValue<boost::gregorian::date>() ).c_str(), "20990909" );
        EXPECT_NE( toYYYYMMDDFromGregorianDate( outputValues_dt.at( 3 ).getValue<boost::gregorian::date>() ).c_str(), "20110101" );
        EXPECT_TRUE( outputValues_dt.size() == 4 );

    }

	TEST_F( TestAQObjInfrastructure, UNIT_VariantMatrixTranspose )
	{
		etrading::VariantMatrix inputMatrix = { { 1, 2 }, {3, 4 } };

		const bool padRaggedMatrix = false;
		etrading::VariantMatrix transposedMatrix = etrading::transpose( inputMatrix, false );

		// main diagonal
		EXPECT_EQ(inputMatrix.at(0).at(0).getValue<int>(), transposedMatrix.at(0).at(0).getValue<int>() );
		EXPECT_EQ(inputMatrix.at(1).at(1).getValue<int>(), transposedMatrix.at(1).at(1).getValue<int>() );

		// off diagonal
		EXPECT_EQ(inputMatrix.at(1).at(0).getValue<int>(), transposedMatrix.at(0).at(1).getValue<int>() );
		EXPECT_EQ(inputMatrix.at(0).at(1).getValue<int>(), transposedMatrix.at(1).at(0).getValue<int>() );
	}

	TEST_F(TestAQObjInfrastructure, UNIT_RaggedVariantMatrixTranspose)
	{
		etrading::VariantMatrix inputMatrix = { { 1, 2 }, {3 } };

		const bool padRaggedMatrix = true;
		etrading::VariantMatrix transposedMatrix = etrading::transpose(inputMatrix, padRaggedMatrix );

		// main diagonal
		EXPECT_EQ(inputMatrix.at(0).at(0).getValue<int>(), transposedMatrix.at(0).at(0).getValue<int>() );
		EXPECT_EQ( std::string(""),						   transposedMatrix.at(1).at(1).getValue<std::string>().c_str()  );

		// off diagonal
		EXPECT_EQ(inputMatrix.at(1).at(0).getValue<int>(), transposedMatrix.at(0).at(1).getValue<int>() );
		EXPECT_EQ(inputMatrix.at(0).at(1).getValue<int>(), transposedMatrix.at(1).at(0).getValue<int>() );
	}


    TEST_F( TestAQObjInfrastructure, UNIT_RuleInterface )
    {
        std::vector<double> testVec = boost::assign::list_of( 1.2 )( 2.231 )( 2253124.23 )( 53245252.3521 );

        AscendingOrderRule<> conditionChecker;
        EXPECT_TRUE( conditionChecker.verify( testVec ) );
        conditionChecker.addRule( BoundaryRule<>( 1.0, std::numeric_limits<double>::max() ) );
        EXPECT_TRUE( conditionChecker.verify( testVec ) );
        conditionChecker.addRule( BoundaryRule<>( 1000.0, 2001.85 ) );
        EXPECT_FALSE( conditionChecker.verify( testVec ) );

        std::vector<int> testVec2 = boost::assign::list_of( 20 )( 30 )( 40 )( 45 )( 24 );
        AscendingOrderRule<std::vector<int>> conditionChecker2;
        EXPECT_FALSE( conditionChecker2.verify( testVec2 ) );
        testVec2.pop_back();
        EXPECT_TRUE( conditionChecker2.verify( testVec2 ) );
        conditionChecker2.addRule( BoundaryRule<std::vector<int>>( 10.5, 46.25 ) );
        EXPECT_TRUE( conditionChecker2.verify( testVec2 ) );
        conditionChecker2.addRule( BoundaryRule<std::vector<int>, int>( 25, 46 ) );
        EXPECT_FALSE( conditionChecker2.verify( testVec2 ) );

        unsigned int testVar = 5;
        EXPECT_TRUE(  ( BoundaryRule<unsigned int, unsigned int>( 2, 10 ) ).verify( testVar ) );
        EXPECT_TRUE(  ( BoundaryRule<unsigned int, unsigned int>( 5, 9 ) ).verify( testVar ) );
        EXPECT_FALSE(  ( BoundaryRule<unsigned int, int>( 0, 2 ) ).verify( testVar ) );
        // the below will not compile instead of giving an exception (which is better than giving an exception at runtime)
        // EXPECT_THROW(AscendingOrderRule<unsigned int> conditionChecker3, ETradingException);

    };

    // a simulation of multiple clients registering
    void register_client( const std::string& myID )
    {
        const std::string clientName = myID + "_from_TestBusinessObject_WorkContext_testfunction";
        boost::this_thread::sleep( boost::posix_time::milliseconds( sampleInteger( 0, 100 ) ) );
        bool isEnvironmentPresent = etrading::EnvironmentPool::getInstance().isPresent( myID );
        ASSERT_EQ( createEnvironment( clientName ), !isEnvironmentPresent );
    }

    TEST_F( TestAQObjInfrastructure, UNIT_Environment )
    {
        // server kicks off...
        etrading::EnvironmentPool::getInstance();

        // clients start to create their work contexts
        std::vector<boost::thread> clientRegistrationThreads;
        std::vector<std::string> clientNames = boost::assign::list_of( "Gautama" )( "Beelzebub" )( "Jesus" )( "Yongyan" )( "Campbell" )( "Ghost" );
        for( unsigned int counter = 0u; counter < clientNames.size(); counter++ )
        {
            clientRegistrationThreads.push_back( boost::thread( register_client, std::ref( clientNames.at( counter ) )));
        }
        std::for_each( clientRegistrationThreads.begin(), clientRegistrationThreads.end(), std::mem_fn( &boost::thread::join ) );

        // check that the registration happened
        bool workCtxtWasCreatedAgainFromMainThread = createEnvironment( "Ghost_from_TestBusinessObject_WorkContext_testfunction" );
        EXPECT_FALSE( workCtxtWasCreatedAgainFromMainThread );

        // register a new client in the main thread
        const std::string CLIENT_NAME = "TestBusinessObject_WorkContext_testfunction";
        bool workCtxtWasCreated = createEnvironment( CLIENT_NAME );
        EXPECT_TRUE( etrading::EnvironmentPool::getInstance().isPresent( CLIENT_NAME ) );
        bool workCtxtWasCreatedAgain = createEnvironment( CLIENT_NAME );
        EXPECT_FALSE( workCtxtWasCreatedAgain );

        //  create a free object
        FreeObject freeObject( "MyFreeObject" );

        DataSchema  sk_2DMatrix( SK2( MATRIX2D, DOUBLE_VALUE, DOUBLE_VALUE, DOUBLE_X, DOUBLE_Y ) );
        std::vector<DataSchema> sk_Vec = boost::assign::list_of( sk_2DMatrix );
        freeObject.setDataSchemas( sk_Vec );

        // set the data itself
		freeObject.setColumnData<double>("MATRIX2D", 0, boost::assign::list_of(1.2)(2.231)(2253124.23)(53245252.3521)(24.124314));
        freeObject.setColumnData<double>("MATRIX2D", 1, boost::assign::list_of( 1.231 )( 23.1 )( 212314364.23 )( 1536724576.44 )( 91224.124314 ) );

        // place the free object on the cache object
        auto& env = *( EnvironmentPool::getInstance().getEnvironment( CLIENT_NAME ).get() );
        copyToCache( freeObject, env );

        // check that the object is on the store and that we can retrieve values from it
        auto& freeObjectStore = getObjectStore<FreeObject>( env.getUID() );

        EXPECT_TRUE( freeObjectStore.has( "MyFreeObject" ) );
        EXPECT_DOUBLE_EQ( freeObjectStore.get( "MyFreeObject" ).get()->getValueCopy( "MATRIX2D", 3, 0 ).getValue<double>(), 53245252.3521 );
        EXPECT_DOUBLE_EQ( freeObjectStore.get( "MyFreeObject" ).get()->getValueCopy( "MATRIX2D", 0, 1 ).getValue<double>(), 1.231 );
        EXPECT_DOUBLE_EQ( freeObjectStore.get( "MyFreeObject" ).get()->getValueCopy( "MATRIX2D", 1, 1 ).getValue<double>(), 23.1 );

        // create a copy
        FreeObject freeObjectCopy = ( *freeObjectStore.get( "MyFreeObject" ).get() );

        // remove it from the store
        freeObjectStore.clear();

        // check that the copy still has values
        EXPECT_DOUBLE_EQ( freeObjectCopy.getValueCopy( "MATRIX2D", 3, 0 ).getValue<double>(), 53245252.3521 );

        // confirm that it is gone from the store
        EXPECT_FALSE( freeObjectStore.has( "MyFreeObject" ) );

        // attempt to break the store
        std::vector<boost::thread> contextPoolBreakers;
        for( unsigned int counter = 0u; counter < contextPoolBreakers.size(); counter++ )
        {
            contextPoolBreakers.push_back( boost::thread( EnvironmentAttacker() ) );
        }
        std::for_each( contextPoolBreakers.begin(), contextPoolBreakers.end(), std::mem_fn( &boost::thread::join ) );

        auto envToBreak = EnvironmentPool::getInstance().getEnvironment(EnvironmentAttacker::CTXT_TOBREAK);
        std::string keysInContainer = containerAsString( envToBreak->getCache<FreeObject>().keys() );

        std::ostringstream logMsg;
        logMsg << "Main Thread sees the FreeObject Cache for the Workcontext " << EnvironmentAttacker::CTXT_TOBREAK << " with Keys: " << keysInContainer;
        logToConsole( logMsg.str() );

        auto& tableDateDoubleStore = getObjectStore<TableDateDouble>( env.getUID() );

        // Create Another Table Date Double
        // --------------------------------
        std::shared_ptr<TableDateDouble> ptrToObject = CreateObjectOnStore<TableDateDouble, TableDateDouble>::create( CLIENT_NAME, "AnotherTDD" );

        EXPECT_TRUE( tableDateDoubleStore .has( "AnotherTDD" ) );
        EXPECT_TRUE( ptrToObject != nullptr );
        // --------------------------------

        // Create Another Work Context Environment
        // ---------------------------------------
        bool anotherWrkCtxtCreated = createEnvironment( "MyWorkContext" );
        auto wrkCtxt = EnvironmentPool::getInstance().getEnvironment( "MyWorkContext" );
        auto& tableDateDoubleStore2 = getObjectStore<TableDateDouble>( "MyWorkContext" );
        
        auto ptrToObject2 = wrkCtxt->createObject<TableDateDouble, TableDateDouble>( "MyTableDateDouble" );
        EXPECT_TRUE( tableDateDoubleStore2.has( "MyTableDateDouble" ) );
        auto ptrToObject3 = wrkCtxt->createObject<TableDateDouble, TableDateDouble>( "MyTableDateDouble" ); // no default template arguments on a member function in C++11 :(
        EXPECT_TRUE( tableDateDoubleStore2.has( "MyTableDateDouble" ) );
        // ---------------------------------------

		// accessObjectInterface
		auto aqObjPtr = wrkCtxt->accessObjectInterface<TableDateDouble>("MyTableDateDouble");

		EXPECT_FALSE(aqObjPtr == nullptr);
		EXPECT_STREQ(aqObjPtr->getRefToName().c_str(), "MyTableDateDouble" );
		EXPECT_TRUE(aqObjPtr->getEnumType() == etrading::TABLE);
		EXPECT_TRUE(dynamic_cast<TableDateDouble*>(aqObjPtr.get()) != nullptr);

        // Clean-Up for Repeated Tests
        resetEnvironment();
    };



    // Test Class: Initialize to 0, Shared Pointer Copy Method adds 10
    struct HasCloneClass 
    {
        HasCloneClass(const int value = 0) : value_(value) {}; 
        virtual std::shared_ptr<HasCloneClass> clone() const
        {
            return std::make_shared<HasCloneClass>(value_ + 10);
        };
        int value_;
    };

    // Test Class: Initialize to 1, Shared Pointer Copy Method adds 20
    struct DerivedHasCloneClass : public HasCloneClass
    {
        DerivedHasCloneClass(const int value = 1) : HasCloneClass(value) {}; 
        virtual std::shared_ptr<HasCloneClass> clone() const
        {
            return std::make_shared<DerivedHasCloneClass>(HasCloneClass::value_ + 20);
        };
    };

    // Test Class: Initialize to 1000, Shared Pointer Copy Method adds zero
    struct DoesNotHaveCloneClass 
    {
        DoesNotHaveCloneClass(const int value = 1000) : value_(value) {}; 
        int value_;
    };

    TEST_F(TestAQObjInfrastructure, UNIT_copySharedPtrToSharedPtr)
    {

        std::shared_ptr<HasCloneClass> ptrBase = std::make_shared<HasCloneClass>();
        std::shared_ptr<HasCloneClass> ptrDerived = std::make_shared<DerivedHasCloneClass>();
        std::shared_ptr<DoesNotHaveCloneClass> ptrNoClone = std::make_shared<DoesNotHaveCloneClass>();

        auto copyOfA = copySharedPtrToSharedPtr(ptrBase);      // Copy method adds 10
        auto copyOfB = copySharedPtrToSharedPtr(ptrDerived);   // Copy method adds 20
        auto copyOfZ = copySharedPtrToSharedPtr(ptrNoClone);   // Copy method adds zero

        // Check the constuctors have been initialized as expected
        EXPECT_EQ(ptrBase->value_,0);
        EXPECT_EQ(ptrDerived->value_,1);
        EXPECT_EQ(ptrNoClone->value_,1000);
        
        // Check the copy methods increment the initialized values as expected
        EXPECT_EQ(copyOfA->value_,10);   
        EXPECT_EQ(copyOfB->value_,21);
        EXPECT_EQ(copyOfZ->value_,1000);

        copyOfZ->value_ = 2000;
        EXPECT_EQ(ptrNoClone->value_,1000);
        EXPECT_EQ(copyOfZ->value_,2000);

    };

    TEST_F( TestAQObjInfrastructure, UNIT_Serialization )
    {
        const std::string WORK_CTXT_NAME = "MyWorkContext";
        EnvironmentPool::getInstance().createEnvironment( WORK_CTXT_NAME );

        auto& env = ( *EnvironmentPool::getInstance().getEnvironment( WORK_CTXT_NAME ).get() );
        auto& tableDateDoubleStore = getObjectStore<TableDateDouble>( WORK_CTXT_NAME );
        auto& freeObjectStore = getObjectStore<FreeObject>( WORK_CTXT_NAME );

        EXPECT_FALSE( tableDateDoubleStore.has( "MyTable" ) );

        TableDateDouble tdd( "MyTable" );
        std::vector<boost::gregorian::date> dates = boost::assign::list_of	( boost::gregorian::date( 2043, 3, 4 ) )
                ( boost::gregorian::date( 2044, 3, 4 ) )
                ( boost::gregorian::date( 2044, 5, 4 ) )
                ( boost::gregorian::date( 2144, 3, 4 ) );
        std::vector<double> values = boost::assign::list_of( 563.34 )( 2342.907 )( 123.52 )( 1950.32 );
        tdd.setDates( dates );
        tdd.setValues( values );

        std::string serializedTdd = tdd.serialize( etrading::serialize::JSON, etrading::serialize::STRING );
        std::cout << serializedTdd << std::endl;

        auto cacheInfoOnDeserialization = deSerializeFromJSON( etrading::serialize::STRING, serializedTdd, env );

        EXPECT_STREQ( cacheInfoOnDeserialization.first.c_str(), "MyTable" );
        EXPECT_TRUE( cacheInfoOnDeserialization.second == etrading::TABLE );
        EXPECT_TRUE( tableDateDoubleStore.has( "MyTable" ) );
        EXPECT_DOUBLE_EQ( tableDateDoubleStore.get( "MyTable" )->getData( 0 ).second, 563.34 );

        std::string directoryToRW = "H:\\Temp\\" ;
        if( directoryExists( directoryToRW ) )
        {
            std::string fileNameToWriteTo = directoryToRW  +  tdd.getName() + "/.json";

            if( fileExists( fileNameToWriteTo ) )
            {
                // delete the file if it exists
                deleteFile( fileNameToWriteTo );
                logToConsole( std::string( "Deleted an existing file: " ) + fileNameToWriteTo );
            }

            tdd.serialize( etrading::serialize::JSON, etrading::serialize::FILE, fileNameToWriteTo );
            EXPECT_TRUE( fileExists( fileNameToWriteTo ) );
            tableDateDoubleStore.erase( "MyTable" );
            EXPECT_FALSE( tableDateDoubleStore.has( "MyTable" ) );

            auto cacheInfoOnDeserialization2 = env.deSerializeFromJSON(  etrading::serialize::FILE, fileNameToWriteTo );

            EXPECT_STREQ( cacheInfoOnDeserialization2.first.c_str(), "MyTable" );
            EXPECT_TRUE( cacheInfoOnDeserialization2.second == etrading::TABLE );
            EXPECT_TRUE( tableDateDoubleStore.has( "MyTable" ) );
            EXPECT_DOUBLE_EQ( tableDateDoubleStore.get( "MyTable" )->getData( 0 ).second, 563.34 );
        }

        EXPECT_FALSE( freeObjectStore.has( "MyFreeObject" ) );

        FreeObject freeObject( "MyFreeObject" );
        DataSchema  sk_2DMatrix( SK2( MATRIX2D, DOUBLE_VALUE, DOUBLE_VALUE, DOUBLE_X, DOUBLE_Y ) );
        DataSchema  sk_2DSettings( SK2( STRING2D, STRING_VALUE, VARIANT_VALUE, VARNAME, VARVALUE ) );
        std::vector<DataSchema> sk_Vec = boost::assign::list_of( sk_2DMatrix )( sk_2DSettings );
        freeObject.setDataSchemas( sk_Vec );

        // set the data itself
        freeObject.setColumnData<double>( "MATRIX2D", 0, boost::assign::list_of( 1.2 )( 2.231 )( 2253124.23 )( 53245252.3521 )( 24.124314 ) );
        freeObject.setColumnData<double>( "MATRIX2D", 1, boost::assign::list_of( 1.231 )( 23.1 )( 212314364.23 )( 1536724576.44 )( 91224.124314 ) );
        freeObject.setColumnData<std::string>( "STRING2D", "VARNAME", boost::assign::list_of( "ALTITUDE" )( "STYLE" ) );
        std::vector<Variant> variableValues2;
        variableValues2.push_back( Variant( 9798.64 ) );
        variableValues2.push_back( Variant( "EBULLIENT" ) );
        freeObject.setColumnData( "STRING2D", "VARVALUE", variableValues2 );

        std::string serializedFO = freeObject.serialize( etrading::serialize::JSON, etrading::serialize::STRING );
        std::cout << serializedFO << std::endl;

        auto deSerializationResult = deSerializeFromJSON(  etrading::serialize::STRING, serializedFO, env );

        std::string nameOfObject = deSerializationResult.first;

        EXPECT_STREQ( nameOfObject.c_str(), "MyFreeObject" );
        EXPECT_TRUE( deSerializationResult.second == etrading::FREE_OBJECT );
        EXPECT_TRUE( freeObjectStore.has( "MyFreeObject" ) );

        freeObject.clearAll();

        EXPECT_DOUBLE_EQ( freeObjectStore.get( "MyFreeObject" ).get()->getValueCopy( "MATRIX2D", 3, 0 ).getValue<double>(), 53245252.3521 );
        EXPECT_DOUBLE_EQ( freeObjectStore.get( "MyFreeObject" ).get()->getValueCopy( "MATRIX2D", 0, 1 ).getValue<double>(), 1.231 );
        EXPECT_DOUBLE_EQ( freeObjectStore.get( "MyFreeObject" ).get()->getValueCopy( "MATRIX2D", 1, 1 ).getValue<double>(), 23.1 );

        EXPECT_STREQ( freeObjectStore.get( "MyFreeObject" ).get()->getValueCopy( "STRING2D", 1, 0 ).getValue<std::string>().c_str(), "STYLE" );
        EXPECT_DOUBLE_EQ( freeObjectStore.get( "MyFreeObject" ).get()->getValueCopy( "STRING2D", 0, 1 ).getValue<double>(), 9798.64 );

        // Clean-Up for Repeated Tests
        resetEnvironment();
    };

    TEST_F( TestAQObjInfrastructure, UNIT_FreeObject )
    {
        FreeObject freeObject( "IamFreelyChangeable" );

        EXPECT_EQ( freeObject.getEnumType(), etrading::FREE_OBJECT );

#define SK2(NAME,C1_TYPE,C2_TYPE,C1_NAME,C2_NAME) \
    #NAME, 2, boost::assign::list_of(etrading::C1_TYPE)(etrading::C2_TYPE), boost::assign::list_of(#C1_NAME)(#C2_NAME)  \

		DataSchema  sk_2DMatrix(  SK2( MATRIX2D , DOUBLE_VALUE , DOUBLE_VALUE, DOUBLE_X, DOUBLE_Y )   );

		EXPECT_STREQ( sk_2DMatrix.getName().c_str(), "MATRIX2D" );
		EXPECT_TRUE( sk_2DMatrix.hasColumnNames() );
		EXPECT_EQ( sk_2DMatrix.getNumberOfColumns(), 2 );
		EXPECT_STREQ( sk_2DMatrix.getColumnNamesRef().at( 0 ).c_str(), "DOUBLE_X" );
		EXPECT_STREQ( sk_2DMatrix.getColumnNamesRef().at( 1 ).c_str(), "DOUBLE_Y" );

		std::vector<DataSchema> sk_Vec = boost::assign::list_of( sk_2DMatrix );

		freeObject.setDataSchemas( sk_Vec );

		freeObject.setColumnData<double>( "MATRIX2D", 0,
								  boost::assign::list_of( 1.2 )( 2.231 )( 2253124.23 )( 53245252.3521 )( 24.124314 ) );
		freeObject.setColumnData<double>( "MATRIX2D", 1,
								  boost::assign::list_of( 1.231 )( 23.1 )( 212314364.23 )( 1536724576.44 )( 91224.124314 ) );

		Variant aValue = freeObject.getValueCopy( "MATRIX2D", 0, 0 );
		EXPECT_DOUBLE_EQ( aValue.getValue<double>(), 1.2 );

		EXPECT_DOUBLE_EQ( freeObject.getValueCopy( "MATRIX2D", 3, 0 ).getValue<double>(), 53245252.3521 );
		EXPECT_DOUBLE_EQ( freeObject.getValueCopy( "MATRIX2D", 0, 1 ).getValue<double>(), 1.231 );
		EXPECT_DOUBLE_EQ( freeObject.getValueCopy( "MATRIX2D", 3, 1 ).getValue<double>(), 1536724576.44 );

		EXPECT_TRUE( freeObject.getValuesCopy( "MATRIX2D", 0 ).size() == 5 );
		EXPECT_TRUE( freeObject.getValuesCopy( "MATRIX2D", 1 ).size() == 5 );

		EXPECT_TRUE( freeObject.getValuesCopy( "MATRIX2D", 0, false ).size() == 2 );
		EXPECT_TRUE( freeObject.getValuesCopy( "MATRIX2D", 1, false ).size() == 2 );

		std::vector<Variant> secondRow = freeObject.getValuesCopy( "MATRIX2D", 1, false );

		EXPECT_DOUBLE_EQ( secondRow.at( 0 ).getValue<double>(),  2.231 );
		EXPECT_DOUBLE_EQ( secondRow.at( 1 ).getValue<double>(),  23.1 );

		std::vector<Variant> fourthRow = freeObject.getValuesCopy( "MATRIX2D", 3, false );

		EXPECT_DOUBLE_EQ( fourthRow.at( 0 ).getValue<double>(),  53245252.3521 );
		EXPECT_DOUBLE_EQ( fourthRow.at( 1 ).getValue<double>(),  1536724576.44 );

		std::vector<Variant> secondColumn = freeObject.getValuesCopy( "MATRIX2D", 1, true );

		EXPECT_DOUBLE_EQ( secondColumn.at( 0 ).getValue<double>(),  1.231 );
		EXPECT_DOUBLE_EQ( secondColumn.at( 4 ).getValue<double>(),  91224.124314 );

		// DataHolder's bounds checks all validate via AQ_THROW (AQLCoreInvalidData directly), not
		// ETradingException - same AQ_THROW/boost::format cleanup fallout as the other tests in
		// this batch.
		EXPECT_THROW( freeObject.getValuesCopy( "MATRIX2D", 2, true ), AQLCoreInvalidData );
		EXPECT_THROW( freeObject.getValuesCopy( "MATRIX2D", 5, false ), AQLCoreInvalidData );

		EXPECT_THROW( freeObject.getValueCopy( "MATRIX2D", 0, 5 ), AQLCoreInvalidData );
		EXPECT_THROW( freeObject.getValueCopy( "MATRIX2D", 2, 4 ), AQLCoreInvalidData );

		freeObject.clearAll();

		EXPECT_THROW( freeObject.getValuesCopy( "MATRIX2D", 0 ), AQLCoreInvalidData );
		EXPECT_THROW( freeObject.getValuesCopy( "MATRIX2D", 1 ), AQLCoreInvalidData );

#define SK3(NAME,C1_TYPE,C2_TYPE,C3_TYPE, C1_NAME,C2_NAME, C3_NAME) \
	#NAME, 3, boost::assign::list_of(etrading::C1_TYPE)(etrading::C2_TYPE)(etrading::C3_TYPE), boost::assign::list_of(#C1_NAME)(#C2_NAME)(#C3_NAME)   \

		DataSchema  sk_2DInfo(  SK2( INFO2D ,STRING_VALUE , VARIANT_VALUE, VARIABLE_NAME, VARIABLE_VALUE )   );
		DataSchema  sk_SwapVariables(  SK3( SWAP_VARIABLES, DOUBLE_VALUE, DOUBLE_VALUE, DATE_VALUE, NOTIONAL, COUPON, MATURITY )   );

		std::vector<DataSchema> sk_SecondUsage = boost::assign::list_of( sk_2DInfo )( sk_SwapVariables );

		freeObject.setDataSchemas( sk_SecondUsage );

		freeObject.setColumnData<std::string>( "INFO2D", 0, boost::assign::list_of( "A" )( "B" )( "C" ) );
		std::vector<Variant> variableValues;  // TODO: would std::initializer_list work here ? C++11
		variableValues.push_back( Variant( 1.231 ) );
		variableValues.push_back( Variant( std::string( "Business_Casual" ) ) );
		variableValues.push_back( Variant( 46 ) );
		freeObject.setColumnData( "INFO2D", "VARIABLE_VALUE", variableValues );

		freeObject.setColumnData<double>( "SWAP_VARIABLES", 0,
								  boost::assign::list_of( 1E6 )( 25E6 )( 1E8 ) );
		freeObject.setColumnData<double>( "SWAP_VARIABLES", "COUPON",
								  boost::assign::list_of( 0.06 )( 0.01 )( 0.15 ) );
		freeObject.setColumnData<boost::gregorian::date>( "SWAP_VARIABLES", "MATURITY",
								  boost::assign::list_of( toGregorianDateFromYYYYMMDD( "20240404" ) )
										  ( toGregorianDateFromYYYYMMDD( "20930303" ) )
										  ( toGregorianDateFromYYYYMMDD( "20990909" ) ) );
		EXPECT_TRUE( freeObject.getValuesCopy( "INFO2D", 0 ).size() != 5 );
		EXPECT_TRUE( freeObject.getValuesCopy( "INFO2D", 1 ).size() != 5 );
		EXPECT_TRUE( freeObject.getValuesCopy( "INFO2D", 0 ).size() == 3 );
		EXPECT_TRUE( freeObject.getValuesCopy( "INFO2D", 1 ).size() == 3 );

		// INFO2D

		EXPECT_STREQ( freeObject.getValueCopy( "INFO2D", 2, 0 ).getValue<std::string>().c_str(), "C" );
		EXPECT_DOUBLE_EQ( freeObject.getValueCopy( "INFO2D", 0, 1 ).getValue<double>(), 1.231 );
		EXPECT_STREQ( freeObject.getValueCopy( "INFO2D", 1, 1 ).getValue<std::string>().c_str(), "Business_Casual" );
		EXPECT_EQ( freeObject.getValueCopy( "INFO2D", 2, 1 ).getValue<int>(), 46 );

		EXPECT_DOUBLE_EQ( freeObject.getValueCopy( "SWAP_VARIABLES", 0, 0 ).getValue<double>(), 1E6 );
		EXPECT_DOUBLE_EQ( freeObject.getValueCopy( "SWAP_VARIABLES", 1, 0 ).getValue<double>(), 25E6 );

		EXPECT_DOUBLE_EQ( freeObject.getValueCopy( "SWAP_VARIABLES", 0, 1 ).getValue<double>(), 0.06 );
		EXPECT_DOUBLE_EQ( freeObject.getValueCopy( "SWAP_VARIABLES", 2, 1 ).getValue<double>(), 0.15 );

		EXPECT_EQ( freeObject.getValueCopy( "SWAP_VARIABLES", 1, 2 ).getValue<boost::gregorian::date>(), toGregorianDateFromYYYYMMDD( "20930303" ) );
		EXPECT_EQ( freeObject.getValueCopy( "SWAP_VARIABLES", 2, 2 ).getValue<boost::gregorian::date>(), toGregorianDateFromYYYYMMDD( "20990909" ) );

		freeObject.clearData();

		EXPECT_TRUE( freeObject.getValuesCopy( "INFO2D", 0 ).size() == 0 );
		EXPECT_TRUE( freeObject.getValuesCopy( "SWAP_VARIABLES", 0 ).size() == 0 );

		freeObject.setColumnData<std::string>( "INFO2D", 0, boost::assign::list_of( "W" )( "X" )( "Y" )( "Z" ) );
		std::vector<Variant> variableValues2;  // TODO: would std::initializer_list work here ? C++11
		variableValues2.push_back( Variant( "Some_Information" ) );
		variableValues2.push_back( Variant( "Business_Casual" ) );
		variableValues2.push_back( Variant( 9798.64 ) );
		variableValues2.push_back( Variant( toGregorianDateFromYYYYMMDD( "21991019" ) ) );
		freeObject.setColumnData( "INFO2D", "VARIABLE_VALUE", variableValues2 );

		EXPECT_STREQ( freeObject.getValueCopy( "INFO2D", 3, 0 ).getValue<std::string>().c_str(), "Z" );
		EXPECT_STREQ( freeObject.getValueCopy( "INFO2D", 0, 1 ).getValue<std::string>().c_str(), "Some_Information" );
		EXPECT_STREQ( freeObject.getValueCopy( "INFO2D", 1, 1 ).getValue<std::string>().c_str(), "Business_Casual" );
		EXPECT_DOUBLE_EQ( freeObject.getValueCopy( "INFO2D", 2, 1 ).getValue<double>(), 9798.64 );
		EXPECT_EQ(	freeObject.getValueCopy( "INFO2D", 3, 1 ).getValue<boost::gregorian::date>(),
					toGregorianDateFromYYYYMMDD( "21991019" ) );

		freeObject.clearData();

		std::map<std::string, Variant> dataToSet;
		dataToSet["Key1"] = 465.46;
		dataToSet["Key2"] = 46;
		dataToSet["Key3"] = "String as std::string";
		dataToSet["Key4"] = "20090909"; // Now a date like this will be interpreted as an int rather than boost::gregorian::date
		dataToSet["Key5"] = false;
		dataToSet["Key6"] = std::string( "More String" );
		dataToSet["Key7"] = boost::gregorian::date( 2099, 9, 9 );

		freeObject.setDataForSchemaWithMap( "INFO2D", dataToSet );

		EXPECT_DOUBLE_EQ( freeObject.getValueCopy( "INFO2D", 0, 1 ).getValue<double>(), 465.46 );
		EXPECT_EQ( freeObject.getValueCopy( "INFO2D", 1, 1 ).getValue<int>(), 46 );
		EXPECT_STREQ( freeObject.getValueCopy( "INFO2D", 2, 1 ).getValue<std::string>().c_str(), "String as std::string" );

		//EXPECT_EQ( freeObject.getValueCopy( "INFO2D", 3, 1 ).getValue<boost::gregorian::date>(), boost::gregorian::from_undelimited_string( "20090909" ) );
		EXPECT_EQ( toGregorianDateFromREGEX( freeObject.getValueCopy( "INFO2D", 3, 1 ).toString()), boost::gregorian::from_undelimited_string( "20090909" ) );

		EXPECT_FALSE( freeObject.getValueCopy( "INFO2D", 4, 1 ).getValue<bool>() );

	}

}
