/*
* @brief			Build market data object from a test file
* @Created:			20th August 2018
* @Author:			Joseph Ye
* @Department:		AlgoQuantHub London Quant Research and Analytics
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/

#pragma once
#include "LACoreTemplateType.h"

namespace google_test
{
	// Builds LWO MarketData Object by invoking the tryMeLWOCurveMarketDataCreate() API.
	std::string createLWOMarketDataObjectFromFileName(const LAString& marketDataFileName);
}