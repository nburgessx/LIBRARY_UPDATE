
#include "EnvironmentUtilities.h"
#include "ObjectUtilities.h"

namespace etrading
{
    std::vector<CachedObjectEnum> availableObjectTypes(
        const std::string& objectName,
        const std::string& environmentName )
    {
        auto& env = ( *EnvironmentPool::getInstance().getEnvironment( environmentName ).get() );
        std::vector<CachedObjectEnum> typesFound;
        std::for_each( Environment::STORED_TYPES.cbegin(), Environment::STORED_TYPES.cend(),
                       [&typesFound, &env, &objectName]( const CachedObjectEnum enumTypeToCheck )
        {
            if( env.hasObject( objectName, enumTypeToCheck ) )
            {
                typesFound.emplace_back( enumTypeToCheck );
            }
        }
                     );
        return typesFound;
    };

    int deleteAllObjects( Environment& env )
    {
        int numOfObjectsCleared = 0;
        std::for_each( Environment::STORED_TYPES.cbegin(), Environment::STORED_TYPES.cend(),
                       [&numOfObjectsCleared, &env]( const CachedObjectEnum enumTypeToCheck )
        {
            numOfObjectsCleared += env.deleteAllObjects( enumTypeToCheck );
        }
                     );
        return numOfObjectsCleared;
    };

};
