// CurveConventionsAndMarketData.h

/*
 * @brief			Curve Conventions and Market Data
 * @Created:		13th November 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#pragma once
#include <vector>
#include <memory>

#include "LADate.h"
#include "CoreEnumerations.h"
#include "LAInterpolationBase.h"
#include "Variant.h"
#include "CurveObject.h"			// For curve object static data conventions and market data

namespace etrading
{
	// =============================== CURVE CONVENTIONS AND MARKET DATA CLASS ==========================================================

	// Class to Manage the Curve Conventions and also LA Curve Naming Convention(s)
	class CurveConventionsAndMarketData
	{

		public:
		CurveConventionsAndMarketData() {};
		~CurveConventionsAndMarketData() {};

		// Copy Constructor
		CurveConventionsAndMarketData( const CurveConventionsAndMarketData& rhs );

		// Clone
		std::shared_ptr<CurveConventionsAndMarketData> clone() const;

		// Assignment Operator
		CurveConventionsAndMarketData & operator=( const CurveConventionsAndMarketData & rhs );

		// Main Constructor(s)
		// ====================================

		// Global Curve Constructor including ARR Curves e.g. to incorporate ARR discounting
		CurveConventionsAndMarketData( const std::shared_ptr<ARRCurveObjectData> arrCurveData, 
									   const std::shared_ptr<OISCurveObjectData> oisCurveData,
									   const std::shared_ptr<SwapCurveObjectData> swapCurveData,
									   const std::shared_ptr<TenorBasisCurveObjectData> tenorBasisCurveData,
									   const std::shared_ptr<XccyBasisCurveObjectData> xccyBasisCurveData,
									   const std::shared_ptr<FwdConstantCurveObjectData> fxFwdConstantCurveData );

		// ARR Curve Constructor
		CurveConventionsAndMarketData( const std::shared_ptr<ARRCurveObjectData> arrCurveData );
		
		// OIS Curve Constructor
		CurveConventionsAndMarketData( const std::shared_ptr<OISCurveObjectData> oisCurveData );

		// Swap Curve Constructor
		CurveConventionsAndMarketData( const std::shared_ptr<SwapCurveObjectData> swapCurveData );

		// Tenor Basis Curve Constructor
		CurveConventionsAndMarketData( const std::shared_ptr<TenorBasisCurveObjectData> tenorBasisCurveData );

		// Xccy Basis Curve Constructor
		CurveConventionsAndMarketData( const std::shared_ptr<XccyBasisCurveObjectData> xccyBasisCurveData );

		// FX Forward Constant Curve Constructor
		CurveConventionsAndMarketData( const std::shared_ptr<FwdConstantCurveObjectData> fxFwdConstantCurveData );


		// Accessors
		CurveTypeEnum curveTypeEnum() const												{ return curveTypeEnum_; }

		std::shared_ptr<ARRCurveObjectData>             arrCurveData() const			{ return arrCurveData_; }
		std::shared_ptr<OISCurveObjectData>             oisCurveData() const			{ return oisCurveData_; }
		std::shared_ptr<SwapCurveObjectData>            swapCurveData() const			{ return swapCurveData_; }
		std::shared_ptr<TenorBasisCurveObjectData>      tenorBasisCurveData() const		{ return tenorBasisCurveData_; }
		std::shared_ptr<XccyBasisCurveObjectData>       xccyBasisCurveData() const		{ return xccyBasisCurveData_; }
		std::shared_ptr<FwdConstantCurveObjectData>     fxFwdConstantCurveData() const	{ return fxFwdConstantCurveData_; }


		private:
		
		CurveTypeEnum curveTypeEnum_;

		std::shared_ptr<ARRCurveObjectData>             arrCurveData_; 
		std::shared_ptr<OISCurveObjectData>             oisCurveData_;
		std::shared_ptr<SwapCurveObjectData>            swapCurveData_;
		std::shared_ptr<TenorBasisCurveObjectData>      tenorBasisCurveData_;
		std::shared_ptr<XccyBasisCurveObjectData>       xccyBasisCurveData_;
		std::shared_ptr<FwdConstantCurveObjectData>     fxFwdConstantCurveData_;

	};

}
