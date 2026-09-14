

#include <sstream>

#include "AQObjUtilities.h"
#include "Environment.h"
#include "CurveResultsContainer.h"	// Curve Group & Curve Results Logic

namespace etrading
{

	std::shared_ptr<Swap> getSwap(const std::string& objectName)
	{
		auto swap  = Environment::defaultEnv().accessObject<Swap>(objectName);
		if(!swap)
		{
            { std::ostringstream aqObjMsg1; aqObjMsg1 << "Swap " << objectName << " does not exist"; AQ_THROW( aqObjMsg1.str() ); }			
		}
		return swap;
	}

	std::shared_ptr<Schedule> getSchedule(const std::string& objectName)
	{
		auto schedule  = Environment::defaultEnv().accessObject<Schedule>(objectName);
		if(!schedule)
		{
            { std::ostringstream aqObjMsg2; aqObjMsg2 << "Schedule " << objectName << " does not exist"; AQ_THROW( aqObjMsg2.str() ); }			
		}
		return schedule;
	}

	std::shared_ptr<CreditModel> getCreditModel( const std::string& objectName, bool throwIfMissing )
	{
		auto creditModel  = Environment::defaultEnv().accessObject<CreditModel>(objectName);
		if ( !creditModel && throwIfMissing )
		{
            { std::ostringstream aqObjMsg3; aqObjMsg3 << "CreditModel " << objectName << " does not exist"; AQ_THROW( aqObjMsg3.str() ); }			
		}
		return creditModel;
	}

	std::shared_ptr<CreditBasketModel> getCreditBasketModel( const std::string& objectName, bool throwIfMissing )
	{
		auto creditBasketModel  = Environment::defaultEnv().accessObject<CreditBasketModel>(objectName);
		if ( !creditBasketModel && throwIfMissing )
		{
            { std::ostringstream aqObjMsg4; aqObjMsg4 << "CreditBasketModel " << objectName << " does not exist"; AQ_THROW( aqObjMsg4.str() ); }			
		}
		return creditBasketModel;
	}

	std::shared_ptr<BondCurve> getBondCurve( const std::string& objectName, bool throwIfMissing )
	{
		auto bondCurve  = Environment::defaultEnv().accessObject<BondCurve>(objectName);
		if ( !bondCurve && throwIfMissing )
		{
            { std::ostringstream aqObjMsg5; aqObjMsg5 << "BondCurve " << objectName << " does not exist"; AQ_THROW( aqObjMsg5.str() ); }			
		}
		return bondCurve;
	}

	std::shared_ptr<InflationCurve> getInflationCurve( const std::string& objectName, bool throwIfMissing )
	{
		auto inflationCurve = Environment::defaultEnv().accessObject<InflationCurve>(objectName);
		if ( ! inflationCurve && throwIfMissing )
		{
			{ std::ostringstream aqObjMsg6; aqObjMsg6 << "InflationCurve " << objectName << " does not exist"; AQ_THROW( aqObjMsg6.str() ); }
		}
		return inflationCurve;
	}
	
    bool doesAQObjExist( const std::string& objectName, const etrading::CachedObjectEnum& objectTypeEnum )
    {
        auto objectNames = etrading::Environment::defaultEnv().getObjectNames( objectTypeEnum );
        return ( etrading::getIndexOf( objectName, objectNames ) >= 0 );
    }

    bool doesAQObjExist( const std::string& objectName, const std::string& objectTypeAsString )
    {
        const etrading::CachedObjectEnum objectTypeEnum = etrading::toCachedObjectEnum( etrading::trim_to_upper( objectTypeAsString.c_str() ) );
        // Call the Enumerated Method
        return doesAQObjExist( objectName, objectTypeEnum );
    }

	LegPtr getLeg(const std::string& objectName)
	{
		auto leg  = Environment::defaultEnv().accessObject<Leg>(objectName);
		if(!leg)
		{
			{ std::ostringstream aqObjMsg7; aqObjMsg7 << "Leg " << objectName << " does not exist"; AQ_THROW( aqObjMsg7.str() ); }			
		}
		return leg;
	}

    BondPtr getBond( const std::string& objectName )
	{
		auto bond  = Environment::defaultEnv().accessObject< Bond >( objectName );
		if( !bond )
		{
			{ std::ostringstream aqObjMsg8; aqObjMsg8 << "Bond " << objectName << " does not exist"; AQ_THROW( aqObjMsg8.str() ); }			
		}
		return bond;
	}

	BondGeneratorPtr getBondGenerator( const std::string& objectName )
	{
		auto bondGen  = Environment::defaultEnv().accessObject<BondGenerator>( objectName );
		if ( ! bondGen )
		{
            { std::ostringstream aqObjMsg9; aqObjMsg9 << "Bond " << objectName << " does not exist"; AQ_THROW( aqObjMsg9.str() ); }			
		}
		return bondGen;
	}

  	SwapGeneratorPtr getSwapGenerator( const std::string& objectName )
	{
		auto swapGen  = Environment::defaultEnv().accessObject<SwapGenerator>( objectName );
		if(!swapGen)
		{
            { std::ostringstream aqObjMsg10; aqObjMsg10 << "Swap Generator '" << objectName << "' does not exist"; AQ_THROW( aqObjMsg10.str() ); }			
		}
		return swapGen;
	}

  	FixingTablePtr getFixingTable(const std::string& objectName, bool throwIfMissing )
	{
		auto fixingTable  = Environment::defaultEnv().accessObject<FixingTable>( objectName );
		if(!fixingTable && throwIfMissing)
		{
            { std::ostringstream aqObjMsg11; aqObjMsg11 << "Fixing Table " << objectName << " does not exist"; AQ_THROW( aqObjMsg11.str() ); }			
		}
		return fixingTable;
	}

	CurveGeneratorPtr getCurveGenerator( const std::string& objectName )
	{
		auto curveGenerator  = Environment::defaultEnv().accessObject<CurveGenerator>( objectName );
		if ( !curveGenerator )
		{
            { std::ostringstream aqObjMsg12; aqObjMsg12 << "CurveGenerator " << objectName << " does not exist"; AQ_THROW( aqObjMsg12.str() ); }			
		}
		return curveGenerator;
	}
	
	CurveMarketDataPtr getCurveMarketData( const std::string& objectName )
	{
		auto curveMarketData = Environment::defaultEnv().accessObject<CurveMarketData>( objectName );
		if ( !curveMarketData )
		{
            { std::ostringstream aqObjMsg13; aqObjMsg13 << "CurveMarketData " << objectName << " does not exist"; AQ_THROW( aqObjMsg13.str() ); }			
		}
		return curveMarketData;
	}

	FXCurvePtr getFxCurve( const std::string& objectName )
	{
		auto fxCurve = Environment::defaultEnv().accessObject<FXCurve>( objectName );
		if ( !fxCurve )
		{
            { std::ostringstream aqObjMsg14; aqObjMsg14 << "FXCurve " << objectName << " does not exist"; AQ_THROW( aqObjMsg14.str() ); }			
		}
		return fxCurve;
	}

	OptionPtr getOption(const std::string& objectName)
	{
		auto option = Environment::defaultEnv().accessObject<Option>(objectName);
		if (!option)
		{
			{ std::ostringstream aqObjMsg15; aqObjMsg15 << "Option " << objectName << " does not exist"; AQ_THROW( aqObjMsg15.str() ); }
		}
		return option;
	}

	VolatilityPtr getVolatility(const std::string& objectName, bool throwIfMissing)
	{
		auto vol = Environment::defaultEnv().accessObject<Volatility>(objectName);
		if (!vol && throwIfMissing)
		{
			{ std::ostringstream aqObjMsg16; aqObjMsg16 << "Volatility " << objectName << " does not exist"; AQ_THROW( aqObjMsg16.str() ); }
		}
		return vol;
	}

	std::shared_ptr<SabrModel> getSabrModel(const std::string& objectName, bool throwIfMissing)
	{
		auto sabrModel = Environment::defaultEnv().accessObject<SabrModel>(objectName);
		if (!sabrModel && throwIfMissing)
		{
			{ std::ostringstream aqObjMsg17; aqObjMsg17 << "SabrModel " << objectName << " does not exist"; AQ_THROW( aqObjMsg17.str() ); }
		}
		return sabrModel;
	}

	SabrMarketDataPtr getSabrMarketData(const std::string& objectName, bool throwIfMissing)
	{
		auto sabrMktData = Environment::defaultEnv().accessObject<SabrMarketData>(objectName);
		if (!sabrMktData && throwIfMissing)
		{
			{ std::ostringstream aqObjMsg18; aqObjMsg18 << "SabrMarketData " << objectName << " does not exist"; AQ_THROW( aqObjMsg18.str() ); }
		}
		return sabrMktData;
	}

    std::shared_ptr<FreeObject> getFreeObject(const std::string& objectName)
	{

		auto freeObject = Environment::defaultEnv().accessObject<FreeObject>(objectName);

		if (!freeObject)
		{
			{ std::ostringstream aqObjMsg19; aqObjMsg19 << "FreeObject " << objectName << " does not exist"; AQ_THROW( aqObjMsg19.str() ); }
		}
		return freeObject;
	}

	// Curve Object Methods
	// --------------------

    SingleCurveObjectPtr getSingleCurveObject( const std::string& objectName, const bool throwIfMissing )
	{
		auto curveObj = Environment::defaultEnv().accessObject<SingleCurveObject>(objectName);
		if (!curveObj && throwIfMissing )
		{
			AQ_THROW( "Curve Object " + objectName + " does not exist" )
		}
		return curveObj;
	}

	MultiCurveObjectPtr getMultiCurveObject( const std::string& objectName, const bool throwIfMissing )
	{
		auto curveObj = Environment::defaultEnv().accessObject<MultiCurveObject>(objectName);
		if (!curveObj && throwIfMissing )
		{
			AQ_THROW( "Curve Object " + objectName + " does not exist" )
		}
		return curveObj;
	}

	DualBootstrappedCurveObjectPtr getDualCurveObject( const std::string& objectName, const bool throwIfMissing )
	{
		auto curveObj = Environment::defaultEnv().accessObject<DualBootstrappedCurveObject>(objectName);
		if (!curveObj && throwIfMissing )
		{
			AQ_THROW( "Curve Object " + objectName + " does not exist" )
		}
		return curveObj;
	}

	// Handle Methods
	// ----------------
	std::string undecorateHandle( const std::string & handle )
	{
		// Token position i.e. position of 'colon'
		size_t tokenPosition = handle.find(":");

		// Performance
		if ( handle.size() == 0 || tokenPosition == std::string::npos )
		{
			return handle;
		}

		// Remove Instance Counter
		const std::string handleWithoutInstanceCounter = handle.substr( 0, tokenPosition );
		return handleWithoutInstanceCounter;
	}

	// Method to Get a Curve Collection from a Curve Handle or Curve Collection
	// We can optionally throw if the input is a curveCollection string
	// Object methods accept handles only, but we need to be backward compatible for legacy methods which require curve collection strings
	std::string getCurveCollectionFromHandle( const std::string & curveName, const bool allowCurveCollections )
	{
		const std::string curveHandleWithoutInstanceCounter = undecorateHandle( curveName );

		// 1) Curve Group Object
		// --------------------------
		std::string curveGroupNameUppercase = curveHandleWithoutInstanceCounter;
		boost::to_upper( curveGroupNameUppercase ); 
		
		const bool isCurveGroup = doesExistCurveGroup( curveGroupNameUppercase );
		if( isCurveGroup )
		{
			const std::string curveCollection = etrading::CurveResultsContainer::getInstance().getCurveResults( curveGroupNameUppercase, "" )->curveGroup()->curveCollection();
			return curveCollection;
		}
		
		// 2) Single Curve Handle
		// --------------------------
		const std::shared_ptr<etrading::SingleCurveObject> singleCurveObject = etrading::getSingleCurveObject( curveHandleWithoutInstanceCounter, false ); // false = throwIfMissing
		if ( singleCurveObject != nullptr )
		{
			const std::string curveCollection = singleCurveObject->getCurveCollection();
			return curveCollection;
		}

		// 3) Multi Curve Handle
		// --------------------------
		const std::shared_ptr<etrading::MultiCurveObject> multiCurveObject = etrading::getMultiCurveObject( curveHandleWithoutInstanceCounter, false ); // false = throwIfMissing
		if ( multiCurveObject != nullptr )
		{
			const std::string curveCollection = multiCurveObject->getCurveCollection();
			return curveCollection;
		}

		// 4) Dual Curve Handle
		// --------------------------
		const std::shared_ptr<etrading::DualBootstrappedCurveObject> dualCurveObject = etrading::getDualCurveObject( curveHandleWithoutInstanceCounter, false ); // false = throwIfMissing
		if ( dualCurveObject != nullptr )
		{
			const std::string curveCollection = dualCurveObject->getCurveCollection();
			return curveCollection;
		}

		// 4) Assume a Curve Collection and not a curve Handle
		AQ_REQUIRE( allowCurveCollections, "Invalid Curve Object: " + curveName + " is not a curve handle. Hint: CurveCollection strings are not accepted in this method.")
		return curveName;
	}

}

