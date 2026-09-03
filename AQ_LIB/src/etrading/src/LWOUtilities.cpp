

#include "LWOUtilities.h"
#include "Environment.h"
#include "CurveResultsContainer.h"	// Curve Group & Curve Results Logic

namespace etrading
{

	std::shared_ptr<Swap> getSwap(const std::string& objectName)
	{
		auto swap  = Environment::defaultEnv().accessObject<Swap>(objectName);
		if(!swap)
		{
            AQ_THROW( (boost::format("Swap %s does not exist") % objectName.c_str()).str().c_str() );			
		}
		return swap;
	}

	std::shared_ptr<Schedule> getSchedule(const std::string& objectName)
	{
		auto schedule  = Environment::defaultEnv().accessObject<Schedule>(objectName);
		if(!schedule)
		{
            AQ_THROW( (boost::format("Schedule %s does not exist") % objectName.c_str()).str().c_str() );			
		}
		return schedule;
	}

	std::shared_ptr<CreditModel> getCreditModel( const std::string& objectName, bool throwIfMissing )
	{
		auto creditModel  = Environment::defaultEnv().accessObject<CreditModel>(objectName);
		if ( !creditModel && throwIfMissing )
		{
            AQ_THROW( (boost::format("CreditModel %s does not exist") % objectName.c_str()).str().c_str() );			
		}
		return creditModel;
	}

	std::shared_ptr<CreditBasketModel> getCreditBasketModel( const std::string& objectName, bool throwIfMissing )
	{
		auto creditBasketModel  = Environment::defaultEnv().accessObject<CreditBasketModel>(objectName);
		if ( !creditBasketModel && throwIfMissing )
		{
            AQ_THROW( (boost::format("CreditBasketModel %s does not exist") % objectName.c_str()).str().c_str() );			
		}
		return creditBasketModel;
	}

	std::shared_ptr<BondCurve> getBondCurve( const std::string& objectName, bool throwIfMissing )
	{
		auto bondCurve  = Environment::defaultEnv().accessObject<BondCurve>(objectName);
		if ( !bondCurve && throwIfMissing )
		{
            AQ_THROW( (boost::format("BondCurve %s does not exist") % objectName.c_str()).str().c_str() );			
		}
		return bondCurve;
	}

	std::shared_ptr<InflationCurve> getInflationCurve( const std::string& objectName, bool throwIfMissing )
	{
		auto inflationCurve = Environment::defaultEnv().accessObject<InflationCurve>(objectName);
		if ( ! inflationCurve && throwIfMissing )
		{
			AQ_THROW( (boost::format("InflationCurve %s does not exist") % objectName.c_str()).str().c_str() );
		}
		return inflationCurve;
	}
	
    bool doesLWOExist( const std::string& objectName, const etrading::CachedObjectEnum& objectTypeEnum )
    {
        auto objectNames = etrading::Environment::defaultEnv().getObjectNames( objectTypeEnum );
        return ( etrading::getIndexOf( objectName, objectNames ) >= 0 );
    }

    bool doesLWOExist( const std::string& objectName, const std::string& objectTypeAsString )
    {
        const etrading::CachedObjectEnum objectTypeEnum = etrading::toCachedObjectEnum( etrading::trim_to_upper( objectTypeAsString.c_str() ) );
        // Call the Enumerated Method
        return doesLWOExist( objectName, objectTypeEnum );
    }

	LegPtr getLeg(const std::string& objectName)
	{
		auto leg  = Environment::defaultEnv().accessObject<Leg>(objectName);
		if(!leg)
		{
			AQ_THROW( (boost::format("Leg %s does not exist") % objectName.c_str()).str().c_str() );			
		}
		return leg;
	}

    BondPtr getBond( const std::string& objectName )
	{
		auto bond  = Environment::defaultEnv().accessObject< Bond >( objectName );
		if( !bond )
		{
			AQ_THROW( (boost::format("Bond %s does not exist") % objectName.c_str()).str().c_str() );			
		}
		return bond;
	}

	BondGeneratorPtr getBondGenerator( const std::string& objectName )
	{
		auto bondGen  = Environment::defaultEnv().accessObject<BondGenerator>( objectName );
		if ( ! bondGen )
		{
            AQ_THROW( (boost::format( "Bond %s does not exist") % objectName.c_str()).str().c_str() );			
		}
		return bondGen;
	}

  	SwapGeneratorPtr getSwapGenerator( const std::string& objectName )
	{
		auto swapGen  = Environment::defaultEnv().accessObject<SwapGenerator>( objectName );
		if(!swapGen)
		{
            AQ_THROW( (boost::format("Swap Generator '%s' does not exist") % objectName.c_str()).str().c_str() );			
		}
		return swapGen;
	}

  	FixingTablePtr getFixingTable(const std::string& objectName, bool throwIfMissing )
	{
		auto fixingTable  = Environment::defaultEnv().accessObject<FixingTable>( objectName );
		if(!fixingTable && throwIfMissing)
		{
            AQ_THROW( (boost::format("Fixing Table %s does not exist") % objectName.c_str()).str().c_str() );			
		}
		return fixingTable;
	}

	CurveGeneratorPtr getCurveGenerator( const std::string& objectName )
	{
		auto curveGenerator  = Environment::defaultEnv().accessObject<CurveGenerator>( objectName );
		if ( !curveGenerator )
		{
            AQ_THROW( (boost::format("CurveGenerator %s does not exist") % objectName.c_str()).str().c_str() );			
		}
		return curveGenerator;
	}
	
	CurveMarketDataPtr getCurveMarketData( const std::string& objectName )
	{
		auto curveMarketData = Environment::defaultEnv().accessObject<CurveMarketData>( objectName );
		if ( !curveMarketData )
		{
            AQ_THROW( (boost::format("CurveMarketData %s does not exist") % objectName.c_str()).str().c_str() );			
		}
		return curveMarketData;
	}

	FXCurvePtr getFxCurve( const std::string& objectName )
	{
		auto fxCurve = Environment::defaultEnv().accessObject<FXCurve>( objectName );
		if ( !fxCurve )
		{
            AQ_THROW( (boost::format( "FXCurve %s does not exist") % objectName.c_str()).str().c_str() );			
		}
		return fxCurve;
	}

	OptionPtr getOption(const std::string& objectName)
	{
		auto option = Environment::defaultEnv().accessObject<Option>(objectName);
		if (!option)
		{
			AQ_THROW((boost::format("Option %s does not exist") % objectName.c_str()).str().c_str());
		}
		return option;
	}

	VolatilityPtr getVolatility(const std::string& objectName, bool throwIfMissing)
	{
		auto vol = Environment::defaultEnv().accessObject<Volatility>(objectName);
		if (!vol && throwIfMissing)
		{
			AQ_THROW((boost::format("Volatility %s does not exist") % objectName.c_str()).str().c_str());
		}
		return vol;
	}

	std::shared_ptr<SabrModel> getSabrModel(const std::string& objectName, bool throwIfMissing)
	{
		auto sabrModel = Environment::defaultEnv().accessObject<SabrModel>(objectName);
		if (!sabrModel && throwIfMissing)
		{
			AQ_THROW((boost::format("SabrModel %s does not exist") % objectName.c_str()).str().c_str());
		}
		return sabrModel;
	}

	SabrMarketDataPtr getSabrMarketData(const std::string& objectName, bool throwIfMissing)
	{
		auto sabrMktData = Environment::defaultEnv().accessObject<SabrMarketData>(objectName);
		if (!sabrMktData && throwIfMissing)
		{
			AQ_THROW((boost::format("SabrMarketData %s does not exist") % objectName.c_str()).str().c_str());
		}
		return sabrMktData;
	}

    std::shared_ptr<FreeObject> getFreeObject(const std::string& objectName)
	{

		auto freeObject = Environment::defaultEnv().accessObject<FreeObject>(objectName);

		if (!freeObject)
		{
			AQ_THROW((boost::format("FreeObject %s does not exist") % objectName.c_str()).str().c_str());
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

