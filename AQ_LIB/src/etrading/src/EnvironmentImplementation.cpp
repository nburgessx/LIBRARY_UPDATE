
#include "EnvironmentImplementation.h"
#include "Environment.h"
#include "LWOCurve.h"

namespace etrading
{
     namespace environment_implementation
    {
        template<>
        bool eraseObject<LWOCurve>( const std::string& objectName, Environment& env )
        {
            auto& store = env.getCache<LWOCurve>();
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
        const unsigned int eraseAllObjects<LWOCurve>( Environment& env )
        {
            auto& store = env.getCache<LWOCurve>();
            auto numberOfObjects = store.size();
            auto curveNames = store.keys();
            std::for_each(curveNames.cbegin(), curveNames.cend(), [&env](const std::string& curveName) 
            {  eraseObject<LWOCurve>(curveName, env); });
            store.clear();
            return numberOfObjects;
        };

    };


}
