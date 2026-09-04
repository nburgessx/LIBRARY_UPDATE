
#include "EnvironmentImplementation.h"
#include "Environment.h"
#include "AQObjCurve.h"

namespace etrading
{
     namespace environment_implementation
    {
        template<>
        bool eraseObject<AQObjCurve>( const std::string& objectName, Environment& env )
        {
            auto& store = env.getCache<AQObjCurve>();
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
        const unsigned int eraseAllObjects<AQObjCurve>( Environment& env )
        {
            auto& store = env.getCache<AQObjCurve>();
            auto numberOfObjects = store.size();
            auto curveNames = store.keys();
            std::for_each(curveNames.cbegin(), curveNames.cend(), [&env](const std::string& curveName) 
            {  eraseObject<AQObjCurve>(curveName, env); });
            store.clear();
            return numberOfObjects;
        };

    };


}
