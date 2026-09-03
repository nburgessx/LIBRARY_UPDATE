#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Environment.h"
#include "EnvironmentPool.h"
#include "CoreEnumerations.h"
#include "ObjectUtilities.h"

namespace etrading
{

    template<typename T>
    ConcurrentMap<std::string, std::shared_ptr<T>>& getObjectStore( const std::string& wrkCtxtName )
    {
        BOOST_STATIC_ASSERT( boost::is_base_of<Cacheable, T>::value );
        return etrading::EnvironmentPool::getInstance().getEnvironment( wrkCtxtName ).get()->getCache<T>();
    }

    template<class UnderlyingType, class BaseType = UnderlyingType>
    struct GetObjectOnStore
    {
        static std::shared_ptr<BaseType> get( const std::string& ctxtName, const std::string& objectName )
        {
            BOOST_STATIC_ASSERT( boost::is_base_of<BaseType, UnderlyingType>::value );
            if( getObjectStore<UnderlyingType>( ctxtName ).has( objectName ) )
            {
                return getObjectStore<UnderlyingType>( ctxtName ).get( objectName );
            }
            else
            {
				return std::shared_ptr<BaseType>(); // nullptr. Use this syntax for Intel Compiler
            }
        };
    };

    template<class ReturnType, class BaseType = ReturnType>
    struct CreateObjectOnStore
    {
        static std::shared_ptr<BaseType> create( const std::string& ctxtName, const std::string& objectName )
        {
            BOOST_STATIC_ASSERT( boost::is_base_of<BaseType, ReturnType>::value );
            auto& env = *( EnvironmentPool::getInstance().getEnvironment( ctxtName ).get() );
            moveToCache<ReturnType>( ReturnType( objectName ), env );
            if( getObjectStore<ReturnType>( ctxtName ).has( objectName ) )
            {
                return getObjectStore<ReturnType>( ctxtName ).get( objectName );
            }
            else
            {
                return std::shared_ptr<BaseType>(); // nullptr. Use this syntax for Intel Compiler
            }
        };
    };

    inline bool createEnvironment( const std::string& clientName )
    {
        return etrading::EnvironmentPool::getInstance().createEnvironment( clientName );
    };

    inline void resetEnvironment()
    {
        return etrading::EnvironmentPool::getInstance().resetEnvironment();
    }

    inline std::string getDefaultEnvironmentName()
    {
        return etrading::EnvironmentPool::getInstance().getDefaultEnvironmentName();
    }

    std::vector<CachedObjectEnum> availableObjectTypes(
        const std::string& objectName,
        const std::string& environmentName = Environment::DEFAULT_ENV_NAME );

    int deleteAllObjects( Environment& env = etrading::Environment::defaultEnv() );

    int deleteAllObjects( const std::string& typeOfObject, Environment& env = etrading::Environment::defaultEnv() );

}