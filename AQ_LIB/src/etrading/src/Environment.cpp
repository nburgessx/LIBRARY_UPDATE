//
// @File: Environment.cpp
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.



#include "Environment.h"
#include "ObjectUtilities.h"
#include "SerializationUtilities.h"


namespace etrading
{
    /*static*/ std::string Environment::DEFAULT_ENV_NAME;  // non-const because assigned in the Environment::initializer_ variable;
    /*static*/ std::vector<CachedObjectEnum> Environment::STORED_TYPES;  // non-const because assigned in the Environment::initializer_ variable;
    /*static*/ std::map<CachedObjectEnum, environment_implementation::CachedObjectFunctionSet> Environment::functionsForCachedObjectMap_;

    /*static*/ const Environment::Init Environment::initializer_;  // will generate the default environment, will initialize the hasMap

    /*static*/ Environment& Environment::defaultEnv()
    {
        auto& env = ( *EnvironmentPool::getInstance().getEnvironment( Environment::DEFAULT_ENV_NAME ).get() );
        return env;
    }

    /*static*/ const char* Environment::CACHED_OBJECT_TYPE = "CACHED_OBJECT_TYPE";


    Environment::Environment( const std::string& uid ) : _uid( uid )
    {}

    const std::string& Environment::getUID() const
    {
        return _uid;
    }

    std::pair<std::string, CachedObjectEnum> Environment::deSerializeFromJSON( const serialize::SerializationTargetEnum targetType,
                                                                               const std::string& jsonStringOrFileName )
    {
        // 

        // Ensure the file extension is included in the targetInfo file name.
        // ------------------------------------------------------------------
        std::string filenameWithExtension = jsonStringOrFileName;
        if(targetType == serialize::FILE )
        {   
            etrading::appendFileExtension( filenameWithExtension, etrading::JSON );
        }
        
        rapidjson::Document jsonDoc = ( targetType == serialize::FILE ) ? createJSONFromFile( filenameWithExtension ) : createJSONFromString( filenameWithExtension );

		return deSerializeJSONDoc( jsonDoc );
	}

	/* @brief	Deserializes a parsed JSON Document (or nested sub-document) into a Lightweight Object
	*  @param[in]	jsonDoc		The parsed JSON document. This is represented as a "Value", to allow any sub-document in the tree to be processed
	*  @returns		A pair containing the deserialized object handle, and the object type enum.
	*/
	std::pair<std::string, CachedObjectEnum> Environment::deSerializeJSONDoc( const rapidjson::Value& jsonDoc )
	{
        checkObjectLabels(jsonDoc); // add check for "CACHED_OBJECT_TYPE"

        // get the enum
        CachedObjectEnum cachedObjectEnum = toCachedObjectEnum( jsonDoc["CACHED_OBJECT_TYPE"].GetString() );
        std::string objectName = jsonDoc[DataHolder::OBJECT_NAME_LABEL.c_str()].GetString();

        /*
        // In the underneath, simply calling:
        to_cached_object_type<cachedObjectEnum>::type resultObject =  createCacheableObject<cachedObjectEnum>( jsonDoc );
        moveToCache( this->_uid.c_str(), std::move(resultObject) );
        // is not possible because one cannot use a dynamic variable to create a template instance statically !!
        // and using decltype(cachedObjectEnum) would not call the specialization
		// this is because to_cached_object_type<cachedObjectEnum>::type is not a constant at compile time...
        */
        if( cachedObjectEnum == FREE_OBJECT )
        {
            auto resultObject = createCacheableObject<FREE_OBJECT>( jsonDoc );
            moveToCache( std::move( resultObject ) , *this );
        }
        else if( cachedObjectEnum == TABLE )
        {
            auto resultObject = createCacheableObject<TABLE>( jsonDoc );
            moveToCache( std::move( resultObject ), *this );
        }
        else if( cachedObjectEnum == CURVE_DEPRECATED )
        {
            auto resultObject = createCacheableObject<CURVE_DEPRECATED>( jsonDoc );
            moveToCache( std::move( resultObject ) , *this );
        }
        else if( cachedObjectEnum == CURVE_BUILD_PROPERTIES )
        {
            auto resultObject = createCacheableObject<CURVE_BUILD_PROPERTIES>( jsonDoc );
            moveToCache( std::move( resultObject ), *this );
        }
        else if( cachedObjectEnum == EXAMPLE_STAND_ALONE )
        {
            auto resultObject = createCacheableObject<EXAMPLE_STAND_ALONE>( jsonDoc );
            moveToCache( std::move( resultObject ), *this );
        }
  		else if( cachedObjectEnum == SWAP_OBJECT )
        {
            // if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
            // this is to avoid slicing derived data 
            auto resultObject = createCacheableObject<SWAP_OBJECT>( jsonDoc );
			registerToCache<Swap>(resultObject);
        }
  		else if( cachedObjectEnum == SWAP_GENERATOR )
        {
            // if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
            // this is to avoid slicing derived data 
            auto resultObject = createCacheableObject<SWAP_GENERATOR>( jsonDoc );
			moveToCache( std::move( resultObject ), *this );
        }
		else if( cachedObjectEnum == SCHEDULE || cachedObjectEnum == FEE_SCHEDULE)
        {
            // if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
            // this is to avoid slicing derived data 
            auto resultObject = createCacheableObject<SCHEDULE>( jsonDoc );
			registerToCache<Schedule>(resultObject);
        }
		else if( cachedObjectEnum == LEG )
        {
            // if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
            // this is to avoid slicing derived data 
            auto resultObject = createCacheableObject<LEG>( jsonDoc );
			registerToCache<Leg>(resultObject);
        }
        else if( cachedObjectEnum == BOND )
        {
            // if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
            // this is to avoid slicing derived data 
            auto resultObject = createCacheableObject<BOND>( jsonDoc );
			registerToCache<Bond>(resultObject);
        }
		else if( cachedObjectEnum == BOND_GENERATOR )
        {
            // if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
            // this is to avoid slicing derived data 
            auto resultObject = createCacheableObject<BOND_GENERATOR>( jsonDoc );
			moveToCache( std::move( resultObject ), *this );
        }
		else if( cachedObjectEnum == CURVE_DATA )
        {
            auto resultObject = createCacheableObject<CURVE_DATA>( jsonDoc );
			moveToCache<CurveData>(std::move(resultObject));
        }
        else if( cachedObjectEnum == FIXING_TABLE )
        {
            auto resultObject = createCacheableObject<FIXING_TABLE>( jsonDoc );
			moveToCache<FixingTable>(std::move(resultObject));
        }
		else if (cachedObjectEnum == CURVE_GENERATOR )
		{
            auto resultObject = createCacheableObject<CURVE_GENERATOR>( jsonDoc );
			moveToCache<CurveGenerator>(std::move(resultObject));
		}
		else if (cachedObjectEnum == CURVE_MARKETDATA )
		{
            auto resultObject = createCacheableObject<CURVE_MARKETDATA>( jsonDoc );
			moveToCache<CurveMarketData>(std::move(resultObject));
		}
		else if (cachedObjectEnum == CURVE )
		{
            auto resultObject = createCacheableObject<CURVE>( jsonDoc );
			moveToCache<SingleCurveObject>(std::move(resultObject));
		}
		else if (cachedObjectEnum == MULTICURVE)
		{
			auto resultObject = createCacheableObject<MULTICURVE>(jsonDoc);
			moveToCache<MultiCurveObject>(std::move(resultObject));
		}
		else if (cachedObjectEnum == OPTION)
		{
			// if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
			// this is to avoid slicing derived data 
			auto resultObject = createCacheableObject<OPTION>(jsonDoc);
			registerToCache<Option>(resultObject);
		}
		else if (cachedObjectEnum == VOLATILITY)
		{
			// if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
			// this is to avoid slicing derived data 
			auto resultObject = createCacheableObject<VOLATILITY>(jsonDoc);
			registerToCache<Volatility>(resultObject);
		}
		else if (cachedObjectEnum == CREDIT_MODEL)
		{
			// if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
			// this is to avoid slicing derived data 
			auto resultObject = createCacheableObject<CREDIT_MODEL>(jsonDoc);
			moveToCache<CreditModel>(std::move(resultObject));
		}
		else if (cachedObjectEnum == CREDIT_BASKET_MODEL)
		{
			// if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
			// this is to avoid slicing derived data 
			auto resultObject = createCacheableObject<CREDIT_BASKET_MODEL>(jsonDoc);
			moveToCache<CreditBasketModel>(std::move(resultObject));
		}
		else if (cachedObjectEnum == BOND_CURVE)
		{
			// if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
			// this is to avoid slicing derived data 
			auto resultObject = createCacheableObject<BOND_CURVE>(jsonDoc);
			moveToCache<BondCurve>(std::move(resultObject));
		}
		else if (cachedObjectEnum == INFLATION_CURVE)
		{
		// if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
		// this is to avoid slicing derived data 
		auto resultObject = createCacheableObject<INFLATION_CURVE>(jsonDoc);
		moveToCache<InflationCurve>( std::move(resultObject) );
		}
		else if (cachedObjectEnum == SABR_MODEL)
		{
			// if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
			// this is to avoid slicing derived data 
			auto resultObject = createCacheableObject<SABR_MODEL>(jsonDoc);
			moveToCache<SabrModel>(std::move(resultObject));
		}
		else if (cachedObjectEnum == SABR_MARKETDATA)
		{
			// if the Cache holds base class pointers we use registerToCache instead of moveToCache or copyToCache
			// this is to avoid slicing derived data 
			auto resultObject = createCacheableObject<SABR_MARKETDATA>(jsonDoc);
			moveToCache<SabrMarketData>(std::move(resultObject));
		}
		else
        {
            throw ETradingException( "De-serialize unsupported object from JSON - deSerializeFromJSON" );
        }
        /*
        // TODO: attempt this implementation later (cfr. ObjectUtilities.h)
        if(etrading::cachePlacementFunction.find(cachedObjectEnum)  != etrading::cachePlacementFunction.end())
        {
        	etrading::cachePlacementFunction.at(cachedObjectEnum)(jsonDoc , this->_uid);
        }  else
        {
            throw ETradingException((boost::format("Attempt to de-serialize an unsupported object type (enumarition: %s) from JSON with input from %s- Environment::deSerializeFromJSON")
        												% toString(cachedObjectEnum) % jsonStringOrFileName).str());
        }
        */
        return std::make_pair( objectName, cachedObjectEnum );
	}

    const bool Environment::hasObject( const std::string& objectName, const CachedObjectEnum objectType )
    {
        if(Environment::functionsForCachedObjectMap_.count(objectType) == 1)
        {
            auto& functionSet = Environment::functionsForCachedObjectMap_[objectType];
            return ( std::get<0>( functionSet ) )( objectName, ( *this ) );
        } 
        else 
        {
            throw etrading::ETradingException( ( boost::format("Objects of type '%s' are not handled in the cache") %  toString(objectType) ).str() ) ;
        }
    }

    const std::vector<std::string> Environment::getObjectNames( const CachedObjectEnum objectType )
    {
        if(Environment::functionsForCachedObjectMap_.count(objectType) == 1)
        {
            auto& functionSet = Environment::functionsForCachedObjectMap_[objectType];
            return ( std::get<1>( functionSet ) )( ( *this ) );
        } 
        else 
        {
            throw etrading::ETradingException( ( boost::format("Objects of type '%s' are not handled in the cache") %  toString(objectType) ).str() ) ;
        }
    }

    const bool Environment::deleteObject( const std::string& objectName, const CachedObjectEnum objectType )
    {
        if(Environment::functionsForCachedObjectMap_.count(objectType) == 1)
        {
            auto& functionSet = Environment::functionsForCachedObjectMap_[objectType];
            return ( std::get<2>( functionSet ) )( objectName, ( *this ) );
        } 
        else 
        {
            throw etrading::ETradingException( ( boost::format("Objects of type '%s' are not handled in the cache") %  toString(objectType) ).str() ) ;
        }
    }

    const unsigned int Environment::deleteAllObjects( const CachedObjectEnum objectType )
    {
        if(Environment::functionsForCachedObjectMap_.count(objectType) == 1)
        {
            auto& functionSet = Environment::functionsForCachedObjectMap_[objectType];
            return ( std::get<3>( functionSet ) )( ( *this ) );
        } 
        else 
        {
            throw etrading::ETradingException( ( boost::format("Objects of type '%s' are not handled in the cache") %  toString(objectType) ).str() ) ;
        }
    }

	std::shared_ptr<IsLWOObject> Environment::accessObjectInterface( const std::string& objectName, const CachedObjectEnum objectType )
	{
        if(Environment::functionsForCachedObjectMap_.count(objectType) == 1)
        {
            auto& functionSet = Environment::functionsForCachedObjectMap_[objectType];
            return ( std::get<4>( functionSet ) )( objectName, ( *this ) );
        } 
        else 
        {
            throw etrading::ETradingException( ( boost::format("Objects of type '%s' are not handled in the cache") %  toString(objectType) ).str() ) ;
        }
	}
		
	// Template Specializations
	// ------------------------

    // specialization for LWOCurve, this delegates to 
    // const unsigned int environment_implementation::eraseAllObjects<LWOCurve>( Environment& env )
    // required because we need to clear the corresponding EntityPool Curve
    template <>
    const unsigned int Environment::deleteAllObjects<LWOCurve>()
    {
        return deleteAllObjects(CURVE_DEPRECATED); // needs to call the deletion of the object pool ...
    };

	template <>
    const bool Environment::deleteObject<LWOCurve>( const std::string& objectName )
    {
        return deleteObject(objectName, CURVE_DEPRECATED); // needs to call the deletion of the object pool ...
    };
}
