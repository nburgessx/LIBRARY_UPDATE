#pragma once
#include "vector"
#include "CurveInterpolation.h" // CurveInterpolation Class & Core Enumerations

namespace validation
{
	// validation interface for tryAqCurvesForwardRate method
	double tryAqMathForwardRate( const AQLDate & fixingDate,
							     const AQLDate & asOfDate,
							     const etrading::InterpolationEnum interpolationEnum,
							     const etrading::StateVariableEnum stateVariableEnum,
							     const std::vector<AQLDate> & xValues,
							     const std::vector<double> & yValues,
								 const etrading::DayCountEnum accrualDaycount,
								 const std::string & curveFrequencyTenor,
							     const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
							     const std::string & fixingCalendar,
							     const AQLDate & joinDate = AQLDate(),
							     const size_t polynomialOrder = 0,
							     const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
								 const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// validation interface for tryAqCurvesForwardRates method
	std::vector<double> tryAqMathForwardRates( const std::vector<AQLDate> & fixingDates,
											   const AQLDate & asOfDate,
											   const etrading::InterpolationEnum interpolationEnum,
											   const etrading::StateVariableEnum stateVariableEnum,
											   const std::vector<AQLDate> & xValues,
											   const std::vector<double> & yValues,
											   const etrading::DayCountEnum accrualDaycount,
											   const std::string & curveFrequencyTenor,
											   const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
											   const std::string & fixingCalendar,
											   const AQLDate & joinDate = AQLDate(),
											   const size_t polynomialOrder = 0,
											   const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
											   const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// validation interface for tryAqCurvesDiscountFactor method
	double tryAqMathDiscountFactor( const AQLDate & paymentDate,
									const AQLDate & asOfDate,
									const etrading::InterpolationEnum interpolationEnum,
									const etrading::StateVariableEnum stateVariableEnum,
									const std::vector<AQLDate> & xValues,
									const std::vector<double> & yValues,
									const etrading::DayCountEnum accrualDaycount,
									const std::string & curveFrequencyTenor,
								    const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
								    const std::string & fixingCalendar,
									const AQLDate & joinDate = AQLDate(),
									const size_t polynomialOrder = 0,
									const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
									const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// validation interface for tryAqCurvesDiscountFactors method
	std::vector<double> tryAqMathDiscountFactors( const std::vector<AQLDate> & paymentDates,
											      const AQLDate & asOfDate,
											      const etrading::InterpolationEnum interpolationEnum,
											      const etrading::StateVariableEnum stateVariableEnum,
											      const std::vector<AQLDate> & xValues,
											      const std::vector<double> & yValues,
												  const etrading::DayCountEnum accrualDaycount,
												  const std::string & curveFrequencyTenor,
											      const etrading::BusinessDayAdjustmentEnum fixingBusinessDayAdjustment,
											      const std::string & fixingCalendar,
											      const AQLDate & joinDate = AQLDate(),
											      const size_t polynomialOrder = 0,
											      const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
												  const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// *** LEGACY METHOD *** validation interface for tryAqCurvesForwardRate method
	double tryAqMathForwardRate( const AQLDate & fixingDate,
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
								 const AQLDate & joinDate = AQLDate(),
								 const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
								 const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// *** LEGACY METHOD *** validation interface for tryAqCurvesForwardRates method
	std::vector<double> tryAqMathForwardRates( const std::vector<AQLDate> & fixingDates,
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
											   const AQLDate & joinDate = AQLDate(),
											   const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
											   const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// *** LEGACY METHOD *** validation interface for tryAqCurvesDiscountFactor method
	double tryAqMathDiscountFactor( const AQLDate & paymentDate,
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
									const AQLDate & joinDate = AQLDate(),
								    const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
									const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// *** LEGACY METHOD *** validation interface for tryAqCurvesDiscountFactors method
	std::vector<double> tryAqMathDiscountFactors( const std::vector<AQLDate> & paymentDates,
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
												  const AQLDate & joinDate = AQLDate(),
											      const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
												  const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );
}
