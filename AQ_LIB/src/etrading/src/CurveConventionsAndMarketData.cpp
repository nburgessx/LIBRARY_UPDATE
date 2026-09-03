// CurveConventionsAndMarketData.cpp

/*
 * @brief			Curve Conventions and Market Data
 * @Created:		13th November 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#include "CurveConventionsAndMarketData.h"
#include "ExceptionMacros.h"
#include "CurveValidation.h"			        // Convert Dates to Terms and vice versa
#include "LAMathInterpolationUtilities.h"		// Get Interpolation Values
#include "CurveUtilities.h"				        // DateFromTenor methods
#include "LACurvePricingObject.h"	            // Methods to get the curve daycount conventions
#include "LAEnumConversion.h"		            // Methods to convert enum values to legacy enums
#include "LADefinitions.h"                      // Needed for the delimiter curve index delimiter token "MULTI_STATIC_DATA_DELIMITER", which is typically a token

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

	 // =============================== CURVE CONVENTIONS AND MARKET DATA CLASS ==========================================================

	// Global Curve Constructor including ARR Curves e.g. to incorporate ARR discounting
	CurveConventionsAndMarketData::CurveConventionsAndMarketData( const std::shared_ptr<ARRCurveObjectData> arrCurveData,
																  const std::shared_ptr<OISCurveObjectData> oisCurveData,
																  const std::shared_ptr<SwapCurveObjectData> swapCurveData,
																  const std::shared_ptr<TenorBasisCurveObjectData> tenorBasisCurveData,
																  const std::shared_ptr<XccyBasisCurveObjectData> xccyBasisCurveData,
																  const std::shared_ptr<FwdConstantCurveObjectData> fxFwdConstantCurveData )
		:	curveTypeEnum_( GLOBAL_CURVETYPE ),
			arrCurveData_( arrCurveData ),
			oisCurveData_( oisCurveData ),
			swapCurveData_( swapCurveData ),
			tenorBasisCurveData_( tenorBasisCurveData ),
			xccyBasisCurveData_( xccyBasisCurveData ),
			fxFwdConstantCurveData_( fxFwdConstantCurveData )
	{
	}

	// ARR Curve Constructor
	CurveConventionsAndMarketData::CurveConventionsAndMarketData( const std::shared_ptr<ARRCurveObjectData> arrCurveData )
		:	curveTypeEnum_( etrading::ARR_CURVETYPE ), arrCurveData_( arrCurveData )
	{
	}

	// OIS Curve Constructor
	CurveConventionsAndMarketData::CurveConventionsAndMarketData( const std::shared_ptr<OISCurveObjectData> oisCurveData )
		:	curveTypeEnum_( etrading::OIS_CURVETYPE ), oisCurveData_( oisCurveData )
	{
	}
	
	// Swap Curve Constructor
	CurveConventionsAndMarketData::CurveConventionsAndMarketData( const std::shared_ptr<SwapCurveObjectData> swapCurveData )
		:	curveTypeEnum_( etrading::SWAP_CURVETYPE ), swapCurveData_( swapCurveData )
	{
	}

	// Tenor Basis Curve Constructor
	CurveConventionsAndMarketData::CurveConventionsAndMarketData( const std::shared_ptr<TenorBasisCurveObjectData> tenorBasisCurveData )
		:	curveTypeEnum_( etrading::TENORBASIS_CURVETYPE ), tenorBasisCurveData_( tenorBasisCurveData )
	{
	}

	// Xccy Basis Curve Constructor
	CurveConventionsAndMarketData::CurveConventionsAndMarketData( const std::shared_ptr<XccyBasisCurveObjectData> xccyBasisCurveData )
		:	curveTypeEnum_( etrading::XCCYBASIS_CURVETYPE ), xccyBasisCurveData_( xccyBasisCurveData )
	{
	}

	// FX Forward Constant Curve Constructor
	CurveConventionsAndMarketData::CurveConventionsAndMarketData( const std::shared_ptr<FwdConstantCurveObjectData> fxFwdConstantCurveData )
		:	curveTypeEnum_( etrading::FWDFXCONST_CURVETYPE ), fxFwdConstantCurveData_( fxFwdConstantCurveData )
	{
	}
	 
	 
	// Copy Constructor
	CurveConventionsAndMarketData::CurveConventionsAndMarketData( const CurveConventionsAndMarketData& rhs )
		:	curveTypeEnum_( rhs.curveTypeEnum_ ),
			arrCurveData_( rhs.arrCurveData_ ), 
			oisCurveData_( rhs.oisCurveData_),
			swapCurveData_( rhs.swapCurveData_ ),
			tenorBasisCurveData_( rhs.tenorBasisCurveData_ ),
			xccyBasisCurveData_( rhs.xccyBasisCurveData_ ),
			fxFwdConstantCurveData_( rhs.fxFwdConstantCurveData_ )
	{
	}

	// Clone
	std::shared_ptr<CurveConventionsAndMarketData> CurveConventionsAndMarketData::clone() const
	{
		return std::make_shared<CurveConventionsAndMarketData>( CurveConventionsAndMarketData( *this ) );
	}

	// Assignment Operator
	CurveConventionsAndMarketData & CurveConventionsAndMarketData::operator=( const CurveConventionsAndMarketData & rhs )
	{
		// For Exception Safety

		// 1. Make a temp copy
		CurveConventionsAndMarketData tempCopy( rhs );

		// 2. Swap Data Members with the temp copy
		std::swap( curveTypeEnum_,				tempCopy.curveTypeEnum_ );
		std::swap( arrCurveData_,				tempCopy.arrCurveData_ ); 
		std::swap( oisCurveData_,				tempCopy.oisCurveData_ );
		std::swap( swapCurveData_,				tempCopy.swapCurveData_ );
		std::swap( tenorBasisCurveData_,		tempCopy.tenorBasisCurveData_ );
		std::swap( xccyBasisCurveData_,			tempCopy.xccyBasisCurveData_ );
		std::swap( fxFwdConstantCurveData_,		tempCopy.fxFwdConstantCurveData_ );

		return *this;
	}

}
