#include "tryMirSwapStubRate.h"

#include "AQLCurveForwardRateHelpers.h"
#include "CurveInstrumentPricing.h"
#include "AQLDateScheduleHelpers.h"
#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "LabelValueBlockValidation.h"
#include "ScheduleValidation.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "StructuredExceptionHandler.h"
#include "SwapUtilities.h"

#include <boost/algorithm/string.hpp>

using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace
{
	StubRateAndFixingDate getStubRateAndFixingDate( AQLDataInstance* dataInstance,
													const AQLString& effDt,
													const AQLString& mat,
													const AQLString& freq,
													const AQLString& dayCt,
													const AQLString& busDayAdj,
													const AQLString& cal,
													const AQLString& rollDayString,
													const AQLString& fixLag,
													bool eomRoll,
													const AQLString& interpolation,
													const AQLString& firstStub,
													const AQLString& lastStub,
													const AQLString& stub,
													const AQLString& crvID,
													const AQLStringVector& curveNames,
													const AQLStringVector& curveTenors,
													const DoubleVector& tenorCurveFixings,
													const AQLString& useCurveName,
													const AQLString& toleranceTenor,
													bool isFwdInterp,
													bool useFwdData )
	{
		AQLDate effectiveDate = etrading::stringToDate( effDt, "#Error: Invalid 'EffectiveDate'." );
        
        // For Stub Purposes Only: we require the fixing end date not the maturity date, so we should not adjust the end date for payment holidays
        AQLDate maturityDate  = etrading::validateMaturityDate( effectiveDate, mat);		

        AQLString interp( etrading::getDefaultValueForEmptyString( interpolation, "SPLINE" ) );

        etrading::validateStringEmptiness( freq,		    "#Error: 'Frequency' must be specified." );
        etrading::validateStringEmptiness( busDayAdj,	"#Error: 'BusinessDayAdjustment' must be specified." );
        etrading::validateStringEmptiness( dayCt,		"#Error: 'Date Count' must be specified." );
        etrading::validateStringEmptiness( cal,		    "#Error: 'Calendar' must be specified." );

        //----------------------------------------
        AQLString stubType( stub );
        AQLString stubTypeTmp( stub );
        AQLString* stubTypePtr = NULL;
        if( stubType.size() != 0  )
        {
            stubTypePtr = const_cast<AQLString*>( &stubTypeTmp );
        }

        // Set-Up First and Last Stub Parameters
        AQLDate* firstOddDate    = NULL;
        AQLDate* lastOddDate     = NULL;
        AQLDate tempFirst;
        AQLDate tempLast;
		
        if( firstStub.size() != 0 && lastStub.size() != 0 )
        {
            throw AQLCoreInvalidData( "#Error: Don't support setting FirstStub and LastStub at the same time. Function won't know which stub period to calculate stub rate for.", __FILE__, __LINE__ );
        }

        if( firstStub.size() != 0 )
        {
            // Client should not specify both the stub type and the first- and lastStubDates
            if( stub.size() != 0 && ( AQLString( stub ).toUpper() ) != "NONE" )
            {
                throw AQLCoreInvalidData( "#Error: Float Leg cannot have both the StubType and First- or LastStubDate specified.", __FILE__, __LINE__ );
            }

            tempFirst       = etrading::stringToDate( firstStub, "#Error: Invalid 'FirstStubDate'." );
            firstOddDate    = & tempFirst;
            stubType		= "SS";		// Artificially set the value of stubType so that later on we know if we are dealing with front or end stub
        }
        else if( lastStub.size() != 0 )
        {
            // Client should not specify both the stub type and the first- and lastStubDates
            if( stub.size() != 0 && ( AQLString( stub ).toUpper() ) != "NONE" )
            {
                throw AQLCoreInvalidData( "#Error: Float Leg cannot have both the StubType and First- or LastStubDate specified.", __FILE__, __LINE__ );
            }

            tempLast        = etrading::stringToDate( lastStub, "#Error: Invalid 'LastStubDate'." );
            lastOddDate     = & tempLast;
            stubType		= "SE";		// Artificially set the value of stubType so that later on we know if we are dealing with front or end stub
        }

        bool useNearestCurve = false;
        if( ( boost::iequals( useCurveName.getCString(), "NATURAL" ) || useCurveName.getCString() == "" || useCurveName.size() == 0 ) )
        {
            if ( toleranceTenor.size() != 0 )
            {
                // Choose the nearest curve when 1. no explicit curve has been chosen to use and 2. a tolerance tenor is given
                useNearestCurve = true;
            }
        }
				
        //
        // Generate the roll day conventions and ensure output roll parameters are initialized
        // -----------------------------------------------------------------------------------
        //
        int* rollDayPtr                             = nullptr;
        AQLString* rollConventionPtr                 = nullptr;

        int  rollDay                                = 0;
        AQLString rollConvention                     = AQLString( "" );
        bool isEOMRoll                              = false;
        bool isStartRoll                            = false;

        // Note: We check for RollDayString = AQLString("0") for backwards compatibility
        if ( rollDayString != AQLString( "0" )  && rollDayString.size() != 0 )
        {
            // Generate Fixed Leg Coupon Roll Conventions if the rollDayString is not empty or set to zero
            rollDayPtr          = &rollDay;
            rollConventionPtr   = &rollConvention;

            etrading::validateAndPopulateRollDayConventions( rollDayString, effectiveDate, maturityDate, &rollDayPtr, &rollConventionPtr, isEOMRoll, isStartRoll );
        }
		
        // Generate the fixing schedule
        DateVector fixingDates;
        etrading::AQLCurveForwardRateHelpers::generateFixingSchedule( fixingDates,
                                                                effectiveDate,
                                                                maturityDate,
                                                                freq,
                                                                busDayAdj,
                                                                cal,
                                                                AQLString( "PRECEDING" ),    // fixingRollConvention
                                                                cal,
                                                                fixLag,
                                                                stubTypePtr,
                                                                firstOddDate,
                                                                lastOddDate,
                                                                rollDayPtr,
                                                                isStartRoll,
                                                                isEOMRoll,
                                                                rollConventionPtr ); // RollConv = Start, End, IMM, EOM or NULL
				
		// Determine if there is actually stub in the swap
		bool isStubDateSpecified = false;
		if (firstOddDate != nullptr || lastOddDate != nullptr)
		{
			isStubDateSpecified = true;
		}

		bool isRegularSwapSchedule = false;
		AQLString rollConv("");
		if (isStubDateSpecified)
		{
			isRegularSwapSchedule = false;
		}
		else
		{
			bool isMaturityDateTenor = etrading::isMaturityDateTenor(mat);
			if (rollConventionPtr != nullptr)
			{
				rollConv = *rollConventionPtr;
			}

			int rollDay(0);
			if (rollDayPtr != NULL)
			{
				rollDay = * rollDayPtr;
			}

			isRegularSwapSchedule = etrading::isRegularSwapSchedule(effectiveDate, maturityDate, isMaturityDateTenor, freq, busDayAdj, cal, rollDay, rollConv);
		}

        StubRateAndFixingDate result = CurveInstrumentPricing::getStubRate(fixingDates,
																			curveNames,
																			curveTenors,
																			tenorCurveFixings,
																			crvID,
																			stubType,
																			interpolation,
																			dayCt,
																			cal,
																			busDayAdj,
																			rollConv,
																			useNearestCurve,
																			isFwdInterp,
																			useFwdData,
																			toleranceTenor,
																			useCurveName,
																			freq,
																			isRegularSwapSchedule );

		return result;
	}
}

namespace validation
{

    /* @brief			validation method for mirSwapStubRate
    *  @param [in]		dataInstance			    Pointer to the entity pool
    *  @param [in]		effDt				The effective start date of the swap, ie, base date + spot date
    *  @param [in]		mat					Swap maturity date or tenor
    *  @param [in]		freq				Floating leg frequency
    *  @param [in]		dayCt				Floating leg day count convention
    *  @param [in]		busDayAdj			Floating leg business day adjustment
    *  @param [in]		cal					Floating leg calendar
    *  @param [in]		rollDayString		Floating leg rolling day
    *  @param [in]		fixLag				Floating leg fixing day lag
    *  @param [in]		eomRoll				Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
    *  @param [in]		interpolation		Interpolation method
    *  @param [in]		fistStub			End date of the front stub period on floating leg
    *  @param [in]		lastStub			Start date of the end stub period on floating leg
    *  @param [in]		stubType			Floating leg stub type
    *  @param [in]		crvID				ID of the curve set
    *  @param [in]		curveNames			A list of curves to interpolate from
    *  @param [in]		curveTenors			A list of tenors corresponding to curve names
    *  @param [in]		tenorCurveFixings	A list of tenors corresponding to curve names
    *  @param [in]		useCurveName		A curve specifically chosen by user to use
    *  @param [in]		toleranceTenor		Tenor that defines if a nearby curve should be chosen for use
    *  @param [in]		isFwdInterp			Boolean that decides if direct interpolation on fwd rates is employed
    *  @param [in]		useFwdData			Use fwd rates directly or derive them from spot rates
    */
    double tryMirSwapStubRate( AQLDataInstance* dataInstance,
                               const AQLString& effDt,
                               const AQLString& mat,
                               const AQLString& freq,
                               const AQLString& dayCt,
                               const AQLString& busDayAdj,
                               const AQLString& cal,
                               const AQLString& rollDayString,
                               const AQLString& fixLag,
                               bool eomRoll,
                               const AQLString& interpolation,
                               const AQLString& firstStub,
                               const AQLString& lastStub,
                               const AQLString& stub,
                               const AQLString& crvID,
                               const AQLStringVector& curveNames,
                               const AQLStringVector& curveTenors,
                               const DoubleVector& tenorCurveFixings,
                               const AQLString& useCurveName,
                               const AQLString& toleranceTenor,
                               bool isFwdInterp,
                               bool useFwdData )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirSwapStubRate_inputs", crvID ) );
            file.write( "generatorFunction", "tryMirSwapStubRate" );
            file.write( "effDt", effDt );
            file.write( "mat", mat );
            file.write( "freq", freq );
            file.write( "dayCt", dayCt );
            file.write( "rollCnv", busDayAdj );
            file.write( "cal", cal );
            file.write( "rollDay", rollDayString );
            file.write( "fixLag", fixLag );
            file.write( "eomRoll", eomRoll );
            file.write( "interpolation", interpolation );
            file.write( "firstStub", firstStub );
            file.write( "lastStub", lastStub );
            file.write( "stub", stub );
            file.write( "crvID", crvID );
            file.write( "curveNames", curveNames );
            file.write( "curveTenors", curveTenors );
            file.write( "tenorCurveFixings", tenorCurveFixings );
            file.write( "useCurveName", useCurveName );
            file.write( "toleranceTenor", toleranceTenor );
            file.write( "isFwdInterp", isFwdInterp );
            file.write( "useFwdData", useFwdData );
        }

        double stubRate = getStubRateAndFixingDate( dataInstance,
												    effDt,
												    mat,
												    freq,
												    dayCt,
												    busDayAdj,
												    cal,
												    rollDayString,
												    fixLag,
												    eomRoll,
												    interpolation,
												    firstStub,
												    lastStub,
												    stub,
												    crvID,
												    curveNames,
												    curveTenors,
												    tenorCurveFixings,
												    useCurveName,
												    toleranceTenor,
												    isFwdInterp,
												    useFwdData ).stubRate_;

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirSwapStubRate_outputs", crvID ) );
            file.write( "output", stubRate );
        }

        return stubRate;

        VALID_EXCEPTION_END
    }

	/* @brief			validation method for mirSwapStubFixingDate
    *  @param [in]		dataInstance			    Pointer to the entity pool
    *  @param [in]		effDt				The effective start date of the swap, ie, base date + spot date
    *  @param [in]		mat					Swap maturity date or tenor
    *  @param [in]		freq				Floating leg frequency
    *  @param [in]		dayCt				Floating leg day count convention
    *  @param [in]		busDayAdj			Floating leg business day adjustment
    *  @param [in]		cal					Floating leg calendar
    *  @param [in]		rollDayString		Floating leg rolling day
    *  @param [in]		fixLag				Floating leg fixing day lag
    *  @param [in]		eomRoll				Boolean that decides if EOM rolling is enforced (only when effective date is on EOM)
    *  @param [in]		interpolation		Interpolation method
    *  @param [in]		fistStub			End date of the front stub period on floating leg
    *  @param [in]		lastStub			Start date of the end stub period on floating leg
    *  @param [in]		stubType			Floating leg stub type
    *  @param [in]		crvID				ID of the curve set
    *  @param [in]		curveNames			A list of curves to interpolate from
    *  @param [in]		curveTenors			A list of tenors corresponding to curve names
    *  @param [in]		tenorCurveFixings	A list of tenors corresponding to curve names
    *  @param [in]		useCurveName		A curve specifically chosen by user to use
    *  @param [in]		toleranceTenor		Tenor that defines if a nearby curve should be chosen for use
    *  @param [in]		isFwdInterp			Boolean that decides if direct interpolation on fwd rates is employed
    *  @param [in]		useFwdData			Use fwd rates directly or derive them from spot rates
    */
    AQLDate tryMirSwapStubFixingDate( AQLDataInstance* dataInstance,
									 const AQLString& effDt,
									 const AQLString& mat,
									 const AQLString& freq,
									 const AQLString& dayCt,
									 const AQLString& busDayAdj,
									 const AQLString& cal,
									 const AQLString& rollDayString,
									 const AQLString& fixLag,
									 bool eomRoll,
									 const AQLString& interpolation,
									 const AQLString& firstStub,
									 const AQLString& lastStub,
									 const AQLString& stub,
									 const AQLString& crvID,
									 const AQLStringVector& curveNames,
									 const AQLStringVector& curveTenors,
									 const DoubleVector& tenorCurveFixings,
									 const AQLString& useCurveName,
									 const AQLString& toleranceTenor,
									 bool isFwdInterp,
									 bool useFwdData )
    {
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirSwapStubFixingDate_inputs", crvID ) );
            file.write( "generatorFunction", "tryMirSwapStubRate" );
            file.write( "effDt", effDt );
            file.write( "mat", mat );
            file.write( "freq", freq );
            file.write( "dayCt", dayCt );
            file.write( "rollCnv", busDayAdj );
            file.write( "cal", cal );
            file.write( "rollDay", rollDayString );
            file.write( "fixLag", fixLag );
            file.write( "eomRoll", eomRoll );
            file.write( "interpolation", interpolation );
            file.write( "firstStub", firstStub );
            file.write( "lastStub", lastStub );
            file.write( "stub", stub );
            file.write( "crvID", crvID );
            file.write( "curveNames", curveNames );
            file.write( "curveTenors", curveTenors );
            file.write( "tenorCurveFixings", tenorCurveFixings );
            file.write( "useCurveName", useCurveName );
            file.write( "toleranceTenor", toleranceTenor );
            file.write( "isFwdInterp", isFwdInterp );
            file.write( "useFwdData", useFwdData );
        }

        const AQLDate stubFixingDate = getStubRateAndFixingDate( dataInstance,
																effDt,
																mat,
																freq,
																dayCt,
																busDayAdj,
																cal,
																rollDayString,
																fixLag,
																eomRoll,
																interpolation,
																firstStub,
																lastStub,
																stub,
																crvID,
																curveNames,
																curveTenors,
																tenorCurveFixings,
																useCurveName,
																toleranceTenor,
																isFwdInterp,
																useFwdData ).fixingDate_;

        if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( decorateFilename( "tryMirSwapStubFixingDate_outputs", crvID ) );
            file.write( "output", stubFixingDate );
        }

        return stubFixingDate;

        VALID_EXCEPTION_END
    }

	

}


