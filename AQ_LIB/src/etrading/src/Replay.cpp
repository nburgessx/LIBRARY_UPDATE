// Replay.cpp

#include "ExceptionMacros.h"
#include "Replay.h"
#include "ReadDataFile.h"
#include "AQLCoreAppError.h"
#include <sstream>

// Replay Generator Functions
#include "ReplayCurveOIS.h"
#include "ReplayCurveSTD.h"
#include "ReplayCurveBasis.h"
#include "ReplayCurveFwdFX.h"

namespace etrading
{
    using etrading::ReadDataFile;

    /* @brief			Function to load and replay a test file
    *  @param [in]		filepath		Full file path to the test csv file
    */
    const AQLString replay( const AQLString& filepath )
    {
        AQ_THROW_IF( filepath.size() == 0 || filepath.isDefined() == false, "Filepath must be Provided." );

        try
        {
            // 1. Load the Input File
            ReadDataFile::Load inputFile( filepath );

            // 2. Identify the Generator function
            const AQLString generatorFunction = inputFile["generatorFunction"];

            // 3. Call the Correct Function
            AQLString result = "#Error: Unable to replay the test file provided";

            // 4. Use Function Pointer to Call the Desired Generator Function

            // Function Pointer Type Definition
            typedef const AQLString ( *functionPointer )( const ReadDataFile::Load & inputFile );

            // Create a Hash Map to lookup function pointers from a string
            std::map< AQLString, functionPointer > functionList;

            // List functions here
            // -------------------------------------------------------------------------


            functionList[ AQLString( "tryAqCurveCalibrateOIS" ) ]          = etrading::replayCurveOIS;
            functionList[ AQLString( "tryAqCurveCalibrateSwap" ) ]            = etrading::replayCurveSTD;
            functionList[ AQLString( "tryAqCurveCalibrateBasis" ) ]			= etrading::replayCurveBasis;
            functionList[ AQLString( "tryAqCurveCalibrateFXForwards" ) ]		= etrading::replayCurveFwdFX;
            // -------------------------------------------------------------------------

            // Throw an Error if the function is not found
            if ( functionList.find( generatorFunction ) == functionList.end() )
            {
                AQLString msg = "#Error: Unable to find the generator function " + generatorFunction;
                AQ_THROW( msg.getCString() );
            }

            // Point functionPointer to functionName
            functionPointer f = functionList.find( generatorFunction )->second;

            // Call the function pointed at and return the result
            result = f( inputFile );

            // 5. Return the result
            return result;

        }
        catch( const ReadDataFile::LoadError& )
        {
            AQ_THROW( "Unable to open the file specified" );
        }
        catch( std::exception& e )
        {
            std::stringstream s;
            s << "#Error: " << e.what();
            AQ_THROW( s.str().c_str() );
        }
        catch( ... )
        {
            AQ_THROW( "Unable to read the file specified" );
        }
    }
}
