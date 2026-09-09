#include <boost/date_time.hpp>

#include "tryAqCurveCalibrateFXForwards.h"

#include "AQLUpdateStaticDataManager.h"
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CurveValidation.h"
#include "ParameterValidation.h"
#include "CurveResultsContainer.h"          // CurveResultsContainer - Singleton object for storage of curve results

using namespace etrading;

namespace validation
{

    /* @brief			validation method for aqCurveCalibrateFXForwards
    *  @param [in]		curveCollection		The curve collection ID
    *  @param [in]		staticDataTable		Name of the curve constructed by this method
    *  @param [in]		curveIndex			Equivalent names of the curve being built
    *  @param [in]		curveConv			General curve properties such as asofdate, ccy, interp, etc
    *  @param [in]		fxFwdConv			FX forward conventions
    */
    AQLString tryAqCurveCalibrateFXForwards( const AQLString& curveCollection,
                                            const AQLString& staticDataTable,
                                            const AQLString& curveIndexInput,
                                            const AQLStringMatrix& curveConv,
                                            const AQLStringMatrix& fxFwdConv )
    {
        VALID_EXCEPTION_START
        
        // AQObj Single Curves Populate Curve Results Objects that Conflict with Other Curve Types, so we must clear the Curve Results Cache
        AQ_CLEAR_CURVE_RESULTS_CACHE

        // Prefix the staticDataTable to the curveIndex Name Set, ensuring to use the ':' delimiter
        std::string curveIndexStdStr =  curveIndexInput.getCString();
        curveIndexStdStr = etrading::addPrefixStringAndCheckForDuplicates( curveIndexStdStr, std::string(staticDataTable.getCString()) );
        AQLString curveIndex  = curveIndexStdStr.c_str();
        
        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveCalibrateFXForwards_inputs", curveCollection, staticDataTable ) );
            file.write( "generatorFunction", "tryAqCurveCalibrateFXForwards" );
            file.write( "curveCollection", curveCollection );
            file.write( "staticDataTable", staticDataTable );
            file.write( "curveIndex", curveIndex );
            file.write( "curveConv", curveConv );
            file.write( "fxFwdConv", fxFwdConv );
        }

        if( fxFwdConv.empty() )
        {
            throw AQLCoreInvalidData( "Input Matrix is empty", __FILE__, __LINE__ );
        }

        if( fxFwdConv[0].size() < 2 )
        {
            AQ_THROW("Invalid Data: Input matix data must have column size 2")
        }

        AQLUpdateStaticDataManager::setUpFwdFXConstantCurve( etrading::getDataInstance(), curveCollection, staticDataTable, fxFwdConv, curveConv, curveIndex );

        //Throw exception if the curve has not been built.
        etrading::getCurveStaticDataTableName( curveCollection, curveIndex );

        AQLString ret( curveCollection + " " + staticDataTable + " Curve has been set" );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryAqCurveCalibrateFXForwards_outputs", curveCollection, staticDataTable ) );
            file.write( "output", ret );
        }

        return ret;

        VALID_EXCEPTION_END
    };

    }
