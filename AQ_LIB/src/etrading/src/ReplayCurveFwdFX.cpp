// replayCurveFwdFX.cpp

#include "ReplayCurveFwdFX.h"
#include "ReadDataFile.h"
#include "LAUpdateStaticDataManager.h"
#include "InitializeAQETrading.h"

namespace etrading
{
    using etrading::InitializeAQETrading;

    const LAString replayCurveFwdFX( const ReadDataFile::Load& inputFile )
    {

        // 1. Read the test file, tryMir/tryMe tests use diff key names
        // ---------------------
        LAString        curveID             = inputFile.getOptional( "curveID", inputFile.getOptional( "curveCollection", LAString() ) );
        LAString        marketName          = inputFile.getOptional( "marketName", inputFile.getOptional( "staticDataTable", LAString() ) );
        LAStringMatrix    fwdfxconstConv      = inputFile.getOptional( "fwdFXConstantConv", inputFile.getOptional( "fxFwdConv", LAStringMatrix() ) );
        LAStringMatrix    generateProps       = inputFile.getOptional( "generalProps", inputFile.getOptional( "curveConv", LAStringMatrix() ) );
        LAString        curveNames          = inputFile.getOptional( "curveNames", inputFile.getOptional( "curveIndex", LAString() ) );

        // 2. Check Test Parameters
        // ------------------------
        if( fwdfxconstConv.empty() )
        {
            throw LACoreInvalidData( "FwdFXConstConv Matrix is empty", __FILE__, __LINE__ );
        }

        if( fwdfxconstConv[0].size() < 2 )
        {
            throw LACoreInvalidData( "FwdFXConstConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        // 3. Build the Fwd FX Curve
        // -------------------------
        const bool checkStaticDataLoaded = true;
        LAUpdateStaticDataManager::setUpFwdFXConstantCurve( InitializeAQETrading::instance( checkStaticDataLoaded ).dataInstance(),
                curveID,
                marketName,
                fwdfxconstConv,
                generateProps,
                curveNames );


        // 4. Return the Result
        // --------------------
        LAString result = curveID + " " + marketName + " Curve has been set";
        return result;
    }
}
