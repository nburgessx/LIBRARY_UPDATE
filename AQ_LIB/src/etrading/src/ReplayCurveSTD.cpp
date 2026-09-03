// replayCurveSTD.cpp

#include "ReplayCurveSTD.h"
#include "ReadDataFile.h"
#include "LAUpdateStaticDataManager.h"
#include "InitializeMLibETrading.h"

namespace etrading
{
    using etrading::InitializeMLibETrading;

    const LAString replayCurveSTD( const ReadDataFile::Load& inputFile )
    {
        // 1. Read the test file, tryMir/tryMe tests use diff key names
        // ---------------------
        LAString        curveID             = inputFile.getOptional( "curveID", inputFile.getOptional( "curveCollection", LAString() ) );
        LAString        marketName          = inputFile.getOptional( "marketName", inputFile.getOptional( "staticDataTable", LAString() ) );
        LAStringMatrix    generalProps        = inputFile.getOptional( "generateProp", inputFile.getOptional( "generalProps", inputFile.getOptional( "curveConv", LAStringMatrix() ) ) );
        LAStringMatrix    moneyConv           = inputFile.getOptional( "moneyConv", inputFile.getOptional( "moneyMarketConv", LAStringMatrix() ) );
        LAStringMatrix    liborRates          = inputFile["liborRates"];
        LAStringMatrix    liborConv           = inputFile["liborConv"];
        LAStringMatrix    swapRates           = inputFile["swapRates"];
        LAStringMatrix    swapConv            = inputFile["swapConv"];
        LAStringMatrix    fra3mRates          = inputFile["fra3mRates"];
        LAStringMatrix    fra6mRates          = inputFile["fra6mRates"];
        LAStringMatrix    fraConv             = inputFile["fraConv"];
        LAStringMatrix    futureRates         = inputFile["futureRates"];
        LAStringMatrix    futureConv          = inputFile["futureConv"];
        LAStringMatrix    adjustSwapConv      = inputFile.getOptional( "adjustSwapConv", inputFile.getOptional( "convexityAdjConv", LAStringMatrix() ) );
        LAStringMatrix    adjustSwapRates     = inputFile.getOptional( "adjustSwapRates", inputFile.getOptional( "convexityAdjRates", LAStringMatrix() ) );
        LAString        curveNames          = inputFile.getOptional( "curveNames", inputFile.getOptional( "curveIndex", LAString() ) );

        // 2. Check Test Parameters
        // ------------------------
        if ( !generalProps.empty() && generalProps[0].size() < 2 )
        {
            throw LACoreInvalidData( "GeneralProps Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !moneyConv.empty() && moneyConv[0].size() < 2 )
        {
            throw LACoreInvalidData( "MoneyConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !liborConv.empty() && liborConv[0].size() < 2 )
        {
            throw LACoreInvalidData( "LiborConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !liborRates.empty() && liborRates[0].size() < 2 )
        {
            throw LACoreInvalidData( "LiborRates Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !swapConv.empty() && swapConv[0].size() < 2 )
        {
            throw LACoreInvalidData( "SwapConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !swapRates.empty() && swapRates[0].size() < 2 )
        {
            throw LACoreInvalidData( "SwapRates Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !fraConv.empty() && fraConv[0].size() < 2 )
        {
            throw LACoreInvalidData( "FraConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !fra3mRates.empty() && fra3mRates[0].size() < 2 )
        {
            throw LACoreInvalidData( "Fra3mRates Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !fra6mRates.empty() && fra6mRates[0].size() < 2 )
        {
            throw LACoreInvalidData( "Fra6mRates Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !futureConv.empty() && futureConv[0].size() < 2 )
        {
            throw LACoreInvalidData( "FutureConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !futureRates.empty() && futureRates[0].size() < 3 )
        {
            throw LACoreInvalidData( "FutureRates Matrix column size must be greater or equal to 3", __FILE__, __LINE__ );
        }

        if ( !adjustSwapConv.empty() && adjustSwapConv[0].size() < 2 )
        {
            throw LACoreInvalidData( "AdjustSwapConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !adjustSwapRates.empty() && adjustSwapRates[0].size() < 2 )
        {
            throw LACoreInvalidData( "AdjustSwapRates Matrix column size must be 2", __FILE__, __LINE__ );
        }

        // 3. Build the STD Curve
        // ----------------------
        const bool checkStaticDataLoaded = true;
        LAUpdateStaticDataManager::setUpSwapCurve( InitializeMLibETrading::instance( checkStaticDataLoaded ).dataInstance(),
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
                                          LAString( "INVALID" ) ); // curveName_DF2: This parameter is deprecated

        // 4. Return the Result
        // --------------------
        LAString result = curveID + " " + marketName + " Curve has been set";
        return result;
    }
}
