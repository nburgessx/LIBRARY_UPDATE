#pragma once
#include "LACoreTemplateType.h"

namespace google_test
{
	// Builds LWO MarketData Object by invoking the tryMeLWOCurveMarketDataCreate() API.
	std::string createLWOMarketDataObjectFromFileName(const LAString& marketDataFileName);
}