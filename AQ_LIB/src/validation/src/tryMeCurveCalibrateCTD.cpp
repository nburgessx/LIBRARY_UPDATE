#include <boost/date_time.hpp>

#include "tryMeCurveCalibrateCTD.h"

#include "AQLUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "ParameterValidation.h"
#include "CurveResultsContainer.h"          // CurveResultsContainer - Singleton object for storage of curve results

using namespace etrading;

namespace validation
{

    /* @brief			validation method for meCurveCalibrateCTD
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		curveName			Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		collateralCurves	The group of collateral curves out of which the CTD curve is constructed
    */
    AQLString tryMeCurveCalibrateCTD( const AQLString& curveCollection,
                                    const AQLString& curveName,
                                    const AQLString& curveIndexInput,
                                    const AQLStringMatrix& curveConv,
                                    const AQLStringVector& collateralCurves )
    {
        VALID_EXCEPTION_START
        
        // LWO Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        AQ_CLEAR_CURVE_RESULTS_CACHE

        // Prefix the curveName to the curveIndex Name Set, ensuring to use the ':' delimiter
        std::string prefixedCurveIndexStr =  curveIndexInput.getCString();
        prefixedCurveIndexStr = etrading::addPrefixStringAndCheckForDuplicates( prefixedCurveIndexStr, std::string(curveName.getCString()) );
        AQLString curveIndex  = prefixedCurveIndexStr.c_str();
        
        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveCalibrateCTD_inputs", curveCollection, curveName ) );
            file.write( "generatorFunction", "tryMeCurveCalibrateCTD" );
            file.write( "curveCollection", curveCollection );
            file.write( "staticDataTable", curveName );
            file.write( "curveIndex", curveIndex );
            file.write( "curveConv", curveConv );
            file.write( "collateralCurves", collateralCurves );
        }

        if( collateralCurves.size() == 0 )
        {
			throw AQLCoreInvalidData( "#Error: No collateral curves are given. ", __FILE__, __LINE__ );
        }
				
        AQLUpdateStaticDataManager::setUpCheapestToDeliverCurve(etrading::getDataInstance(),
														   curveCollection, 
														   curveName, 
														   curveIndex, 
														   curveConv,
														   collateralCurves);

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );

        AQLString ret( curveCollection + " " + curveName + " Curve has been set" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveCalibrateCTD_outputs", curveCollection, curveName ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    };

    }
