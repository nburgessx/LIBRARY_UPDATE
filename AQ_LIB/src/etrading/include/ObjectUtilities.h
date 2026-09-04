#pragma once

//
// @Description: This is a series of functions that help create/convert classes derived from CachedObject or access Objects from the Store (through perhaps other base pointers)

#include <vector>
#include <map>
#include <memory>
#include <utility>
#include <boost/date_time.hpp>
#include <boost/lexical_cast.hpp>
#include <rapidjson/document.h>

#include "CoreEnumerations.h"
#include "FreeObject.h"
#include "TableDateDouble.h"
#include "Variant.h"
#include "DataSchema.h"
#include "AQObjCurve.h"
#include "SerializeContainedData.h"
#include "ExampleObjects.h"
#include "Environment.h"
#include "SwapUtilities.h"
#include "CurveData.h"
#include "FixingTable.h"
#include "CurveGenerator.h"
#include "CurveMarketData.h"
#include "SingleCurveObject.h"
#include "MultiCurveObject.h"
#include "Bond.h"
#include "BondFactory.h"
#include "BondUtilities.h"
#include "Option.h"
#include "OptionFactory.h"
#include "Volatility.h"
#include "CreditModel.h"
#include "CreditBasketModel.h"
#include "BondCurves.h"
#include "InflationCurve.h"
#include "SabrModel.h"
#include "SabrMarketData.h"

using rapidjson::SizeType;
using rapidjson::Document;
using rapidjson::Value;
using rapidjson::StringRef;

namespace etrading
{
    const char* const DATASCHEMAS    = "STRUCTURED_KEYS";
	const char* const NESTED_OBJECTS = "NESTED_OBJECTS";

    template<CachedObjectEnum Z>
    struct to_cached_object_type
    { };

    template<>
    struct to_cached_object_type<TABLE>
    {
        typedef etrading::TableDateDouble type;
    };

    template<>
    struct to_cached_object_type<FREE_OBJECT>
    {
        typedef etrading::FreeObject type;
    };

    template<>
    struct to_cached_object_type<CURVE_DEPRECATED>
    {
        typedef etrading::AQObjCurve type;
    };

    template<>
    struct to_cached_object_type<CURVE_BUILD_PROPERTIES>
    {
        typedef etrading::CurveBuildProperties type;
    };

    template<>
    struct to_cached_object_type<EXAMPLE_STAND_ALONE>
    {
        typedef etrading::StandAlone type;
    };

    template<>
    struct to_cached_object_type<SWAP_OBJECT>
    {
        // note that for objects held through base pointer 
        // we need to return a pointer to a base class from
        // reading a JSON file (to avoid slicing when returning 
        // from the read)
        typedef std::shared_ptr<Swap> type;
    };

    template<>
    struct to_cached_object_type<SWAP_GENERATOR>
    {
        // note that for objects held through base pointer 
        // we need to return a pointer to a base class from
        // reading a JSON file (to avoid slicing when returning 
        // from the read)
        typedef SwapGenerator type;
    };

    template<>
    struct to_cached_object_type<LEG>
    {
        // note that for objects held through base pointer 
        // we need to return a pointer to a base class from
        // reading a JSON file (to avoid slicing when returning 
        // from the read)
        typedef std::shared_ptr<Leg> type;
    };

    template<>
    struct to_cached_object_type<BOND>
    {
        // note that for objects held through base pointer 
        // we need to return a pointer to a base class from
        // reading a JSON file (to avoid slicing when returning 
        // from the read)
        typedef std::shared_ptr<Bond> type;
    };

    template<>
	struct to_cached_object_type<BOND_GENERATOR>
    {
        // note that for objects held through base pointer 
        // we need to return a pointer to a base class from
        // reading a JSON file (to avoid slicing when returning 
        // from the read)
        typedef BondGenerator type;
    };

    template<>
    struct to_cached_object_type<SCHEDULE>
    {
        // note that for objects held through base pointer 
        // we need to return a pointer to a base class from
        // reading a JSON file (to avoid slicing when returning 
        // from the read)
        typedef std::shared_ptr<Schedule> type;
    };

	template<>
    struct to_cached_object_type<CURVE_DATA>
    {
        typedef etrading::CurveData type;
    };

    template<>
    struct to_cached_object_type<FIXING_TABLE>
    {
        typedef etrading::FixingTable type;
    };

	template<>
    struct to_cached_object_type<CURVE_GENERATOR>
    {
        typedef etrading::CurveGenerator type;
    };

	template<>
    struct to_cached_object_type<CURVE_MARKETDATA>
    {
        typedef etrading::CurveMarketData type;
    };
	template<>
    struct to_cached_object_type<CURVE>
    {
        typedef etrading::SingleCurveObject type;
    };
	template<>
	struct to_cached_object_type<MULTICURVE>
	{
		typedef etrading::MultiCurveObject type;
	};

	template<>
    struct to_cached_object_type<FX_CURVE>
    {
        typedef etrading::FXCurve type;
    };

	template<>
	struct to_cached_object_type<OPTION>
	{
		// note that for objects held through base pointer 
		// we need to return a pointer to a base class from
		// reading a JSON file (to avoid slicing when returning 
		// from the read)
		typedef std::shared_ptr<Option> type;
	};

	template<>
	struct to_cached_object_type<VOLATILITY>
	{
		// note that for objects held through base pointer 
		// we need to return a pointer to a base class from
		// reading a JSON file (to avoid slicing when returning 
		// from the read)
		typedef std::shared_ptr<Volatility> type;
	};

	template<>
	struct to_cached_object_type<CREDIT_MODEL>
	{
		// note that for objects held through base pointer 
		// we need to return a pointer to a base class from
		// reading a JSON file (to avoid slicing when returning 
		// from the read)
		typedef etrading::CreditModel type;
	};

	template<>
	struct to_cached_object_type<CREDIT_BASKET_MODEL>
	{
		// note that for objects held through base pointer 
		// we need to return a pointer to a base class from
		// reading a JSON file (to avoid slicing when returning 
		// from the read)
		typedef etrading::CreditBasketModel type;
	};
	
	template<>
	struct to_cached_object_type<BOND_CURVE>
	{
		// note that for objects held through base pointer 
		// we need to return a pointer to a base class from
		// reading a JSON file (to avoid slicing when returning 
		// from the read)
		typedef etrading::BondCurve type;
	};

	template<>
	struct to_cached_object_type<INFLATION_CURVE>
	{
		// note that for objects held through base pointer 
		// we need to return a pointer to a base class from
		// reading a JSON file (to avoid slicing when returning 
		// from the read)
		typedef etrading::InflationCurve type;
	};

	template<>
	struct to_cached_object_type<SABR_MODEL>
	{
		// note that for objects held through base pointer 
		// we need to return a pointer to a base class from
		// reading a JSON file (to avoid slicing when returning 
		// from the read)
		typedef etrading::SabrModel type;
	};

	template<>
	struct to_cached_object_type<SABR_MARKETDATA>
	{
		// note that for objects held through base pointer 
		// we need to return a pointer to a base class from
		// reading a JSON file (to avoid slicing when returning 
		// from the read)
		typedef etrading::SabrMarketData type;
	};

	FreeObject CreateFreeObject(const rapidjson::Value& jsonDoc );
	FreeObject createFreeObjectFromGrid( const std::string& objectName,
										 const std::vector<std::string>& columnNames,
										 const std::vector<ContainedTypeEnum>& colTypes,
										 const VariantMatrix& rangeData,
										 const std::string& schemaName,
										 const bool allowJaggedData );

    void checkObjectLabels(const rapidjson::Value& jsonDoc);
    void checkObjectLabel(const rapidjson::Value& jsonDoc, const char* objectLabelName );
    
    void checkArrayIndexLabel(  const rapidjson::Value& inputValue, 
                                const rapidjson::SizeType& minSize );
    
    void checkValidDataSchema( const rapidjson::Value& inputValue );
    const int memberCount(const rapidjson::Value& inputValue );


    template<class CachedObjectType>
    void copyToCache(	const CachedObjectType& cachedObjectInstance,
                        Environment& wrkContext = Environment::defaultEnv() )
    {
        ConcurrentMap<std::string, std::shared_ptr<CachedObjectType>>& objectStore = wrkContext.getCache<CachedObjectType>();
        objectStore.set( cachedObjectInstance.getRefToName(), std::make_shared<CachedObjectType>( cachedObjectInstance ) );
    };

    template<class CachedObjectType>
    void moveToCache(	CachedObjectType&& cachedObjectInstance ,
                        Environment& wrkContext = Environment::defaultEnv() )
    {
        ConcurrentMap<std::string, std::shared_ptr<CachedObjectType>>& objectStore = wrkContext.getCache<CachedObjectType>();
        objectStore.set( cachedObjectInstance.getRefToName(), std::make_shared<CachedObjectType>( std::ref( cachedObjectInstance ) ) );
    };

    template<class CachedObjectType>
    void registerToCache(	const std::shared_ptr<CachedObjectType>& cachedObjectInstance,
                            Environment& wrkContext = Environment::defaultEnv() )
    {
        ConcurrentMap<std::string, std::shared_ptr<CachedObjectType>>& objectStore = wrkContext.getCache<CachedObjectType>();
        objectStore.set( cachedObjectInstance.get()->getRefToName(), cachedObjectInstance );
    };

    template<CachedObjectEnum Z>
    inline typename to_cached_object_type<Z>::type createEmptyCachedObjectType( const std::string& objectName )
    {
        typedef typename to_cached_object_type<Z>::type ReturnType;
        return ReturnType( objectName );
    };

    template<CachedObjectEnum Z>
    inline typename to_cached_object_type<Z>::type createCacheableObject( const rapidjson::Value& jsonDoc )
    {
        throw ETradingException("createCacheableObject was called on a Cached Object Type without its de-serialization implemented");
        //std::string objectName = jsonDoc[CachedObject::OBJECT_NAME_LABEL.c_str()].GetString();
        //return createEmptyCachedObjectType<Z>( objectName )
    };

    template<>
    inline typename to_cached_object_type<FREE_OBJECT>::type createCacheableObject<FREE_OBJECT>( const rapidjson::Value& jsonDoc )
    {
		return CreateFreeObject(jsonDoc);
    };

    template<>
    typename to_cached_object_type<TABLE>::type createCacheableObject<TABLE>( const rapidjson::Value& jsonDoc );

    template<>
    typename to_cached_object_type<CURVE_DEPRECATED>::type createCacheableObject<CURVE_DEPRECATED>( const rapidjson::Value& jsonDoc );

    template<>
    typename to_cached_object_type<CURVE_BUILD_PROPERTIES>::type createCacheableObject<CURVE_BUILD_PROPERTIES>( const rapidjson::Value& jsonDoc );

    template<>
    typename to_cached_object_type<EXAMPLE_STAND_ALONE>::type createCacheableObject<EXAMPLE_STAND_ALONE>( const rapidjson::Value& jsonDoc );
    
 	template<>
    typename to_cached_object_type<SCHEDULE>::type createCacheableObject<SCHEDULE>( const rapidjson::Value& jsonDoc );

    template<>
    typename to_cached_object_type<LEG>::type createCacheableObject<LEG>( const rapidjson::Value& jsonDoc );

    template<>
    typename to_cached_object_type<BOND>::type createCacheableObject<BOND>( const rapidjson::Value& jsonDoc );
      
	template<>
	typename to_cached_object_type<BOND_GENERATOR>::type createCacheableObject<BOND_GENERATOR>( const rapidjson::Value& jsonDoc );

    template<>
	typename to_cached_object_type<SWAP_OBJECT>::type createCacheableObject<SWAP_OBJECT>( const rapidjson::Value& jsonDoc );

    template<>
	typename to_cached_object_type<SWAP_GENERATOR>::type createCacheableObject<SWAP_GENERATOR>( const rapidjson::Value& jsonDoc );

    template<>
	typename to_cached_object_type<CURVE_DATA>::type createCacheableObject<CURVE_DATA>( const rapidjson::Value& jsonDoc );

    template<>
	typename to_cached_object_type<FIXING_TABLE>::type createCacheableObject<FIXING_TABLE>( const rapidjson::Value& jsonDoc );

	template<>
	typename to_cached_object_type<CURVE_GENERATOR>::type createCacheableObject<CURVE_GENERATOR>( const rapidjson::Value& jsonDoc );

	template<>
	typename to_cached_object_type<CURVE_MARKETDATA>::type createCacheableObject<CURVE_MARKETDATA>( const rapidjson::Value& jsonDoc );

	template<>
	typename to_cached_object_type<CURVE>::type createCacheableObject<CURVE>( const rapidjson::Value& jsonDoc );

	template<>
	typename to_cached_object_type<MULTICURVE>::type createCacheableObject<MULTICURVE>(const rapidjson::Value& jsonDoc);

    template<>
	typename to_cached_object_type<FX_CURVE>::type createCacheableObject<FX_CURVE>( const rapidjson::Value& jsonDoc );

	template<>
	typename to_cached_object_type<OPTION>::type createCacheableObject<OPTION>(const rapidjson::Value& jsonDoc);

	template<>
	typename to_cached_object_type<VOLATILITY>::type createCacheableObject<VOLATILITY>(const rapidjson::Value& jsonDoc);

	template<>
	typename to_cached_object_type<CREDIT_MODEL>::type createCacheableObject<CREDIT_MODEL>(const rapidjson::Value& jsonDoc);

	template<>
	typename to_cached_object_type<CREDIT_BASKET_MODEL>::type createCacheableObject<CREDIT_BASKET_MODEL>(const rapidjson::Value& jsonDoc);
	
	template<>
	typename to_cached_object_type<BOND_CURVE>::type createCacheableObject<BOND_CURVE>(const rapidjson::Value& jsonDoc);

	template<>
	typename to_cached_object_type<INFLATION_CURVE>::type createCacheableObject<INFLATION_CURVE>(const rapidjson::Value& jsonDoc);

	template<>
	typename to_cached_object_type<SABR_MODEL>::type createCacheableObject<SABR_MODEL>(const rapidjson::Value& jsonDoc);

	template<>
	typename to_cached_object_type<SABR_MARKETDATA>::type createCacheableObject<SABR_MARKETDATA>(const rapidjson::Value& jsonDoc);
}

