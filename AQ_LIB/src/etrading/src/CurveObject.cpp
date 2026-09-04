#include "CurveObject.h"
#include "ExceptionMacros.h"
#include "Variant.h"
#include "LabelValueBlock.h"
#include "CurveUtilities.h"
#include <vector>
#include <string>


namespace
{
	/* @brief	Used for XCCY and CONST FXFWD curves:
	*			Searches the given convention matrix and replaces AGAINST / TARGET strings with the supplied collection names.
	*			This allows the same curve convention to be used in multiple collections.
	* param[in]	conventions				The input conventions key-value pair
	* param[in]	domesticCurveCollection	The collection which will contain the curve we are building
	* param[in] foreignCurveCollection	The foreign-currency collection, containing foreign curve dependencies
	*/
	etrading::LabelValueBlock modifyConventionsAndApplyCurveCollections( AQLStringMatrix& conventions, const std::string& domesticCurveCollection, const std::string& foreignCurveCollection )
	{
		size_t numRows = conventions.size();
		for (size_t i = 0; i < numRows; i++)
		{
			AQLString key = conventions[i][0];
			key.toUpper();
			if ( key.findString( "FORECAST" ) != -1 || key.findString( "DISCOUNT" ) != -1 )
			{
				AQLString& value = conventions[i][1];
				if ( foreignCurveCollection.empty() )
				{
					value.exchange( "AGAINST:", "" );
				}
				else
				{
					value.exchange( "AGAINST", foreignCurveCollection.c_str() );
				}

				if ( domesticCurveCollection.empty() )
				{
					value.exchange( "TARGET:", "" );
				}
				else
				{
					value.exchange( "TARGET",  domesticCurveCollection.c_str() );
				}
			}
		}

        etrading::LabelValueBlock conventionsLVB( conventions );
        return conventionsLVB;
	}

	/* @brief	Build the Curve Convention LVB appending the 'AsOfDate' to the CurveProperties String Matrix Block
	*  @param[out]		curveConv			The Curve Convention block
	*  @param[in]		aqoCurveGenerator	Curve generator object
	*  @param[in]		marketDataAsOfDate	As of date
	*/
	void getCurveConventionBlock( etrading::LabelValueBlock& curveConvLVB, const etrading::CurveGeneratorPtr& aqoCurveGenerator, const std::string& marketDataAsOfDate)
	{
		AQLStringMatrix curveProperties = aqoCurveGenerator->toAQLStringMatrix( "CURVEPROPERTIES" );

		// Push the MarketData AsOfDate into the CurveConventions, as required by the underlying curve calibration routines.
		AQLString asOfDateKey( "AsOfDate" );
        AQLString asOfDateValue( marketDataAsOfDate.c_str() );
		
        curveConvLVB = etrading::LabelValueBlock( curveProperties, asOfDateKey, asOfDateValue );
	}
}


namespace etrading
{
	// String constants used only in this file
	const std::string CURVE_BUILD_SCHEMA_NAME           = "CURVEBUILDPARAMETERS";
	const std::string DATA_NAMES                        = "AttributeNames";
	const std::string DATA_VALUES                       = "AttributeValues";
	const std::string OBJECT_NAME                       = "ObjectName";
	const std::string CURVE_GENERATOR_NAME              = "CurveGeneratorName";
	const std::string CURVE_MARKET_CALIBRATION_DATAAME  = "CurveMarketDataName";
	const std::string DOMESTIC_CURVE_COLLECTION         = "DomesticCurveCollection";
	const std::string FOREIGN_CURVE_COLLECTION          = "ForeignCurveCollection";
	const std::string CURVE_COUNT                       = "CurveCount";

	/* @brief Constructor used by deserialization
	* @param[in] objectName    The name of this CurveObject instance
	* @param[in] freeObject    A freeObject constructed from the serialized data
	*/
	CurveObject::CurveObject( const std::string& objectName) : IsAQObject(objectName, CURVE)
	{
    }

	/* @brief Copy Constructor
	 */
	CurveObject::CurveObject(const CurveObject& rhs) 
		: IsAQObject(rhs.getRefToName(), CURVE)
	{
		// The copy constructor does not need to invoke CurveObject::calibrateCurve() :
		// The curve will have already been built by the rhs object.
	}


	//-------------------------------------------------------------------------------------------------//
	//-------------------------------------------------------------------------------------------------//

	/* @brief Constructor of OIS curve data
	 */
	OISCurveObjectData::OISCurveObjectData(const CurveGeneratorPtr& aqoCurveGenerator, const CurveMarketDataPtr& aqoCurveMarketData, const std::string& marketDataAsOfDate, bool useSwaps, const std::string& curveNameIn, const std::string& curveIndexIn)
	{
		getCurveConventionBlock(curveConvLVB_, aqoCurveGenerator, marketDataAsOfDate);

		curveName_       = curveNameIn.c_str();
		curveIndex_      = curveIndexIn.c_str();

		oisRates_        = aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_OIS );
		oisConvLVB_      = LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix(  GENERATOR_COMPONENTS::KEY_OIS ) );
		
        // Optional: OIS FIXINGS
        // ----------------------
        histRates_       = aqoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_OISFIXINGS ) ? etrading::retrieveFixingTableMatrix(aqoCurveMarketData->toAQLStringMatrix(GENERATOR_COMPONENTS::KEY_OISFIXINGS)) : AQLStringMatrix(0);

		loBasisRates_    = aqoCurveMarketData->doesKeyExist(GENERATOR_COMPONENTS::KEY_LIBOROISBASISSPREADS) ? aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_LIBOROISBASISSPREADS ) : AQLStringMatrix(0);
		loBasisConvLVB_  = aqoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_LIBOROISBASIS) ? LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_LIBOROISBASIS ) ) : LabelValueBlock();
		swapConvLVB_     = aqoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_SWAPS) ? LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_SWAPS ) ): LabelValueBlock();
		
		forwardAdjustments_ = aqoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS) ? aqoCurveGenerator->toStandardStringMatrix( GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS ) : StandardStringMatrix();

		// When used in dual-bootstrapping context, we do not extract Libor swap par rates from the OIS curve
		if (useSwaps)
		{
            // Libor Swap Rates on an OIS Curve are Optional and may not exist
            swapRates_ = aqoCurveMarketData->doesKeyExist(GENERATOR_COMPONENTS::KEY_SWAPS) ? aqoCurveMarketData->toAQLStringMatrix(  GENERATOR_COMPONENTS::KEY_SWAPS ) : AQLStringMatrix(0);
		}

        // Optional Market Data Bump / Shift Parameter
        shiftSize_ = 0.0;
	}

    // Function to apply a flat-shift to outright instruments only
    void OISCurveObjectData::applyFlatShift( const double & shiftSize )
    {
        // Check if bump already applied - don't allow multiple bumps, otherwise we have no way to restore the data, if required.
        AQ_REQUIRE( AQ_IS_EQUAL_ZERO( shiftSize_ ), "Unable to apply Flat-Shift to OIS Market Data: The market data has a shift applied to it already" )
        shiftSize_ = shiftSize;

        // Flat-Shift Bumps should bump outright instruments only
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( oisRates_, shiftSize_, OIS_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( swapRates_, shiftSize_, SWAP_MARKETDATA );
    }

    // Function to remove a flat-shift a clear an existing flat-shift
    void OISCurveObjectData::restoreDataAndClearFlatShift()
    {
        if ( AQ_IS_EQUAL_ZERO( shiftSize_ ) )
        {
            // Do nothing if the shift-size applied was zero
            return;
        }

        // Remove Flat-Shifts
        // Flat-Shift Bumps should bump outright instruments only
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( oisRates_, -shiftSize_, OIS_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( swapRates_, -shiftSize_, SWAP_MARKETDATA );

        // Restore ShiftSize Parameter
        shiftSize_ = 0.0;
    }

	// Function to perturb the nth market data instrument
	void OISCurveObjectData::perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize )
	{
		shiftSize_				= shiftSize;
		riskInstrumentType_		= riskInstrumentType;
		nthRiskInstrument_		= nthRiskInstrument;

        // Perturb Instrument
		switch( riskInstrumentType_ )
		{
			case OIS_MARKETDATA:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( oisRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case SWAP_MARKETDATA:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( swapRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case LIBOR_OIS_BASISSPREAD_MARKETDATA:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( loBasisRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			default:
				AQ_THROW("Unable to Pertrub OIS Curve Market Data: Only OIS, LIBOROISBASISSPREADS or SWAPS Instruments can be Perturbed.")
		}
	}

	// Function to restore market data and clear perturbation shifts
	void OISCurveObjectData::restoreDataAndClearPerturbation()
	{
		if ( AQ_IS_EQUAL_ZERO( shiftSize_ ) )
        {
            // Do nothing if the shift-size applied was zero
            return;
        }

        // Remove Perturbation Shifts
        perturb( riskInstrumentType_, nthRiskInstrument_, -shiftSize_ ); 
		
		// Reset Perturbation Parameters
		shiftSize_				= 0.0;
		riskInstrumentType_		= NONE_MARKETDATA;
		nthRiskInstrument_		= 0u;
	}

	// Function to get the peturbed instrument tenors
	StandardStringVector OISCurveObjectData::instrumentTenors( const CurveMarketDataEnum & riskInstrumentType ) const
	{
		switch( riskInstrumentType )
		{
			case OIS_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( oisRates_, riskInstrumentType );
				break;
			case SWAP_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( swapRates_, riskInstrumentType );
				break;
			case LIBOR_OIS_BASISSPREAD_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( loBasisRates_, riskInstrumentType );
				break;
			default:
				AQ_THROW("Unable to Determine OIS Tenors from Curve Market Data: Only OIS, LIBOROISBASISSPREADS or SWAPS Instrument Market Data is permitted.")
		}
	}

	// Function to identify which instruments are outright instruments to be included in risk totals
	std::vector<bool> OISCurveObjectData::isOutrightInstrumentVector( const CurveMarketDataEnum & riskInstrumentType ) const
	{
		switch( riskInstrumentType )
		{
			case OIS_MARKETDATA:
				return std::vector<bool>( oisRates_.size(), true );
				break;
			case SWAP_MARKETDATA:
				return std::vector<bool>( swapRates_.size(), true );
				break;
			case LIBOR_OIS_BASISSPREAD_MARKETDATA:
				return std::vector<bool>( loBasisRates_.size(), false );
				break;
			default:
				AQ_THROW("Unable to Determine if OIS Outright Instruments from Curve Market Data: Only OIS, LIBOROISBASISSPREADS or SWAPS Instrument Market Data is permitted.")
		}
	}

	/* @brief Constructor of ARR curve data
	*/
    ARRCurveObjectData::ARRCurveObjectData(const CurveGeneratorPtr& aqoCurveGenerator, const CurveMarketDataPtr& aqoCurveMarketData, const std::string& marketDataAsOfDate, bool useSwaps, const std::string& curveNameIn, const std::string& curveIndexIn)
    {
		getCurveConventionBlock( curveConvLVB_, aqoCurveGenerator, marketDataAsOfDate );

		curveName_      = curveNameIn.c_str();
		curveIndex_     = curveIndexIn.c_str();

		oisRates_       = aqoCurveMarketData->toAQLStringMatrix(GENERATOR_COMPONENTS::KEY_OIS);
		auto oisConv     = aqoCurveGenerator->toAQLStringMatrix(GENERATOR_COMPONENTS::KEY_OIS) ;

		// Optional: LinearSplineJoinDate
		const LabelValueBlock marketDataPropertiesLVB = aqoCurveMarketData->toAQLStringMatrix(GENERATOR_COMPONENTS::KEY_MARKETDATAPROPERTIES);
		const AQLString joinDateStr = marketDataPropertiesLVB.getOptionalValueAsAQLString(CURVEGENERATOR_CURVEPROPERTIES_KEY::INTERPOLATION_JOIN_DATE);

		if (joinDateStr.size() > 0)
		{
			// Push the MarketData LinearSplineJoinDate into the CurveConventions, as required by the underlying curve calibration routines.
			AQLStringVector joinDateKeyValue;
			joinDateKeyValue.push_back(CURVEGENERATOR_CURVEPROPERTIES_KEY::INTERPOLATION_JOIN_DATE);
			joinDateKeyValue.push_back(joinDateStr);
			oisConv.push_back(joinDateKeyValue);
		}
		oisConvLVB_ = LabelValueBlock(oisConv);


		// Optional: OIS FIXINGS
		// ----------------------
		histRates_ = aqoCurveMarketData->doesKeyExist(GENERATOR_COMPONENTS::KEY_OISFIXINGS) ? etrading::retrieveFixingTableMatrix(aqoCurveMarketData->toAQLStringMatrix(GENERATOR_COMPONENTS::KEY_OISFIXINGS)) : AQLStringMatrix(0);

		loBasisRates_   = aqoCurveMarketData->doesKeyExist(GENERATOR_COMPONENTS::KEY_LIBOROISBASISSPREADS) ? aqoCurveMarketData->toAQLStringMatrix(GENERATOR_COMPONENTS::KEY_LIBOROISBASISSPREADS) : AQLStringMatrix(0);
		loBasisConvLVB_ = aqoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_LIBOROISBASIS) ? LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix(GENERATOR_COMPONENTS::KEY_LIBOROISBASIS) ) : LabelValueBlock();
		swapConvLVB_    = aqoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_SWAPS) ? LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix(GENERATOR_COMPONENTS::KEY_SWAPS) ) : LabelValueBlock();

		forwardAdjustments_ = aqoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS) ? aqoCurveGenerator->toStandardStringMatrix( GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS ) : StandardStringMatrix();

		// When used in dual-bootstrapping context, we do not extract Libor swap par rates from the OIS curve
		if (useSwaps)
		{
			// Libor Swap Rates on an OIS Curve are Optional and may not exist
			swapRates_ = aqoCurveMarketData->doesKeyExist(GENERATOR_COMPONENTS::KEY_SWAPS) ? aqoCurveMarketData->toAQLStringMatrix(GENERATOR_COMPONENTS::KEY_SWAPS) : AQLStringMatrix(0);
		}

        // Optional Market Data Bump / Shift Parameter
        shiftSize_ = 0.0;
	}

    // Function to apply a flat-shift to outright instruments only
    void ARRCurveObjectData::applyFlatShift( const double & shiftSize )
    {
        // Check if bump already applied - don't allow multiple bumps, otherwise we have no way to restore the data, if required.
        AQ_REQUIRE( AQ_IS_EQUAL_ZERO( shiftSize_ ), "Unable to apply Flat-Shift to ARR Curve Market Data: The market data has a shift applied to it already" )
        shiftSize_ = shiftSize;

        // Flat-Shift Bumps should bump outright instruments only
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( oisRates_, shiftSize_, OIS_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( swapRates_, shiftSize_, SWAP_MARKETDATA );
    }

    // Function to remove a flat-shift a clear an existing flat-shift
    void ARRCurveObjectData::restoreDataAndClearFlatShift()
    {
        if ( AQ_IS_EQUAL_ZERO( shiftSize_ ) )
        {
            // Do nothing if the shift-size applied was zero
            return;
        }

        // Remove Flat-Shifts
        // Flat-Shift Bumps should bump outright instruments only
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( oisRates_, -shiftSize_, OIS_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( swapRates_, -shiftSize_, SWAP_MARKETDATA );

        // Restore ShiftSize Parameter
        shiftSize_ = 0.0;
    }

	// Function to perturb the nth market data instrument
	void ARRCurveObjectData::perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize )
	{
		shiftSize_				= shiftSize;
		riskInstrumentType_		= riskInstrumentType;
		nthRiskInstrument_		= nthRiskInstrument;

        // Perturb Instrument
		switch( riskInstrumentType_ )
		{
			case OIS_MARKETDATA:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( oisRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case SWAP_MARKETDATA:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( swapRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case LIBOR_OIS_BASISSPREAD_MARKETDATA:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( loBasisRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			default:
				AQ_THROW("Unable to Pertrub ARR Curve Market Data: Only OIS, LIBOROISBASISSPREADS or SWAPS Instruments can be Perturbed.")
		}
	}

	// Function to restore market data and clear perturbation shifts
	void ARRCurveObjectData::restoreDataAndClearPerturbation()
	{
		if ( AQ_IS_EQUAL_ZERO( shiftSize_ ) )
        {
            // Do nothing if the shift-size applied was zero
            return;
        }
        
        // Remove Perturbation Shifts
		perturb( riskInstrumentType_, nthRiskInstrument_, -shiftSize_ );

		// Reset Perturbation Parameters
		shiftSize_				= 0.0;
		riskInstrumentType_		= NONE_MARKETDATA;
		nthRiskInstrument_		= 0u;
	}

	// Function to get the peturbed instrument tenors
	StandardStringVector ARRCurveObjectData::instrumentTenors( const CurveMarketDataEnum & riskInstrumentType ) const
	{
		switch( riskInstrumentType )
		{
			case OIS_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( oisRates_, riskInstrumentType );
				break;
			case SWAP_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( swapRates_, riskInstrumentType );
				break;
			case LIBOR_OIS_BASISSPREAD_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( loBasisRates_, riskInstrumentType );
				break;
			default:
				AQ_THROW("Unable to Determine ARR Tenors from Curve Market Data: Only OIS, LIBOROISBASISSPREADS or SWAPS Instrument Market Data is permitted.")
		}
	}

	// Function to identify which instruments are outright instruments to be included in risk totals
	std::vector<bool> ARRCurveObjectData::isOutrightInstrumentVector( const CurveMarketDataEnum & riskInstrumentType ) const
	{
		switch( riskInstrumentType )
		{
			case OIS_MARKETDATA:
				return std::vector<bool>( oisRates_.size(), true );
				break;
			case SWAP_MARKETDATA:
				return std::vector<bool>( swapRates_.size(), true );
				break;
			case LIBOR_OIS_BASISSPREAD_MARKETDATA:
				return std::vector<bool>( loBasisRates_.size(), false );
				break;
			default:
				AQ_THROW("Unable to Determine if ARR Outright Instruments from Curve Market Data: Only OIS, LIBOROISBASISSPREADS or SWAPS Instrument Market Data is permitted.")
		}
	}

	/* @brief Constructor of swap curve data
	 */
	SwapCurveObjectData::SwapCurveObjectData(const CurveGeneratorPtr& aqoCurveGenerator, const CurveMarketDataPtr& aqoCurveMarketData, const std::string& frequency, const std::string& marketDataAsOfDate, const std::string& curveNameIn, const std::string& curveIndexIn)
	{
		getCurveConventionBlock(curveConvLVB_, aqoCurveGenerator, marketDataAsOfDate);

		curveName_          = curveNameIn.c_str();
		curveIndex_         = curveIndexIn.c_str();

		moneyMarketConvLVB_ = LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix(  GENERATOR_COMPONENTS::KEY_MONEYMARKET ) );
		liborRates_         = aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_LIBORFIXINGS );
		liborConvLVB_       = LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix(  GENERATOR_COMPONENTS::KEY_LIBORFIXINGS ) );
		swapRates_          = aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_SWAPS );
		swapConvLVB_        = LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix(  GENERATOR_COMPONENTS::KEY_SWAPS ) );
		
		forwardAdjustments_ = aqoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS) ? aqoCurveGenerator->toStandardStringMatrix( GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS ) : StandardStringMatrix();

        // Optional: FRAS
        // -----------------
        fraRates_           = aqoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_FRAS ) ? aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_FRAS ) : AQLStringMatrix(0);
        fraConvLVB_         = aqoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_FRAS ) ? LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_FRAS ) ) : LabelValueBlock();

		// If FRA market data is provided, determine the FRA tenor (3M or 6M)
		if (fraRates_.size() > 0)
		{
			etrading::FrequencyEnum frequencyEnum = etrading::toFrequencyEnum( frequency );
			switch (frequencyEnum)
			{
				case etrading::SEMI_ANNUAL_FREQUENCY:
					fra6mRates_ = fraRates_;
					break;

				case etrading::QUARTERLY_FREQUENCY:
					fra3mRates_ = fraRates_;
					break;

				default:
					throw AQLCoreInvalidData( ( boost::format( "#Error: Invalid FRA Data: Only 3M and 6M FRAs are supported. Current FRA frequency: %s." )  
							% frequency ).str().c_str(), __FILE__, __LINE__ );
			}
		}

        // Optional: FUTURES
        // -----------------
        futureRates_        = aqoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_FUTURES ) ? aqoCurveMarketData->toAQLStringMatrix(GENERATOR_COMPONENTS::KEY_FUTURES ) : AQLStringMatrix(0);
        futureConvLVB_      = aqoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_FUTURES ) ? LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_FUTURES ) ) : LabelValueBlock();

		// Optional Fields - Tenor Basis Instruments
        // -----------------------------------------
        basisAdjConvLVB_    = aqoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_BASISSWAPS) ? LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_BASISSWAPS ) ) : LabelValueBlock();
        basisAdjRates_      = aqoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_BASISSWAPS) ? aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_BASISSWAPS ) : AQLStringMatrix(0);

        // Optional Market Data Bump / Shift Parameter
        shiftSize_ = 0.0;
	}

    // Function to apply a flat-shift to outright instruments only
    void SwapCurveObjectData::applyFlatShift( const double & shiftSize )
    {
        // Check if bump already applied - don't allow multiple bumps, otherwise we have no way to restore the data, if required.
        AQ_REQUIRE( AQ_IS_EQUAL_ZERO( shiftSize_ ), "Unable to apply Flat-Shift to Swap Curve Market Data: The market data has a shift applied to it already" )
        shiftSize_ = shiftSize;
        
        // Flat-Shift Bumps should bump outright instruments only
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( liborRates_,     shiftSize_,     LIBOR_FIXING_TABLE );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( futureRates_,    shiftSize_,     FUTURES_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( fraRates_,       shiftSize_,     FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( fra3mRates_,     shiftSize_,     FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( fra6mRates_,     shiftSize_,     FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( swapRates_,      shiftSize_,     SWAP_MARKETDATA );
    }

    // Function to remove a flat-shift a clear an existing flat-shift
    void SwapCurveObjectData::restoreDataAndClearFlatShift()
    {
        if ( AQ_IS_EQUAL_ZERO( shiftSize_ ) )
        {
            // Do nothing if the shift-size applied was zero
            return;
        }

        // Remove Flat-Shifts
        // Flat-Shift Bumps should bump outright instruments only
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( liborRates_,     -shiftSize_,      LIBOR_FIXING_TABLE );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( futureRates_,    -shiftSize_,      FUTURES_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( fraRates_,       -shiftSize_,      FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( fra3mRates_,     -shiftSize_,      FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( fra6mRates_,     -shiftSize_,      FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( swapRates_,      -shiftSize_,      SWAP_MARKETDATA );

        // Restore ShiftSize Parameter
        shiftSize_ = 0.0;
    }

	// Function to perturb the nth market data instrument
	void SwapCurveObjectData::perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize )
	{
		shiftSize_				= shiftSize;
		riskInstrumentType_		= riskInstrumentType;
		nthRiskInstrument_		= nthRiskInstrument;

        // Perturb Instrument
		switch( riskInstrumentType_ )
		{
			case LIBOR_FIXING_TABLE:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( liborRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case FUTURES_MARKETDATA:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( futureRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case FRA_MARKETDATA:
			{
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( fraRates_,   nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				
				const bool is3MFra = fra3mRates_.size() == 0 ? false : true;
				const bool is6MFra = fra6mRates_.size() == 0 ? false : true;
				AQ_REQUIRE( !is3MFra || !is6MFra, "Invalid Market Data: Cannot calibrate to 3M and 6M FRA Instruments within the same Swap Curve" )

				if ( is3MFra )
				{
					CurveMarketData::perturbMarketDataFromAQLStringMatrix( fra3mRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				}
				
				if ( is6MFra )
				{
					CurveMarketData::perturbMarketDataFromAQLStringMatrix( fra6mRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				}

				break;
			}
			case SWAP_MARKETDATA:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( swapRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case BASIS_SWAP_MARKETDATA:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( basisAdjRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			default:
				AQ_THROW("Unable to Pertrub Swap Curve Market Data: Only LIBORFIXINGS, FUTURES, FRAS, SWAPS or BASISSWAPS Instruments can be Perturbed.")
		}
	}

	// Function to restore market data and clear perturbation shifts
	void SwapCurveObjectData::restoreDataAndClearPerturbation()
	{
		if ( AQ_IS_EQUAL_ZERO( shiftSize_ ) )
        {
            // Do nothing if the shift-size applied was zero
            return;
        }

		// Remove Perturbation Shifts - Note we can't apply a zero shift, since we use that value to track bump state and check if a perturbation is already applied
		perturb( riskInstrumentType_, nthRiskInstrument_, -shiftSize_ );

        // Reset Perturbation Parameters
        shiftSize_				= 0.0;
		riskInstrumentType_		= NONE_MARKETDATA;
		nthRiskInstrument_		= 0u;
	}

	// Function to get the peturbed instrument tenors
	StandardStringVector SwapCurveObjectData::instrumentTenors( const CurveMarketDataEnum & riskInstrumentType ) const
	{
		switch( riskInstrumentType )
		{
			case LIBOR_FIXING_TABLE:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( liborRates_, riskInstrumentType );
				break;
			case FUTURES_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( futureRates_, riskInstrumentType );
				break;
			case FRA_MARKETDATA:
			{
				const bool is3MFra = fra3mRates_.size() == 0 ? false : true;
				const bool is6MFra = fra6mRates_.size() == 0 ? false : true;
				AQ_REQUIRE( !is3MFra || !is6MFra, "Invalid Market Data: Cannot calibrate to 3M and 6M FRA Instruments within the same Swap Curve" )
				
				if ( is3MFra )
				{
					return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( fra3mRates_, riskInstrumentType );
				}
				else if ( is6MFra )
				{
					return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( fra6mRates_, riskInstrumentType );
				}
				else
				{
					// Null Vector
					return StandardStringVector();
				}

				break;
			}
			case SWAP_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( swapRates_, riskInstrumentType );
				break;
			case BASIS_SWAP_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( basisAdjRates_, riskInstrumentType );
				break;
			default:
				AQ_THROW("Unable to Determine Tenors from Swap Curve Market Data: Only LIBORFIXINGS, FUTURES, FRAS, SWAPS or BASISSWAPS Instrument Market Data is permitted.")
		}
	}

	// Function to identify which instruments are outright instruments to be included in risk totals
	std::vector<bool> SwapCurveObjectData::isOutrightInstrumentVector( const CurveMarketDataEnum & riskInstrumentType ) const
	{
		switch( riskInstrumentType )
		{
			case LIBOR_FIXING_TABLE:
				return std::vector<bool>( liborRates_.size(), true );
				break;
			case FUTURES_MARKETDATA:
				return std::vector<bool>( futureRates_.size(), true );
				break;
			case FRA_MARKETDATA:
			{
				const bool is3MFra = fra3mRates_.size() == 0 ? false : true;
				const bool is6MFra = fra6mRates_.size() == 0 ? false : true;
				AQ_REQUIRE( !is3MFra || !is6MFra, "Invalid Market Data: Cannot calibrate to 3M and 6M FRA Instruments within the same Swap Curve" )

				if( is3MFra )
				{
					return std::vector<bool>( fra3mRates_.size(), true );
				}
				else if( is6MFra )
				{
					return std::vector<bool>( fra6mRates_.size(), true );
				}
				else
				{
					// Null Vector
					return std::vector<bool>();
				}

				break;
			}
			case SWAP_MARKETDATA:
				return std::vector<bool>( swapRates_.size(), true );
				break;
			case BASIS_SWAP_MARKETDATA:
				return std::vector<bool>( basisAdjRates_.size(), false );
				break;
			default:
				AQ_THROW("Unable to Determine if Outright Instruments from Swap Curve Market Data: Only LIBORFIXINGS, FUTURES, FRAS, SWAPS or BASISSWAPS Instrument Market Data is permitted.")
		}
	}

	/* @brief Constructor of tenor basis curve data
	 */
	TenorBasisCurveObjectData::TenorBasisCurveObjectData(const CurveGeneratorPtr& aqoCurveGenerator, const CurveMarketDataPtr& aqoCurveMarketData, const std::string frequency, const std::string& marketDataAsOfDate, const std::string& domesticCurveCollection, const std::string& foreignCurveCollection, const std::string& curveNameIn, const std::string& curveIndexIn)
	{
		getCurveConventionBlock(curveConvLVB_, aqoCurveGenerator, marketDataAsOfDate);

		curveName_           = curveNameIn.c_str();
		curveIndex_          = curveIndexIn.c_str();

		basisRates_          = aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_BASISSWAPS );
		basisConvLVB_        = LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix(  GENERATOR_COMPONENTS::KEY_BASISSWAPS ) );
		moneyMarketConvLVB_  = AQLStringMatrix( 0 );

		forwardAdjustments_ = aqoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS) ? aqoCurveGenerator->toStandardStringMatrix( GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS ) : StandardStringMatrix();

        // Optional Fields - Fixing Table
        // ------------------------------
        liborRates_         = aqoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_LIBORFIXINGS) ?  aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_LIBORFIXINGS ) : AQLStringMatrix(0);
        liborConvLVB_       = aqoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_LIBORFIXINGS) ? LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_LIBORFIXINGS ) ) : LabelValueBlock();
        // ------------------------------

        // Optional: FRAS
        // -----------------
        fraRates_           = aqoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_FRAS ) ? aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_FRAS ) : AQLStringMatrix(0);
        fraConvLVB_         = aqoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_FRAS ) ? LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_FRAS ) ) : LabelValueBlock();

		AQLStringMatrix tmpStringMatrix = basisConvLVB_.toAQLStringMatrix();
		basisConvLVB_ = modifyConventionsAndApplyCurveCollections( tmpStringMatrix, domesticCurveCollection, foreignCurveCollection );

		// It only makes sense to search for FRA configuration and data if the curve frequency is SEMI-ANNUAL or QUARTERLY
		etrading::FrequencyEnum frequencyEnum = etrading::toFrequencyEnum( frequency );
		if ( (frequencyEnum == etrading::SEMI_ANNUAL_FREQUENCY) || (frequencyEnum == etrading::QUARTERLY_FREQUENCY) )
		{
            fraRates_       = aqoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_FRAS) ? aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_FRAS ) : AQLStringMatrix(0);
            fraConvLVB_     = aqoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_FRAS) ? aqoCurveGenerator->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_FRAS ) : AQLStringMatrix(0);
		}
        
        // Optional Market Data Bump / Shift Parameter
        shiftSize_ = 0.0;
	}

    // Function to apply a flat-shift to outright and basis instruments
    void TenorBasisCurveObjectData::applyFlatShift( const double & shiftSize )
    {
         // Check if bump already applied - don't allow multiple bumps, otherwise we have no way to restore the data, if required.
        AQ_REQUIRE( AQ_IS_EQUAL_ZERO( shiftSize_ ), "Unable to apply Flat-Shift to Tenor-Basis Curve Market Data: The market data has a shift applied to it already" )
        shiftSize_ = shiftSize;
        
        // *** For Tenor Basis *** Flat-Shift Bumps should bump outright and basis instruments
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( liborRates_,     shiftSize_,     LIBOR_FIXING_TABLE );
		CurveMarketData::bumpMarketDataFromAQLStringMatrix( fraRates_,       shiftSize_,     FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( basisRates_,		shiftSize_,     BASIS_SWAP_MARKETDATA );
    }

    // Function to remove a flat-shift a clear an existing flat-shift
    void TenorBasisCurveObjectData::restoreDataAndClearFlatShift()
    {
        if ( AQ_IS_EQUAL_ZERO( shiftSize_ ) )
        {
            // Do nothing if the shift-size applied was zero
            return;
        }

        // Remove Flat-Shifts
		// *** For Tenor Basis *** Flat-Shift Bumps should bump outright and basis instruments
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( liborRates_,     -shiftSize_,	LIBOR_FIXING_TABLE );
		CurveMarketData::bumpMarketDataFromAQLStringMatrix( fraRates_,       -shiftSize_,	FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( basisRates_,		-shiftSize_,	BASIS_SWAP_MARKETDATA );
		
        // Restore ShiftSize Parameter
        shiftSize_ = 0.0;
    }

	// Function to perturb the nth market data instrument
	void TenorBasisCurveObjectData::perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize )
	{
		shiftSize_				= shiftSize;
		riskInstrumentType_		= riskInstrumentType;
		nthRiskInstrument_		= nthRiskInstrument;

        // Perturb Instrument
		switch( riskInstrumentType_ )
		{
			case LIBOR_FIXING_TABLE:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( liborRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case FRA_MARKETDATA:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( fraRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case BASIS_SWAP_MARKETDATA:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( basisRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			default:
				AQ_THROW("Unable to Pertrub Tenor-Basis Curve Market Data: Only LIBORFIXINGS, FRAS or BASISSWAPS Instruments can be Perturbed.")
		}
	}

	// Function to restore market data and clear perturbation shifts
	void TenorBasisCurveObjectData::restoreDataAndClearPerturbation()
	{
		if ( AQ_IS_EQUAL_ZERO( shiftSize_ ) )
        {
            // Do nothing if the shift-size applied was zero
            return;
        }

		// Track Original ShiftSize
        // const double originalShift = shiftSize_; <---Unused Variable
        
        // Remove Perturbation Shifts
		perturb( riskInstrumentType_, nthRiskInstrument_, -shiftSize_ );

		// Reset Perturbation Parameters
        shiftSize_				= 0.0;
		riskInstrumentType_		= NONE_MARKETDATA;
		nthRiskInstrument_		= 0u;
	}

	// Function to get the peturbed instrument tenors
	StandardStringVector TenorBasisCurveObjectData::instrumentTenors( const CurveMarketDataEnum & riskInstrumentType ) const
	{
		switch( riskInstrumentType )
		{
			case LIBOR_FIXING_TABLE:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( liborRates_, riskInstrumentType );
				break;
			case FRA_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( fraRates_, riskInstrumentType );
				break;
			case BASIS_SWAP_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( basisRates_, riskInstrumentType );
				break;
			default:
				AQ_THROW("Unable to Determine Tenors from Tenor Basis Curve Market Data: Only LIBORFIXINGS, FRAS or BASISSWAPS Instrument Market Data is permitted.")
		}
	}

	// Function to identify which instruments are outright instruments to be included in risk totals
	std::vector<bool> TenorBasisCurveObjectData::isOutrightInstrumentVector( const CurveMarketDataEnum & riskInstrumentType ) const
	{
		switch( riskInstrumentType )
		{
			case LIBOR_FIXING_TABLE:
				return std::vector<bool>( liborRates_.size(), true );
				break;
			case FRA_MARKETDATA:
				return std::vector<bool>( fraRates_.size(), true );
				break;
			case BASIS_SWAP_MARKETDATA:
				return std::vector<bool>( basisRates_.size(), false );
				break;
			default:
				AQ_THROW("Unable to Determine if Outright Instruments from Tenor Basis Curve Market Data: Only LIBORFIXINGS, FRAS or BASISSWAPS Instrument Market Data is permitted.")
		}
	}
	
	/* @brief Constructor of xccy basis curve data
	 */
	XccyBasisCurveObjectData::XccyBasisCurveObjectData(const CurveGeneratorPtr& aqoCurveGenerator, const CurveMarketDataPtr& aqoCurveMarketData, const std::string& marketDataAsOfDate, const std::string& domesticCurveCollection, const std::string& foreignCurveCollection, const std::string& curveNameIn, const std::string& curveIndexIn)
	{
		getCurveConventionBlock(curveConvLVB_, aqoCurveGenerator, marketDataAsOfDate);

		curveName_      = curveNameIn.c_str();
		curveIndex_     = curveIndexIn.c_str();

		basisRates_     = aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_XCCYSWAPS );
		basisConvLVB_   = LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_XCCYSWAPS ) );
		
		forwardAdjustments_ = aqoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS) ? aqoCurveGenerator->toStandardStringMatrix( GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS ) : StandardStringMatrix();

        // Optional Instruments
        fxFwdRates_     = aqoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_FXFWDS)   ? aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_FXFWDS )    : AQLStringMatrix(0);
		fxFwdConvLVB_   = aqoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_FXFWDS)    ? LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_FXFWDS ) ) : LabelValueBlock();
		spotFxRates_    = aqoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_FXSPOTS)  ? aqoCurveMarketData->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_FXSPOTS )   : AQLStringMatrix(0);

		AQLStringMatrix tmpStringMatrix = basisConvLVB_.toAQLStringMatrix();
		basisConvLVB_   = modifyConventionsAndApplyCurveCollections( tmpStringMatrix, domesticCurveCollection, foreignCurveCollection );

        // Optional Market Data Bump / Shift Parameter
        shiftSize_ = 0.0;
	}

    // Function to apply a flat-shift to outright instruments only
    void XccyBasisCurveObjectData::applyFlatShift( const double & shiftSize )
    {
        // Check if bump already applied - don't allow multiple bumps, otherwise we have no way to restore the data, if required.
        AQ_REQUIRE( AQ_IS_EQUAL_ZERO( shiftSize_ ), "Unable to apply Flat-Shift to Xccy Basis Curve Market Data: The market data has a shift applied to it already" )
        shiftSize_ = shiftSize;
        
		// *** For Xccy Basis *** Flat-Shift Bumps should bump outright and basis instruments
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( fxFwdRates_,     shiftSize_,     FXFWD_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( spotFxRates_,    shiftSize_,     FXSPOT_MARKETDATA );
		CurveMarketData::bumpMarketDataFromAQLStringMatrix( basisRates_,		shiftSize_,		XCCY_SWAP_MARKETDATA ); // basis = xccy basis
    }

	// Function to remove a flat-shift a clear an existing flat-shift
    void XccyBasisCurveObjectData::restoreDataAndClearFlatShift()
    {
        if ( AQ_IS_EQUAL_ZERO( shiftSize_ ) )
        {
            // Do nothing if the shift-size applied was zero
            return;
        }

        // Remove Flat-Shifts
		// *** For Xccy Basis *** Flat-Shift Bumps should bump outright and basis instruments
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( fxFwdRates_,     -shiftSize_,    FXFWD_MARKETDATA );
        CurveMarketData::bumpMarketDataFromAQLStringMatrix( spotFxRates_,    -shiftSize_,    FXSPOT_MARKETDATA );
		CurveMarketData::bumpMarketDataFromAQLStringMatrix( basisRates_,		-shiftSize_,	XCCY_SWAP_MARKETDATA ); // basis = xccy basis

        // Restore ShiftSize Parameter
        shiftSize_ = 0.0;
    }

	// Function to perturb the nth market data instrument
	void XccyBasisCurveObjectData::perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize )
	{
		shiftSize_				= shiftSize;
		riskInstrumentType_		= riskInstrumentType;
		nthRiskInstrument_		= nthRiskInstrument;

        // Perturb Instrument
		switch( riskInstrumentType_ )
		{
			case FXFWD_MARKETDATA:
				// Tenor-Basis Curves Contain Xccy Basis Placeholders - Not used in this case
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( fxFwdRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case FXSPOT_MARKETDATA:
				// Tenor-Basis Curves Contain Xccy Basis Placeholders - Not used in this case
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( spotFxRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case XCCY_SWAP_MARKETDATA:
				CurveMarketData::perturbMarketDataFromAQLStringMatrix( basisRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			default:
				AQ_THROW("Unable to Pertrub Xccy-Basis Curve Market Data: Only FXFORWARDS, FXSPOTS or XCCYSWAPS Instruments can be Perturbed.")
		}
	}

	// Function to restore market data and clear perturbation shifts
	void XccyBasisCurveObjectData::restoreDataAndClearPerturbation()
	{
		if ( AQ_IS_EQUAL_ZERO( shiftSize_ ) )
        {
            // Do nothing if the shift-size applied was zero
            return;
        }

		// Remove Perturbation Shifts - Note we can't apply a zero shift, since we use that value to track bump state and check if a perturbation is already applied
		perturb( riskInstrumentType_, nthRiskInstrument_, -shiftSize_ );

		// Reset Perturbation Parameters
        shiftSize_				= 0.0;
		riskInstrumentType_		= NONE_MARKETDATA;
		nthRiskInstrument_		= 0u;
	}

	// Function to get the peturbed instrument tenors
	StandardStringVector XccyBasisCurveObjectData::instrumentTenors( const CurveMarketDataEnum & riskInstrumentType ) const
	{
		switch( riskInstrumentType )
		{
			case FXFWD_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( fxFwdRates_, riskInstrumentType );
				break;
			case FXSPOT_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( spotFxRates_, riskInstrumentType );
				break;
			case XCCY_SWAP_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromAQLStringMatrix( basisRates_, riskInstrumentType );
				break;
			default:
				AQ_THROW("Unable to Determine Tenors from Xccy Basis Curve Market Data: Only FXFORWARDS, FXSPOTS or XCCYSWAPS Instrument Market Data is permitted.")
		}
	}

	// Function to identify which instruments are outright instruments to be included in risk totals
	std::vector<bool> XccyBasisCurveObjectData::isOutrightInstrumentVector( const CurveMarketDataEnum & riskInstrumentType ) const
	{
		switch( riskInstrumentType )
		{
			case FXFWD_MARKETDATA:
				return std::vector<bool>( fxFwdRates_.size(), true );
				break;
			case FXSPOT_MARKETDATA:
				return std::vector<bool>( spotFxRates_.size(), true );
				break;
			case XCCY_SWAP_MARKETDATA:
				return std::vector<bool>( basisRates_.size(), false );
				break;
			default:
				AQ_THROW("Unable to Determine if Outright Instruments from Xccy Basis Curve Market Data: Only FXFORWARDS, FXSPOTS or XCCYSWAPS Instrument Market Data is permitted.")
		}
	}

	/* @brief Constructor of fwd constant curve data
	 */
	FwdConstantCurveObjectData::FwdConstantCurveObjectData(const CurveGeneratorPtr& aqoCurveGenerator, const std::string& marketDataAsOfDate, const std::string& domesticCurveCollection, const std::string& foreignCurveCollection, const std::string& curveNameIn, const std::string& curveIndexIn)
	{
		getCurveConventionBlock(curveConvLVB_, aqoCurveGenerator, marketDataAsOfDate);

		curveName_          = curveNameIn.c_str();
		curveIndex_         = curveIndexIn.c_str();

		fwdfxconstConvLVB_  = LabelValueBlock( aqoCurveGenerator->toAQLStringMatrix( GENERATOR_COMPONENTS::KEY_FWDFXCONST ) );
		AQLStringMatrix tmpStringMatrix = fwdfxconstConvLVB_.toAQLStringMatrix();
		fwdfxconstConvLVB_  = modifyConventionsAndApplyCurveCollections( tmpStringMatrix, domesticCurveCollection, foreignCurveCollection );

		forwardAdjustments_ = aqoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS) ? aqoCurveGenerator->toStandardStringMatrix( GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS ) : StandardStringMatrix();

        // Optional Market Data Bump / Shift Parameter
        shiftSize_ = 0.0;
	}

    // Function to apply a flat-shift to outright instruments only
    void FwdConstantCurveObjectData::applyFlatShift( const double & shiftSize )
    {
        // Pure Virtual Method not required in this case therefore .... do nothing
    }

    // Function to remove a flat-shift a clear an existing flat-shift
    void FwdConstantCurveObjectData::restoreDataAndClearFlatShift()
    {
        // Pure Virtual Method not required in this case therefore .... do nothing
    }

	// Function to perturb the nth market data instrument
	void FwdConstantCurveObjectData::perturb( const CurveMarketDataEnum & riskInstrumentType, const size_t nthRiskInstrument, const double & shiftSize )
	{
		 // Pure Virtual Method not required in this case therefore .... do nothing
	}

	// Function to restore market data and clear perturbation shifts
	void FwdConstantCurveObjectData::restoreDataAndClearPerturbation()
	{
		// Pure Virtual Method not required in this case therefore .... do nothing
	}

}
