// Replay.cpp

#include "Replay.h"
#include "ReadDataFile.h"
#include "LACoreAppError.h"
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
    const LAString replay( const LAString& filepath )
    {
        if ( filepath.size() == 0 || filepath.isDefined() == false )
        {
            throw LACoreInvalidData( "#Error: Filepath must be Provided.", __FILE__, __LINE__ );
        }

        try
        {
            // 1. Load the Input File
            ReadDataFile::Load inputFile( filepath );

            // 2. Identify the Generator function
            const LAString generatorFunction = inputFile["generatorFunction"];

            // 3. Call the Correct Function
            LAString result = "#Error: Unable to replay the test file provided";

            // 4. Use Function Pointer to Call the Desired Generator Function

            // Function Pointer Type Definition
            typedef const LAString ( *functionPointer )( const ReadDataFile::Load & inputFile );

            // Create a Hash Map to lookup function pointers from a string
            std::map< LAString, functionPointer > functionList;

            // List functions here
            // -------------------------------------------------------------------------

            functionList[ LAString( "tryMirSetUpOISCurve" ) ]             = etrading::replayCurveOIS;
            functionList[ LAString( "tryMirSetUpSwapCurve" ) ]            = etrading::replayCurveSTD;
            functionList[ LAString( "tryMirSetUpBasisSwapCurve" ) ]       = etrading::replayCurveBasis;
            functionList[ LAString( "tryMirSetUpFwdFXConstantCurve" ) ]   = etrading::replayCurveFwdFX;

            functionList[ LAString( "tryMeCurveCalibrateOIS" ) ]          = etrading::replayCurveOIS;
            functionList[ LAString( "tryMeCurveCalibrateSwap" ) ]            = etrading::replayCurveSTD;
            functionList[ LAString( "tryMeCurveCalibrateBasis" ) ]			= etrading::replayCurveBasis;
            functionList[ LAString( "tryMeCurveCalibrateFXForwards" ) ]		= etrading::replayCurveFwdFX;
            // -------------------------------------------------------------------------

            // Throw an Error if the function is not found
            if ( functionList.find( generatorFunction ) == functionList.end() )
            {
                LAString msg = "#Error: Unable to find the generator function " + generatorFunction;
                throw LACoreInvalidData( msg.getCString(), __FILE__, __LINE__ );
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
            throw LACoreInvalidData( "#Error: Unable to open the file specified", __FILE__, __LINE__ );
        }
        catch( std::exception& e )
        {
            std::stringstream s;
            s << "#Error: " << e.what();
            throw LACoreInvalidData( s.str().c_str() , __FILE__, __LINE__ );
        }
        catch( ... )
        {
            throw LACoreInvalidData( "#Error: Unable to read the file specified", __FILE__, __LINE__ );
        }
    }
}
