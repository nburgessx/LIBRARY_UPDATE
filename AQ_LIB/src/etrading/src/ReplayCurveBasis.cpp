// replayCurveBasis.cpp

#include "ReplayCurveBasis.h"
#include "ReadDataFile.h"
#include "LAUpdateStaticDataManager.h"
#include "InitializeMLibETrading.h"

namespace etrading
{
    using etrading::InitializeMLibETrading;

    const LAString replayCurveBasis( const ReadDataFile::Load& inputFile )
    {

        // 1. Read the test file, tryMir/tryMe tests use diff key names
        // ---------------------
        LAString        curveID             = inputFile.getOptional( "curveID", inputFile.getOptional( "curveCollection", LAString() ) );
        LAString        marketName          = inputFile.getOptional( "marketName", inputFile.getOptional( "staticDataTable", LAString() ) );
        LAStringMatrix    basisRates          = inputFile["basisRates"];
        LAStringMatrix    basisConv           = inputFile["basisConv"];
        LAStringMatrix    fwdFXs              = inputFile.getOptional( "fwdFXs", inputFile.getOptional( "fxFwdRates", LAStringMatrix() ) );
        LAStringMatrix    fwdConv             = inputFile.getOptional( "fwdConv", inputFile.getOptional( "fxFwdConv", LAStringMatrix() ) );
        LAStringMatrix    spotFXs             = inputFile.getOptional( "spotFXs", inputFile.getOptional( "spotFxRates", LAStringMatrix() ) );
        LAStringMatrix    generalProps        = inputFile.getOptional( "generalProps", inputFile.getOptional( "curveConv", LAStringMatrix() ) );
        LAStringMatrix    moneyConv           = inputFile.getOptional( "moneyConv", LAStringMatrix( 0 ) );
        LAString        curveNames          = inputFile.getOptional( "curveNames", inputFile.getOptional( "curveIndex", LAString() ) );
		LAStringMatrix    fraConv				= inputFile.getOptional( "fraConv", LAStringMatrix( 0 ) );
		LAStringMatrix    fraRates            = inputFile.getOptional( "fraRates", LAStringMatrix( 0 ) );

        // 2. Check Test Parameters
        // ------------------------
        if( ( fwdFXs.empty() && basisRates.empty() ) || basisConv.empty() || generalProps.empty() )
        {
            throw LACoreInvalidData( "BasisRates, BasisConv and / or GeneralProps Matrices are empty", __FILE__, __LINE__ );
        }

        if( ( fwdFXs.empty() && 2 > basisRates[0].size() ) || 2 > basisConv[0].size() || 2 > generalProps[0].size() )
        {
            throw LACoreInvalidData( "BasisRates, BasisConv and / or GeneralProps Matrices column size must be 2", __FILE__, __LINE__ );
        }

        if ( !fwdFXs.empty() && fwdFXs[0].size() < 2 )
        {
            throw LACoreInvalidData( "FwdFX Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !fwdConv.empty() && fwdConv[0].size() < 2 )
        {
            throw LACoreInvalidData( "FwdConv Matrix column size must be 2", __FILE__, __LINE__ );
        }

        if ( !spotFXs.empty() && spotFXs[0].size() < 3 )
        {
            throw LACoreInvalidData( "SpotFX Matrix column size must be 3", __FILE__, __LINE__ );
        }

        // 3. Build the Basis Curve
        // ----------------------
        const bool checkStaticDataLoaded = true;
        LAUpdateStaticDataManager::setUpBasisCurve( InitializeMLibETrading::instance( checkStaticDataLoaded ).dataInstance(),
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
        LAString result = curveID + " " + marketName + " Curve has been set";
        return result;
    }
}
