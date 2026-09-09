#pragma once
#include "AQLCoreTemplateType.h"

namespace google_test
{
	// Builds AQObj MarketData Object by invoking the tryAqCurveMarketDataCreate() API.
	std::string createAQObjMarketDataObjectFromFileName(const AQLString& marketDataFileName);
}