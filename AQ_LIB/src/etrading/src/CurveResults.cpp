// CurveResults.cpp

/*
 * @brief			Curve Results Class
 * @Created:		3rd April 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#include "CurveResults.h"
#include "ExceptionMacros.h"
#include "LAMathInterpolationUtilities.h"		// Get Interpolation Values
#include "CurveValidation.h"			        // Convert Dates to Terms and vice versa
#include "CurveUtilities.h"				        // DateFromTenor methods
#include "LACurvePricingObject.h"	            // Methods to get the curve daycount conventions
#include "LAEnumConversion.h"		            // Methods to convert enum values to legacy enums

// Interpolation Methods
#include "LAMonotoneConvexInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAParabolicInterpolation.h"
#include "LALinearSplineInterpolation.h"
#include "LALinearMonotoneSplineInterpolation.h"
#include "LAConstrainedSplineInterpolation.h"
#include "LAStepInterpolation.h"
#include "LALinearInterpolation.h"


namespace etrading
{
    
    // ================================= HELPER METHOD(S) ===================================================================
    
	// Shared Helper Method to Create Discount Factor Results Object
    CurveDescriptionAndDiscountFactorResults curveDescriptionAndDiscountFactorResults( const LAStringMatrix & curveLVB,
																					   const LAStringMatrix & discFactorParameterLVB,
																					   const LAStringMatrix & discountFactors,
																					   const StandardStringMatrix & forwardAdjustments )
    {
        // Input Validation
        if ( !curveLVB.empty() && curveLVB[0].size() != 2 )
        {
            MLIB_THROW("Invalid Data: CurveLVB data cannot be empty and must have column size 2")
        }

        if ( !discFactorParameterLVB.empty() && discFactorParameterLVB[0].size() != 2 )
        {
            MLIB_THROW("Invalid Data: ParameterLVB data cannot be empty and must have column size 2")
        }

        if ( !discountFactors.empty() && discountFactors[0].size() != 2 )
        {
            MLIB_THROW("Invalid Data: DiscountFactorLVB data cannot be empty and must have column size 2")
        }

		if ( !forwardAdjustments.empty() )
        {
			MLIB_REQUIRE( forwardAdjustments[0].size() == 4, "Invalid Data: Forward adjustment tables must have 4 columns:- AdjustmentType, StartDate, EndDate, RateOrSpread ");
        }

        // Extract Curve LVB Values
        LabelValueBlock curveLVB_( curveLVB );
        const etrading::CurveTypeEnum curveTypeEnum                 = etrading::toCurveTypeEnum( curveLVB_.getCompulsoryValue( "CURVETYPE" ) );
        const etrading::CurveTenorEnum curveTenorEnum               = etrading::toCurveTenorEnum( curveLVB_.getCompulsoryValue( "CURVETENOR" ) );
        const std::string curveCollection                           = curveLVB_.getCompulsoryValue( "CURVECOLLECTION" );
        const std::string curveIndex                                = curveLVB_.getCompulsoryValue( "CURVEINDEX" );
        
        // Extract Parameter LVB Values
        LabelValueBlock parameterLVB_( discFactorParameterLVB );
        const LADate asOfDate                                       = parameterLVB_.getCompulsoryValueAsDate("ASOFDATE");
		const etrading::InterpolationEnum interpolationEnum         = etrading::toInterpolationEnum( parameterLVB_.getCompulsoryValue("INTERPOLATION") );
		const etrading::DayCountEnum daycountEnum				    = etrading::toDayCountEnum( parameterLVB_.getCompulsoryValue("DAYCOUNT") );
        const etrading::BusinessDayAdjustmentEnum fixingBusDayAdj   = etrading::toBusinessDayAdjustmentEnum( parameterLVB_.getCompulsoryValue("FIXINGBUSINESSDAYADJUSTMENT") );
        const std::string fixingCalendar                            = parameterLVB_.getCompulsoryValueAsString("FIXINGCALENDAR");

		LADate joinDate;
		LADate spotDate;
		
		// Join- and Spot Date is only required for Linear-Spline Interpolation to calculate 'joinDateAsDouble'
		if ( interpolationEnum == etrading::LINEARSPLINE_INTERPOLATION )
		{
			joinDate = parameterLVB_.getCompulsoryValueAsDate("JOINDATE");
			spotDate = parameterLVB_.getCompulsoryValueAsDate( "SPOTDATE" );
		}
		else
		{
			joinDate = parameterLVB_.getOptionalValueAsDate("JOINDATE", LADate() );
			spotDate = parameterLVB_.getOptionalValueAsDate( "SPOTDATE", LADate() );
		}

        // Extract Discount Factor Information
        std::vector<LADate> paymentDates_( discountFactors.size() );
        std::vector<double> discountFactors_( discountFactors.size() );
        etrading::populateDateValueVectorsFromStringMatrix( paymentDates_, discountFactors_, discountFactors );

        // Create Discount Factor Results Object
        // -------------------------------------------------------------------------------
        CurveDescriptionAndDiscountFactorResults curveDescriptionAndDiscountFactorResults;

        curveDescriptionAndDiscountFactorResults.curveDescription_      = std::shared_ptr<etrading::CurveDescription>( new etrading::CurveDescription( curveTypeEnum, curveTenorEnum, curveCollection, curveIndex  ) );
        curveDescriptionAndDiscountFactorResults.discountFactorsResults_= std::shared_ptr<etrading::DiscountFactorResults>( new etrading::DiscountFactorResults( curveTenorEnum, asOfDate, spotDate, curveCollection, curveIndex, interpolationEnum, paymentDates_, discountFactors_, daycountEnum, fixingBusDayAdj, fixingCalendar, joinDate, forwardAdjustments ) );
        
        return curveDescriptionAndDiscountFactorResults;
    }

    // ================================= CURVE RESULTS CLASS ================================================================

    // Copy Constructor
    CurveResults::CurveResults( const CurveResults& rhs ) 
        : curveGroup_(rhs.curveGroup_),
		  curveDescription_(rhs.curveDescription_),
		  curveConventionsAndMarketData_(rhs.curveConventionsAndMarketData_),
		  discountFactorResults_(rhs.discountFactorResults_),
		  jacobianResultsByDiscountFactor_(rhs.jacobianResultsByDiscountFactor_),
		  jacobianResultsByForwardRate_(rhs.jacobianResultsByForwardRate_),
		  jacobianResultsByCompoundRate_(rhs.jacobianResultsByCompoundRate_)
	{
	}

	// Clone
	std::shared_ptr<CurveResults> CurveResults::clone() const
	{
		return std::make_shared<CurveResults>( CurveResults( *this ) );
	}

    // Assignment Operator
    CurveResults & CurveResults::operator=( const CurveResults & rhs )
    {
		// For Performance 
		if ( &rhs == this )
		{
			return *this;

		}

        // For Exception Safety
            
        // 1. Make a temp copy
        CurveResults temp( rhs );

        // 2. Swap Data Members with the temp copy
		std::swap( curveGroup_,							temp.curveGroup_						);
		std::swap( curveDescription_,					temp.curveDescription_					);
		std::swap( curveConventionsAndMarketData_,		temp.curveConventionsAndMarketData_		);
        std::swap( discountFactorResults_,				temp.discountFactorResults_				);
        std::swap( jacobianResultsByDiscountFactor_,	temp.jacobianResultsByDiscountFactor_	);
		std::swap( jacobianResultsByForwardRate_,		temp.jacobianResultsByForwardRate_		);
		std::swap( jacobianResultsByCompoundRate_,		temp.jacobianResultsByCompoundRate_		);

        return *this;
    }

	// Curve Collection Object Constructor
	CurveResults::CurveResults( const std::shared_ptr<CurveGroup> & curveGroup )
		: curveGroup_(curveGroup)
    {
    }

    // Curve Info Constructor
    CurveResults::CurveResults( const std::shared_ptr<CurveDescription> & curveDescpription )
        : curveDescription_(curveDescpription)
    {
    }

    // Discount Factor Constructor
    CurveResults::CurveResults( const std::shared_ptr<CurveDescription> & curveDescpription,
								const std::shared_ptr<CurveConventionsAndMarketData> & curveConventionsAndMarketData,
                                const std::shared_ptr<DiscountFactorResults> & discountFactorResults )
        : curveDescription_(curveDescpription),
		  curveConventionsAndMarketData_( curveConventionsAndMarketData ),
		  discountFactorResults_(discountFactorResults)
    {
    }

    // Jacobian Constructor
    CurveResults::CurveResults( const std::shared_ptr<CurveDescription> & curveDescpription,
								const std::shared_ptr<CurveConventionsAndMarketData> & curveConventionsAndMarketData,
								const std::shared_ptr<DiscountFactorResults> & discountFactorResults,
								const std::shared_ptr<JacobianResults> & jacobianResultsByDiscountFactor,
								const std::shared_ptr<JacobianResults> & jacobianResultsByForwardRate,
								const std::shared_ptr<JacobianResults> & jacobianResultsByCompoundRate )
        : curveDescription_(curveDescpription),
		  curveConventionsAndMarketData_( curveConventionsAndMarketData ),
		  discountFactorResults_(discountFactorResults),
		  jacobianResultsByDiscountFactor_(jacobianResultsByDiscountFactor),
		  jacobianResultsByForwardRate_(jacobianResultsByForwardRate),
		  jacobianResultsByCompoundRate_(jacobianResultsByCompoundRate)
    {
    }


    // Accessors
	std::shared_ptr<CurveGroup> CurveResults::curveGroup() const
    { 
		MLIB_REQUIRE( curveGroup_ != nullptr, "CurveGroup does not exist" )
        return curveGroup_;
    }

    std::shared_ptr<CurveDescription> CurveResults::curveDescription() const
    { 
		MLIB_REQUIRE( curveDescription_ != nullptr, "Invalid Curve Results: CurveDescription does not exist" )
        return curveDescription_;
    }

	std::shared_ptr<CurveConventionsAndMarketData> CurveResults::curveConventionsAndMarketData() const
	{
		MLIB_REQUIRE( curveConventionsAndMarketData_ != nullptr, "Invalid Curve Results: CurveConventionsAndMarketData does not exist" )
			return curveConventionsAndMarketData_;
	}

    std::shared_ptr<DiscountFactorResults> CurveResults::discountFactorResults() const
    { 
        MLIB_REQUIRE( discountFactorResults_ != nullptr, "Invalid Curve Results: DiscountFactorResults do not exist" )
        return discountFactorResults_;
    }
    
    std::shared_ptr<JacobianResults> CurveResults::jacobianResultsByDiscountFactor() const
    {
        MLIB_REQUIRE( jacobianResultsByDiscountFactor_ != nullptr, "Invalid Curve Results: JacobianResultsByDiscountFactor does not exist" )
        return jacobianResultsByDiscountFactor_;
    }

	std::shared_ptr<JacobianResults> CurveResults::jacobianResultsByForwardRate() const
    {
        MLIB_REQUIRE( jacobianResultsByForwardRate_ != nullptr, "Invalid Curve Results: JacobianResultsByForwardRate does not exist" )
        return jacobianResultsByForwardRate_;
    }

	std::shared_ptr<JacobianResults> CurveResults::jacobianResultsByCompoundRate() const
    {
        MLIB_REQUIRE( jacobianResultsByCompoundRate_ != nullptr, "Invalid Curve Results: JacobianResultsByCompoundRate does not exist" )
        return jacobianResultsByCompoundRate_;
    }

}
