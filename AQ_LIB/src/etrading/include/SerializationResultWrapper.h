#pragma once

#include <memory>
#include <rapidjson/document.h>

namespace etrading
{
	struct SerializationResultWrapper
    {
        SerializationResultWrapper() : jsonDocument() {}
        std::shared_ptr<rapidjson::Document> jsonDocument;
    };	
}