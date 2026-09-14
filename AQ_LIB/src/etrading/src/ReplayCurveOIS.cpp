// replayCurveOIS.cpp

#include "ReplayCurveOIS.h"
#include "ReadDataFile.h"
#include "AQLUpdateStaticDataManager.h"
#include "InitializeETrading.h"

namespace etrading
{
    using etrading::InitializeETrading;

    const AQLString replayCurveOIS( const ReadDataFile::Load& inputFile )
    {

        // 1. Read the test file, tryMir/tryMe tests use diff key names
        // ---------------------
        AQLString        curveID             = inputFile.getOptional( "curveID", inputFile.getOptional( "curveCollection", AQLString() ) );
        AQLString        marketName          = inputFile.getOptional( "marketName", inputFile.getOptional( "staticDataTable", AQLString() ) );
        AQLStringMatrix    generateProps       = inputFile.getOptional( "generalProps", inputFile.getOptional( "curveConv", AQLStringMatrix() ) );
        AQLStringMatrix    oisRates            = inputFile["oisRates"];
        AQLStringMatrix    oisConv             = inputFile["oisConv"];
        AQLString        curveNames          = inputFile.getOptional( "curveNames", inputFile.getOptional( "curveIndex", AQLString() ) );
        AQLStringMatrix    oisHistRates        = inputFile.getOptional( "oisHistRates", inputFile.getOptional( "oisHistoricalRates", AQLStringMatrix() ) );
        AQLStringMatrix    loBasisRates        = inputFile.getOptional( "loBasisRates", inputFile.getOptional( "liborOisBasisRates", AQLStringMatrix() ) );
        AQLStringMatrix    loBasisConv         = inputFile.getOptional( "loBasisConv", inputFile.getOptional( "liborOisBasisConv", AQLStringMatrix() ) );
        AQLStringMatrix    swapRates           = inputFile["swapRates"];
        AQLStringMatrix    swapConv            = inputFile["swapConv"];


        // 2. Check Test Parameters
        // ------------------------
        AQ_THROW_IF( !generateProps.empty() && generateProps[0].size() < 2, "GenerateProps Matrix column size must be 2" );

        AQ_THROW_IF( !oisConv.empty() && oisConv[0].size() < 2, "OisConv Matrix column size must be 2" );

        AQ_THROW_IF( !oisRates.empty() && oisRates[0].size() < 2, "OisRates Matrix column size must be 2" );

        // 3. Build the OIS Curve
        // ----------------------
        const bool checkStaticDataLoaded = true;
        AQLUpdateStaticDataManager::setUpOISCurve( InitializeETrading::instance( checkStaticDataLoaded ).dataInstance(),
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
        AQLString result = curveID + " " + marketName + " Curve has been set";
        return result;
    }
}
