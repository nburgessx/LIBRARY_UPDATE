// replayCurveFwdFX.cpp

#include "ReplayCurveFwdFX.h"
#include "ReadDataFile.h"
#include "LAUpdateStaticDataManager.h"
#include "InitializeAQETrading.h"

namespace etrading
{
    using etrading::InitializeAQETrading;

    const AQLString replayCurveFwdFX( const ReadDataFile::Load& inputFile )
    {

        // 1. Read the test file, tryMir/tryMe tests use diff key names
        // ---------------------
        AQLString        curveID             = inputFile.getOptional( "curveID", inputFile.getOptional( "curveCollection", AQLString() ) );
        AQLString        marketName          = inputFile.getOptional( "marketName", inputFile.getOptional( "staticDataTable", AQLString() ) );
        AQLStringMatrix    fwdfxconstConv      = inputFile.getOptional( "fwdFXConstantConv", inputFile.getOptional( "fxFwdConv", AQLStringMatrix() ) );
        AQLStringMatrix    generateProps       = inputFile.getOptional( "generalProps", inputFile.getOptional( "curveConv", AQLStringMatrix() ) );
        AQLString        curveNames          = inputFile.getOptional( "curveNames", inputFile.getOptional( "curveIndex", AQLString() ) );

        // 2. Check Test Parameters
        // ------------------------
        if( fwdfxconstConv.empty() )
        {
            throw AQLCoreInvalidData( "FwdFXConstConv Matrix is empty", __FILE__, __LINE__ );
        }

        if( fwdfxconstConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "FwdFXConstConv Matrix column size must be 2", __FILE__, __LINE__ );
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
        AQLString result = curveID + " " + marketName + " Curve has been set";
        return result;
    }
}
