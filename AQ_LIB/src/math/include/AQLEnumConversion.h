// AQLEnumConversion.h

// @File:				AQLEnumConversion.h
// @Description:		Methods to convert to and from legacy enumerated types
// @Created:			15th May 2019
// @Author:				Nicholas Burgess
// @Department:			Quant Research & Analytics
//
// The copyright to the computer program(s) herein is the property of AlgoQuantHub.

#pragma once

#include "CoreEnumerations.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataConvention.h"

namespace etrading
{
	DayCount toAQLDaycount( const DayCountEnum & daycountEnum );
	RateConvention toAQLRateConvention( const CompoundingFrequencyEnum & compoundFrequencyEnum );
}