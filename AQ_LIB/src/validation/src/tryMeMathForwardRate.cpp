#include "tryMeCurveInterpolation.h"
#include "StructuredExceptionHandler.h"
#include "CurveUtilities.h"
#include "CreateDataFile.h"
#include "AQLDate.h"
#include "RecordMacros.h"					// Record Macros
#include "CoreEnumerations.h"				// isHybridInterpolation method
#include "CurveInterpolation.h"				// CurveInterpolation Class
#include "AQLMathInterpolationUtilities.h"	// includes buildInterpolator
#include "CurveValidation.h"				// convert curve dates to terms

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{
    
	//validation interface for tryMeCurveForwardRate method
	double tryMeMathForwardRate( const AQLDate & fixingDate,
							     const AQLDate & asOfDate,
							     const etrading::InterpolationEnum interpolationEnum,
							     const etrading::StateVariableEnum stateVariableEnum,
							     const std::vector<AQLDate> & xValues,
							     const std::vector<double> & yValues,
								 const etrading::DayCountEnum accrualDaycount,
								 const std::string & curveFrequencyTenor,
								 const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
								 const std::string & fixingCalendar,
							     const AQLDate & joinDate,
							     const size_t polynomialOrder,
							     const StandardStringMatrix & forwardAdjustmentTable,
								 const etrading::CompoundingFrequencyEnum & compoundFreqEnum )
	{
		VALID_EXCEPTION_START

		// TODO: Cannot Write Out StandardStringMatrix - Fix this
		RECORD_INPUTS( fixingDate, asOfDate, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusinessDayAdjustment, fixingCalendar, joinDate, polynomialOrder, compoundFreqEnum ) // forwardAdjustmentTable
		
		AQ_REQUIRE( xValues.size() == yValues.size(), "Invalid Input: Interpolation xValues and yValues must be the same size" )
		AQ_REQUIRE( xValues.size() > 0, "Invalid Input: Interpolation xValues and yValues cannot be empty" )
		AQ_REQUIRE( joinDate == AQLDate() || joinDate >= asOfDate, "Invalid Input: JoinDate cannot be before the curve AsOfDate" )

		const etrading::CurveInterpolation curveModule( asOfDate, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusinessDayAdjustment, fixingCalendar, joinDate, polynomialOrder, forwardAdjustmentTable );
		const double forwardRate = curveModule.forwardRate( fixingDate );

		RECORD_OUTPUTS_AND_RETURN_RESULT( forwardRate )
		
		VALID_EXCEPTION_END
	}

	// validation interface for tryMeCurveForwardRates method
	std::vector<double> tryMeMathForwardRates( const std::vector<AQLDate> & fixingDates,
											   const AQLDate & asOfDate,
											   const etrading::InterpolationEnum interpolationEnum,
											   const etrading::StateVariableEnum stateVariableEnum,
											   const std::vector<AQLDate> & xValues,
											   const std::vector<double> & yValues,
											   const etrading::DayCountEnum accrualDaycount,
											   const std::string & curveFrequencyTenor,
											   const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
											   const std::string & fixingCalendar,
											   const AQLDate & joinDate,
											   const size_t polynomialOrder,
											   const StandardStringMatrix & forwardAdjustmentTable,
											   const etrading::CompoundingFrequencyEnum & compoundFreqEnum )
	{
		VALID_EXCEPTION_START
		
		// TODO: Cannot Write Out StandardStringMatrix - Fix this
		RECORD_INPUTS( fixingDates, asOfDate, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusinessDayAdjustment, fixingCalendar, joinDate, polynomialOrder, compoundFreqEnum ) // forwardAdjustmentTable
		
		AQ_REQUIRE( xValues.size() == yValues.size(), "Invalid Input: Interpolation xValues and yValues must be the same size" )
		AQ_REQUIRE( xValues.size() > 0, "Invalid Input: Interpolation xValues and yValues cannot be empty" )
		AQ_REQUIRE( joinDate == AQLDate() || joinDate >= asOfDate, "Invalid Input: JoinDate cannot be before the curve AsOfDate" )

		const etrading::CurveInterpolation curveModule( asOfDate, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusinessDayAdjustment, fixingCalendar, joinDate, polynomialOrder, forwardAdjustmentTable );
		const std::vector<double> forwardRates = curveModule.forwardRates( fixingDates );

		RECORD_OUTPUTS_AND_RETURN_RESULT( forwardRates )
		
		VALID_EXCEPTION_END
	}

	// validation interface for tryMeCurveDiscountFactor method
	double tryMeMathDiscountFactor( const AQLDate & paymentDate,
									const AQLDate & asOfDate,
									const etrading::InterpolationEnum interpolationEnum,
									const etrading::StateVariableEnum stateVariableEnum,
									const std::vector<AQLDate> & xValues,
									const std::vector<double> & yValues,
									const etrading::DayCountEnum accrualDaycount,
									const std::string & curveFrequencyTenor,
								    const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
								    const std::string & fixingCalendar,
									const AQLDate & joinDate,
									const size_t polynomialOrder,
									const StandardStringMatrix & forwardAdjustmentTable,
								    const etrading::CompoundingFrequencyEnum & compoundFreqEnum )
	{
		VALID_EXCEPTION_START
		
		// TODO: Cannot Write Out StandardStringMatrix - Fix this
		RECORD_INPUTS( paymentDate, asOfDate, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusinessDayAdjustment, fixingCalendar, joinDate, polynomialOrder, compoundFreqEnum ) // forwardAdjustmentTable
		AQ_REQUIRE( xValues.size() == yValues.size(), "Invalid Input: Interpolation xValues and yValues must be the same size" )
		AQ_REQUIRE( xValues.size() > 0, "Invalid Input: Interpolation xValues and yValues cannot be empty" )
		AQ_REQUIRE( joinDate == AQLDate() || joinDate >= asOfDate, "Invalid Input: JoinDate cannot be before the curve AsOfDate" )

		const etrading::CurveInterpolation curveModule( asOfDate, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusinessDayAdjustment, fixingCalendar, joinDate, polynomialOrder, forwardAdjustmentTable );
		const double discountFactor = curveModule.discountFactor( paymentDate );

		RECORD_OUTPUTS_AND_RETURN_RESULT( discountFactor )
		
		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for tryMeCurveDiscountFactors method
	*  @param [in]		paymentDatesAsTerm
	*  @param [in]		xValues
	*  @param [in]		yValues
	*  @param [in]		forwardAdjustments
	*/
	std::vector<double> tryMeMathDiscountFactors( const std::vector<AQLDate> & paymentDates,
											      const AQLDate & asOfDate,
											      const etrading::InterpolationEnum interpolationEnum,
											      const etrading::StateVariableEnum stateVariableEnum,
											      const std::vector<AQLDate> & xValues,
											      const std::vector<double> & yValues,
												  const etrading::DayCountEnum accrualDaycount,
												  const std::string & curveFrequencyTenor,
											      const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
											      const std::string & fixingCalendar,
											      const AQLDate & joinDate,
											      const size_t polynomialOrder,
											      const StandardStringMatrix & forwardAdjustmentTable,
												  const etrading::CompoundingFrequencyEnum & compoundFreqEnum )
	{
		VALID_EXCEPTION_START
		
		// TODO: Cannot Write Out StandardStringMatrix - Fix this
		RECORD_INPUTS( paymentDates, asOfDate, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusinessDayAdjustment, fixingCalendar, joinDate, polynomialOrder, compoundFreqEnum ) // forwardAdjustmentTable
		
		AQ_REQUIRE( xValues.size() == yValues.size(), "Invalid Input: Interpolation xValues and yValues must be the same size" )
		AQ_REQUIRE( xValues.size() > 0, "Invalid Input: Interpolation xValues and yValues cannot be empty" )
		AQ_REQUIRE( joinDate == AQLDate() || joinDate >= asOfDate, "Invalid Input: JoinDate cannot be before the curve AsOfDate" )

		const etrading::CurveInterpolation curveModule( asOfDate, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusinessDayAdjustment, fixingCalendar, joinDate, polynomialOrder, forwardAdjustmentTable );
		const std::vector<double> discountFactors = curveModule.discountFactors( paymentDates );

		RECORD_OUTPUTS_AND_RETURN_RESULT( discountFactors )
		
		VALID_EXCEPTION_END
	}

	// *** LEGACY METHOD *** validation interface for tryMeCurveForwardRate method
	double tryMeMathForwardRate( const AQLDate & fixingDate,
								 const AQLDate & asOfDate,
								 const std::string & curveCollection,
								 const std::string & curveIndex,
								 const bool useForwardInterpolation,
								 const etrading::InterpolationEnum interpolationEnum,
								 const etrading::StateVariableEnum stateVariableEnum,
								 const std::vector<AQLDate> & xValues,
								 const std::vector<double> & yValues,
								 const etrading::DayCountEnum accrualDaycount,
								 const std::string & curveFrequencyTenor,
								 const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
								 const std::string & fixingCalendar,
								 const AQLDate & joinDate,
								 const StandardStringMatrix & forwardAdjustmentTable,
								 const etrading::CompoundingFrequencyEnum & compoundFreqEnum )
	{
		VALID_EXCEPTION_START

		// TODO: Cannot Write Out StandardStringMatrix - Fix this
		RECORD_INPUTS( fixingDate, asOfDate, curveCollection, curveIndex, useForwardInterpolation, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusinessDayAdjustment, fixingCalendar, joinDate, compoundFreqEnum ) // forwardAdjustmentTable
		
		AQ_REQUIRE( xValues.size() == yValues.size(), "Invalid Input: Interpolation xValues and yValues must be the same size" )
		AQ_REQUIRE( xValues.size() > 0, "Invalid Input: Interpolation xValues and yValues cannot be empty" )
		AQ_REQUIRE( joinDate == AQLDate() || joinDate >= asOfDate, "Invalid Input: JoinDate cannot be before the curve AsOfDate" )

		// Convert Dates to Terms
		double joinDateAsDouble = etrading::convertCurveDateToTerm( asOfDate, joinDate );
		std::vector<double> xValuesAsDouble = etrading::convertCurveDatesToTerms( asOfDate, xValues );

		// Build Legacy Interpolator Object
		std::shared_ptr<AQLInterpolationBase> laInterpolationObject
			= AQLMathInterpolationUtilities::buildInterpolator( xValuesAsDouble,
															   yValues,
															   interpolationEnum,
															   joinDateAsDouble );
		
		// Build Curve Interpolation Module
		const etrading::CurveInterpolation curveModule( asOfDate,
														curveCollection,
														curveIndex,
														interpolationEnum,
														stateVariableEnum,
														laInterpolationObject,
														accrualDaycount, 
														curveFrequencyTenor,
														fixingBusinessDayAdjustment,
														fixingCalendar,
														forwardAdjustmentTable );

		const double forwardRate = curveModule.forwardRate( fixingDate, useForwardInterpolation );

		RECORD_OUTPUTS_AND_RETURN_RESULT( forwardRate )
		
		VALID_EXCEPTION_END
	}

	// *** LEGACY METHOD *** validation interface for tryMeCurveForwardRates method
	std::vector<double> tryMeMathForwardRates( const std::vector<AQLDate> & fixingDates,
											   const AQLDate & asOfDate,
											   const std::string & curveCollection,
											   const std::string & curveIndex,
											   const bool useForwardInterpolation,
											   const etrading::InterpolationEnum interpolationEnum,
											   const etrading::StateVariableEnum stateVariableEnum,
											   const std::vector<AQLDate> & xValues,
											   const std::vector<double> & yValues,
											   const etrading::DayCountEnum accrualDaycount,
											   const std::string & curveFrequencyTenor,
											   const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
											   const std::string & fixingCalendar,
											   const AQLDate & joinDate,
											   const StandardStringMatrix & forwardAdjustmentTable,
											   const etrading::CompoundingFrequencyEnum & compoundFreqEnum )
	{
		VALID_EXCEPTION_START
		
		// TODO: Cannot Write Out StandardStringMatrix - Fix this
		RECORD_INPUTS( fixingDates, asOfDate, curveCollection, curveIndex, useForwardInterpolation, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusinessDayAdjustment, fixingCalendar, joinDate, compoundFreqEnum ) // forwardAdjustmentTable
		
		AQ_REQUIRE( xValues.size() == yValues.size(), "Invalid Input: Interpolation xValues and yValues must be the same size" )
		AQ_REQUIRE( xValues.size() > 0, "Invalid Input: Interpolation xValues and yValues cannot be empty" )
		AQ_REQUIRE( joinDate == AQLDate() || joinDate >= asOfDate, "Invalid Input: JoinDate cannot be before the curve AsOfDate" )

		// Convert Dates to Terms
		double joinDateAsDouble = etrading::convertCurveDateToTerm( asOfDate, joinDate );
		std::vector<double> xValuesAsDouble = etrading::convertCurveDatesToTerms( asOfDate, xValues );

		// Build Legacy Interpolator Object
		std::shared_ptr<AQLInterpolationBase> laInterpolationObject
			= AQLMathInterpolationUtilities::buildInterpolator( xValuesAsDouble,
															   yValues,
															   interpolationEnum,
															   joinDateAsDouble );

		// Build Curve Interpolation Module		
		const etrading::CurveInterpolation curveModule( asOfDate,
														curveCollection,
														curveIndex,
														interpolationEnum,
														stateVariableEnum,
														laInterpolationObject,
														accrualDaycount, 
														curveFrequencyTenor,
														fixingBusinessDayAdjustment,
														fixingCalendar,
														forwardAdjustmentTable );

		const std::vector<double> forwardRates = curveModule.forwardRates( fixingDates, useForwardInterpolation );

		RECORD_OUTPUTS_AND_RETURN_RESULT( forwardRates )
		
		VALID_EXCEPTION_END
	}

	// *** LEGACY METHOD *** validation interface for tryMeCurveDiscountFactor method
	double tryMeMathDiscountFactor( const AQLDate & paymentDate,
									const AQLDate & asOfDate,
									const std::string & curveCollection,
									const std::string & curveIndex,
									const bool useForwardInterpolation,
									const etrading::InterpolationEnum interpolationEnum,
									const etrading::StateVariableEnum stateVariableEnum,
									const std::vector<AQLDate> & xValues,
									const std::vector<double> & yValues,
									const etrading::DayCountEnum accrualDaycount,
									const std::string & curveFrequencyTenor,
									const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
									const std::string & fixingCalendar,
									const AQLDate & joinDate,
									const StandardStringMatrix & forwardAdjustmentTable,
									const etrading::CompoundingFrequencyEnum & compoundFreqEnum )
	{
		VALID_EXCEPTION_START

		// TODO: Cannot Write Out StandardStringMatrix - Fix this
		RECORD_INPUTS( paymentDate, asOfDate, curveCollection, curveIndex, useForwardInterpolation, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusinessDayAdjustment, fixingCalendar, joinDate, compoundFreqEnum ) // forwardAdjustmentTable
		
		AQ_REQUIRE( xValues.size() == yValues.size(), "Invalid Input: Interpolation xValues and yValues must be the same size" )
		AQ_REQUIRE( xValues.size() > 0, "Invalid Input: Interpolation xValues and yValues cannot be empty" )
		AQ_REQUIRE( joinDate == AQLDate() || joinDate >= asOfDate, "Invalid Input: JoinDate cannot be before the curve AsOfDate" )

		// Convert Dates to Terms
		double joinDateAsDouble = etrading::convertCurveDateToTerm( asOfDate, joinDate );
		std::vector<double> xValuesAsDouble = etrading::convertCurveDatesToTerms( asOfDate, xValues );

		// Build Legacy Interpolator Object
		std::shared_ptr<AQLInterpolationBase> laInterpolationObject
			= AQLMathInterpolationUtilities::buildInterpolator( xValuesAsDouble,
															   yValues,
															   interpolationEnum,
															   joinDateAsDouble );

		// Build Curve Interpolation Module
		const etrading::CurveInterpolation curveModule( asOfDate,
														curveCollection,
														curveIndex,
														interpolationEnum,
														stateVariableEnum,
														laInterpolationObject,
														accrualDaycount, 
														curveFrequencyTenor,
														fixingBusinessDayAdjustment,
														fixingCalendar,
														forwardAdjustmentTable );

		const double discountFactor = curveModule.discountFactor( paymentDate );

		RECORD_OUTPUTS_AND_RETURN_RESULT( discountFactor )
		
		VALID_EXCEPTION_END
	}

	// *** LEGACY METHOD *** validation interface for tryMeCurveDiscountFactors method
	std::vector<double> tryMeMathDiscountFactors( const std::vector<AQLDate> & paymentDates,
												  const AQLDate & asOfDate,
												  const std::string & curveCollection,
												  const std::string & curveIndex,
												  const bool useForwardInterpolation,
												  const etrading::InterpolationEnum interpolationEnum,
												  const etrading::StateVariableEnum stateVariableEnum,
												  const std::vector<AQLDate> & xValues,
												  const std::vector<double> & yValues,
												  const etrading::DayCountEnum accrualDaycount,
												  const std::string & curveFrequencyTenor,
												  const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
												  const std::string & fixingCalendar,
												  const AQLDate & joinDate,
												  const StandardStringMatrix & forwardAdjustmentTable,
												  const etrading::CompoundingFrequencyEnum & compoundFreqEnum )
	{
		VALID_EXCEPTION_START
		
		// TODO: Cannot Write Out StandardStringMatrix - Fix this
		RECORD_INPUTS( paymentDates, asOfDate, curveCollection, curveIndex, useForwardInterpolation, interpolationEnum, stateVariableEnum, xValues, yValues, accrualDaycount, curveFrequencyTenor, fixingBusinessDayAdjustment, fixingCalendar, joinDate, compoundFreqEnum ) // forwardAdjustmentTable
		
		AQ_REQUIRE( xValues.size() == yValues.size(), "Invalid Input: Interpolation xValues and yValues must be the same size" )
		AQ_REQUIRE( xValues.size() > 0, "Invalid Input: Interpolation xValues and yValues cannot be empty" )
		AQ_REQUIRE( joinDate == AQLDate() || joinDate >= asOfDate, "Invalid Input: JoinDate cannot be before the curve AsOfDate" )

		// Convert Dates to Terms
		double joinDateAsDouble = etrading::convertCurveDateToTerm( asOfDate, joinDate );
		std::vector<double> xValuesAsDouble = etrading::convertCurveDatesToTerms( asOfDate, xValues );

		// Build Legacy Interpolator Object
		std::shared_ptr<AQLInterpolationBase> laInterpolationObject
			= AQLMathInterpolationUtilities::buildInterpolator( xValuesAsDouble,
															   yValues,
															   interpolationEnum,
															   joinDateAsDouble );

		// Build Curve Interpolation Module
		const etrading::CurveInterpolation curveModule( asOfDate,
														curveCollection,
														curveIndex,
														interpolationEnum,
														stateVariableEnum,
														laInterpolationObject,
														accrualDaycount, 
														curveFrequencyTenor,
														fixingBusinessDayAdjustment,
														fixingCalendar,
														forwardAdjustmentTable );

		const std::vector<double> discountFactors = curveModule.discountFactors( paymentDates );

		RECORD_OUTPUTS_AND_RETURN_RESULT( discountFactors )
		
		VALID_EXCEPTION_END
	}

}