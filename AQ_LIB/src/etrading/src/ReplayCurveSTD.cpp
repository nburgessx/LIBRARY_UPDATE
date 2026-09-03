// replayCurveSTD.cpp

#include "ReplayCurveSTD.h"
#include "ReadDataFile.h"
#include "LAUpdateStaticDataManager.h"
#include "InitializeAQETrading.h"

namespace etrading
{
    using etrading::InitializeAQETrading;

    const AQLString replayCurveSTD( const ReadDataFile::Load& inputFile )
    {
        // 1. Read the test file, tryMir/tryMe tests use diff key names
        // ---------------------
        AQLString        curveID             = inputFile.getOptional( "curveID", inputFile.getOptional( "curveCollection", AQLString() ) );
        AQLString        marketName          = inputFile.getOptional( "marketName", inputFile.getOptional( "staticDataTable", AQLString() ) );
        AQLStringMatrix    generalProps        = inputFile.getOptional( "generateProp", inputFile.getOptional( "generalProps", inputFile.getOptional( "curveConv", AQLStringMatrix() ) ) );
        AQLStringMatrix    moneyConv           = inputFile.getOptional( "moneyConv", inputFile.getOptional( "moneyMarketConv", AQLStringMatrix() ) );
        AQLStringMatrix    liborRates          = inputFile["liborRates"];
        AQLStringMatrix    liborConv           = inputFile["liborConv"];
        AQLStringMatrix    swapRates           = inputFile["swapRates"];
        AQLStringMatrix    swapConv            = inputFile["swapConv"];
        AQLStringMatrix    fra3mRates          = inputFile["fra3mRates"];
        AQLStringMatrix    fra6mRates          = inputFile["fra6mRates"];
        AQLStringMatrix    fraConv             = inputFile["fraConv"];
        AQLStringMatrix    futureRates         = inputFile["futureRates"];
        AQLStringMatrix    futureConv          = inputFile["futureConv"];
        AQLStringMatrix    adjustSwapConv      = inputFile.getOptional( "adjustSwapConv", inputFile.getOptional( "convexityAdjConv", AQLStringMatrix() ) );
        AQLStringMatrix    adjustSwapRates     = inputFile.getOptional( "adjustSwapRates", inputFile.getOptional( "convexityAdjRates", AQLStringMatrix() ) );
        AQLString        curveNames          = inputFile.getOptional( "curveNames", inputFile.getOptional( "curveIndex", AQLString() ) );

        // 2. Check Test Parameters
        // ------------------------
        if ( !generalProps.empty() && generalProps[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "GeneralProps Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !moneyConv.empty() && moneyConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "MoneyConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !liborConv.empty() && liborConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "LiborConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !liborRates.empty() && liborRates[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "LiborRates Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !swapConv.empty() && swapConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "SwapConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !swapRates.empty() && swapRates[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "SwapRates Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !fraConv.empty() && fraConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "FraConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !fra3mRates.empty() && fra3mRates[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "Fra3mRates Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !fra6mRates.empty() && fra6mRates[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "Fra6mRates Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !futureConv.empty() && futureConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "FutureConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !futureRates.empty() && futureRates[0].size() < 3 )
        {
            throw AQLCoreInvalidData( "FutureRates Matrix column size must be greater or equal to 3", __FILE__, __LINE__ );
        }

        if ( !adjustSwapConv.empty() && adjustSwapConv[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "AdjustSwapConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !adjustSwapRates.empty() && adjustSwapRates[0].size() < 2 )
        {
            throw AQLCoreInvalidData( "AdjustSwapRates Matrix column size must be 2", __FILE__, __LINE__ );
        }

        // 3. Build the STD Curve
        // ----------------------
        const bool checkStaticDataLoaded = true;
        LAUpdateStaticDataManager::setUpSwapCurve( InitializeAQETrading::instance( checkStaticDataLoaded ).dataInstance(),
                                          curveID,
                                          marketName,
                                          generalProps,
                                          moneyConv,
                                          liborRates,
                                          liborConv,
                                          swapRates,
                                          swapConv,
                                          fra3mRates,
                                          fra6mRates,
                                          fraConv,
                                          futureRates,
                                          futureConv,
                                          adjustSwapConv,
                                          adjustSwapRates,
                                          curveNames,
                                          AQLString( "INVALID" ) ); // curveName_DF2: This parameter is deprecated

        // 4. Return the Result
        // --------------------
        AQLString result = curveID + " " + marketName + " Curve has been set";
        return result;
    }
}
