/*
 * @brief			Interface for curve interpolation methods
 * @Created:		18th February 2020
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once
#include "vector"
#include "CurveInterpolation.h" // CurveInterpolation Class & Core Enumerations

namespace validation_api
{
	// validation interface for tryMeCurveForwardRate method
	double tryMeMathForwardRate( const LADate & fixingDate,
							     const LADate & asOfDate,
							     const etrading::InterpolationEnum interpolationEnum,
							     const etrading::StateVariableEnum stateVariableEnum,
							     const std::vector<LADate> & xValues,
							     const std::vector<double> & yValues,
								 const etrading::DayCountEnum accrualDaycount,
								 const std::string & curveFrequencyTenor,
							     const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
							     const std::string & fixingCalendar,
							     const LADate & joinDate = LADate(),
							     const size_t polynomialOrder = 0,
							     const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
								 const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// validation interface for tryMeCurveForwardRates method
	std::vector<double> tryMeMathForwardRates( const std::vector<LADate> & fixingDates,
											   const LADate & asOfDate,
											   const etrading::InterpolationEnum interpolationEnum,
											   const etrading::StateVariableEnum stateVariableEnum,
											   const std::vector<LADate> & xValues,
											   const std::vector<double> & yValues,
											   const etrading::DayCountEnum accrualDaycount,
											   const std::string & curveFrequencyTenor,
											   const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
											   const std::string & fixingCalendar,
											   const LADate & joinDate = LADate(),
											   const size_t polynomialOrder = 0,
											   const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
											   const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// validation interface for tryMeCurveDiscountFactor method
	double tryMeMathDiscountFactor( const LADate & paymentDate,
									const LADate & asOfDate,
									const etrading::InterpolationEnum interpolationEnum,
									const etrading::StateVariableEnum stateVariableEnum,
									const std::vector<LADate> & xValues,
									const std::vector<double> & yValues,
									const etrading::DayCountEnum accrualDaycount,
									const std::string & curveFrequencyTenor,
								    const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
								    const std::string & fixingCalendar,
									const LADate & joinDate = LADate(),
									const size_t polynomialOrder = 0,
									const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
									const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// validation interface for tryMeCurveDiscountFactors method
	std::vector<double> tryMeMathDiscountFactors( const std::vector<LADate> & paymentDates,
											      const LADate & asOfDate,
											      const etrading::InterpolationEnum interpolationEnum,
											      const etrading::StateVariableEnum stateVariableEnum,
											      const std::vector<LADate> & xValues,
											      const std::vector<double> & yValues,
												  const etrading::DayCountEnum accrualDaycount,
												  const std::string & curveFrequencyTenor,
											      const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
											      const std::string & fixingCalendar,
											      const LADate & joinDate = LADate(),
											      const size_t polynomialOrder = 0,
											      const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
												  const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// *** LEGACY METHOD *** validation interface for tryMeCurveForwardRate method
	double tryMeMathForwardRate( const LADate & fixingDate,
							     const LADate & asOfDate,
								 const std::string & curveCollection,
								 const std::string & curveIndex,
								 const bool useForwardInterpolation,
							     const etrading::InterpolationEnum interpolationEnum,
							     const etrading::StateVariableEnum stateVariableEnum,
							     const std::vector<LADate> & xValues,
							     const std::vector<double> & yValues,
								 const etrading::DayCountEnum accrualDaycount,
								 const std::string & curveFrequencyTenor,
								 const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
								 const std::string & fixingCalendar,
								 const LADate & joinDate = LADate(),
								 const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
								 const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// *** LEGACY METHOD *** validation interface for tryMeCurveForwardRates method
	std::vector<double> tryMeMathForwardRates( const std::vector<LADate> & fixingDates,
											   const LADate & asOfDate,
											   const std::string & curveCollection,
											   const std::string & curveIndex,
											   const bool useForwardInterpolation,
											   const etrading::InterpolationEnum interpolationEnum,
											   const etrading::StateVariableEnum stateVariableEnum,
											   const std::vector<LADate> & xValues,
											   const std::vector<double> & yValues,
											   const etrading::DayCountEnum accrualDaycount,
											   const std::string & curveFrequencyTenor,
											   const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
											   const std::string & fixingCalendar,
											   const LADate & joinDate = LADate(),
											   const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
											   const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// *** LEGACY METHOD *** validation interface for tryMeCurveDiscountFactor method
	double tryMeMathDiscountFactor( const LADate & paymentDate,
									const LADate & asOfDate,
									const std::string & curveCollection,
								    const std::string & curveIndex,
								    const bool useForwardInterpolation,
									const etrading::InterpolationEnum interpolationEnum,
									const etrading::StateVariableEnum stateVariableEnum,
									const std::vector<LADate> & xValues,
									const std::vector<double> & yValues,
									const etrading::DayCountEnum accrualDaycount,
									const std::string & curveFrequencyTenor,
								    const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
								    const std::string & fixingCalendar,
									const LADate & joinDate = LADate(),
								    const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
									const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// *** LEGACY METHOD *** validation interface for tryMeCurveDiscountFactors method
	std::vector<double> tryMeMathDiscountFactors( const std::vector<LADate> & paymentDates,
											      const LADate & asOfDate,
												  const std::string & curveCollection,
												  const std::string & curveIndex,
												  const bool useForwardInterpolation,
											      const etrading::InterpolationEnum interpolationEnum,
											      const etrading::StateVariableEnum stateVariableEnum,
											      const std::vector<LADate> & xValues,
											      const std::vector<double> & yValues,
												  const etrading::DayCountEnum accrualDaycount,
											      const std::string & curveFrequencyTenor,
											      const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
											      const std::string & fixingCalendar,
												  const LADate & joinDate = LADate(),
											      const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
												  const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );
}
