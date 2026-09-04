//
// @Description: This is a test program


#pragma once

#include <string>
#include <memory>
#include <vector>
#include <boost/any.hpp>
#include <boost/optional.hpp>

#include "CoreEnumerations.h"
#include "HasInstance.h"
#include "Variant.h"


namespace etrading
{

	struct SerializationResultWrapper;

    /*
    SerializationResult  returns the result of a serialization (could be either string or or file)
    conversion operator guarantees conversion to std::string (file name if FILE, json std::string if STRING)
    */
    class SerializationResult : 
        public HasConstInstance<serialize::SerializationMethodEnum>,
        public HasConstInstance<serialize::SerializationTargetEnum>
    {
    public:
        SerializationResult( serialize::SerializationMethodEnum method,
                             serialize::SerializationTargetEnum target,
                             const SerializationResultWrapper& result,
                             const std::string& info = "" );
        operator std::string() const;
        
    private:
        std::shared_ptr<SerializationResultWrapper> result_;
        std::string info_;   // holds the file name when writing to a file
    };



}