#pragma once

#include "AQLDate.h"
#include "CoreEnumerations.h"
#include "AQLCoreTemplateType.h"	// Include StandardString and AQLString Typedefs
#include "LabelValueBlock.h"
#include "CreditModel.h"

namespace etrading
{
	// HELPER METHODS
	// ----------------

	// Helper Struct to Valuation Settings Data & Validate if the Input String Matrix is LVB Compatible
	struct ValuationSettingsData
	{
		ValuationSettingsData( const StandardStringMatrix & inputs );
		
		StandardStringMatrix rawData_;
		size_t nRows_;
		size_t nCols_;

		bool isSingleValue_;
		bool isSingleRow_;
		bool isSingleCol_;
		bool isLVB_;
	};

	// VALUATION SETTINGS
	// -----------------------

	class ValuationSettings
	{
	public:

		// Simple Swap Case: User provides Curve Name, which can be a CurveHandle or a CurveCollection string
		ValuationSettings( const std::string & curveName, const bool allowCurveCollections = true );

		// General Swap Case: User provides ValuationSettings as a string matrix
		ValuationSettings( const StandardStringMatrix & inputMatrix, const bool allowCurveCollections = true );

		// Used by Swap,  legName is used in searching curveCollection, and the DEFAULT valuationDate is the found curveCollection's asOfDate
		ValuationSettings( const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const AQLString& legName, const bool& includeAccruedInterest = true);

		// Used by Bond
		ValuationSettings( const AQLDate& valuationDate, const std::string& curveCollection = std::string() );
		ValuationSettings( const LabelValueBlock& valuationSettingsLVB );

		// Used by credit model
		ValuationSettings( const CreditModel& creditModel, const LabelValueBlock& fixingTableNames );

		// Default Constructor
		ValuationSettings();
		~ValuationSettings() {};

		// Get Accessors
		double getFXSpot() const							{ return fxSpot_; };
		double getFXAsOfDateRate() const					{ return fxAsOfDateRate_; };
		AQLDate getValuationDate() const						{ return valuationDate_; };
		AQLDate getSettlementDate() const;
		double getFloatBondCurrentCouponRate() const;
		double getFloatBondAssumedRate() const;
		double getFloatBondIndexToNextCoupon() const;
		double getFloatBondQuotedMargin() const;

		std::string getCurveCollection() const				{ return curveCollection_; };
		std::string getCreditModelName() const				{ return creditModelName_; };
		std::string getVolatilityModelName() const			{ return volatilityModelName_; };
		bool getIncludeAccruedInterest() const				{ return includeAccruedInterest_; };
		ConvexityMethodEnum getConvexityMethod() const		{ return convexityMethod_; };
		LabelValueBlock getValuationSettingsLVB() const		{ return valuationSettingsLVB_; };
		
		std::string getFixingTableName(const std::string& legName, const ScheduleTypeEnum& legType) const;

		// Set Accessors
		void setSettlementDate( const AQLDate& settlementDate ) { settlementDate_ = settlementDate; };

		// Static Method to replace all curve object names with curve collections
		// inputData - An (N X 2) string matrix representing a raw valuation settings LVB
		static StandardStringMatrix convertDataFromObjectsToCurveCollections( const StandardStringMatrix & valuationSettings, const bool allowCurveCollections = true );

	private:

		// Helper Method(s)
		// --------------------------	
		
		// Private Method to Initialize Class Member Data to Default Values
		void initialize();

		// Private Method to Set-up a Single Curve
		void setSingleCurve( const std::string & curveName, const bool allowCurveCollections = true );

		// Private Method to Detect if a curveName string is a CurveHandle pointing to a curve object
		// or a simple string representing a curve collection
		bool isCurveHandle( const std::string & curveName ) const;

		// Member Data
		// --------------------------
		
		// Provide one curve handle for multi-curves or a vector of curve handles for single curves
		std::vector<std::string> curveHandles_;
		std::string curveCollection_;
		std::string creditModelName_;
		std::string volatilityModelName_;
		
		AQLDate valuationDate_;
		AQLDate settlementDate_;			// spot date, used for bonds
		
		bool includeAccruedInterest_;
		double fxSpot_;					
		double fxAsOfDateRate_;

		ConvexityMethodEnum convexityMethod_;
	
		LabelValueBlock fixingTableNames_;
		LabelValueBlock valuationSettingsLVB_;
	};

}
