// replayCurveBasis.cpp

#include "ReplayCurveBasis.h"
#include "ReadDataFile.h"
#include "AQLUpdateStaticDataManager.h"
#include "InitializeETrading.h"

namespace etrading
{
    using etrading::InitializeETrading;

    const AQLString replayCurveBasis( const ReadDataFile::Load& inputFile )
    {

        // 1. Read the test file, tryMir/tryMe tests use diff key names
        // ---------------------
        AQLString        curveID             = inputFile.getOptional( "curveID", inputFile.getOptional( "curveCollection", AQLString() ) );
        AQLString        marketName          = inputFile.getOptional( "marketName", inputFile.getOptional( "staticDataTable", AQLString() ) );
        AQLStringMatrix    basisRates          = inputFile["basisRates"];
        AQLStringMatrix    basisConv           = inputFile["basisConv"];
        AQLStringMatrix    fwdFXs              = inputFile.getOptional( "fwdFXs", inputFile.getOptional( "fxFwdRates", AQLStringMatrix() ) );
        AQLStringMatrix    fwdConv             = inputFile.getOptional( "fwdConv", inputFile.getOptional( "fxFwdConv", AQLStringMatrix() ) );
        AQLStringMatrix    spotFXs             = inputFile.getOptional( "spotFXs", inputFile.getOptional( "spotFxRates", AQLStringMatrix() ) );
        AQLStringMatrix    generalProps        = inputFile.getOptional( "generalProps", inputFile.getOptional( "curveConv", AQLStringMatrix() ) );
        AQLStringMatrix    moneyConv           = inputFile.getOptional( "moneyConv", AQLStringMatrix( 0 ) );
        AQLString        curveNames          = inputFile.getOptional( "curveNames", inputFile.getOptional( "curveIndex", AQLString() ) );
		AQLStringMatrix    fraConv				= inputFile.getOptional( "fraConv", AQLStringMatrix( 0 ) );
		AQLStringMatrix    fraRates            = inputFile.getOptional( "fraRates", AQLStringMatrix( 0 ) );

        // 2. Check Test Parameters
        // ------------------------
        AQ_THROW_IF( ( fwdFXs.empty() && basisRates.empty() ) || basisConv.empty() || generalProps.empty(), "BasisRates, BasisConv and / or GeneralProps Matrices are empty" );

        AQ_THROW_IF( ( fwdFXs.empty() && 2 > basisRates[0].size() ) || 2 > basisConv[0].size() || 2 > generalProps[0].size(), "BasisRates, BasisConv and / or GeneralProps Matrices column size must be 2" );

        AQ_THROW_IF( !fwdFXs.empty() && fwdFXs[0].size() < 2, "FwdFX Matrix column size must be 2" );

        AQ_THROW_IF( !fwdConv.empty() && fwdConv[0].size() < 2, "FwdConv Matrix column size must be 2" );

        AQ_THROW_IF( !spotFXs.empty() && spotFXs[0].size() < 3, "SpotFX Matrix column size must be 3" );

        // 3. Build the Basis Curve
        // ----------------------
        const bool checkStaticDataLoaded = true;
        AQLUpdateStaticDataManager::setUpBasisCurve( InitializeETrading::instance( checkStaticDataLoaded ).dataInstance(),
                                           curveID,
                                           marketName,
                                           basisRates,
                                           basisConv,
                                           fwdFXs,
                                           fwdConv,
                                           spotFXs,
                                           generalProps,
                                           moneyConv,
                                           curveNames,
										   fraConv,
										   fraRates);

        // 4. Return the Result
        // --------------------
        AQLString result = curveID + " " + marketName + " Curve has been set";
        return result;
    }
}
