// DiscountFactorResults.h

/*
 * @brief			Discount Factor Results Class
 * @Created:		15th July 2017
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once
#include <vector>
#include <memory>

#include "LADate.h"
#include "CoreEnumerations.h"
#include "LAInterpolationBase.h"
#include "Variant.h"
#include "CurveInterpolation.h"		// Curve Interpolation Module with turn-of-year logic

namespace etrading
{
    
    // Typedefs
    typedef std::vector<double> VectorDouble;
    typedef std::vector<LADate> VectorDate;
    typedef std::vector<double> DoubleVector;   // For Backwards compatibility
    typedef std::vector<LADate> DateVector;     // For Backwards compatibility
    

    // =============================== DISCOUNT FACTOR RESULTS CLASS ==================================================================

    // Class to store and retrieve discount factors
    class DiscountFactorResults
    {

    public:
        DiscountFactorResults() {};
        ~DiscountFactorResults() {};
        
        // Copy Constructor
        DiscountFactorResults( const DiscountFactorResults& rhs );

		// Copy and Override Constructor
		DiscountFactorResults( const DiscountFactorResults& rhs, const VectorDate& paymentDates, const VectorDouble& discountFactors, const LADate & joinDate = LADate() );

		// Clone
		std::shared_ptr<DiscountFactorResults> clone() const;

        // Assignment Operator
        DiscountFactorResults & operator=( const DiscountFactorResults & rhs );

        // Object Pool Constructor - Object Pool Stores Dates as Doubles representing Year Fractions
		// Note: Spot Date should be Curve Swap Calibration Instrument Spot Date - needed for the linear spline join date calculation
        DiscountFactorResults( const CurveTenorEnum & curveTenor,
                               const LADate & asOfDate,
							   const LADate & spotDate,
							   const std::string & curveCollection,		// Needed to support legacy isFwdInter = true
							   const std::string & curveIndex,			// Needed to support legacy isFwdInter = true
                               const InterpolationEnum & interpolationEnum,
                               const VectorDouble & paymentDatesInTermFormat,
                               const VectorDouble & discountFactors,
							   const DayCountEnum & daycount,
                               const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                               const StandardString & fixingCalendar,
                               const LADate & joinDate = LADate(),
							   const StandardStringMatrix & forwardAdjustments = StandardStringMatrix() );
        
        // Server Side (Non-Object Pool) Constructor - Dates entered as regular dates here instead of object pool doubles representing year fractions
		// Note: Spot Date should be Curve Swap Calibration Instrument Spot Date - needed for the linear spline join date calculation
        DiscountFactorResults( const CurveTenorEnum & curveTenor,
                               const LADate & asOfDate,
							   const LADate & spotDate,
							   const std::string & curveCollection,		// Needed to support legacy isFwdInter = true
							   const std::string & curveIndex,			// Needed to support legacy isFwdInter = true
                               const InterpolationEnum & interpolationEnum,
                               const VectorDate & paymentDates,
                               const VectorDouble & discountFactors,
							   const DayCountEnum & daycount,
                               const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                               const StandardString & fixingCalendar,
                               const LADate & joinDate = LADate(),
							   const StandardStringMatrix & forwardAdjustments = StandardStringMatrix() );

		// Object Pool Constructor - Object Pool Stores Dates as Doubles representing Year Fractions
		// No Spot Date Required, but joinDateAsDouble instead
		DiscountFactorResults( const CurveTenorEnum & curveTenor,
							   const LADate & asOfDate,
							   const std::string & curveCollection,		// Needed to support legacy isFwdInter = true
							   const std::string & curveIndex,			// Needed to support legacy isFwdInter = true
							   const InterpolationEnum & interpolationEnum,
							   const VectorDouble & paymentDatesInTermFormat,
							   const VectorDouble & discountFactors,
							   const DayCountEnum & daycount,
                               const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                               const StandardString & fixingCalendar,
							   const double & joinDateAsDouble = 0.0,
							   const StandardStringMatrix & forwardAdjustments = StandardStringMatrix() );

		// Server Side (Non-Object Pool) Constructor - Dates entered as regular dates here instead of object pool doubles representing year fractions
		// No Spot Date Required, but joinDateAsDouble instead
		DiscountFactorResults( const CurveTenorEnum & curveTenor,
							   const LADate & asOfDate,
							   const std::string & curveCollection,		// Needed to support legacy isFwdInter = true
							   const std::string & curveIndex,			// Needed to support legacy isFwdInter = true
							   const InterpolationEnum & interpolationEnum,
							   const VectorDate & paymentDates,
							   const VectorDouble & discountFactors,
							   const DayCountEnum & daycount,
                               const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                               const StandardString & fixingCalendar,
							   const double & joinDateAsDouble,
							   const StandardStringMatrix & forwardAdjustments = StandardStringMatrix() );

        // ================================================ DISCOUNT FACTOR METHODS ================================================

		void setDiscountFactorsUsingTerms( const VectorDouble & paymentDatesInTermsFormat, const VectorDouble & discountFactors );

		void setDiscountFactors( const VectorDate & paymentDates, const VectorDouble & discountFactors );

        double getDiscountFactor( const double & paymentDateInTermFormat ) const;
        
        double getDiscountFactor( const LADate & paymentDate ) const;

        VectorDouble getDiscountFactors( const VectorDate & paymentDates ) const;
        
        VectorDouble getDiscountFactors( const VectorDouble & paymentDatesInTermFormat ) const;

        // ================================================ IMPLIED FORWARD RATE METHODS ================================================
        
        double implyForwardRate( const double & fromDateInTermFormat,
                                 const double & toDateInTermFormat,
								 //const bool isFwdInter = false, // Legacy parameter: interpolate on pseudo Forwards when using a discount factor state variable
                                 const bool isFwdInter,
								 const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING ) const;

        double implyForwardRate( const LADate & fromDate,
                                 const LADate & toDate,
								 //const bool isFwdInter = false, // Legacy parameter: interpolate on pseudo Forwards when using a discount factor state variable
                                 const bool isFwdInter,
								 const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING ) const;

        double implyForwardRate( const double & fixingDateInTermFormat,
                                 const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                 const StandardString & fixingCalendar,
								 //const bool isFwdInter = false, // Legacy parameter: interpolate on pseudo Forwards when using a discount factor state variable
                                 const bool isFwdInter,
								 const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING ) const;

        double implyForwardRate( const LADate & fixingDate,
                                 const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                 const StandardString & fixingCalendar,
								 //const bool isFwdInter = false, // Legacy parameter: interpolate on pseudo Forwards when using a discount factor state variable
                                 const bool isFwdInter,
								 const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING ) const;

        VectorDouble ImplyForwardRates( const VectorDouble & fromDatesInTermFormat,
                                        const VectorDouble & toDatesInTermFormat,
										//const bool isFwdInter = false, // Legacy parameter: interpolate on pseudo Forwards when using a discount factor state variable
                                        const bool isFwdInter,
										const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING ) const;

        VectorDouble implyForwardRates( const VectorDate & fromDates,
                                        const VectorDate & toDates,
										//const bool isFwdInter = false, // Legacy parameter: interpolate on pseudo Forwards when using a discount factor state variable
                                        const bool isFwdInter,
										const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING ) const;

        VectorDouble implyForwardRates( const VectorDouble & fixingDatesInTermsFormat,
                                        const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                        const StandardString & fixingCalendar,
										//const bool isFwdInter = false, // Legacy parameter: interpolate on pseudo Forwards when using a discount factor state variable
                                        const bool isFwdInter,
										const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING ) const;

        VectorDouble implyForwardRates( const VectorDate & fixingDates,
                                        const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                        const StandardString & fixingCalendar,
										//const bool isFwdInter = false, // Legacy parameter: interpolate on pseudo Forwards when using a discount factor state variable
                                        const bool isFwdInter,
										const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING ) const;

		// Method to imply the forward rates on the discount factor pillar points
		VectorDouble implyForwardRates( //const bool isFwdInter = false ) const; // Legacy parameter: interpolate on pseudo Forwards when using a discount factor state variable
										const bool isFwdInter ) const;

        // Accessors
        VectorDate paymentDates() const;
        VariantMatrix paymentDatesInTermsFormatAndDiscountFactors() const;
        VariantMatrix paymentDatesAndDiscountFactors() const;
        
        VectorDouble paymentDatesInTermFormat() const               { return paymentDatesInTermFormat_; }
        VectorDouble discountFactors() const                        { return discountFactors_;			}
        
        CurveTenorEnum curveTenorEnum() const                       { return curveTenorEnum_;           }
        StandardString curveTenor() const                           { return curveTenor_;               }
        LADate asOfDate() const                                     { return asOfDate_;                 }
        LADate spotDate() const                                     { return spotDate_;                 }
        LADate joinDate() const                                     { return joinDate_;                 }
        double joinDateAsDouble() const                             { return joinDateAsDouble_;         }
        std::string curveCollection() const							{ return curveCollection_;			}
	    std::string curveIndex() const								{ return curveIndex_;				}
		DayCountEnum daycount() const                               { return daycount_;                 }
        BusinessDayAdjustmentEnum fixingBusinessDayAdj() const      { return fixingBusinessDayAdj_;     }
        StandardString fixingCalendar() const                       { return fixingCalendar_;           }
		StandardStringMatrix forwardAdjustments() const				{ return forwardAdjustments_;       }

    private:
        
		// Constructor and Copy Constructor Helper Method to initialize the Interpolation Object
		// This helper method initializes the interpolator object once all member data has been set up.
		void initializeInterpolator();

		// Object Pool Method - Object Pool Stores Dates as Doubles representing Year Fractions
        // Object Pool - Helper Method to initialize the discount factor class
		// Note: Spot Date should be Curve Swap Calibration Instrument Spot Date - needed for the linear spline join date calculation
        void initializeDiscountFactorResults( const CurveTenorEnum & curveTenor,
                                              const LADate & asOfDate,
											  const LADate & spotDate,
											  const std::string & curveCollection,		// Needed to support legacy isFwdInter = true
											  const std::string & curveIndex,			// Needed to support legacy isFwdInter = true
											  const InterpolationEnum & interpolationEnum,
                                              const VectorDouble & paymentDatesInTermFormat,
                                              const VectorDouble & discountFactors,
											  const DayCountEnum & daycount,
                                              const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                              const StandardString & fixingCalendar,
                                              const LADate & joinDate = LADate(),
											  const StandardStringMatrix & forwardAdjustments = StandardStringMatrix() );
		
		// Server Side (Non-Object Pool) Method - Dates entered as regular dates here instead of object pool doubles representing year fractions
		// Helper Method to initialize the discount factor class
		// No Spot Date Required, but joinDateAsDouble instead
		void initializeDiscountFactorResults( const CurveTenorEnum & curveTenor,
											  const LADate & asOfDate,
											  const std::string & curveCollection,		// Needed to support legacy isFwdInter = true
											  const std::string & curveIndex,			// Needed to support legacy isFwdInter = true
											  const InterpolationEnum & interpolationEnum,
											  const VectorDouble & paymentDatesInTermFormat,
											  const VectorDouble & discountFactors,
											  const DayCountEnum & daycount,
                                              const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
                                              const StandardString & fixingCalendar,
											  const double & joinDateAsDouble = 0.0,
											  const StandardStringMatrix & forwardAdjustments = StandardStringMatrix() );

        CurveTenorEnum curveTenorEnum_; 
        StandardString curveTenor_;

        LADate asOfDate_;
		LADate spotDate_;
        LADate joinDate_;
		double joinDateAsDouble_;
		DayCountEnum daycount_;
        BusinessDayAdjustmentEnum fixingBusinessDayAdj_;
        StandardString fixingCalendar_;

		// Needed to support legacy isFwdInter = true
		std::string curveCollection_;
	    std::string curveIndex_;

		// Forward Adjustments: Spreads, Jumps and Turn-of-Year (TOY) Adjustments
		StandardStringMatrix forwardAdjustments_;

		// Stores the Interpolation Object used for Forward Adjustments and Turn-of-Year Features
		std::shared_ptr<CurveInterpolation> interpolator_;
		
		// Stores the Underlying Interpolation Object
		std::shared_ptr<LAInterpolationBase> baseInterpolationMethod_;	
        InterpolationEnum interpolationEnum_;

        VectorDouble paymentDatesInTermFormat_;   
        VectorDouble discountFactors_;
		
    };

}

