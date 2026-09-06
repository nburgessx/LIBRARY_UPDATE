#pragma once

#include <memory>
#include <string>
#include <vector>

#include "CoreEnumerations.h"
#include "AQLCoreTemplateType.h"
#include "AQLDate.h"
#include "PolynomialInterpolation.h"
#include "AQLInterpolationBase.h"

namespace etrading
{
	// FORWARD ADJUSTMENT HELPER CLASS
	// ----------------------------------------------------------------------------------------------------------------------
	namespace
	{
		struct TurnData
		{
			// Default Constructor
			TurnData() {}

			// Main Constructor
			TurnData( const TurnOfYearAdjustmentTypeEnum turnType, const double & startDateAsDouble, const double & endDateAsDouble, const double & adjustment )
				: turnType_(turnType), startDateAsDouble_(startDateAsDouble), endDateAsDouble_(endDateAsDouble), adjustment_(adjustment)
			{}
			
			TurnOfYearAdjustmentTypeEnum turnType_;
			double startDateAsDouble_;
			double endDateAsDouble_;
			double adjustment_;
		};
	}

	class ForwardAdjustments
	{
		public:

			// Default Constructor
			ForwardAdjustments() {}

			// Main Constructor
			ForwardAdjustments( const AQLDate& asOfDate, const StandardStringMatrix & forwardAdjustments );

			// Methods
			std::vector<double> adjustForwardRates(const std::vector<double> & fixingDatesAsTerms, std::vector<double>& forwardRates) const;
			std::vector<double> adjustDiscountFactors(const std::vector<double> & paymentDatesAsTerms, std::vector<double>& discountFactors) const;

			// Accessors
			AQLDate asOfDate() const								{ return asOfDate_; };
			StandardStringMatrix forwardAdjustments() const		{ return forwardAdjustments_; };
			std::map< double, TurnData > turnData() const		{ return turnData_; }; // Turn Data Map - Key: StartDateAsDouble, Value: TurnData

		private:

			// Member Data
			AQLDate asOfDate_;
			StandardStringMatrix forwardAdjustments_;
			
			// Turn Data Map - Key: StartDateAsDouble, Value: TurnData
			std::map< double, TurnData > turnData_;
	};

	// Interpolation Pointer TypeDef
	typedef std::shared_ptr<ForwardAdjustments> ForwardAdjustmentsPtr;

	// INTERPOLATION HELPER STRUCT
	// ----------------------------------------------------------------------------------------------------------------------
	struct Interpolator
	{
		// Default Constructor
		Interpolator() {}

		// Constructor using Dates - *** Piecewise Constant = FALSE ***
		Interpolator( const AQLDate & asOfDate,
					  const InterpolationEnum interpolationEnum,
					  const StateVariableEnum stateVariableEnum,
					  const std::vector<AQLDate> & xValues,
					  const std::vector<double> & yValues,
					  const DayCountEnum accrualDaycount,
					  const std::string & curveFrequencyTenor,
					  const BusinessDayAdjustmentEnum fixingBusDayAdj,
					  const std::string & fixingCalendar,
					  const AQLDate & joinDate = AQLDate(),
					  const size_t polynomialOrder = 0 );

		// Constructor with Dates Represented as Terms - *** Piecewise Constant = FALSE ***
		Interpolator( const AQLDate & asOfDate,
					  const InterpolationEnum interpolationEnum,
					  const StateVariableEnum stateVariableEnum,
					  const std::vector<double> & xValues,
					  const std::vector<double> & yValues,
					  const DayCountEnum accrualDaycount,
					  const std::string & curveFrequencyTenor,
					  const BusinessDayAdjustmentEnum fixingBusDayAdj,
					  const std::string & fixingCalendar,
					  const double & joinDateAsDouble = 0.0,
					  const size_t polynomialOrder = 0 );

		// Constructor for Legacy Interpolator for Backwards Compatibility - *** Piecewise Constant = TRUE ***
		// Allows isFwdInter = true, where curveCollection and curveIndex is required
		Interpolator( const AQLDate & asOfDate,
					  const std::string & curveCollection,
					  const std::string & curveIndex,
					  const InterpolationEnum interpolationEnum,
					  const StateVariableEnum stateVariableEnum,
					  const std::shared_ptr<AQLInterpolationBase> & aqInterpolationPtr,
					  const DayCountEnum accrualDaycount,
					  const std::string & curveFrequencyTenor,
					  const BusinessDayAdjustmentEnum fixingBusDayAdj,
					  const std::string & fixingCalendar );

		// Constructor for Legacy Interpolator for Backwards Compatibility - *** Piecewise Constant = TRUE ***
		// Does not allows isFwdInter = true, where curveCollection and curveIndex is required
		Interpolator( const AQLDate & asOfDate,
					  const InterpolationEnum interpolationEnum,
					  const StateVariableEnum stateVariableEnum,
					  const std::shared_ptr<AQLInterpolationBase> & aqInterpolationPtr,
					  const DayCountEnum accrualDaycount,
					  const std::string & curveFrequencyTenor,
					  const BusinessDayAdjustmentEnum fixingBusDayAdj,
					  const std::string & fixingCalendar );

		static double joinDateAsDouble( const AQLDate& asOfDate, const AQLDate& joinDate );

		AQLDate						asOfDate_;
		std::string					curveCollection_;
		std::string					curveIndex_;
		InterpolationEnum			interpolationEnum_;
		StateVariableEnum			stateVariableEnum_;
		std::vector<double>			xValues_;
		std::vector<double>			yValues_;
		DayCountEnum				accrualDaycount_;
		BusinessDayAdjustmentEnum	fixingBusDayAdj_;
		std::string					fixingBusDayAdjString_;
		std::string					fixingCalendar_;
		
		// Needed for differentiation and integration in the accrual not curve Act365 Basis
		std::vector<double>			xValuesAccrualDaycountBasis_;

		// For Hybrid / Mixed Interpolation
		double						joinDateAsDouble_;
		
		// For Polynomial Interpolation
		size_t						polynomialOrder_;
		std::shared_ptr<PolynomialInterpolation> polynomialInterpolator_;

		// For Backwards Compatibility we allow the user to specify if rates are piecewise constant between cashflows and ...
		// use the legacy interpolation classes. Piecewise constant is undesirable.
		bool assumePiecewiseConstant_;
		std::shared_ptr<AQLInterpolationBase> aqInterpolationPtr_;
		std::string curveFrequencyTenor_;
		

		private:
		
		// Private Constructor Helper - *** Piecewise Constant = FALSE ***
		void init( const AQLDate & asOfDate,
				   const InterpolationEnum interpolationEnum,
				   const StateVariableEnum stateVariableEnum,
				   const std::vector<double> & xValues,
				   const std::vector<double> & yValues,
				   const DayCountEnum accrualDaycount,
				   const std::string & curveFrequencyTenor,
				   const BusinessDayAdjustmentEnum fixingBusDayAdj,
				   const std::string & fixingCalendar,
				   const double & joinDateAsDouble,
				   const size_t polynomialOrder );

		// Private Constructor Helper - *** Piecewise Constant = TRUE ***
		void init( const AQLDate & asOfDate,
				   const std::string & curveCollection,
				   const std::string & curveIndex,
				   const InterpolationEnum interpolationEnum,
				   const StateVariableEnum stateVariableEnum,
				   const std::shared_ptr<AQLInterpolationBase> & aqInterpolationObject,
				   const DayCountEnum accrualDaycount,
				   const std::string & curveFrequencyTenor,
				   const BusinessDayAdjustmentEnum fixingBusDayAdj,
				   const std::string & fixingCalendar );
	};

	// Interpolation Pointer TypeDef
	typedef std::shared_ptr<Interpolator> InterpolationPtr;

	// ----------------------------------------------------------------------------------------------------------------------

    class CurveInterpolation
    {
    public:

		// Constructor with packaged interpolator
		CurveInterpolation( const InterpolationPtr & interpolator,
							const ForwardAdjustmentsPtr & forwardAdjustments = nullptr );		
		
		// Constructor with interpolation raw inputs as dates
		CurveInterpolation( const AQLDate & asOfDate,
							const InterpolationEnum interpolationEnum,
							const StateVariableEnum stateVariableEnum,
							const std::vector<AQLDate> & xValues,
							const std::vector<double> & yValues,
							const DayCountEnum accrualDaycount,
							const std::string & curveFrequencyTenor,
							const BusinessDayAdjustmentEnum fixingBusDayAdj,
							const std::string & fixingCalendar,
							const AQLDate & joinDate = AQLDate(),
							const size_t polynomialOrder = 0,
							const StandardStringMatrix & forwardAdjustmentsStringMatrix = StandardStringMatrix() );	
		
		// Constructor with interpolation raw inputs as terms
		CurveInterpolation( const AQLDate & asOfDate,
							const InterpolationEnum interpolationEnum,
							const StateVariableEnum stateVariableEnum,
							const std::vector<double> & xValues,
							const std::vector<double> & yValues,
							const DayCountEnum accrualDaycount,
							const std::string & curveFrequencyTenor,
							const BusinessDayAdjustmentEnum fixingBusDayAdj,
							const std::string & fixingCalendar,
							const double & joinDateAsDouble = 0.0,
							const size_t polynomialOrder = 0,
							const StandardStringMatrix & forwardAdjustmentsStringMatrix = StandardStringMatrix() );	
		
		// *** Legacy *** Constructor with interpolation raw inputs and allows isFwdInter = true
		CurveInterpolation( const AQLDate & asOfDate,
							const std::string & curveCollection,
							const std::string & curveIndex,
							const InterpolationEnum interpolationEnum,
							const StateVariableEnum stateVariableEnum,
							const std::shared_ptr<AQLInterpolationBase> & aqInterpolationObject,
							const DayCountEnum accrualDaycount,
							const std::string & curveFrequencyTenor,
							const BusinessDayAdjustmentEnum fixingBusDayAdj,
							const std::string & fixingCalendar,
							const StandardStringMatrix & forwardAdjustmentsStringMatrix = StandardStringMatrix() );	
		
		// *** Legacy *** Constructor with interpolation raw inputs, does not allow isFwdInter = true
		CurveInterpolation( const AQLDate & asOfDate,
							const InterpolationEnum interpolationEnum,
							const StateVariableEnum stateVariableEnum,
							const std::shared_ptr<AQLInterpolationBase> & aqInterpolationObject,
							const DayCountEnum accrualDaycount,
							const std::string & curveFrequencyTenor,
							const BusinessDayAdjustmentEnum fixingBusDayAdj,
							const std::string & fixingCalendar,
							const StandardStringMatrix & forwardAdjustmentsStringMatrix = StandardStringMatrix() );	

		// Copy Constructor
		CurveInterpolation(const CurveInterpolation & rhs);

		// Assignment Operator
		CurveInterpolation & operator=(const CurveInterpolation & rhs);

		// Clone
		virtual std::shared_ptr<CurveInterpolation> clone() const;

		// Methods using Terms - Calibration Methods Provide Dates in Terms or Double Format
		// *** Important Note: For backwards compatibility we support and assume piecewiseConstant rates between cashflows ***
		// --------------------------------------------------------------------------------------
		double discountFactor( const double & paymentDateAsTerms, const CompoundingFrequencyEnum compoundFrequency = SIMPLE_COMPOUNDING ) const; 
		std::vector<double> discountFactors(const std::vector<double> & paymentDatesAsTerms, const CompoundingFrequencyEnum compoundFrequency = SIMPLE_COMPOUNDING ) const;
		
		double forwardRate(const double & fixingDateAsTerm, const bool useForwardInterpolation = false, const CompoundingFrequencyEnum compoundFrequency = SIMPLE_COMPOUNDING ) const;
		std::vector<double> forwardRates(const std::vector<double> & fixingDatesAsTerms, const bool useForwardInterpolation = false, const CompoundingFrequencyEnum compoundFrequency = SIMPLE_COMPOUNDING ) const;
		
		// Method to calculate the OIS or ARR effective compound rate(s)
		std::vector<double> compoundRates(const std::vector<double> & fixingDatesAsTerms, const OISCompoundingEnum & compoundingEnum ) const;

		// Methods using Dates (Server Users Provide Dates)
		// These methods will call the above methods taking dates in AQLDate format
		// *** Important Note: For backwards compatibility we support and assume piecewiseConstant rates between cashflows ***
		// --------------------------------------------------------------------------------------
		double discountFactor( const AQLDate & paymentDate ) const;
		std::vector<double> discountFactors( const std::vector<AQLDate> & paymentDates ) const;
		
		double forwardRate( const AQLDate & fixingDate, const bool & useForwardInterpolation = false ) const;
		std::vector<double> forwardRates( const std::vector<AQLDate> & fixingDate, const bool useForwardInterpolation = false ) const;
		
		// Method to calculate the OIS or ARR effective compound rate(s)
		std::vector<double> compoundRates( const std::vector<AQLDate> & fixingDates, const OISCompoundingEnum & compoundingEnum ) const;

		// Helper functions to do interpolation from state variables
		double interpolate( const double & x ) const;
		std::vector<double> interpolate( const std::vector<double> & xVector ) const;

		// Helper functions to perform integration
		double integrate( const double & lowerBound, const double & upperBound ) const;
		std::vector<double> integrate( const std::vector<double> & lowerBounds, const std::vector<double> & upperBounds ) const;

		// Helper functions to perform instantaneous differentation 
		double differentiate(const double & x) const;
		std::vector<double> differentiate( const std::vector<double> & xVector ) const;

		// Helper functions to perform discrete differentiation over a range of x values
		double differentiate(const double & fromXPoint, const double & toXPoint) const;
		std::vector<double> differentiate( const std::vector<double> & fromXPoints, const std::vector<double> & toXPoints ) const;

		// Accessors
		// --------------------------------------------------------------------------------------
		InterpolationPtr interpolator() const { return interpolator_; };
		ForwardAdjustmentsPtr forwardAdjustments() const { return forwardAdjustments_; };

    private:

		// Method to calculate the fixing end date as a double relatative to the asOfDate
		double fixingEndTerm( const double & fixingStartTerm,
							  const std::string & curveFrequencyTenor,
							  const std::string & businessDayAdjustment,
							  const std::string & calendar ) const;

		// Method to calculate the fixing end dates as a vector of doubles relatative to the asOfDate
		std::vector<double> fixingEndTerms( const std::vector<double> & fixingStartTerms,
											const std::string & curveFrequencyTenor,
											const std::string & businessDayAdjustment,
											const std::string & calendar ) const;

		InterpolationPtr interpolator_;
		ForwardAdjustmentsPtr forwardAdjustments_;
    };

}
