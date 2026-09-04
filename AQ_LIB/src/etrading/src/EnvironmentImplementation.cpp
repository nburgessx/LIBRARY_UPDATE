
#include "EnvironmentImplementation.h"
#include "Environment.h"
#include "AQOCurve.h"

namespace etrading
{
     namespace environment_implementation
    {
        template<>
        bool eraseObject<AQOCurve>( const std::string& objectName, Environment& env )
        {
            auto& store = env.getCache<AQOCurve>();
            if( store.has( objectName ) )
            {
                auto ptrCurve = store.get(objectName);
                if(ptrCurve != nullptr)
                {
                    ptrCurve->removeUnderlyingEntityPoolCurve();
                }
                store.erase( objectName );
            }
            return store.has( objectName );
        };

        template<>
        const unsigned int eraseAllObjects<AQOCurve>( Environment& env )
        {
            auto& store = env.getCache<AQOCurve>();
            auto numberOfObjects = store.size();
            auto curveNames = store.keys();
            std::for_each(curveNames.cbegin(), curveNames.cend(), [&env](const std::string& curveName) 
            {  eraseObject<AQOCurve>(curveName, env); });
            store.clear();
            return numberOfObjects;
        };

    };


}
