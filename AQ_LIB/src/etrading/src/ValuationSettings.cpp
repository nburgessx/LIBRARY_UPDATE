/*
* @brief			Class the defines the Cashflow DataProvider, which contains the dynamic cashflow data
*					such as discount factors and float rates
* @Created:		2 May 2018
* @Author:			Ian Castleton
* @Department:		Quant Research & Analytics
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/

#include "ValuationSettings.h"
#include "SettingsValidation.h"
#include "LWOUtilities.h"					// Undecorate Handle Method & Curve Object Accessors for Single, Dual and Globally Calibrate Curves
#include "CurveResultsContainer.h"			// CurveGroup Object Methods

namespace etrading
{
	// HELPER METHODS
	// ----------------

	// Helper Struct to Valuation Settings Data & Validate if the Input String Matrix is LVB Compatible
	ValuationSettingsData::ValuationSettingsData( const StandardStringMatrix & inputs )
	{
		MLIB_REQUIRE( !inputs.empty(),		"Invalid Valuation Settings: Input data is empty" )
		MLIB_REQUIRE( !inputs[0].empty(),	"Invalid Valuation Settings: Input data is empty" )

		rawData_			= inputs;
		nRows_				= inputs.size();
		nCols_				= inputs[0].size();

		isSingleValue_		= false;
		isSingleRow_		= false;
		isSingleCol_		= false;
		isLVB_				= false;

		// *** IMPORTANT *** Do not rearrange the cases, the logic here depends on the case order

		// Case 1: Single Value Case
		// -------------------
		if( nRows_ == 1 )
		{
			if ( nCols_ == 1 )
			{	
				isSingleValue_ = true;
				return;
			}
			else
			{
				isSingleRow_ = true;
				return;
			}
		}
		
		// Case 2: Here we only have a Column of Curve Handles (Single Curve Framework)
		// -------------------
		if( nCols_ == 1 )
		{
			isSingleValue_ = true;
			return;
		}

		// LVB Case
		// ---------------------

		MLIB_REQUIRE( nCols_ == 2, "Invalid Valuation Settings: LabelValueBlock data must have exactly 2 columns" )

		// Check Input Matrix is Rectangular
		size_t thisColSize = nCols_;
		for( size_t thisRow = 0; thisRow < nRows_; ++thisRow )
		{
			thisColSize = rawData_[thisRow].size();
			MLIB_REQUIRE( thisColSize == nCols_, "Invalid ValuationSettings: Only Rectangular Input Data is Permitted" )
		}

		isLVB_ = true;
		return;

	}
	
	// VALUATION SETTINGS
	// -----------------------

	// Simple Swap Case: User provides Curve Name, which can be a CurveGroup, CurveHandle or a CurveCollection string
	ValuationSettings::ValuationSettings( const std::string & curveName, const bool allowCurveCollections )
	{
		// Initialize Default Member Data
		initialize();

		// Set Single Curve
		setSingleCurve( curveName );
	}

	// General Swap Case: User provides ValuationSettings as a string matrix
	ValuationSettings::ValuationSettings( const StandardStringMatrix &  inputMatrix, const bool allowCurveCollections )
	{
		// Initialize Default Member Data
		initialize();

		// Set Member Data
		// ----------------

		// Firstly initialize and validate the valuation settings data Block, which also checks data dimensions
		ValuationSettingsData valuationSettings( inputMatrix );
		
		// Case 1: Single Value CurveName Provided
		if( valuationSettings.isSingleValue_ )
		{
			// Call Constructor taking single curveName as input
			const std::string curveName = inputMatrix[0][0];
			setSingleCurve( curveName, allowCurveCollections );
			return;
		}

		// Case 2: Single Column of CurveHandles Provided (Single Curve Framework)
		if( valuationSettings.isSingleCol_ )
		{
			// Check all the Curve Collections are the same - This is a single curve framework requirement
			const std::string curveCollection = valuationSettings.rawData_[0][0];
			for( size_t i = 0; i < valuationSettings.nCols_; ++i )
			{
				const std::string thisCurveCollection = getCurveCollectionFromHandle( inputMatrix[i][0], allowCurveCollections );
				MLIB_REQUIRE( thisCurveCollection == curveCollection, "Invalid Valuation Settings: single curve handle list must be part of the same curve collection and in the same currency" )
			}

			// Set Single Curve
			setSingleCurve( curveCollection, allowCurveCollections );
			return;
		}

		// Case 3: Label Value Block with a Single Row
		// Assume input is "CurveCollection", "CurveObject/CurveCollection"
		if( valuationSettings.isSingleRow_ )
		{
			// Legacy Special Case for Backwards Compatibility: Single Row with Value entered as the Key with the Value Blank
			// Get the Curve Collection from Column 0 if Column 1 is blank
			const size_t valueColumn = (inputMatrix[0][1] == "") ? 0 : 1;
			
			// Set Single Curve
			const std::string curveCollection = getCurveCollectionFromHandle( inputMatrix[0][valueColumn], allowCurveCollections );
			setSingleCurve( curveCollection, allowCurveCollections );
			return;
		}

		// Case 4: Label Value Block Input
		// This is for Xccy Pricing and more complex structures
		if( valuationSettings.isLVB_ )
		{
			// Update the ValuationSettingsLVB with the modified results, where curve handles are replaced by curve collections 
			valuationSettingsLVB_ = convertDataFromObjectsToCurveCollections( inputMatrix, allowCurveCollections );
			return;
		}

		// We should not get here
		MLIB_THROW( "Invalid Valuation Settings: Must be a CurveGroup, CurveHandle, CurveCollection or a LabelValueBlock (LVB)" )
	}

	//Used by Swap,  legName is used in searching curveCollection, and the DEFAULT valuationDate is the found curveCollection's asOfDate
	ValuationSettings::ValuationSettings( const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const LAString& legName, const bool& includeAccruedInterest )
	{
		// Initialize Default Member Data
		initialize();
		
		// Set Member Data
		// ----------------

		fixingTableNames_			= fixingTableNames;
		includeAccruedInterest_		= includeAccruedInterest;
		valuationSettingsLVB_		= valuationSettingsLVB;

		fxSpot_ = getFXSpotFromValuationSettings(valuationSettingsLVB, false);
		fxAsOfDateRate_ = getFXAsOfDateFromValuationSettings(valuationSettingsLVB, false);

		// Get credit model name 
		creditModelName_ = getCreditModelFromValuationSettings(valuationSettingsLVB, legName).getCString();

		if (creditModelName_.empty())
		{
			curveCollection_ = getLWOCurveCollectionFromValuationSettings(valuationSettingsLVB, legName).getCString();
		}
		else
		{
			// If there is credit model, get the curveCollection from credit model
			auto creditModelPtr = getCreditModel(creditModelName_, true);
			curveCollection_ = creditModelPtr->getCDSCurveCollection();
		}

		valuationDate_ = getValuationDateFromValuationSettings(valuationSettingsLVB, false);

		const LADate curveAsOfDate = getCurveAsOfDate(curveCollection_.c_str());

		if (valuationDate_ == LADate())
		{
			//Backward compatibility, if valuationDate is NOT specified in valuationSettingsLVB, use Curve's AsOfDate
			valuationDate_ = curveAsOfDate;
		}
		else
		{
			// Check Curve AsOf Date Matches the ValuationDate Specified in the Valuation Settings
			MLIB_REQUIRE(curveAsOfDate <= valuationDate_, "Invalid Curve Build Date: Curve build date cannot be greater than the valuation date in validation settings");
		}

		volatilityModelName_ = getVolatilityModelFromValuationSettings(valuationSettingsLVB, "", false).getCString();

		std::string convexityMethodStr = getKeyFromValuationSettings(VALUATION_SETTING_KEYS::CONVEXITY_METHOD, valuationSettingsLVB, "", false).getCString();
		convexityMethod_ = convexityMethodStr.empty() ? STANDARD_CONVEXITY : toConvexityMethodEnum(convexityMethodStr);

	}

	// *** TODO to use settlementDate for bonds
	//Used by Bond
	ValuationSettings::ValuationSettings(const LADate& valuationDate, const std::string& curveCollection )
	{
		// Initialize Default Member Data
		initialize();

		// Set Member Data
		valuationDate_			= valuationDate;
		curveCollection_ 		= curveCollection;
	}

	// Used by Bond
	ValuationSettings::ValuationSettings( const LabelValueBlock& valuationSettingsLVB )
	{
		// Initialize Default Member Data
		initialize();

		// Set Member Data
		valuationSettingsLVB_ 	= valuationSettingsLVB;
	}


	//Used by credit model
	ValuationSettings::ValuationSettings(const CreditModel& creditModel, const LabelValueBlock& fixingTableNames)
	{
		// Initialize Default Member Data
		initialize();

		// Set Member Data
		fxSpot_					= 1.0;
		fxAsOfDateRate_			= 1.0;
		valuationDate_			= creditModel.getAsOfDate();
		curveCollection_		= creditModel.getCDSCurveCollection();
		creditModelName_		= creditModel.getRefToName();
		fixingTableNames_		= fixingTableNames;
	}

	//Default Constructor
	ValuationSettings::ValuationSettings()
	{
		// Initialize Default Member Data
		initialize();
	}


	// Accessors
	// -------------------------------
	std::string ValuationSettings::getFixingTableName(const std::string& legName, const ScheduleTypeEnum& legType) const
	{
		return findFixingTableName(fixingTableNames_, legName, legType);

	}

	LADate ValuationSettings::getSettlementDate() const
	{
		// Return the override value, if present. For backwards compatibility.
		if ( settlementDate_ != LADate() )
		{
			return settlementDate_;
		}
	
		const LADate settlementDate = getSettlementDateFromValuationSettings( valuationSettingsLVB_, true ); // throw if missing
		return settlementDate;

	}

	double ValuationSettings::getFloatBondCurrentCouponRate() const
	{
		const double floatBondCurrentCouponRate = getFloatBondCurrentCouponRateFromValuationSettings( valuationSettingsLVB_, false ); // Set to NaN if missing
		return floatBondCurrentCouponRate;
	}

	double ValuationSettings::getFloatBondAssumedRate() const
	{
		const double assumedRate = getFloatBondAssumedRateFromValuationSettings( valuationSettingsLVB_, true ); // throw if missing
		return assumedRate;
	}

	double ValuationSettings::getFloatBondIndexToNextCoupon() const
	{
		const double indexToNextCoupon = getFloatBondIndexToNextCouponFromValuationSettings( valuationSettingsLVB_, true ); // throw if missing
		return indexToNextCoupon;
	}

	double ValuationSettings::getFloatBondQuotedMargin() const
	{
		const double quotedMargin = getFloatBondQuotedMarginFromValuationSettings( valuationSettingsLVB_, true ); // throw if missing
		return quotedMargin;
	}
	

	// Private Method to replace all curve object names with curve collections
	// inputData - A (N X 2) string matrix representing a raw valuation settings LVB
	StandardStringMatrix ValuationSettings::convertDataFromObjectsToCurveCollections( const StandardStringMatrix & valuationSettings, const bool allowCurveCollections )
	{
		MLIB_REQUIRE( !valuationSettings.empty(), "Invalid Valuation Settings: The valuation settings data is empty" )
		MLIB_REQUIRE( !valuationSettings[0].empty(), "Invalid Valuation Settings: The valuation settings data is empty" )
		MLIB_REQUIRE( valuationSettings[0].size() == 2, "Invalid Valuation Settings: Must have exactly 2 columns with a key and value" )

		StandardStringMatrix modifiedValuationSettings = valuationSettings;
		
		// Search Valuation Settings for "CurveCollection & "LegName:Fixed/Float/Fee" Keys if the corresponding value
		// is a handle replace it with the curve collection name
		// -------------------------------------------------------------------------------------------------------------

		const StandardString searchToken = ":";
		for ( size_t row = 0; row < modifiedValuationSettings.size(); ++row )
		{
			const StandardString thisKey = modifiedValuationSettings[row][0]; // Column 0
			const StandardString thisValue = modifiedValuationSettings[row][1]; // Column 1

			// a) Search for "CurveCollection" Key
			// --------------------------------------
			if( boost::iequals( thisKey, "CURVECOLLECTION" ) )
			{
				// Replace Corresponding Value with Curve Collection Name
				modifiedValuationSettings[row][1] = getCurveCollectionFromHandle( thisValue, allowCurveCollections );
				continue; // skip to next row
			}

			// b) "LegName:Fixed/Float/Fee" Key
			// --------------------------------------
			const size_t tokenPosition = thisKey.find_first_of( searchToken );

			bool foundToken = ( tokenPosition != std::string::npos );
			if( foundToken )
			{
				// Get Key Suffix - Note: substr(index) returns sub-string from index to end of string
				const StandardString keySuffix = thisKey.substr( tokenPosition+1 );
				if( boost::iequals( keySuffix, "FLOAT" ) || boost::iequals( keySuffix, "FIXED" ) || boost::iequals( keySuffix, "FEE" ) )
				{
					// Replace Corresponding Value with Curve Collection Name
					modifiedValuationSettings[row][1] = getCurveCollectionFromHandle( thisValue, allowCurveCollections ); // Column 1
				}
			}
		}

		return modifiedValuationSettings;
	}

	// Private Helper Method(s)
	// ----------------------------
	
	// Private Method to Initialize Class Member Data to Default Values
	void ValuationSettings::initialize()
	{
		curveHandles_					= std::vector<std::string>();
		
		curveCollection_				= "";
		creditModelName_				= "";
		volatilityModelName_			= "";
		
		valuationDate_					= LADate();
		settlementDate_					= LADate();		// spot date, used for bonds
		
		includeAccruedInterest_			= true;			// default as true
		fxSpot_					        = std::numeric_limits<double>::quiet_NaN();
		fxAsOfDateRate_						= std::numeric_limits<double>::quiet_NaN();

		convexityMethod_				= etrading::STANDARD_CONVEXITY; // TODO: Fix Bad Enum - This is the Linear Rate Model by Antoon Pelsser
		
		fixingTableNames_				= LabelValueBlock();
		valuationSettingsLVB_			= LabelValueBlock();
	}
	
	// Private Method to Set-up a Single Curve
	void ValuationSettings::setSingleCurve( const std::string & curveName, const bool allowCurveCollections )
	{
		// 1) Get Curve Collection from Curve Group Object, Curve Handle or Curve Collection String
		curveCollection_ = getCurveCollectionFromHandle( curveName, allowCurveCollections );
		
		// 2) Build the Valuation Settings Object
		const StandardStringVector		keys	= { "CURVECOLLECTION" };
		const StandardStringVector		values	= { curveCollection_  };
		
		// 3) Return the ValuationSettingsLVB result
		const etrading::LabelValueBlock valuationSettingsLVB( keys, values );
		valuationSettingsLVB_ =  valuationSettingsLVB;

		// 4) Set the Valuation Date
		valuationDate_ = LADate();
		if ( doesCurveExist( curveCollection_.c_str() ))
		{ 
			valuationDate_ = getCurveAsOfDate( curveCollection_.c_str() );
		}
	}

	// Private Method to Detect if a curveName string is a CurveHandle pointing to a curve object
	// or a simple string representing a curve collection
	bool ValuationSettings::isCurveHandle( const std::string & curveName ) const
	{
		// TODO / CONSIDER : What if we have the same curve handle used to create a Single and a Multi-Curve?
		
		// This will never occur in Excel when 'Sticky Handles' the default behaviour are enabled,
		// where handles names have an Excel Address Hash appended to the handle name as a suffix.
		
		// Note: In the code below the single curve dominates.
		
		bool isCurveHandle = false;
		const std::string curveHandleWithoutInstanceCounter = undecorateHandle( curveName );

		// a) Single Curve
		// --------------------------
		const std::shared_ptr<etrading::SingleCurveObject> singleCurveObject = etrading::getSingleCurveObject( curveHandleWithoutInstanceCounter, false ); // false = throwIfMissing
		if ( singleCurveObject != nullptr )
		{
			isCurveHandle = true;
			return isCurveHandle;
		}

		// b) Multi Curve
		// --------------------------
		const std::shared_ptr<etrading::MultiCurveObject> multiCurveObject = etrading::getMultiCurveObject( curveHandleWithoutInstanceCounter, false ); // false = throwIfMissing
		if ( multiCurveObject != nullptr )
		{
			isCurveHandle = true;
			return isCurveHandle;
		}

		// c) Dual Curve
		// --------------------------
		const std::shared_ptr<etrading::DualBootstrappedCurveObject> dualCurveObject = etrading::getDualCurveObject( curveHandleWithoutInstanceCounter, false ); // false = throwIfMissing
		if ( dualCurveObject != nullptr )
		{
			isCurveHandle = true;
			return isCurveHandle;
		}

		return isCurveHandle;
	}

	
}

