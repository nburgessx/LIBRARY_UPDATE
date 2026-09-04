// AQLEnumConversion.h

// @Description:		Methods to convert to and from legacy enumerated types

#pragma once

#include "CoreEnumerations.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceDataConvention.h"

namespace etrading
{
	DayCount toAQLDaycount( const DayCountEnum & daycountEnum );
	RateConvention toAQLRateConvention( const CompoundingFrequencyEnum & compoundFrequencyEnum );
}