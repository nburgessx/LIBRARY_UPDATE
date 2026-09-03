#pragma once
#include "AQLCoreTemplateType.h"

namespace google_test
{
	// Builds LWO MarketData Object by invoking the tryMeLWOCurveMarketDataCreate() API.
	std::string createLWOMarketDataObjectFromFileName(const AQLString& marketDataFileName);
}