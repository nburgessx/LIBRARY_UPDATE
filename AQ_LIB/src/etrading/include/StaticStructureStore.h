#pragma once

//
// @File: StaticStructureStore.h
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of AlgoQuantHub.



#include <map>
#include <string>
#include <vector>


#include "Singleton.h"
#include "CoreEnumerations.h"
#include "DataSchema.h"
#include "ETradingException.h"
#include "NamesBiMap.h"

namespace etrading
{

    // @StaticStructureStore : A singleton that holds all StaticData required for restricting object behaviour
    // to restrict object behaviour objects dervied from DataHolder need to have their DataSchemas set (without the possibility to change their underlying structure)
    // hence they are set as unchangeable (ideally const) objects in CACHEDOBJECT_STRUCTURES
    // these structures are then used under the hood when an object derived object from DataHolder is created (except for the case of a FreeObject).
    class StaticStructureStore : public Singleton<StaticStructureStore>
    {
    public:
        const std::vector<DataSchema> getCachedObjectStructure( const CachedObjectEnum enumObject ) const;
        const std::vector<std::string> getKeyColumns( const etrading::CachedObjectEnum enumObject, const std::string& skName ) const;

        const std::string toString( const serialize::SerializationMethodEnum srEnum ) const;

    private:
        friend etrading::Singleton<StaticStructureStore>;
        StaticStructureStore();

        const NamesBiMap<serialize::SerializationMethodEnum> SERIALIZEMETHOD_NAMES;
        std::map<etrading::CachedObjectEnum, std::vector<DataSchema> > CACHEDOBJECT_STRUCTURES; // TODO: C++11 initializer lists can make this const

		template<typename T>
		std::map<etrading::CachedObjectEnum, std::vector<DataSchema> > makeMap(const T& mapInitializer)
		{
			std::map<etrading::CachedObjectEnum, std::vector<DataSchema> > theMap = mapInitializer;
			return theMap;
		}
		

        StaticStructureStore( const StaticStructureStore& )
        {
            throw etrading::ETradingException( "Copy constructor of StaticStructureStore should never get called" );
        }; // TODO: C++11 = delete
        StaticStructureStore& operator=( const StaticStructureStore& )
        {
            throw etrading::ETradingException( "Assignment operator of StaticStructureStore should never get called" ) ;
        }; // TODO: C++11 = delete

    };

}
