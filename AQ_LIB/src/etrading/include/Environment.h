#pragma once

//
// @Description: This is a test program


#include <vector>
#include <memory>
#include <map>
#include <algorithm>
#include <string>
#include <utility>
#include <iostream>
#include <tuple>

#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/fusion/adapted/std_pair.hpp>  // make sure we use the boost adaptation of the std::pair ...
#include <boost/fusion/include/std_pair.hpp>
#include <boost/assign.hpp>

#include "Cacheable.h"
#include "DataSchema.h"
#include "CommonConstants.h"
#include "CoreEnumerations.h"
#include "ETradingException.h"
#include "ConcurrentMap.h"
#include "FreeObject.h"
#include "TableDateDouble.h"
#include "CurveBuildProperties.h"
#include "AQObjCurve.h"
#include "SerializeContainedData.h"
#include "StaticStructureStore.h"
#include "EnvironmentPool.h"
#include "TypeName.h"
#include "UserUtilities.h"
#include "ExampleObjects.h"
#include "Bond.h"
#include "BondGenerator.h"
#include "Swap.h"
#include "SwapGenerator.h"
#include "Schedule.h"
#include "CurveData.h"
#include "EnvironmentImplementation.h"
#include "FixingTable.h"
#include "CurveGenerator.h"
#include "CurveMarketData.h"
#include "SingleCurveObject.h"
#include "MultiCurveObject.h"
#include "DualBootstrappedCurveObject.h"
#include "FXCurve.h"
#include "Option.h"
#include "Volatility.h"
#include "CreditModel.h"
#include "CreditBasketModel.h"
#include "BondCurves.h"
#include "InflationCurve.h"
#include "SabrModel.h"
#include "SabrMarketData.h"


namespace etrading
{
    class Environment;

    template <typename T>
    environment_implementation::CachedObjectFunctionSet functionTupleForAQObj()
    {
        return std::make_tuple(
                   &environment_implementation::has<T>,
                   &environment_implementation::keys<T>,
                   &environment_implementation::eraseObject<T>,
                   &environment_implementation::eraseAllObjects<T>,
				   &environment_implementation::accessObjectInterface<T>) ;
    };

    // uses clone method if available, else the copy CTOR
    template < typename T>
    std::shared_ptr<T> copySharedPtrToSharedPtr(const std::shared_ptr<T>& instance)
    {
        if(instance != nullptr)
        {
            environment_implementation::GetCopyImpl<T,environment_implementation::has_clone<T>::value> copier;
            std::shared_ptr<T> retPtr = copier(*(instance.get()));
            return retPtr;
        }
        return std::shared_ptr<T>();
    };

    // this class encapsulates the data containment of a specific type
    // within the Enviroment  class (i.e. in is the map for each object type)
    template<typename T>
    struct HasObjectsOfType
    {
    protected:
        ConcurrentMap<std::string, std::shared_ptr<T>>& retrieveInternalMap() 
        {
            return objectMap_;
        }
        ConcurrentMap<std::string, std::shared_ptr<T>> objectMap_;
    };

    // @Environment : a class that represents the context of an individual client
    class Environment : public HasObjectsOfType<FreeObject>,
                        public HasObjectsOfType<TableDateDouble>,
                        public HasObjectsOfType<AQObjCurve>,
                        public HasObjectsOfType<CurveBuildProperties>,
                        public HasObjectsOfType<StandAlone>,
                        public HasObjectsOfType<BaseObject>,
                        public HasObjectsOfType<Swap>,
                        public HasObjectsOfType<SwapGenerator>,
                        public HasObjectsOfType<Schedule>,
                        public HasObjectsOfType<Leg>,
                        public HasObjectsOfType<Bond>,
		                public HasObjectsOfType<CurveData>,
                        public HasObjectsOfType<FixingTable>,
		                public HasObjectsOfType<CurveGenerator>,
		                public HasObjectsOfType<CurveMarketData>,
						public HasObjectsOfType<SingleCurveObject>,
		                public HasObjectsOfType<FXCurve>,
                        public HasObjectsOfType<BondGenerator>,
						public HasObjectsOfType<Option>,
						public HasObjectsOfType<Volatility>,
						public HasObjectsOfType<MultiCurveObject>,
						public HasObjectsOfType<DualBootstrappedCurveObject>,
						public HasObjectsOfType<CreditModel>,
						public HasObjectsOfType<CreditBasketModel>,
						public HasObjectsOfType<BondCurve>,
						public HasObjectsOfType<InflationCurve>,
						public HasObjectsOfType<SabrModel>,
						public HasObjectsOfType<SabrMarketData>
	{
    public:
        static std::string DEFAULT_ENV_NAME;
        static std::vector<CachedObjectEnum> STORED_TYPES;
    private:
        static std::map<CachedObjectEnum, environment_implementation::CachedObjectFunctionSet> functionsForCachedObjectMap_;
        static struct Init
        {
            Init()
            {
                DEFAULT_ENV_NAME = etrading::EnvironmentPool::getInstance().getDefaultEnvironmentName();

                if( !etrading::EnvironmentPool::getInstance().isPresent( DEFAULT_ENV_NAME ) )
                {
                    etrading::EnvironmentPool::getInstance().createEnvironment( DEFAULT_ENV_NAME );
                }
                
				STORED_TYPES = { TABLE,
                                 FREE_OBJECT,
                                 CURVE_DEPRECATED,
                                 CURVE_BUILD_PROPERTIES,
                                 EXAMPLE_STAND_ALONE,
                                 EXAMPLE_BASE,
                                 SWAP_OBJECT,
                                 SWAP_GENERATOR,
                                 SCHEDULE,
                                 LEG,
                                 BOND,
                                 CURVE_DATA,
                                 FIXING_TABLE,
							     CURVE_GENERATOR,
							     CURVE_MARKETDATA,
								 CURVE,
								 MULTICURVE,
                                 FX_CURVE,
	                             BOND_GENERATOR,
								 OPTION,
								 VOLATILITY,
								 CREDIT_MODEL,
								 CREDIT_BASKET_MODEL,
								 BOND_CURVE,
								 INFLATION_CURVE,
								 SABR_MODEL,
								 SABR_MARKETDATA};

                functionsForCachedObjectMap_[CURVE_DEPRECATED]          = functionTupleForAQObj<AQObjCurve>();
                functionsForCachedObjectMap_[TABLE]                     = functionTupleForAQObj<TableDateDouble>();
                functionsForCachedObjectMap_[CURVE_BUILD_PROPERTIES]    = functionTupleForAQObj<CurveBuildProperties>();
                functionsForCachedObjectMap_[FREE_OBJECT]               = functionTupleForAQObj<FreeObject>();
                functionsForCachedObjectMap_[EXAMPLE_STAND_ALONE]       = functionTupleForAQObj<StandAlone>();
                functionsForCachedObjectMap_[EXAMPLE_BASE]              = functionTupleForAQObj<BaseObject>();
                functionsForCachedObjectMap_[SWAP_OBJECT]               = functionTupleForAQObj<Swap>(); 
                functionsForCachedObjectMap_[SWAP_GENERATOR]            = functionTupleForAQObj<SwapGenerator>(); 
				functionsForCachedObjectMap_[SCHEDULE]                  = functionTupleForAQObj<Schedule>(); 
				functionsForCachedObjectMap_[LEG]                       = functionTupleForAQObj<Leg>(); 
                functionsForCachedObjectMap_[BOND]                      = functionTupleForAQObj<Bond>(); 
				functionsForCachedObjectMap_[CURVE_DATA]                = functionTupleForAQObj<CurveData>(); 
                functionsForCachedObjectMap_[FIXING_TABLE]              = functionTupleForAQObj<FixingTable>(); 
				functionsForCachedObjectMap_[CURVE_GENERATOR]           = functionTupleForAQObj<CurveGenerator>();
				functionsForCachedObjectMap_[CURVE_MARKETDATA]          = functionTupleForAQObj<CurveMarketData>();
				functionsForCachedObjectMap_[CURVE]                     = functionTupleForAQObj<SingleCurveObject>();
				functionsForCachedObjectMap_[MULTICURVE]				= functionTupleForAQObj<MultiCurveObject>();
				functionsForCachedObjectMap_[FX_CURVE]                  = functionTupleForAQObj<FXCurve>();
				functionsForCachedObjectMap_[BOND_GENERATOR]            = functionTupleForAQObj<BondGenerator>();
				functionsForCachedObjectMap_[OPTION]					= functionTupleForAQObj<Option>();
				functionsForCachedObjectMap_[VOLATILITY]				= functionTupleForAQObj<Volatility>();
				functionsForCachedObjectMap_[CREDIT_MODEL]				= functionTupleForAQObj<CreditModel>();
				functionsForCachedObjectMap_[CREDIT_BASKET_MODEL]		= functionTupleForAQObj<CreditBasketModel>();
				functionsForCachedObjectMap_[BOND_CURVE]				= functionTupleForAQObj<BondCurve>();
				functionsForCachedObjectMap_[INFLATION_CURVE]			= functionTupleForAQObj<InflationCurve>();
				functionsForCachedObjectMap_[SABR_MODEL]				= functionTupleForAQObj<SabrModel>();
				functionsForCachedObjectMap_[SABR_MARKETDATA]		    = functionTupleForAQObj<SabrMarketData>();
			};
        } const initializer_;
    public:
        static Environment& defaultEnv();

        const std::string& getUID() const;

        // TODO : set the default template argument "class BaseTypeToReturn = UnderlyingType" as soon as C++11 is available
        template<class UnderlyingType, class BaseTypeToReturn>
        std::shared_ptr<BaseTypeToReturn> createObject( const std::string& objectName )
        {
            BOOST_STATIC_ASSERT( boost::is_base_of<BaseTypeToReturn, UnderlyingType>::value );
            BOOST_STATIC_ASSERT( boost::is_base_of<Cacheable, UnderlyingType>::value );
            auto& objectStore = this->getCache<UnderlyingType>();
            objectStore.set( objectName, std::make_shared<UnderlyingType>( UnderlyingType( objectName ) ) );
			return ( objectStore.has( objectName ) ) ? objectStore.get( objectName ) : std::shared_ptr<BaseTypeToReturn>();
        };

        template <typename Z>
        ConcurrentMap<std::string, std::shared_ptr<Z>>& getCache()
        {
            return HasObjectsOfType<Z>::retrieveInternalMap(); 
        };
        
        // "DELETE ALL" methods
        // deleting all objects corresponding to a specific Cached Enum
        const unsigned int deleteAllObjects( const CachedObjectEnum objectType );
        // deleting all objects corresponding to class held in the shared_ptrs
        template <typename Z>
        const unsigned int deleteAllObjects()
        {
            auto& objectStore = this->getCache<Z>();
            const unsigned int numberOfObjects = objectStore.size();
            objectStore.clear();
            return numberOfObjects;
        };

        // "DELETE" methods
        const bool deleteObject( const std::string& objectName, const CachedObjectEnum objectType );
        template <typename Z>
        const bool deleteObject( const std::string& objectName )
        {
            auto& objectStore = this->getCache<Z>();
            if( objectStore.has( objectName ) )
            {
                objectStore.erase( objectName );
            };
            return objectStore.has( objectName );
        };

        // "HAS" methods
        const bool hasObject( const std::string& objectName, const CachedObjectEnum objectType );
        template <typename Z>
        const bool hasObject( const std::string& objectName )
        {
            auto& objectStore = this->getCache<Z>();
            return objectStore.has( objectName );
        };

        // "GET" methods
        const std::vector<std::string> getObjectNames( const CachedObjectEnum objectType );
        template <typename Z>
        const std::vector<std::string> getObjectNames()
        {
            return getCache<Z>().keys();
        };

        // "COPY"
        template <typename Z>
        Z copyObject( const std::string& objectName )
        {
            auto& objectStore = this->getCache<Z>();
            if( objectStore.has( objectName ) )
            {
                return std::move( Z( *objectStore.get( objectName ) ) );
            }
            else
            {
                { std::ostringstream aqMsg10;
aqMsg10 << "Environment " << getUID().c_str() << " does not contain an object named " << objectName.c_str() << " in its Cache for " << TypeName::get<Z>().c_str(); AQ_THROW( aqMsg10.str() ); }
            }
        };

        // "ACCESS"
        template <typename Z>
        std::shared_ptr<Z> accessObject( const std::string& objectName )
        {
			const std::string::size_type indexInString = objectName.find_last_of( AQOBJ_KEY::AQOBJ_OBJECT_COUNTER_DELIMITER );
			const std::string objectNameWithoutCounter = indexInString == std::string::npos ? objectName :  objectName.substr( 0, indexInString );

            auto& objectStore = this->getCache<Z>();
            if( objectStore.has( objectNameWithoutCounter ) )
            {
                return objectStore.get( objectNameWithoutCounter );
            }
            else
            {
				return std::shared_ptr<Z>(); // nullptr for Linux
            }
        };
		std::shared_ptr<IsAQObject> accessObjectInterface( const std::string& objectName, const CachedObjectEnum objectType );
        template <typename Z>
		std::shared_ptr<IsAQObject> accessObjectInterface( const std::string& objectName )
		{
            auto& objectStore = this->getCache<Z>();
            if( objectStore.has( objectName ) )
            {
				return std::static_pointer_cast<IsAQObject>(objectStore.get( objectName ));
            }
            else
            {
				return std::shared_ptr<IsAQObject>(); // nullptr for Linux
            }
		};

        // jsonStringOrFileName: the file name if targetType is etrading::FILE 
        // jsonStringOrFileName: the input JSON string if targetType is etrading::STRING
        std::pair<std::string, CachedObjectEnum>
        deSerializeFromJSON(	const serialize::SerializationTargetEnum targetType, const std::string& jsonStringOrFileName );

		/* @brief		Deserializes a parsed JSON Document (or nested sub-document) into a Lightweight Object
		*  @param[in]	jsonDoc		The parsed JSON document. This is represented as a "Value", to allow any sub-document in the tree to be processed
		*  @returns		A pair containing the deserialized object handle, and the object type enum.
		*/
		std::pair<std::string, CachedObjectEnum> deSerializeJSONDoc( const rapidjson::Value& jsonDoc );

    private:
        Environment( const std::string& uid );
        friend EnvironmentPool;
        const std::string _uid;

        static const char* CACHED_OBJECT_TYPE;

    };

    /*
    	example usage:
    	targetType = FILE , jsonStringOrFileName = "filename.json"
    	targetType = STRING, jsonStringOrFileName = "{ actual JSON }"
    */
    inline std::pair<std::string, CachedObjectEnum>
    deSerializeFromJSON(	const serialize::SerializationTargetEnum targetType,
                            const std::string& jsonStringOrFileName,
                            Environment& env = Environment::defaultEnv() )
    {
        
        return env.deSerializeFromJSON( targetType, jsonStringOrFileName );
    };

	// Template Specializations
	// ------------------------

    // specialization for AQObjCurve, this delegates to 
    // const unsigned int environment_implementation::eraseAllObjects<AQObjCurve>( Environment& env )
    // required because we need to clear the corresponding EntityPool Curve
    template <>
    const unsigned int Environment::deleteAllObjects<AQObjCurve>();

	template <>
    const bool Environment::deleteObject<AQObjCurve>( const std::string& objectName );


    // Place implementations here, to avoid problem with incomplete types in EnvironmentImplementation.h
    namespace environment_implementation
    {
        template<typename T>
        bool has( const std::string& objectName, Environment& env )
        {
            auto& store = env.getCache<T>();
            return store.has( objectName );
        };

        template<typename T>
        std::vector<std::string> keys( Environment& env )
        {
            return env.getCache<T>().keys();
        };

        template<typename T>
        bool eraseObject( const std::string& objectName, Environment& env )
        {
            auto& store = env.getCache<T>();
            if( store.has( objectName ) )
            {
                store.erase( objectName );
            }
            return store.has( objectName );
        };

        template<typename T>
        const unsigned int eraseAllObjects( Environment& env )
        {
            auto& store = env.getCache<T>();
            auto numberOfObjects = store.size();
            store.clear();
            return numberOfObjects;
        };

	    template<typename T>
        std::shared_ptr<IsAQObject> accessObjectInterface( const std::string& objectName,  Environment& env )
	    {
	        return env.accessObjectInterface<T>(objectName);
	    }

    }


};
