// replayCurveFwdFX.cpp

#include "ReplayCurveFwdFX.h"
#include "ReadDataFile.h"
#include "AQLUpdateStaticDataManager.h"
#include "InitializeETrading.h"

namespace etrading
{
    using etrading::InitializeETrading;

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
        AQ_THROW_IF( fwdfxconstConv.empty(), "FwdFXConstConv Matrix is empty" );

        AQ_THROW_IF( fwdfxconstConv[0].size() < 2, "FwdFXConstConv Matrix column size must be 2" );

        // 3. Build the Fwd FX Curve
        // -------------------------
        const bool checkStaticDataLoaded = true;
        AQLUpdateStaticDataManager::setUpFwdFXConstantCurve( InitializeETrading::instance( checkStaticDataLoaded ).dataInstance(),
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
