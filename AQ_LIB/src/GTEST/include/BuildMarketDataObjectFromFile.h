#pragma once
#include "AQLCoreTemplateType.h"

namespace google_test
{
	// Builds AQO MarketData Object by invoking the tryAqObjectsCurveMarketDataCreate() API.
	std::string createLWOMarketDataObjectFromFileName(const AQLString& marketDataFileName);
}