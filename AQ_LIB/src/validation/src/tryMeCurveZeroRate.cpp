#include "tryMeCurveZeroRate.h"
#include "LACurveForwardRateHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "CurveUtilities.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{



    /* @brief			validation interface for meCurveZeroRateFromYearFractions
    *  @param [in]		yearFractions	A  list of given year fractions
    *  @param [in]		curveCollection	Curve collection id
    *  @param [in]		curveIndex		Curve index name. Default to STD
    *  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @return			Zero rate over the yearFractions from the curve's asof date
    */
    DoubleVector tryMeCurveZeroRatesFromYearFractions( const DoubleVector& yearFractions,
            const LAString& curveCollection,
            const LAString& curveIndex,
            const LAString& frequency,
            const LAString& dayCount,
			const std::string& fwdInter)

    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveZeroRatesFromYearFractions_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryMeCurveZeroRatesFromYearFractions" );
            file.write( "yearFractions", yearFractions );
            file.write( "curveCollection", curveCollection );
            file.write( "curveIndex", curveIndex );
            file.write( "frequency", frequency );
            file.write( "dayCount", dayCount );
            file.write( "fwdInter", fwdInter );
        }

        DoubleVector arr = etrading::getCurveZeroRatesFromYearFractions( yearFractions, curveCollection, curveIndex, frequency, dayCount, etrading::toBooleanEnum(fwdInter));

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveZeroRatesFromYearFractions_outputs", curveCollection, curveIndex ) );
            file.write( "output", arr );
        }

        return arr;

        VALID_EXCEPTION_END
    }


    /* @brief			validation interface for meCurveZeroRateFromTenors
    *  @param [in]		tenors			A  list of tenors
    *  @param [in]		curveCollection	Curve collection id
    *  @param [in]		curveIndex		Curve index name. Default to STD
    *  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @param [in]		calendar		Calendar
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc. Default to NO_CHANGE
    *  @return			Zero rate over the tenors from the curve's asof date
    */
    DoubleVector tryMeCurveZeroRatesFromTenors( const LAStringVector& tenors,
            const LAString& curveCollection,
            const LAString& curveIndex,
            const LAString& frequency,
            const LAString& dayCount,
            const LAString& calendar,
            const LAString& businessDayAdj,
			const std::string& fwdInter )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveZeroRatesFromTenors_inputs", curveCollection, curveIndex ) );
            file.write( "generatorFunction", "tryMeCurveZeroRatesFromTenors" );
            file.write( "tenors", tenors );
            file.write( "curveCollection", curveCollection );
            file.write( "curveIndex", curveIndex );
            file.write( "frequency", frequency );
            file.write( "dayCount", dayCount );
            file.write( "businessDayAdj", businessDayAdj );
            file.write( "calendar", calendar );
            file.write( "fwdInter", fwdInter );
        }

        // Validate parameters
        if( tenors.size() == 0 )
        {
            throw LACoreInvalidData( "a size of vector is zero.", __FILE__, __LINE__ );
        }

        LAString curIndex( curveIndex );
        LAString interp;
        LAString dayC( dayCount );
        LAString freq( frequency );
        bool isFwdInter;
        LAString bdAdj( businessDayAdj );

        etrading::populateZeroRateConventions( curveCollection, curIndex, etrading::toBooleanEnum(fwdInter), interp, dayC, freq, isFwdInter, bdAdj );

        LAString cal = etrading::getDefaultCalendarForEmptyString( calendar, curveCollection );

        DoubleArray arr = etrading::LACurveForwardRateHelpers::getMultiZeroRate( tenors, etrading::getDataInstance(), curveCollection,
                          freq, dayC, bdAdj, cal, interp, curIndex, isFwdInter );

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateCurvename( "tryMeCurveZeroRatesFromTenors_outputs", curveCollection, curIndex ) );
            file.write( "output", arr );
        }

        return arr;

        VALID_EXCEPTION_END
    }

}

