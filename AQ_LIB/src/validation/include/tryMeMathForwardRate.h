#pragma once
#include "vector"
#include "CurveInterpolation.h" // CurveInterpolation Class & Core Enumerations

namespace validation
{
	// validation interface for tryMeCurveForwardRate method
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
							     const AQLDate & joinDate = AQLDate(),
							     const size_t polynomialOrder = 0,
							     const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
								 const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

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
											   const AQLDate & joinDate = AQLDate(),
											   const size_t polynomialOrder = 0,
											   const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
											   const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

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
									const AQLDate & joinDate = AQLDate(),
									const size_t polynomialOrder = 0,
									const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
									const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

	// validation interface for tryMeCurveDiscountFactors method
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
											      const AQLDate & joinDate = AQLDate(),
											      const size_t polynomialOrder = 0,
											      const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
												  const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

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
								 const AQLDate & joinDate = AQLDate(),
								 const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
								 const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

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
											   const AQLDate & joinDate = AQLDate(),
											   const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
											   const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

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
									const AQLDate & joinDate = AQLDate(),
								    const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
									const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );

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
												  const AQLDate & joinDate = AQLDate(),
											      const StandardStringMatrix & forwardAdjustmentTable = StandardStringMatrix(),
												  const etrading::CompoundingFrequencyEnum & compoundFreqEnum = etrading::SIMPLE_COMPOUNDING );
}
