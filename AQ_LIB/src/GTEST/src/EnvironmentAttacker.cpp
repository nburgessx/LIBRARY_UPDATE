#include "EnvironmentAttacker.h"

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
#include "UserUtilities.h"

using etrading::ETradingException;
using etrading::FreeObject;
using etrading::DataSchema;
using etrading::sampleInteger;
using etrading::rUniform;
using etrading::containerAsString;
using etrading::logToConsole;
using etrading::ContainedTypeEnum;


namespace google_test
{
    // set the data structure / /etrading::ContainedTypeEnum::
#define SK2(NAME,C1_TYPE,C2_TYPE,C1_NAME,C2_NAME) \
    #NAME, 2, boost::assign::list_of(etrading::C1_TYPE)(etrading::C2_TYPE), boost::assign::list_of(#C1_NAME)(#C2_NAME)  \

    const std::string EnvironmentAttacker::CTXT_TOBREAK = "Ghost_from_TestBusinessObject_WorkContext_testfunction";

    EnvironmentAttacker::EnvironmentAttacker()  : numberOfAttempts_( sampleInteger( 500, 600 ) ),
        wrkContextToCrack_( etrading::EnvironmentPool::getInstance().getEnvironment( CTXT_TOBREAK ) )
    {};

    void EnvironmentAttacker::operator()()
    {
        if( wrkContextToCrack_.get() == nullptr )
        {
            return;
        }

        for( int counter = 0; counter < numberOfAttempts_; counter++ )
        {
            int actionNumber = sampleInteger( 0, 2000 );

            if( actionNumber > 1750 )
            {
                wrkContextToCrack_.get()->getCache<FreeObject>().clear();
#if defined(_DEBUG)
                // Comment this out or run the RELEASE mode if you really want to test this because the logToConsole creates unnatural throttling
                if( counter % 120 == 0 )
                {
                    std::string toLog = ( boost::format( "Thread %s (%i of %i) cleared the FreeObject Cache for the Workcontext %s" )
                                          % boost::lexical_cast<std::string>( boost::this_thread::get_id() )
                                          % boost::lexical_cast<std::string>( counter )
                                          % boost::lexical_cast<std::string>( numberOfAttempts_ )
                                          % wrkContextToCrack_.get()->getUID().c_str() ).str();
                    logToConsole( toLog );
                    std::string toLogAfter = ( boost::format( "Thread %s (%i of %i) sees a FreeObject Cache of size %i for the Workcontext %s" )
                                               % boost::lexical_cast<std::string>( boost::this_thread::get_id() )
                                               % boost::lexical_cast<std::string>( counter )
                                               % boost::lexical_cast<std::string>( numberOfAttempts_ )
                                               % wrkContextToCrack_.get()->getCache<FreeObject>().size()
                                               % wrkContextToCrack_.get()->getUID().c_str()
                                             ).str();
                    logToConsole( toLogAfter );
                }
#endif
            }
            else
            {
                std::string objectName = "MyFreeObject_";
                FreeObject freeObject( objectName );
                DataSchema  sk_2DMatrix( SK2( MATRIX2D, DOUBLE_VALUE, DOUBLE_VALUE, DOUBLE_X, DOUBLE_Y ) );
                std::vector<DataSchema> sk_Vec = boost::assign::list_of( sk_2DMatrix );
                freeObject.setDataSchemas( sk_Vec );
                std::vector<double> xVec;
                const int numberOfRows = sampleInteger( 2, 10 );
                for( int xcounter = 0; xcounter < numberOfRows; xcounter++ )
                {
                    xVec.push_back( rUniform( rUniform( 0.0, 10.0 ), rUniform( 10.0, 100.0 ) ) );
                }
                std::vector<double> yVec;
                for( int ycounter = 0; ycounter < numberOfRows; ycounter++ )
                {
                    yVec.push_back( rUniform( rUniform( 0.0, 10.0 ), rUniform( 10.0, 100.0 ) ) );
                }
                freeObject.setColumnData( "MATRIX2D", 0, xVec );
                freeObject.setColumnData( "MATRIX2D", 1, yVec );
                objectName += boost::lexical_cast<std::string>( numberOfRows );
                wrkContextToCrack_.get()->getCache<FreeObject>().set( objectName, std::make_shared<FreeObject>( freeObject ) );
#if defined(_DEBUG)
                // Comment this out or run the RELEASE mode if you really want to test this because the logToConsole creates unnatural throttling
                if( counter % 120 == 0 )
                {
                    std::string toLog = ( boost::format( "Thread %s (%i of %i) set %s on the FreeObject Cache for the Workcontext %s" )
                                          % boost::lexical_cast<std::string>( boost::this_thread::get_id() )
                                          % boost::lexical_cast<std::string>( counter )
                                          % boost::lexical_cast<std::string>( numberOfAttempts_ )
                                          % objectName
                                          % wrkContextToCrack_.get()->getUID().c_str() ).str();
                    logToConsole( toLog );
                    std::string toLogAfter = ( boost::format( "Thread %s (%i of %i) sees a FreeObject Cache of size %i with Keys %s for the Workcontext %s" )
                                               % boost::lexical_cast<std::string>( boost::this_thread::get_id() )
                                               % boost::lexical_cast<std::string>( counter )
                                               % boost::lexical_cast<std::string>( numberOfAttempts_ )
                                               % wrkContextToCrack_.get()->getCache<FreeObject>().size()
                                               % containerAsString( wrkContextToCrack_.get()->getCache<FreeObject>().keys() )
                                               % wrkContextToCrack_.get()->getUID().c_str()
                                             ).str();
                    logToConsole( toLogAfter );
                }
#endif
            }

            if( counter % 150 == 0 )
            {
                std::string toLog = ( boost::format( "Thread %s at (%i of %i)" )
                                      % boost::lexical_cast<std::string>( boost::this_thread::get_id() )
                                      % boost::lexical_cast<std::string>( counter )
                                      % boost::lexical_cast<std::string>( numberOfAttempts_ ) ).str();
                logToConsole( toLog );
            }
        } // for(int counter = 0; counter < numberOfAttempts_; counter++)
    };
}
