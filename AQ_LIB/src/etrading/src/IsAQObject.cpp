
#include "IsAQObject.h"
#include "SchemaObject.h"


namespace etrading
{
    IsAQObject::IsAQObject( const std::string& objectName, const CachedObjectEnum enumValue )
        : HasName( objectName ), Cacheable( enumValue )
    {};

    SerializationResult IsAQObject::serialize(	const serialize::SerializationMethodEnum method,
                                                const serialize::SerializationTargetEnum target,
                                                const std::string& targetInfo,
                                                std::vector<std::string>& variableNames,
                                                std::vector<Variant>& variableValues ) const
    {
        return toSchemaObject().serialize( method, target, targetInfo, variableNames, variableValues );
    };

	SerializationResult IsAQObject::serialize( const serialize::SerializationMethodEnum method,
												const serialize::SerializationTargetEnum target,
												const std::string& targetInfo ) const
	{
		std::vector<std::string> variableNames;
		std::vector<Variant>     variableValues;
		return serialize( method, target, targetInfo, variableNames, variableValues );
	}
}
