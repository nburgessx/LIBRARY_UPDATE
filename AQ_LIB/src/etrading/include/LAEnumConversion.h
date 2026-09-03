// LAEnumConversion.h

// @File:				LAEnumConversion.h
// @Description:		Methods to convert to and from legacy enumerated types
// @Created:			15th May 2019
// @Author:				Nicholas Burgess
// @Department:			Quant Research & Analytics
//
// The copyright to the computer program(s) herein is the property of Mizuho International London.

#pragma once

#include "CoreEnumerations.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceDataConvention.h"

namespace etrading
{
	DayCount toLADaycount( const DayCountEnum & daycountEnum );
	RateConvention toLARateConvention( const CompoundingFrequencyEnum & compoundFrequencyEnum );
}