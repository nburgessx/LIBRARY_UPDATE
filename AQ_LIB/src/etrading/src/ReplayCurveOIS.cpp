// replayCurveOIS.cpp

#include "ReplayCurveOIS.h"
#include "ReadDataFile.h"
#include "LAUpdateStaticDataManager.h"
#include "InitializeMLibETrading.h"

namespace etrading
{
    using etrading::InitializeMLibETrading;

    const LAString replayCurveOIS( const ReadDataFile::Load& inputFile )
    {

        // 1. Read the test file, tryMir/tryMe tests use diff key names
        // ---------------------
        LAString        curveID             = inputFile.getOptional( "curveID", inputFile.getOptional( "curveCollection", LAString() ) );
        LAString        marketName          = inputFile.getOptional( "marketName", inputFile.getOptional( "staticDataTable", LAString() ) );
        LAStringMatrix    generateProps       = inputFile.getOptional( "generalProps", inputFile.getOptional( "curveConv", LAStringMatrix() ) );
        LAStringMatrix    oisRates            = inputFile["oisRates"];
        LAStringMatrix    oisConv             = inputFile["oisConv"];
        LAString        curveNames          = inputFile.getOptional( "curveNames", inputFile.getOptional( "curveIndex", LAString() ) );
        LAStringMatrix    oisHistRates        = inputFile.getOptional( "oisHistRates", inputFile.getOptional( "oisHistoricalRates", LAStringMatrix() ) );
        LAStringMatrix    loBasisRates        = inputFile.getOptional( "loBasisRates", inputFile.getOptional( "liborOisBasisRates", LAStringMatrix() ) );
        LAStringMatrix    loBasisConv         = inputFile.getOptional( "loBasisConv", inputFile.getOptional( "liborOisBasisConv", LAStringMatrix() ) );
        LAStringMatrix    swapRates           = inputFile["swapRates"];
        LAStringMatrix    swapConv            = inputFile["swapConv"];


        // 2. Check Test Parameters
        // ------------------------
        if ( !generateProps.empty() && generateProps[0].size() < 2 )
        {
            throw LACoreInvalidData( "GenerateProps Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !oisConv.empty() && oisConv[0].size() < 2 )
        {
            throw LACoreInvalidData( "OisConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !oisRates.empty() && oisRates[0].size() < 2 )
        {
            throw LACoreInvalidData( "OisRates Matrix column size must be 2", __FILE__, __LINE__ );
        }

        // 3. Build the OIS Curve
        // ----------------------
        const bool checkStaticDataLoaded = true;
        LAUpdateStaticDataManager::setUpOISCurve( InitializeMLibETrading::instance( checkStaticDataLoaded ).dataInstance(),
												  curveID,
												  marketName,
												  generateProps,
												  oisRates,
												  oisConv,
												  curveNames,
												  oisHistRates,
												  loBasisRates,
												  loBasisConv,
												  swapRates,
												  swapConv );

        // 4. Return the Result
        // --------------------
        LAString result = curveID + " " + marketName + " Curve has been set";
        return result;
    }
}
