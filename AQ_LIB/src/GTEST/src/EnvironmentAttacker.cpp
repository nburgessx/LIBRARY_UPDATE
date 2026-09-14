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
#include <sstream>
#include <thread>

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
                    std::ostringstream logMsg;
                    logMsg << "Thread " << boost::this_thread::get_id() << " (" << counter << " of " << numberOfAttempts_
                           << ") cleared the FreeObject Cache for the Workcontext " << wrkContextToCrack_.get()->getUID();
                    logToConsole( logMsg.str() );
                    std::ostringstream logMsgAfter;
                    logMsgAfter << "Thread " << boost::this_thread::get_id() << " (" << counter << " of " << numberOfAttempts_
                                << ") sees a FreeObject Cache of size " << wrkContextToCrack_.get()->getCache<FreeObject>().size()
                                << " for the Workcontext " << wrkContextToCrack_.get()->getUID();
                    logToConsole( logMsgAfter.str() );
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
                    std::ostringstream logMsg;
                    logMsg << "Thread " << boost::this_thread::get_id() << " (" << counter << " of " << numberOfAttempts_
                           << ") set " << objectName << " on the FreeObject Cache for the Workcontext " << wrkContextToCrack_.get()->getUID();
                    logToConsole( logMsg.str() );
                    std::ostringstream logMsgAfter;
                    logMsgAfter << "Thread " << boost::this_thread::get_id() << " (" << counter << " of " << numberOfAttempts_
                                << ") sees a FreeObject Cache of size " << wrkContextToCrack_.get()->getCache<FreeObject>().size()
                                << " with Keys " << containerAsString( wrkContextToCrack_.get()->getCache<FreeObject>().keys() )
                                << " for the Workcontext " << wrkContextToCrack_.get()->getUID();
                    logToConsole( logMsgAfter.str() );
                }
#endif
            }

            if( counter % 150 == 0 )
            {
				std::ostringstream stream;

                stream << "Thread " << std::this_thread::get_id()
				       << " at (" << counter
				       << " of " << numberOfAttempts_ << ")";

				std::string toLog = stream.str();
                logToConsole( toLog );
            }
        } // for(int counter = 0; counter < numberOfAttempts_; counter++)
    };
}
