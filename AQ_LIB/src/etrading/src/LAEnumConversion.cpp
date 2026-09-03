// LAEnumConversion.cpp

// @File:				LAEnumConversion.cpp
// @Description:		Methods to convert to and from legacy enumerated types
// @Created:			15th May 2019
// @Author:				Nicholas Burgess
// @Department:			Quant Research & Analytics
//
// The copyright to the computer program(s) herein is the property of Mizuho International London.

#include "LAEnumConversion.h"
#include "ExceptionMacros.h"

namespace etrading
{
	DayCount toLADaycount( const DayCountEnum & daycountEnum )
	{
        switch( daycountEnum )
        {
            case ACT_360_DAYCOUNT:
                return ACT_360;
                break;

            case ACT_365_DAYCOUNT:
                return ACT_365;
                break;

            case N30_360_DAYCOUNT:
                return N30_360;
                break;

            case E30_360_DAYCOUNT:
                return E30_360;
                break;

            case ACT_ACT_DAYCOUNT:
                return ACT_ACT;
                break;

            case ACT_365_FJ_DAYCOUNT:
                return ACT_365_FJ;
                break;

			case ONE_DAYCOUNT:
				return DAYCOUNT_ONE;
				break;

			case HALF_DAYCOUNT:
				return DAYCOUNT_HALF;
				break;

			case QUARTER_DAYCOUNT:
				return DAYCOUNT_QUARTER;
				break;

            default:
				MLIB_THROW("Invalid Daycount: Only ACT/360, ACT/365, 30/360, 30E/360, ACT/ACT, ACT/365_FJ supported")
                break;
        }
	}

	
	RateConvention toLARateConvention( const CompoundingFrequencyEnum & compoundFrequencyEnum )
	{
		switch( compoundFrequencyEnum )
        {
			case SIMPLE_COMPOUNDING:
                return SIMPL;
                break;
			
			case CONTINUOUS_COMPOUNDING:
                return CONT;

			case ANNUAL_COMPOUNDING:
                return ANN;
                break;

            case SEMI_ANNUAL_COMPOUNDING:
                return SA;
                break;

            case QUARTERLY_COMPOUNDING:
                return QTR;
                break;

            case MONTHLY_COMPOUNDING:
                return MNT;
                break;

            default:
				MLIB_THROW("Invalid Compound Frequency: Only SIMPLE, CONTINUOUS, ANNUAL, SEMI-ANNUAL, QUARTERLY, MONTHLY supported")
                break;
        };
	}
	
}