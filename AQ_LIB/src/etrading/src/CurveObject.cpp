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
	etrading::LabelValueBlock modifyConventionsAndApplyCurveCollections( LAStringMatrix& conventions, const std::string& domesticCurveCollection, const std::string& foreignCurveCollection )
	{
		size_t numRows = conventions.size();
		for (size_t i = 0; i < numRows; i++)
		{
			LAString key = conventions[i][0];
			key.toUpper();
			if ( key.findString( "FORECAST" ) != -1 || key.findString( "DISCOUNT" ) != -1 )
			{
				LAString& value = conventions[i][1];
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
	*  @param[in]		lwoCurveGenerator	Curve generator object
	*  @param[in]		marketDataAsOfDate	As of date
	*/
	void getCurveConventionBlock( etrading::LabelValueBlock& curveConvLVB, const etrading::CurveGeneratorPtr& lwoCurveGenerator, const std::string& marketDataAsOfDate)
	{
		LAStringMatrix curveProperties = lwoCurveGenerator->toLAStringMatrix( "CURVEPROPERTIES" );

		// Push the MarketData AsOfDate into the CurveConventions, as required by the underlying curve calibration routines.
		LAString asOfDateKey( "AsOfDate" );
        LAString asOfDateValue( marketDataAsOfDate.c_str() );
		
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
	CurveObject::CurveObject( const std::string& objectName) : IsLWOObject(objectName, CURVE)
	{
    }

	/* @brief Copy Constructor
	 */
	CurveObject::CurveObject(const CurveObject& rhs) 
		: IsLWOObject(rhs.getRefToName(), CURVE)
	{
		// The copy constructor does not need to invoke CurveObject::calibrateCurve() :
		// The curve will have already been built by the rhs object.
	}


	//-------------------------------------------------------------------------------------------------//
	//-------------------------------------------------------------------------------------------------//

	/* @brief Constructor of OIS curve data
	 */
	OISCurveObjectData::OISCurveObjectData(const CurveGeneratorPtr& lwoCurveGenerator, const CurveMarketDataPtr& lwoCurveMarketData, const std::string& marketDataAsOfDate, bool useSwaps, const std::string& curveNameIn, const std::string& curveIndexIn)
	{
		getCurveConventionBlock(curveConvLVB_, lwoCurveGenerator, marketDataAsOfDate);

		curveName_       = curveNameIn.c_str();
		curveIndex_      = curveIndexIn.c_str();

		oisRates_        = lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_OIS );
		oisConvLVB_      = LabelValueBlock( lwoCurveGenerator->toLAStringMatrix(  GENERATOR_COMPONENTS::KEY_OIS ) );
		
        // Optional: OIS FIXINGS
        // ----------------------
        histRates_       = lwoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_OISFIXINGS ) ? etrading::retrieveFixingTableMatrix(lwoCurveMarketData->toLAStringMatrix(GENERATOR_COMPONENTS::KEY_OISFIXINGS)) : LAStringMatrix(0);

		loBasisRates_    = lwoCurveMarketData->doesKeyExist(GENERATOR_COMPONENTS::KEY_LIBOROISBASISSPREADS) ? lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_LIBOROISBASISSPREADS ) : LAStringMatrix(0);
		loBasisConvLVB_  = lwoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_LIBOROISBASIS) ? LabelValueBlock( lwoCurveGenerator->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_LIBOROISBASIS ) ) : LabelValueBlock();
		swapConvLVB_     = lwoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_SWAPS) ? LabelValueBlock( lwoCurveGenerator->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_SWAPS ) ): LabelValueBlock();
		
		forwardAdjustments_ = lwoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS) ? lwoCurveGenerator->toStandardStringMatrix( GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS ) : StandardStringMatrix();

		// When used in dual-bootstrapping context, we do not extract Libor swap par rates from the OIS curve
		if (useSwaps)
		{
            // Libor Swap Rates on an OIS Curve are Optional and may not exist
            swapRates_ = lwoCurveMarketData->doesKeyExist(GENERATOR_COMPONENTS::KEY_SWAPS) ? lwoCurveMarketData->toLAStringMatrix(  GENERATOR_COMPONENTS::KEY_SWAPS ) : LAStringMatrix(0);
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
        CurveMarketData::bumpMarketDataFromLAStringMatrix( oisRates_, shiftSize_, OIS_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( swapRates_, shiftSize_, SWAP_MARKETDATA );
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
        CurveMarketData::bumpMarketDataFromLAStringMatrix( oisRates_, -shiftSize_, OIS_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( swapRates_, -shiftSize_, SWAP_MARKETDATA );

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
				CurveMarketData::perturbMarketDataFromLAStringMatrix( oisRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case SWAP_MARKETDATA:
				CurveMarketData::perturbMarketDataFromLAStringMatrix( swapRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case LIBOR_OIS_BASISSPREAD_MARKETDATA:
				CurveMarketData::perturbMarketDataFromLAStringMatrix( loBasisRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
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
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( oisRates_, riskInstrumentType );
				break;
			case SWAP_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( swapRates_, riskInstrumentType );
				break;
			case LIBOR_OIS_BASISSPREAD_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( loBasisRates_, riskInstrumentType );
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
    ARRCurveObjectData::ARRCurveObjectData(const CurveGeneratorPtr& lwoCurveGenerator, const CurveMarketDataPtr& lwoCurveMarketData, const std::string& marketDataAsOfDate, bool useSwaps, const std::string& curveNameIn, const std::string& curveIndexIn)
    {
		getCurveConventionBlock( curveConvLVB_, lwoCurveGenerator, marketDataAsOfDate );

		curveName_      = curveNameIn.c_str();
		curveIndex_     = curveIndexIn.c_str();

		oisRates_       = lwoCurveMarketData->toLAStringMatrix(GENERATOR_COMPONENTS::KEY_OIS);
		auto oisConv     = lwoCurveGenerator->toLAStringMatrix(GENERATOR_COMPONENTS::KEY_OIS) ;

		// Optional: LinearSplineJoinDate
		const LabelValueBlock marketDataPropertiesLVB = lwoCurveMarketData->toLAStringMatrix(GENERATOR_COMPONENTS::KEY_MARKETDATAPROPERTIES);
		const LAString joinDateStr = marketDataPropertiesLVB.getOptionalValueAsLAString(CURVEGENERATOR_CURVEPROPERTIES_KEY::INTERPOLATION_JOIN_DATE);

		if (joinDateStr.size() > 0)
		{
			// Push the MarketData LinearSplineJoinDate into the CurveConventions, as required by the underlying curve calibration routines.
			LAStringVector joinDateKeyValue;
			joinDateKeyValue.push_back(CURVEGENERATOR_CURVEPROPERTIES_KEY::INTERPOLATION_JOIN_DATE);
			joinDateKeyValue.push_back(joinDateStr);
			oisConv.push_back(joinDateKeyValue);
		}
		oisConvLVB_ = LabelValueBlock(oisConv);


		// Optional: OIS FIXINGS
		// ----------------------
		histRates_ = lwoCurveMarketData->doesKeyExist(GENERATOR_COMPONENTS::KEY_OISFIXINGS) ? etrading::retrieveFixingTableMatrix(lwoCurveMarketData->toLAStringMatrix(GENERATOR_COMPONENTS::KEY_OISFIXINGS)) : LAStringMatrix(0);

		loBasisRates_   = lwoCurveMarketData->doesKeyExist(GENERATOR_COMPONENTS::KEY_LIBOROISBASISSPREADS) ? lwoCurveMarketData->toLAStringMatrix(GENERATOR_COMPONENTS::KEY_LIBOROISBASISSPREADS) : LAStringMatrix(0);
		loBasisConvLVB_ = lwoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_LIBOROISBASIS) ? LabelValueBlock( lwoCurveGenerator->toLAStringMatrix(GENERATOR_COMPONENTS::KEY_LIBOROISBASIS) ) : LabelValueBlock();
		swapConvLVB_    = lwoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_SWAPS) ? LabelValueBlock( lwoCurveGenerator->toLAStringMatrix(GENERATOR_COMPONENTS::KEY_SWAPS) ) : LabelValueBlock();

		forwardAdjustments_ = lwoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS) ? lwoCurveGenerator->toStandardStringMatrix( GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS ) : StandardStringMatrix();

		// When used in dual-bootstrapping context, we do not extract Libor swap par rates from the OIS curve
		if (useSwaps)
		{
			// Libor Swap Rates on an OIS Curve are Optional and may not exist
			swapRates_ = lwoCurveMarketData->doesKeyExist(GENERATOR_COMPONENTS::KEY_SWAPS) ? lwoCurveMarketData->toLAStringMatrix(GENERATOR_COMPONENTS::KEY_SWAPS) : LAStringMatrix(0);
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
        CurveMarketData::bumpMarketDataFromLAStringMatrix( oisRates_, shiftSize_, OIS_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( swapRates_, shiftSize_, SWAP_MARKETDATA );
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
        CurveMarketData::bumpMarketDataFromLAStringMatrix( oisRates_, -shiftSize_, OIS_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( swapRates_, -shiftSize_, SWAP_MARKETDATA );

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
				CurveMarketData::perturbMarketDataFromLAStringMatrix( oisRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case SWAP_MARKETDATA:
				CurveMarketData::perturbMarketDataFromLAStringMatrix( swapRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case LIBOR_OIS_BASISSPREAD_MARKETDATA:
				CurveMarketData::perturbMarketDataFromLAStringMatrix( loBasisRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
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
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( oisRates_, riskInstrumentType );
				break;
			case SWAP_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( swapRates_, riskInstrumentType );
				break;
			case LIBOR_OIS_BASISSPREAD_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( loBasisRates_, riskInstrumentType );
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
	SwapCurveObjectData::SwapCurveObjectData(const CurveGeneratorPtr& lwoCurveGenerator, const CurveMarketDataPtr& lwoCurveMarketData, const std::string& frequency, const std::string& marketDataAsOfDate, const std::string& curveNameIn, const std::string& curveIndexIn)
	{
		getCurveConventionBlock(curveConvLVB_, lwoCurveGenerator, marketDataAsOfDate);

		curveName_          = curveNameIn.c_str();
		curveIndex_         = curveIndexIn.c_str();

		moneyMarketConvLVB_ = LabelValueBlock( lwoCurveGenerator->toLAStringMatrix(  GENERATOR_COMPONENTS::KEY_MONEYMARKET ) );
		liborRates_         = lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_LIBORFIXINGS );
		liborConvLVB_       = LabelValueBlock( lwoCurveGenerator->toLAStringMatrix(  GENERATOR_COMPONENTS::KEY_LIBORFIXINGS ) );
		swapRates_          = lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_SWAPS );
		swapConvLVB_        = LabelValueBlock( lwoCurveGenerator->toLAStringMatrix(  GENERATOR_COMPONENTS::KEY_SWAPS ) );
		
		forwardAdjustments_ = lwoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS) ? lwoCurveGenerator->toStandardStringMatrix( GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS ) : StandardStringMatrix();

        // Optional: FRAS
        // -----------------
        fraRates_           = lwoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_FRAS ) ? lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_FRAS ) : LAStringMatrix(0);
        fraConvLVB_         = lwoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_FRAS ) ? LabelValueBlock( lwoCurveGenerator->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_FRAS ) ) : LabelValueBlock();

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
					throw LACoreInvalidData( ( boost::format( "#Error: Invalid FRA Data: Only 3M and 6M FRAs are supported. Current FRA frequency: %s." )  
							% frequency ).str().c_str(), __FILE__, __LINE__ );
			}
		}

        // Optional: FUTURES
        // -----------------
        futureRates_        = lwoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_FUTURES ) ? lwoCurveMarketData->toLAStringMatrix(GENERATOR_COMPONENTS::KEY_FUTURES ) : LAStringMatrix(0);
        futureConvLVB_      = lwoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_FUTURES ) ? LabelValueBlock( lwoCurveGenerator->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_FUTURES ) ) : LabelValueBlock();

		// Optional Fields - Tenor Basis Instruments
        // -----------------------------------------
        basisAdjConvLVB_    = lwoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_BASISSWAPS) ? LabelValueBlock( lwoCurveGenerator->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_BASISSWAPS ) ) : LabelValueBlock();
        basisAdjRates_      = lwoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_BASISSWAPS) ? lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_BASISSWAPS ) : LAStringMatrix(0);

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
        CurveMarketData::bumpMarketDataFromLAStringMatrix( liborRates_,     shiftSize_,     LIBOR_FIXING_TABLE );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( futureRates_,    shiftSize_,     FUTURES_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( fraRates_,       shiftSize_,     FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( fra3mRates_,     shiftSize_,     FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( fra6mRates_,     shiftSize_,     FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( swapRates_,      shiftSize_,     SWAP_MARKETDATA );
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
        CurveMarketData::bumpMarketDataFromLAStringMatrix( liborRates_,     -shiftSize_,      LIBOR_FIXING_TABLE );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( futureRates_,    -shiftSize_,      FUTURES_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( fraRates_,       -shiftSize_,      FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( fra3mRates_,     -shiftSize_,      FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( fra6mRates_,     -shiftSize_,      FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( swapRates_,      -shiftSize_,      SWAP_MARKETDATA );

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
				CurveMarketData::perturbMarketDataFromLAStringMatrix( liborRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case FUTURES_MARKETDATA:
				CurveMarketData::perturbMarketDataFromLAStringMatrix( futureRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case FRA_MARKETDATA:
			{
				CurveMarketData::perturbMarketDataFromLAStringMatrix( fraRates_,   nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				
				const bool is3MFra = fra3mRates_.size() == 0 ? false : true;
				const bool is6MFra = fra6mRates_.size() == 0 ? false : true;
				AQ_REQUIRE( !is3MFra || !is6MFra, "Invalid Market Data: Cannot calibrate to 3M and 6M FRA Instruments within the same Swap Curve" )

				if ( is3MFra )
				{
					CurveMarketData::perturbMarketDataFromLAStringMatrix( fra3mRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				}
				
				if ( is6MFra )
				{
					CurveMarketData::perturbMarketDataFromLAStringMatrix( fra6mRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				}

				break;
			}
			case SWAP_MARKETDATA:
				CurveMarketData::perturbMarketDataFromLAStringMatrix( swapRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case BASIS_SWAP_MARKETDATA:
				CurveMarketData::perturbMarketDataFromLAStringMatrix( basisAdjRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
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
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( liborRates_, riskInstrumentType );
				break;
			case FUTURES_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( futureRates_, riskInstrumentType );
				break;
			case FRA_MARKETDATA:
			{
				const bool is3MFra = fra3mRates_.size() == 0 ? false : true;
				const bool is6MFra = fra6mRates_.size() == 0 ? false : true;
				AQ_REQUIRE( !is3MFra || !is6MFra, "Invalid Market Data: Cannot calibrate to 3M and 6M FRA Instruments within the same Swap Curve" )
				
				if ( is3MFra )
				{
					return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( fra3mRates_, riskInstrumentType );
				}
				else if ( is6MFra )
				{
					return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( fra6mRates_, riskInstrumentType );
				}
				else
				{
					// Null Vector
					return StandardStringVector();
				}

				break;
			}
			case SWAP_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( swapRates_, riskInstrumentType );
				break;
			case BASIS_SWAP_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( basisAdjRates_, riskInstrumentType );
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
	TenorBasisCurveObjectData::TenorBasisCurveObjectData(const CurveGeneratorPtr& lwoCurveGenerator, const CurveMarketDataPtr& lwoCurveMarketData, const std::string frequency, const std::string& marketDataAsOfDate, const std::string& domesticCurveCollection, const std::string& foreignCurveCollection, const std::string& curveNameIn, const std::string& curveIndexIn)
	{
		getCurveConventionBlock(curveConvLVB_, lwoCurveGenerator, marketDataAsOfDate);

		curveName_           = curveNameIn.c_str();
		curveIndex_          = curveIndexIn.c_str();

		basisRates_          = lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_BASISSWAPS );
		basisConvLVB_        = LabelValueBlock( lwoCurveGenerator->toLAStringMatrix(  GENERATOR_COMPONENTS::KEY_BASISSWAPS ) );
		moneyMarketConvLVB_  = LAStringMatrix( 0 );

		forwardAdjustments_ = lwoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS) ? lwoCurveGenerator->toStandardStringMatrix( GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS ) : StandardStringMatrix();

        // Optional Fields - Fixing Table
        // ------------------------------
        liborRates_         = lwoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_LIBORFIXINGS) ?  lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_LIBORFIXINGS ) : LAStringMatrix(0);
        liborConvLVB_       = lwoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_LIBORFIXINGS) ? LabelValueBlock( lwoCurveGenerator->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_LIBORFIXINGS ) ) : LabelValueBlock();
        // ------------------------------

        // Optional: FRAS
        // -----------------
        fraRates_           = lwoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_FRAS ) ? lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_FRAS ) : LAStringMatrix(0);
        fraConvLVB_         = lwoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_FRAS ) ? LabelValueBlock( lwoCurveGenerator->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_FRAS ) ) : LabelValueBlock();

		LAStringMatrix tmpStringMatrix = basisConvLVB_.toLAStringMatrix();
		basisConvLVB_ = modifyConventionsAndApplyCurveCollections( tmpStringMatrix, domesticCurveCollection, foreignCurveCollection );

		// It only makes sense to search for FRA configuration and data if the curve frequency is SEMI-ANNUAL or QUARTERLY
		etrading::FrequencyEnum frequencyEnum = etrading::toFrequencyEnum( frequency );
		if ( (frequencyEnum == etrading::SEMI_ANNUAL_FREQUENCY) || (frequencyEnum == etrading::QUARTERLY_FREQUENCY) )
		{
            fraRates_       = lwoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_FRAS) ? lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_FRAS ) : LAStringMatrix(0);
            fraConvLVB_     = lwoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_FRAS) ? lwoCurveGenerator->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_FRAS ) : LAStringMatrix(0);
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
        CurveMarketData::bumpMarketDataFromLAStringMatrix( liborRates_,     shiftSize_,     LIBOR_FIXING_TABLE );
		CurveMarketData::bumpMarketDataFromLAStringMatrix( fraRates_,       shiftSize_,     FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( basisRates_,		shiftSize_,     BASIS_SWAP_MARKETDATA );
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
        CurveMarketData::bumpMarketDataFromLAStringMatrix( liborRates_,     -shiftSize_,	LIBOR_FIXING_TABLE );
		CurveMarketData::bumpMarketDataFromLAStringMatrix( fraRates_,       -shiftSize_,	FRA_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( basisRates_,		-shiftSize_,	BASIS_SWAP_MARKETDATA );
		
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
				CurveMarketData::perturbMarketDataFromLAStringMatrix( liborRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case FRA_MARKETDATA:
				CurveMarketData::perturbMarketDataFromLAStringMatrix( fraRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case BASIS_SWAP_MARKETDATA:
				CurveMarketData::perturbMarketDataFromLAStringMatrix( basisRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
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
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( liborRates_, riskInstrumentType );
				break;
			case FRA_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( fraRates_, riskInstrumentType );
				break;
			case BASIS_SWAP_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( basisRates_, riskInstrumentType );
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
	XccyBasisCurveObjectData::XccyBasisCurveObjectData(const CurveGeneratorPtr& lwoCurveGenerator, const CurveMarketDataPtr& lwoCurveMarketData, const std::string& marketDataAsOfDate, const std::string& domesticCurveCollection, const std::string& foreignCurveCollection, const std::string& curveNameIn, const std::string& curveIndexIn)
	{
		getCurveConventionBlock(curveConvLVB_, lwoCurveGenerator, marketDataAsOfDate);

		curveName_      = curveNameIn.c_str();
		curveIndex_     = curveIndexIn.c_str();

		basisRates_     = lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_XCCYSWAPS );
		basisConvLVB_   = LabelValueBlock( lwoCurveGenerator->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_XCCYSWAPS ) );
		
		forwardAdjustments_ = lwoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS) ? lwoCurveGenerator->toStandardStringMatrix( GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS ) : StandardStringMatrix();

        // Optional Instruments
        fxFwdRates_     = lwoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_FXFWDS)   ? lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_FXFWDS )    : LAStringMatrix(0);
		fxFwdConvLVB_   = lwoCurveGenerator->doesKeyExist( GENERATOR_COMPONENTS::KEY_FXFWDS)    ? LabelValueBlock( lwoCurveGenerator->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_FXFWDS ) ) : LabelValueBlock();
		spotFxRates_    = lwoCurveMarketData->doesKeyExist( GENERATOR_COMPONENTS::KEY_FXSPOTS)  ? lwoCurveMarketData->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_FXSPOTS )   : LAStringMatrix(0);

		LAStringMatrix tmpStringMatrix = basisConvLVB_.toLAStringMatrix();
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
        CurveMarketData::bumpMarketDataFromLAStringMatrix( fxFwdRates_,     shiftSize_,     FXFWD_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( spotFxRates_,    shiftSize_,     FXSPOT_MARKETDATA );
		CurveMarketData::bumpMarketDataFromLAStringMatrix( basisRates_,		shiftSize_,		XCCY_SWAP_MARKETDATA ); // basis = xccy basis
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
        CurveMarketData::bumpMarketDataFromLAStringMatrix( fxFwdRates_,     -shiftSize_,    FXFWD_MARKETDATA );
        CurveMarketData::bumpMarketDataFromLAStringMatrix( spotFxRates_,    -shiftSize_,    FXSPOT_MARKETDATA );
		CurveMarketData::bumpMarketDataFromLAStringMatrix( basisRates_,		-shiftSize_,	XCCY_SWAP_MARKETDATA ); // basis = xccy basis

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
				CurveMarketData::perturbMarketDataFromLAStringMatrix( fxFwdRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case FXSPOT_MARKETDATA:
				// Tenor-Basis Curves Contain Xccy Basis Placeholders - Not used in this case
				CurveMarketData::perturbMarketDataFromLAStringMatrix( spotFxRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
				break;
			case XCCY_SWAP_MARKETDATA:
				CurveMarketData::perturbMarketDataFromLAStringMatrix( basisRates_, nthRiskInstrument_, shiftSize_, riskInstrumentType_ );
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
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( fxFwdRates_, riskInstrumentType );
				break;
			case FXSPOT_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( spotFxRates_, riskInstrumentType );
				break;
			case XCCY_SWAP_MARKETDATA:
				return CurveMarketData::getMarketDataTenorsFromLAStringMatrix( basisRates_, riskInstrumentType );
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
	FwdConstantCurveObjectData::FwdConstantCurveObjectData(const CurveGeneratorPtr& lwoCurveGenerator, const std::string& marketDataAsOfDate, const std::string& domesticCurveCollection, const std::string& foreignCurveCollection, const std::string& curveNameIn, const std::string& curveIndexIn)
	{
		getCurveConventionBlock(curveConvLVB_, lwoCurveGenerator, marketDataAsOfDate);

		curveName_          = curveNameIn.c_str();
		curveIndex_         = curveIndexIn.c_str();

		fwdfxconstConvLVB_  = LabelValueBlock( lwoCurveGenerator->toLAStringMatrix( GENERATOR_COMPONENTS::KEY_FWDFXCONST ) );
		LAStringMatrix tmpStringMatrix = fwdfxconstConvLVB_.toLAStringMatrix();
		fwdfxconstConvLVB_  = modifyConventionsAndApplyCurveCollections( tmpStringMatrix, domesticCurveCollection, foreignCurveCollection );

		forwardAdjustments_ = lwoCurveGenerator->doesKeyExist(GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS) ? lwoCurveGenerator->toStandardStringMatrix( GENERATOR_COMPONENTS::KEY_FORWARDADJUSTMENTS ) : StandardStringMatrix();

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
