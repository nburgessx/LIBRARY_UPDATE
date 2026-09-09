// Replay.cpp

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
        if ( filepath.size() == 0 || filepath.isDefined() == false )
        {
            throw AQLCoreInvalidData( "#Error: Filepath must be Provided.", __FILE__, __LINE__ );
        }

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
                throw AQLCoreInvalidData( msg.getCString(), __FILE__, __LINE__ );
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
            throw AQLCoreInvalidData( "#Error: Unable to open the file specified", __FILE__, __LINE__ );
        }
        catch( std::exception& e )
        {
            std::stringstream s;
            s << "#Error: " << e.what();
            throw AQLCoreInvalidData( s.str().c_str() , __FILE__, __LINE__ );
        }
        catch( ... )
        {
            throw AQLCoreInvalidData( "#Error: Unable to read the file specified", __FILE__, __LINE__ );
        }
    }
}
