#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>

#include "CoreEnumerations.h"
#include "ETradingException.h"
#include "ExceptionMacros.h"

namespace etrading
{

	// This function takes a const char and returns a new std::string which is trimmed and upper case
	std::string trim_to_upper(const char* inputStr)
	{
		std::string resultString(inputStr);
		boost::trim(resultString);
		return boost::to_upper_copy<std::string>(resultString);
	};

	// This function takes a std::string and returns a new std::string which is trimmed and upper case
	std::string trim_to_upper( const std::string & inputStr )
	{
		std::string resultString(inputStr);
		boost::trim(resultString);
		return boost::to_upper_copy<std::string>(resultString);
	}

    std::string toString( const MarketQuoteTypeEnum enumValue )
    {
        switch( enumValue )
        {
            case FRA_QUOTE:
                return "FRA";
                break;
            case CENTRALBANK_SWAP:
                return "CENTRALBANK_SWAP";
                break;
            case IRS_SWAP:
                return "IRS_SWAP";
                break;
            case IR_FUTURES:
                return "IR_FUTURES";
                break;
            case FXRATES:
                return "FXRATES";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: MarketQuoteTypeEnum number '%i' is not convertable to a string" ) % enumValue  ).str()  );
                break;
        }
    }

	std::string toString( const SwapQuoteTypeEnum enumValue )
	{
        switch( enumValue )
        {
            case OIS_SWAPTYPE:
                return "OIS";
                break;
            case STD_SWAPTYPE:
                return "STD";
                break;
            case BASIS_SWAPTYPE:
                return "BASIS";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: SwapQuoteTypeEnum number '%i' is not convertable to a string" ) % enumValue ).str()  );
                break;
        }
	}

	SwapQuoteTypeEnum toSwapQuoteTypeEnum(const std::string& enumString)
	{
         std::string uCaseString = etrading::trim_to_upper( enumString.c_str() );
        if( uCaseString == "OIS" || uCaseString == "OIS_SWAPTYPE" || uCaseString == "OIS_SWAP" || uCaseString == "OIS_TYPE" || uCaseString == "OIS_QUOTE" ||
			uCaseString == "OIS SWAPTYPE" || uCaseString == "OIS SWAP" || uCaseString == "OIS TYPE" || uCaseString == "OIS QUOTE" )
        {
            return OIS_SWAPTYPE;
        }
        if( uCaseString == "STD" || uCaseString == "STANDARD" || uCaseString == "NORMAL" || uCaseString == "STD_SWAPTYPE"   )
        {
            return STD_SWAPTYPE;
        }
        if( uCaseString == "BASIS" || uCaseString == "BASIS_SWAPTYPE" || uCaseString == "BASIS_SWAP" || uCaseString == "BASIS SWAP" || 
            uCaseString == "BASISSWAP" || uCaseString == "TENORBASIS" || uCaseString == "TENOR_BASIS" || uCaseString == "TENOR" || 
            uCaseString == "TENOR_SWAP" || uCaseString == "TENORSWAP")
        {
            return BASIS_SWAPTYPE;
        }
		throw ETradingException(  ( boost::format( "#Error: The string %s cannot be converted to a SwapQuoteTypeEnum (should be 'OIS','STD' or 'BASIS')" ) % enumString ).str()  );
	}

	std::string toString( const CentralBankTypeEnum enumValue )
	{
        switch( enumValue )
        {
            case BOE_CENTRAL_BANK:
                return "BOE";
                break;
            case ECB_CENTRAL_BANK:
                return "ECB";
                break;
            case FED_CENTRAL_BANK:
                return "FED";
                break;
            case BOJ_CENTRAL_BANK:
                return "BOJ";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: CentralBankTypeEnum number '%i' is not convertable to a string" ) % enumValue ).str()  );
                break;
        };
	}

	CentralBankTypeEnum toCentralBankTypeEnum(const std::string& enumString)
	{
         std::string uCaseString = etrading::trim_to_upper( enumString.c_str() );
        if( uCaseString == "BOE" || uCaseString == "UK" || uCaseString == "GB" || uCaseString == "GBP" || 
			uCaseString == "BANK OF ENGLAND" || uCaseString == "BANK_OF_ENGLAND")
        {
            return BOE_CENTRAL_BANK;
        }
        if( uCaseString == "ECB" || uCaseString == "EURO" || uCaseString == "EUR" || 
			uCaseString == "EUROPEAN_CENTRAL_BANK" || uCaseString == "EUROPEAN CENTRAL BANK" )
        {
            return ECB_CENTRAL_BANK;
        }
        if( uCaseString == "FED" || uCaseString == "USA" || uCaseString == "US" || uCaseString == "USD" || 
			uCaseString == "FEDERAL_RESERVE" || uCaseString == "FEDERAL RESERVE")
        {
            return FED_CENTRAL_BANK;
        }
        if( uCaseString == "BOJ" || uCaseString == "JAPAN" || uCaseString == "JPN" || uCaseString == "JPY" ||
			uCaseString == "BANK_OF_JAPAN" || uCaseString == "BANK OF JAPAN" || uCaseString == "NICHIGIN")
        {
            return BOJ_CENTRAL_BANK;
        }
		throw ETradingException(  ( boost::format( "#Error: The string %s cannot be converted to a CentralBankTypeEnum (should be 'BOE','BOJ','FED' or 'ECB')" ) % enumString ).str()  );
	}

     CurveTenorEnum toCurveTenorEnum( const FRAPeriodEnum fraEnum )
    {
        switch( fraEnum )
        {
            case _3M_FRAPERIOD:
                return CURVE_TENOR_3M;
                break;
            case _6M_FRAPERIOD:
                return CURVE_TENOR_6M;
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: FRAPeriodEnum number '%s' is invalid or not convertable to a curve Tenor (FRAPeriod should be _3M_FRAPERIOD or _6M_FRAPERIOD)" ) % fraEnum).str()  );
                break;
        }
    }

	 FRAPeriodEnum toFRAPeriodEnum( const CurveTenorEnum curveTenorEnum  )
	{
		switch(curveTenorEnum)
		{
		case CURVE_TENOR_3M:
			return _3M_FRAPERIOD;
			break;
		case CURVE_TENOR_6M:
			return _6M_FRAPERIOD;
			break;
		default:
			throw ETradingException(  ( boost::format( "#Error: FRAPeriods of %i are invalid (should be 3M or 6M)" ) % curveTenorEnum ).str()  );
            break;		
		}		
	}

     std::string toString( const ContainedTypeEnum enumValue )
    {
        switch( enumValue )
        {
            case INTEGER_VALUE:
                return "INTEGER";
                break;
            case DOUBLE_VALUE:
                return "DOUBLE";
                break;
            case DATE_VALUE:
                return "DATE";
                break;
            case STRING_VALUE:
                return "STRING";
                break;
            case BOOL_VALUE:
                return "BOOL";
                break;
            case VARIANT_VALUE:
                return "VARIANT";
                break;
            case EMPTY_VALUE:
                return "EMPTY";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: ContainedTypeEnum '%i' is invalid or not convertable to a string" ) % enumValue ).str()  );
                break;
        }
    }

     ContainedTypeEnum toContainedTypeEnum( const std::string& enumString )
    {
         std::string uCaseString = etrading::trim_to_upper( enumString.c_str() );
        if( uCaseString == "INTEGER" || uCaseString == "INT" || uCaseString == "I" )
        {
            return INTEGER_VALUE;
        }
        if( uCaseString == "DOUBLE" || uCaseString == "D" )
        {
            return DOUBLE_VALUE;
        }
        if( uCaseString == "DATE" || uCaseString == "DT" )
        {
            return DATE_VALUE;
        }
        if( uCaseString == "STRING" || uCaseString == "STR" || uCaseString == "S" || uCaseString == "CHAR" || uCaseString == "CHARACTER" )
        {
            return STRING_VALUE;
        }
        if( uCaseString == "BOOL" || uCaseString == "BOOLEAN"  || uCaseString == "B" )
        {
            return BOOL_VALUE;
        }
        if( uCaseString == "VARIANT" || uCaseString == "V"  || uCaseString == "VAR" )
        {
            return VARIANT_VALUE;
        }
        if( uCaseString == "EMPTY" )
        {
            return EMPTY_VALUE;
        }
		throw ETradingException(  ( boost::format( "#Error: The string %s cannot be converted to a ContainedTypeEnum" ) % enumString ).str()  );
    }

     std::string toString( const ExtrapolationTypeEnum enumValue )
    {
        switch( enumValue )
        {
            case CONSTANT_EXTRAPOLATION:
                return "CONSTANT";
                break;
            case LINEAR_EXTRAPOLATION:
                return "LINEAR";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: ExtrapolationType '%i' invalid, must be CONSTANT or LINEAR." ) % enumValue ).str()  );
                break;
        }
    }

     ExtrapolationTypeEnum toExtrapolationTypeEnum( const std::string& enumStringInput )
    {
         std::string enumString = etrading::trim_to_upper(enumStringInput.c_str());
        if( enumString == "CONSTANT"  || enumString == "CONST" || enumString == "CONSTANT_EXTRAPOLATION" || enumString == "CT"  )
        {
            return CONSTANT_EXTRAPOLATION;
        }
        if( enumString == "LINEAR"  || enumString == "LINEAR_EXTRAPOLATION" || enumString == "LIN"  )
        {
            return LINEAR_EXTRAPOLATION;
        }
		throw ETradingException(  ( boost::format( "#Error: ExtrapolationType '%s' invalid, must be CONSTANT or LINEAR." ) % enumString ).str()  );
    }

     std::string toString( const DayCountEnum enumValue )
    {
        //map to string accepted by core mlib LACoreComponentManager.cpp's getDayCount(const LAString &key)
        switch( enumValue )
        {
            case NONE_DAYCOUNT:
                return "";
                break;
            case ACT_360_DAYCOUNT:
                return "ACT/360";
                break;
            case ACT_365_DAYCOUNT:
                return "ACT/365";
                break;
            case N30_360_DAYCOUNT:
                return "30/360"; 
                break;
            case E30_360_DAYCOUNT:
                return "30E/360";
                break;
            case E30_360_ISDA_DAYCOUNT:
                return "30E/360ISDA";
                break;
            case ACT_ACT_DAYCOUNT:
                return "ACT/ACT";
                break;
            case ACT_365_FJ_DAYCOUNT:
                return "ACT/365FJ";
                break;
			case ONE_DAYCOUNT:
				return "1/1";
				break;
			case HALF_DAYCOUNT:
				return "1/2";
				break;
			case QUARTER_DAYCOUNT:
				return "1/4";
				break;
            default:
				throw ETradingException(  ( boost::format( "#Error: Daycount '%i' invalid, must be ACT/360, ACT/365, N30/360, E30/360, ACT/ACT, ACT/365_FJ, 1/1, 1/2 or 1/4." ) % enumValue ).str()  );
                break;
        }
    }

     DayCountEnum toDayCountEnum( const std::string& inputString )
    {
         std::string enumString = trim_to_upper( inputString.c_str() );
        if( enumString == "" || enumString == "NONE"  )
        {
            return NONE_DAYCOUNT;
        }
        if( enumString == "ACT_360" || enumString == "ACT/360"  )
        {
            return ACT_360_DAYCOUNT;
        }
        if( enumString == "ACT_365"  || enumString == "ACT/365"  )
        {
            return ACT_365_DAYCOUNT;
        }
        if( enumString == "N30_360"  || enumString == "N30/360"  || enumString == "30/360" || enumString == "30_360")
        {
            return N30_360_DAYCOUNT;
        }
        if( enumString == "E30_360"  || enumString == "E30/360" || enumString == "30E/360" || enumString == "30/360_ISMA"  || enumString == "30_360_ISMA" || enumString == "30/360 ISMA" || enumString == "ISMA-30/360" )
        {
            return E30_360_DAYCOUNT;
        }
        if( enumString == "E30_360_ISDA"  || enumString == "E30/360 ISDA" || enumString == "30E/360 ISDA" || enumString == "30E/360_ISDA"  || enumString == "30E_360_ISDA" || enumString == "30/360 German" || enumString == "30/360_German" || enumString == "30_360_German" || enumString == "GERMAN:30/360" )
        {
            return E30_360_ISDA_DAYCOUNT;
        }
        if( enumString == "ACT_ACT"   || enumString == "ACT/ACT"  ||
                enumString == "ACT_365_ISDA"  || enumString == "ACT/365_ISDA"   )
        {
            return ACT_ACT_DAYCOUNT;
        }
        if( enumString == "ACT_365_FJ"  ||  enumString == "ACT/365_FJ" || enumString == "ACT/365FJ" || enumString == "NL/365")
        {
            return ACT_365_FJ_DAYCOUNT;
        }
		if ( enumString == "1/1" )
		{
			return ONE_DAYCOUNT;
		}
		if ( enumString == "1/2" )
		{
			return HALF_DAYCOUNT;
		}
		if ( enumString == "1/4" )
		{
			return QUARTER_DAYCOUNT;
		}
		throw ETradingException(  ( boost::format( "#Error: Daycount '%s' invalid, must be ACT/360, ACT/365, N30/360, E30/360, ACT/ACT, ACT/365_ISDA, 1/1, 1/2 or 1/4." ) % enumString ).str()  );
    }

     std::string toString( const CompoundingMethodEnum enumValue )
    {
        switch( enumValue )
        {
            case NONE_COMPOUNDING_METHOD:
                return "NONE";
                break;
            case GEOMETRIC_COMPOUNDING_METHOD:
                return "GEOMETRIC";
                break;
            case ARITHMETIC_COMPOUNDING_METHOD:
                return "ARITHMETIC";
                break;
            case FLAT_COMPOUNDING_METHOD:
                return "FLAT";
                break;
            case SIMPLE_COMPOUNDING_METHOD:
                return "SIMPLE";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: CompoundingMethod '%i' invalid, must be 'GEOMETRIC', 'ARITHMETIC', 'FLAT', 'SIMPLE', or 'NONE'." ) % enumValue).str()  );
                break;
        }
    }
    
    CompoundingMethodEnum toCompoundingMethodEnum( const std::string& enumStringInput )
    {
        const std::string& enumString = etrading::trim_to_upper(enumStringInput.c_str());
        if( enumString ==  "NONE_COMPOUNDING" ||  enumString == "NONE" ||   enumString == "")
        {
            return NONE_COMPOUNDING_METHOD;
        }
        if( enumString ==  "GEOMETRIC_COMPOUNDING" || enumString == "GEOMETRIC" || enumString == "GEOMETRICAVERAGE" || enumString == "GEOM" || enumString == "NORMAL" )
        {
            return GEOMETRIC_COMPOUNDING_METHOD;
        }
        if( enumString ==  "ARITHMETIC_COMPOUNDING"   ||  enumString == "ARITHMETIC" || enumString == "ARITHMETICAVERAGE"  ||  enumString == "ARITH" || enumString == "AVERAGE" )
        {
            return ARITHMETIC_COMPOUNDING_METHOD;
        }
        if( enumString ==  "FLAT_COMPOUNDING"   ||  enumString == "FLAT" || enumString == "FLATAVERAGE" )
        {
            return FLAT_COMPOUNDING_METHOD;
        }
        if( enumString ==  "SIMPLE_COMPOUNDING"   ||  enumString == "SIMPLE" || enumString == "SIMPLEAVERAGE" )
        {
            return SIMPLE_COMPOUNDING_METHOD;
        }
    	throw ETradingException(  ( boost::format( "#Error: CompoundingMethod '%i' invalid, must be 'GEOMETRIC', 'ARITHMETIC', 'FLAT', 'SIMPLE', or 'NONE'." ) % enumString).str()  );
    }

     std::string toString( const CurveTenorEnum enumValue )
    {
        switch( enumValue )
        {
        	case NONE_CURVE_TENOR:
				return "NONE";
				break;
			case CURVE_TENOR_1D:
                return "1D";
                break;
            case CURVE_TENOR_1M:
                return "1M";
                break;
            case CURVE_TENOR_3M:
                return "3M";
                break;
            case CURVE_TENOR_6M:
                return "6M";
                break;
            case CURVE_TENOR_12M:
                return "12M";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: CurveTenor '%i' invalid, must be '1D', '1M', '3M', '6M' or '12M'." ) % enumValue ).str()  );
                break;
        }
    }

     CurveTenorEnum toCurveTenorEnum( const std::string& enumInputString )
    {
         std::string enumString = etrading::trim_to_upper(enumInputString.c_str());
		if (enumString.size() == 0 || enumString == "NONE")
		{
			return NONE_CURVE_TENOR;
		}

		if( enumString == "1D" || enumString == "DAILY"  | enumString == "BUSINESS_DAYS" || enumString == "_1D" || enumString == "DAY"   )
        {
            return CURVE_TENOR_1D;
        }

        if( enumString == "1M"  || enumString == "MONTHLY" || enumString == "_1M"  || enumString == "MONTH"    )
        {
            return CURVE_TENOR_1M;
        }

        if( enumString == "3M"  || enumString == "QUARTERLY" || enumString == "_3M"  || enumString == "QUARTER"    )
        {
            return CURVE_TENOR_3M;
        }

        if( enumString == "6M"  || enumString == "SEMI-ANNUAL" || enumString == "_6M"  || enumString == "SEMI_ANNUAL"
                || enumString == "SEMI-ANNUALLY"  || enumString == "SEMI_ANNUALLY ")
        {
            return CURVE_TENOR_6M;
        }

        if( enumString == "12M" || enumString == "ANNUAL" || enumString == "_12M" || enumString == "1Y"
                 || enumString == "ANNUALLY" || enumString == "YEARLY"  || enumString == "YEAR")
        {
            return CURVE_TENOR_12M;
        }

        throw ETradingException(  ( boost::format( "#Error: CurveTenor '%i' invalid, must be '1D', '1M', '3M', '6M' or '12M'." ) % enumString ).str()  );
    }
     double toYearFraction( const CurveTenorEnum enumValue, const bool isLeapYear )
    {
        switch( enumValue )
        {
            case CURVE_TENOR_1D:
                return isLeapYear ? 1.0 / 366.0 : 1.0 / 365;
                break;
            case CURVE_TENOR_1M:
                return 1.0 / 12.0;
                break;
            case CURVE_TENOR_3M:
                return 0.25;
                break;
            case CURVE_TENOR_6M:
                return 0.5;
                break;
            case CURVE_TENOR_12M:
                return 1.0;
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: CurveTenor value of '%i' invalid, must be '1D', '1M', '3M', '6M' or '12M'." ) % enumValue ).str()  );
                break;
        }
    }

    std::string toFrequencyFromCurveTenor(const std::string& curveTenor )
    {
        if( curveTenor == "1D" )
        {
            return "DAILY";
        }

        if( curveTenor == "1M" )
        {
            return "MONTHLY";
        }

        if( curveTenor == "3M" )
        {
            return "QUARTERLY";
        }

        if( curveTenor == "6M" )
        {
            return "SEMI-ANNUAL";
        }

        if( curveTenor == "12M" )
        {
            return "ANNUAL";
        }

        AQ_THROW("Invalid CurveTenorEnum: Must be '1D', '1M', '3M', '6M' or '12'.")
    }
    
     FrequencyEnum toFrequencyEnumFromCurveTenorEnum( const CurveTenorEnum curveTenorEnum )
    {
        switch( curveTenorEnum )
        {
            case CURVE_TENOR_1D:
                return DAILY_FREQUENCY;
                break;
            case CURVE_TENOR_1M:
                return MONTHLY_FREQUENCY;
                break;
            case CURVE_TENOR_3M:
                return QUARTERLY_FREQUENCY;
                break;
            case CURVE_TENOR_6M:
                return SEMI_ANNUAL_FREQUENCY;
                break;
            case CURVE_TENOR_12M:
                return ANNUAL_FREQUENCY;
                break;
            default:
				AQ_THROW("Invalid CurveTenorEnum: Must be 'CURVE_TENOR_1D', 'CURVE_TENOR_1M', 'CURVE_TENOR_3M', 'CURVE_TENOR_6M' or 'CURVE_TENOR_12M'.")
                break;
        }
    }

     std::string toString( const CompoundingFrequencyEnum enumValue )
    {
        switch( enumValue )
        {
			case NONE_COMPOUNDING:
				return "NONE";
				break;
			case SIMPLE_COMPOUNDING:
                return "SIMPLE";
                break;
            case CONTINUOUS_COMPOUNDING:
                return "CONTINUOUS";
				break;
			case ANNUAL_COMPOUNDING:
                return "ANNUAL";
                break;
            case SEMI_ANNUAL_COMPOUNDING:
                return "SEMI-ANNUAL";
                break;
            case QUARTERLY_COMPOUNDING:
                return "QUARTERLY";
                break;
            case MONTHLY_COMPOUNDING:
                return "MONTHLY";
                break;
            case LUNAR_COMPOUNDING:
                return "LUNAR";
                break;
            case WEEKLY_COMPOUNDING:
                return "WEEKLY";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: CompoundingFrequency '%i' invalid, must be 'ANNUAL', 'SEMI_ANNUAL', 'QUARTERLY', 'MONTHLY', 'WEEKLY', 'LUNAR', 'SIMPLE' or ' CONTINUOUS'" ) % enumValue ).str()  );
                break;
        };
    }

     CompoundingFrequencyEnum toCompoundingFrequencyEnum( const std::string& enumString )
    {
		std::string compareString = trim_to_upper( enumString.c_str() );
		
		if (compareString.size() == 0 || compareString == "NONE")
		{
			return NONE_COMPOUNDING;
		}
		if( compareString == "SIMPLE" )
        {
            return SIMPLE_COMPOUNDING;
        }
        if( compareString == "CONTINUOUS" )
        {
            return CONTINUOUS_COMPOUNDING;
        }
		if( compareString == "ANNUAL" || compareString == "YEARLY" || compareString == "1Y" || compareString == "YEAR" || compareString == "_1Y")
        {
            return ANNUAL_COMPOUNDING;
        }
        if( compareString == "SEMI_ANNUAL"  || compareString == "SEMI-ANNUAL" || compareString == "6M" || compareString == "_6M" )
        {
            return SEMI_ANNUAL_COMPOUNDING;
        }
        if( compareString == "QUARTERLY" || compareString == "3M" || compareString == "_3M" )
        {
            return QUARTERLY_COMPOUNDING;
        }
        if( compareString == "MONTHLY" || compareString == "MONTH" || compareString == "1M" || compareString == "_1M" )
        {
            return MONTHLY_COMPOUNDING;
        }
        if( compareString == "LUNAR" )
        {
            return LUNAR_COMPOUNDING;
        }
        if( compareString == "WEEKLY" || compareString == "WEEK" || compareString == "1W" || compareString == "_1W" )
        {
            return WEEKLY_COMPOUNDING;
        }
        throw ETradingException(  ( boost::format( "#Error: CompoundingFrequency '%i' invalid, must be 'ANNUAL', 'SEMI_ANNUAL', 'QUARTERLY', 'MONTHLY', 'WEEKLY', 'LUNAR', 'SIMPLE' or ' CONTINUOUS'" ) % enumString ).str()  );
    }

     std::string toTermString( const CompoundingFrequencyEnum enumValue )
    {
        if ( enumValue == ANNUAL_COMPOUNDING )
        {
            return "1Y";
        }
        else if ( enumValue == SEMI_ANNUAL_COMPOUNDING )
        {
            return "6M";
        }
        else if ( enumValue == QUARTERLY_COMPOUNDING )
        {
            return "3M";
        }
        else if ( enumValue == MONTHLY_COMPOUNDING )
        {
            return "1M";
        }
        else if ( enumValue == WEEKLY_COMPOUNDING )
        {
            return "1W";
        }
        else
        {
            throw LACoreInvalidData( "#Error: Frequency, must be Annual, Semi-Annual, Quarterly, Monthly or Weekly for conversion to a Term String.", __FILE__, __LINE__ );
        }
    }

     std::string toString( const StubTypeEnum enumValue )
    {
        switch( enumValue )
        {
            case NONE_STUBTYPE:
                return "NONE";
                break;
            case SHORT_START_STUBTYPE:
                return "SHORTSTART";
                break;
            case LONG_START_STUBTYPE:
                return "LONGSTART";
                break;
            case SHORT_END_STUBTYPE:
                return "SHORTEND";
                break;
            case LONG_END_STUBTYPE:
                return "LONGEND";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: StubType '%i' invalid, must be 'NONE', 'ShortStart', 'LongStart', 'ShortEnd', or 'LongEnd'." ) % enumValue ).str()  );
                break;
        };
    }

     StubTypeEnum toStubTypeEnum( const std::string& enumString )
    {
         std::string compareString = trim_to_upper( enumString.c_str() );
        if(compareString == "" || compareString == "NONE" )
        {
            return NONE_STUBTYPE;
        }
        if( compareString == "SHORTSTART" || compareString == "SS" || compareString == "SHORT_START" || compareString == "SHORT START")
        {
            return SHORT_START_STUBTYPE;
        }
		if (compareString == "LONGSTART" || compareString == "LS" || compareString == "LONG_START" || compareString == "LONG START")
        {
            return LONG_START_STUBTYPE;
        }
		if (compareString == "SHORTEND" || compareString == "SE" || compareString == "SHORT_END" || compareString == "SHORT END")
        {
            return SHORT_END_STUBTYPE;
        }
		if (compareString == "LONGEND" || compareString == "LE" || compareString == "LONG_END" || compareString == "LONG END")
        {
            return LONG_END_STUBTYPE;
        }
		throw ETradingException(  ( boost::format( "#Error: StubType '%i' invalid, must be 'NONE', 'ShortStart', 'LongStart', 'ShortEnd', or 'LongEnd'." ) % enumString ).str()  );
    }


     std::string toString( const CurveTypeEnum enumValue )
    {
        switch( enumValue )
        {
			case NONE_CURVETYPE:
				return "NONE";
				break;
			case OIS_CURVETYPE:
                return "OIS";
                break;
			case ARR_CURVETYPE:
				return "ARR";
				break;
			case SWAP_CURVETYPE:
                return "SWAP";
                break;
            case TENORBASIS_CURVETYPE:
                return "TENORBASIS";
                break;
            case XCCYBASIS_CURVETYPE:
                return "XCCYBASIS";
                break;
            case FWDFXCONST_CURVETYPE:
                return "FWDFXCONST";
                break;
            case FX_CURVETYPE:
                return "FX";
                break;
			case GLOBAL_CURVETYPE:
				return "GLOBAL";
				break;
            default:
				throw ETradingException((boost::format("#Error: CurveType value '%i' invalid, must be 'OIS', 'ARR', 'STD', 'XCCY', 'FWDFXCONST', 'FX' or 'NONE'.") % enumValue).str());
				break;
        }
    }

     CurveTypeEnum toCurveTypeEnum( const std::string& inputEnumString )
    {
        std::string enumString = etrading::trim_to_upper(inputEnumString.c_str());
		if( enumString == "" || enumString == "NONE" )
		{
			 return NONE_CURVETYPE;
		}
		if( enumString == "OIS" || enumString == "OIS_CURVE" )
        {
            return OIS_CURVETYPE;
        }
		if (enumString == "ARR" || enumString == "ARR_CURVE")
		{
			return ARR_CURVETYPE;
		}
		if( enumString == "SWAP"  || enumString == "STD" || enumString == "SWAP_CURVE" )
        {
            return SWAP_CURVETYPE;
        }
        if( enumString == "TENORBASIS" || enumString == "BASIS"  || enumString == "BASIS_CURVE" )
        {
            return TENORBASIS_CURVETYPE;
        }
        if( enumString == "XCCYBASIS" || enumString == "XCCY" || enumString == "XCCY_CURVE" )
        {
            return XCCYBASIS_CURVETYPE;
        }
        if( enumString == "FWDFXCONST" || enumString == "FXFWD" ||  enumString == "FXFORWARD" || enumString == "FXFWD_CURVE" )
        {
            return FWDFXCONST_CURVETYPE;
        }
        if( enumString == "FX")
        {
            return FX_CURVETYPE;
        }
		if( enumString == "GLOBAL" )
		{
			return GLOBAL_CURVETYPE;
		}
		throw ETradingException((boost::format("#Error: CurveType '%s' invalid, must be 'OIS', 'ARR', 'SWAP', 'TENORBASIS', 'XCCYBASIS', 'FWDFXCONST', 'FX' or 'NONE'") % enumString).str());
	 }

	std::string toString( const CurveCalibrationTypeEnum enumValue )
    {
        switch( enumValue )
        {
			case NONE_CURVE_CALIBRATION:
				return "NONE";
				break;
			case SINGLE_CURVE_CALIBRATION:
                return "SINGLE";
                break;
			case DUAL_CURVE_CALIBRATION:
				return "DUAL";
				break;
			case MULTI_CURVE_CALIBRATION:
                return "MULTI";
                break;
            default:
				throw ETradingException((boost::format("#Error: Invalid Curve Calibration Type: Type '%i' invalid, must be 'SINGLE', 'DUAL', 'MULTI', 'GLOBAL' or 'NONE'.") % enumValue).str());
				break;
        }
    }

    CurveCalibrationTypeEnum toCurveCalibrationTypeEnum( const std::string& inputEnumString )
    {
        std::string enumString = etrading::trim_to_upper(inputEnumString.c_str());
		if( enumString == "" || enumString == "NONE" )
		{
			 return NONE_CURVE_CALIBRATION;
		}
		if( enumString == "SINGLE" || enumString == "SINGLE_CURVE" )
        {
            return SINGLE_CURVE_CALIBRATION;
        }
		if (enumString == "DUAL" || enumString == "DUAL_CURVE")
		{
			return DUAL_CURVE_CALIBRATION;
		}
		if( enumString == "MULTI"  || enumString == "MULTI_CURVE" || enumString == "GLOBAL" || enumString == "GLOBAL_CURVE" )
        {
            return MULTI_CURVE_CALIBRATION;
        }
        throw ETradingException( ( boost::format("#Error: Invalid Curve Calibration Type: Type '%i' invalid, must be 'SINGLE', 'DUAL', 'MULTI', 'GLOBAL' or 'NONE'.") % enumString).str());
	 }


     std::string toString( const BusinessDayAdjustmentEnum enumValue )
    {
        switch( enumValue )
        {
            case NONE_BUSINESS_DAY_ADJ:
                return "";
                break;
			case NO_CHANGE:
                return "NO_CHANGE";
                break;
            case FOLLOWING:
                return "FOLLOWING";
                break;
            case MOD_FOLLOWING:
                return "MOD_FOLLOWING";
                break;
            case PRECEDING:
                return "PRECEDING";
                break;
            case MOD_PRECEDING:
                return "MOD_PRECEDING";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: BusinessDayAdjustment value '%i' invalid, must be 'Following', 'Mod_Following', 'Preceding', 'Mod_Preceding' or 'No_Change'." ) % enumValue ).str()  );
                break;
        }
    }

     BusinessDayAdjustmentEnum toBusinessDayAdjustmentEnum( const std::string& inputEnumString )
    {
         std::string enumString = etrading::trim_to_upper(inputEnumString.c_str());
        if( enumString == "" || enumString == "NONE" )
        {
            return NONE_BUSINESS_DAY_ADJ;
        }
        if( enumString == "FOLLOWING" || enumString == "F" || enumString == "FOL" )
        {
            return FOLLOWING;
        }
        if( enumString == "MOD_FOLLOWING" || enumString == "MF" || enumString == "MOD FOLLOWING" || enumString == "MODFOLLOWING" )
        {
            return MOD_FOLLOWING;
        }
        if( enumString == "PRECEDING" || enumString == "P" || enumString == "PREC" )
        {
            return PRECEDING;
        }
        if( enumString == "MOD_PRECEDING" || enumString == "MP" || enumString == "MOD_PREC" || enumString == "MODPREC" || enumString == "MOD PREC")
        {
            return MOD_PRECEDING;
        }
        if( enumString == "NO_CHANGE" || enumString == "UNADJUSTED" || enumString == "U" || enumString == "NO CHANGE" || enumString == "NOCHANGE" )
        {
            return NO_CHANGE;
        }
		throw ETradingException(  ( boost::format( "#Error: BusinessDayAdjustment '%s' invalid, must be 'FOLLOWING', 'MOD_FOLLOWING', 'PRECEDING', 'MOD_PRECEDING', 'NO_CHANGE'." ) % enumString ).str()  );
    }

     std::string toString( const InterpolationEnum enumValue )
    {
        // why prefer switch to if-else ? http://stackoverflow.com/questions/1028437/why-switch-case-and-not-if-else-if
        switch( enumValue )
        {
            case RIGHT_CONTINUOUS_INTERPOLATION:
                return "RIGHT_CONTINUOUS";
                break;
            case LEFT_CONTINUOUS_INTERPOLATION:
                return "LEFT_CONTINUOUS";
                break;
            case STEP_INTERPOLATION:
                return "STEP";
                break;
            case LINEAR_INTERPOLATION:
                return "LINEAR";
                break;
			case LINEAR_WITH_FLAT_EXTRAPOLATION:
				return "LINEAR_WITH_FLAT_EXTRAPOLATION";
				break;
            case MONOTONE_CONVEX_INTERPOLATION:
                return "MONOTONE_CONVEX";
                break;
            case SPLINE_INTERPOLATION:
                return "SPLINE";
                break;
			case MONOTONESPLINE_INTERPOLATION:
                return "MONOTONESPLINE";
                break;
			case MONOTONEPARABOLIC_INTERPOLATION:
                return "MONOTONEPARABOLIC";
                break;
            case NATURAL_SPLINE_INTERPOLATION:
                return "NATURALSPLINE";
                break;
            case CLAMPED_SPLINE_INTERPOLATION:
                return "CLAMPEDSPLINE";
                break;
            case PARABOLIC_INTERPOLATION:
                return "PARABOLIC";
                break;
            case LINEARSPLINE_INTERPOLATION:
                return "LINEARSPLINE";
                break;
			case LINEARMONOTONESPLINE_INTERPOLATION:
                return "LINEARMONOTONESPLINE";
                break;
			case LINEARMONOTONEPARABOLIC_INTERPOLATION:
                return "LINEARMONOTONEPARABOLIC";
                break;
            case CONSTRAINED_SPLINE_INTERPOLATION:
                return "CONSTRAINED_SPLINE";
                break;
			case NELSON_SIEGEL_INTERPOLATION:
				return "NELSONSIEGEL";
			case SVENSSON_INTERPOLATION:
				return "SVENSSON";
			case POLYNOMIAL_INTERPOLATION:
				return "POLYNOMIAL";
			case LOG_LINEAR_INTERPOLATION:
				return "LOGLINEAR";
            default:
				AQ_THROW("Invalid Interpolation Type: Must be SPLINE, MONOTONESPLINE, LINEARSPLINE, LINEARMONOTONESPLINE, LINEAR, LINEAR_WITH_FLAT_EXTRAPOLATION, RIGHT_CONTINUOUS, LEFT_CONTINUOUS, LOGLINEAR or STEP")
                break;
        }
    }

    InterpolationEnum toInterpolationEnum( const std::string& enumString )
    {
         std::string compareString = trim_to_upper( enumString.c_str() );

        if( compareString == "STEP" || compareString == "FN_STEPINTERPOLATION" )
        {
            return STEP_INTERPOLATION;
        }
        if( compareString == "LEFT_CONTINUOUS" || compareString == "LEFTCONTINUOUS" || compareString == "FN_LEFTCONTINUOUSINTERPOLATION" )
        {
            return LEFT_CONTINUOUS_INTERPOLATION;
        }
        if( compareString == "RIGHT_CONTINUOUS" || compareString == "RIGHTCONTINUOUS" || compareString == "FN_RIGHTCONTINUOUSINTERPOLATION" ) 
        {
            return RIGHT_CONTINUOUS_INTERPOLATION;
        }
        if( compareString == "LINEAR"  || compareString == "FN_LINEARINTERPOLATION" )
        {
            return LINEAR_INTERPOLATION;
        }
		if (compareString == "LINEAR_WITH_FLAT_EXTRAPOLATION" || compareString == "LINEARWITHFLATEXTRAPOLATION" )
		{
			return LINEAR_WITH_FLAT_EXTRAPOLATION;
		}
		if( compareString == "MONOTONE_CONVEX"  || compareString == "MONOTONECONVEX"  || compareString == "FN_MONOTONECONVEXINTERPOLATION" )
        {
            return MONOTONE_CONVEX_INTERPOLATION;
        }
        if( compareString == "SPLINE"  || compareString == "FN_SPLINEINTERPOLATION" )
        {
            return SPLINE_INTERPOLATION;
        }
		if( compareString == "MONOTONESPLINE"  || compareString == "FN_MONOTONESPLINEINTERPOLATION" )
        {
            return MONOTONESPLINE_INTERPOLATION;
        }
		if( compareString == "MONOTONEPARABOLIC"  || compareString == "FN_MONOTONEPARABOLICINTERPOLATION" )
        {
            return MONOTONEPARABOLIC_INTERPOLATION;
        }
        if( compareString == "PARABOLIC"  || compareString == "FN_PARABOLICINTERPOLATION" )
        {
            return PARABOLIC_INTERPOLATION;
        }
        if( compareString == "LINEARSPLINE"  || compareString == "LINEAR_SPLINE" || compareString == "FN_LINEARSPLINEINTERPOLATION" )
        {
            return LINEARSPLINE_INTERPOLATION;
        }
		if( compareString == "LINEARMONOTONESPLINE"  || compareString == "LINEAR_MONOTONE_SPLINE" || compareString == "FN_LINEARMONOTONESPLINEINTERPOLATION" )
        {
            return LINEARMONOTONESPLINE_INTERPOLATION;
        }
		if( compareString == "LINEARMONOTONEPARABOLIC"  || compareString == "LINEAR_MONOTONE_PARABOLIC" || compareString == "FN_LINEARMONOTONEPARABOLICINTERPOLATION" )
        {
            return LINEARMONOTONEPARABOLIC_INTERPOLATION;
        }
        if( compareString == "CONSTRAINED_SPLINE" || compareString == "CONSTRAINEDSPLINE" || compareString == "FN_CONSTRAINEDSPLINEINTERPOLATION" )
        {
            return CONSTRAINED_SPLINE_INTERPOLATION;
        }
	    if( compareString == "NELSONSIEGEL")
        {
            return NELSON_SIEGEL_INTERPOLATION;
        }
		if( compareString == "SVENSSON")
        {
            return SVENSSON_INTERPOLATION;
        }
		if( compareString == "POLYNOMIAL")
        {
            return POLYNOMIAL_INTERPOLATION;
        }
		if (compareString == "LOGLINEAR")
		{
			return LOG_LINEAR_INTERPOLATION;
		}
		AQ_THROW("Invalid Interpolation Type: Must be SPLINE, MONOTONESPLINE, LINEARSPLINE, LINEARMONOTONESPLINE, LINEAR, LINEAR_WITH_FLAT_EXTRAPOLATION, RIGHT_CONTINUOUS, LEFT_CONTINUOUS, LOGLINEAR or STEP")
    }

	bool isHybridInterpolation( const InterpolationEnum enumValue )
	{
		bool isHybrid = false;

        switch( enumValue )
        {
            case LINEARSPLINE_INTERPOLATION:
            case LINEARMONOTONESPLINE_INTERPOLATION:
            case LINEARMONOTONEPARABOLIC_INTERPOLATION:
			{
				isHybrid = true;
				break;
			}
			default:
			{
				isHybrid = false;
				break;
			}
        }

		return isHybrid;
	}

	bool isHybridInterpolation( const std::string& enumString )
	{
		const InterpolationEnum interpolationEnum = toInterpolationEnum( enumString );
		return isHybridInterpolation( interpolationEnum );
	}

    std::string toString( const etrading::CachedObjectEnum coEnum )
    {
        switch( coEnum )
        {
            case TABLE:
                return "TABLE";
                break;
            case MATRIX:
                return "MATRIX";
                break;
            case CURVE_DEPRECATED:
                return "CURVE_DEPRECATED";
                break;
            case CURVE_BUILD_PROPERTIES:
                return "CURVE_BUILD_PROPERTIES";
                break;
            case SWAP_GENERATOR:
                return "SWAP_GENERATOR";
                break;
            case FREE_OBJECT:
                return "FREE_OBJECT";
                break;
            case EXAMPLE_STAND_ALONE:
                return "EXAMPLE_STAND_ALONE";
            case EXAMPLE_BASE:
                return "EXAMPLE_BASE";
            case SWAP_OBJECT:
                return "SWAP";
            case LEG:
                return "LEG"; 
            case BOND:
                return "BOND"; 
            case SCHEDULE:
                return "SCHEDULE";
            case FIXING_TABLE:
                return "FIXING_TABLE";
            case CURVE_DATA:
                return "CURVE_DATA";
            case FEE_SCHEDULE:
                return "FEE_SCHEDULE";
            case LEG_STATIC_DATA:
                return "LEG_STATIC_DATA";
            case LEG_GENERATOR:
                return "LEG_GENERATOR";
            case BESPOKE_SCHEDULE_PROPERTIES:
                return "BESPOKE_SCHEDULE_PROPERTIES";
			case CURVE_GENERATOR:
                return "CURVE_GENERATOR";
			case CURVE_MARKETDATA:
                return "CURVE_MARKETDATA";
			case CURVE:
                return "CURVE";
			case MULTICURVE:
				return "MULTICURVE";
			case FX_CURVE:
                return "FX_CURVE";
			case BOND_GENERATOR:
                return "BOND_GENERATOR";
                break;
			case OPTION:
				return "OPTION";
			case VOLATILITY:
				return "VOLATILITY";
			case CREDIT_MODEL:
				return "CREDIT_MODEL";
			case CREDIT_BASKET_MODEL:
				return "CREDIT_BASKET_MODEL";
			case BOND_CURVE:
				return "BOND_CURVE";
			case INFLATION_CURVE:
				return "INFLATION_CURVE";
			case SABR_MODEL:
				return "SABR_MODEL";
			case SABR_MARKETDATA:
				return "SABR_MARKETDATA";
			default:
				throw ETradingException(  ( boost::format( "#Error: ObjectType value '%i' not supported" ) % coEnum  ).str()  );
                break;
        }
    }

     CachedObjectEnum toCachedObjectEnum( const std::string& incomingString )
    {
         std::string enumString = etrading::trim_to_upper( incomingString.c_str() );
        if( enumString == "TABLE" )
        {
            return TABLE;
        }
        if( enumString == "MATRIX" )
        {
            return MATRIX;
        }
        if( enumString == "CURVE_DEPRECATED" )
        {
            return CURVE_DEPRECATED;
        }
        if( enumString == "SWAP_GENERATOR" )
        {
            return SWAP_GENERATOR;
        }
        if( enumString == "FREE_OBJECT" || enumString == "GRID" )
        {
            return FREE_OBJECT;
        }
        if( enumString == "CURVE_BUILD_PROPERTIES" || enumString == "CURVE_PROPERTIES" )
        {
            return CURVE_BUILD_PROPERTIES;
        }
        if( enumString == "EXAMPLE_STAND_ALONE" )
        {
            return EXAMPLE_STAND_ALONE;
        }
        if( enumString == "EXAMPLE_BASE" )
        {
            return EXAMPLE_BASE;
        }
		if( enumString == "SWAP")
        {
			return SWAP_OBJECT;
        }
		if( enumString == "LEG" )
        {
			return LEG;
        }
        if( enumString == "BOND" )
        {
			return BOND;
        }
		if( enumString == "SCHEDULE" )
        {
			return SCHEDULE;
        }
        if( enumString == "FIXING_TABLE" )
        {
            return FIXING_TABLE;
        }
        if( enumString == "CURVE_DATA" )
        {
            return CURVE_DATA;
        }
        if( enumString == "FEE_SCHEDULE" )
        {
            return FEE_SCHEDULE;
        }
        if( enumString == "LEG_STATIC_DATA" )
        {
            return LEG_STATIC_DATA;
        }
        if( enumString == "LEG_GENERATOR" )
        {
            return LEG_GENERATOR;
        }
        if( enumString == "BESPOKE_SCHEDULE_PROPERTIES" )
        {
            return BESPOKE_SCHEDULE_PROPERTIES;
        }
		if( enumString == "CURVE_GENERATOR" )
        {
            return CURVE_GENERATOR;
        }
		if( enumString == "CURVE_MARKETDATA" )
        {
            return CURVE_MARKETDATA;
        }
		if( enumString == "CURVE" )
        {
            return CURVE;
        }
		if (enumString == "MULTICURVE")
		{
			return MULTICURVE;
		}
		if( enumString == "FX_CURVE" )
        {
            return FX_CURVE;
        }
		if( enumString == "BOND_GENERATOR" )
        {
            return BOND_GENERATOR;
        }
		if (enumString == "OPTION")
		{
			return OPTION;
		}
		if (enumString == "VOLATILITY")
		{
			return VOLATILITY;
		}
		if (enumString == "CREDIT_MODEL")
		{
			return CREDIT_MODEL;
		}
		if (enumString == "CREDIT_BASKET_MODEL")
		{
			return CREDIT_BASKET_MODEL;
		}
		if (enumString == "BOND_CURVE")
		{
			return BOND_CURVE;
		}
		if (enumString == "INFLATION_CURVE")
		{
			return INFLATION_CURVE;
		}
		if (enumString == "SABR_MODEL")
		{
			return SABR_MODEL;
		}
		if (enumString == "SABR_MARKETDATA")
		{
			return SABR_MARKETDATA;
		}
		if ( enumString.empty() )
        {
			throw ETradingException( "#Error: No Object type provided. Available objects include: TABLE, FEE_SCHEDULE, CURVE, BOND, BOND_GENERATOR, SWAP, SWAP_GENERATOR, SCHEDULE, FIXING_TABLE, CURVE_DATA , CURVE_GENERATOR, CURVE_MARKETDATA, FX_CURVE, OPTION, VOLATILITY, CREDIT_MODEL, CREDIT_BASKET_MODEL, BOND_CURVE, SABR_MODEL, SABR_MARKETDATA and FREE_OBJECT." );
        }
		throw ETradingException(  ( boost::format( "#Error: Object type '%s' not supported. Available objects include: TABLE, CURVE, BOND, BOND_GENERATOR, SWAP, SWAP_GENERATOR, SCHEDULE, FIXING_TABLE, CURVE_DATA, CURVE_GENERATOR, CURVE_MARKETDATA, FX_CURVE, OPTION, VOLATILITY, CREDIT_MODEL, CREDIT_BASKET_MODEL, BOND_CURVE, SABR_MODEL, SABR_MARKETDATA and FREE_OBJECT." ) % enumString ).str()  );
    }

	 std::string toString( const etrading::CurveGeneratorEnum ccEnum )
    {
        switch( ccEnum )
        {
			case NONE_CURVEGENERATOR:
                return "NONE";
            case CURVE_PROPERTIES:
                return "CURVEPROPERTIES";
			case MONEY_MARKET_CONVENTIONS:
				return "MONEYMARKET";
			case LIBOR_FIXING_CONVENTIONS:
				return "LIBORFIXINGS";
			case FRA_CONVENTIONS:
				return "FRAS";
			case FUTURES_CONVENTIONS:
				return "FUTURES";
			case SWAP_CONVENTIONS:
				return "SWAPS";
			case OIS_CONVENTIONS:
				return "OIS";
			case LIBOR_OIS_BASIS_CONVENTIONS:
				return "LIBOROISBASIS";
			case BASIS_SWAP_CONVENTIONS:
				return "BASISSWAPS";
			case XCCY_BASIS_CONVENTIONS:
				return "XCCYSWAPS";
			case FXFWD_CONVENTIONS:
				return "FXFWDS";
			case FWDFXCONST_CONVENTIONS:
				return "FWDFXCONST";
            default:
                throw ETradingException(  ( boost::format( "#Error: CurveGenerator type '%i' not supported" ) % ccEnum  ).str()  );
                break;
		}
	}

	 CurveGeneratorEnum toCurveGeneratorEnum( const std::string& incomingString )
    {
         std::string enumString = etrading::trim_to_upper( incomingString.c_str() );
		if( (enumString == "NONE") || (enumString == "") )
        {
            return NONE_CURVEGENERATOR;
        }
		if( enumString == "CURVEPROPERTIES" )
        {
            return CURVE_PROPERTIES;
        }
		if( enumString == "MONEYMARKET" )
        {
            return MONEY_MARKET_CONVENTIONS;
        }
		if( enumString == "LIBORFIXINGS" )
        {
            return LIBOR_FIXING_CONVENTIONS;
        }
		if( enumString == "FRAS" )
        {
            return FRA_CONVENTIONS;
        }
		if( enumString == "FUTURES" )
        {
            return FUTURES_CONVENTIONS;
        }
		if( enumString == "SWAPS" )
        {
            return SWAP_CONVENTIONS;
        }
		if( enumString == "OIS" )
        {
            return OIS_CONVENTIONS;
        }
		if( enumString == "LIBOROISBASIS" )
        {
            return LIBOR_OIS_BASIS_CONVENTIONS;
        }
		if( enumString == "BASISSWAPS" )
		{
			return BASIS_SWAP_CONVENTIONS;
		}
		if( enumString == "XCCYSWAPS" )
		{
			return XCCY_BASIS_CONVENTIONS;
		}
		if( enumString == "FXFWDS" )
		{
			return FXFWD_CONVENTIONS;
		}
		if( enumString == "FWDFXCONST" )
		{
			return FWDFXCONST_CONVENTIONS;
		}
		if ( enumString.empty() )
        {
			throw ETradingException("#Error: No property key provided. Available property keys include: NONE, CURVEPROPERTIES, MONEYMARKET, LIBORFIXINGS, FRAS, FUTURES, SWAPS, OIS, BASISSWAPS, XCCYBASIS, FXFWDS, FWDFXCONST.");
        }
		throw ETradingException(  ( boost::format( "#Error: Property key '%s' not supported. Available property keys include: NONE, CURVEPROPERTIES, MONEYMARKET, LIBORFIXINGS, FRAS, FUTURES, SWAPS, OIS, BASISSWAPS, XCCYBASIS, FXFWD, FWDFXCONST." ) % enumString ).str()  );
	}

	 std::string toString( const etrading::CurveMarketDataEnum cmdEnum )
    {
        switch( cmdEnum )
        {
			case NONE_MARKETDATA:
				return "NONE";
			case MARKETDATA_PROPERTIES:
				return "MARKETDATAPROPERTIES";
			case LIBOR_FIXING_TABLE:
                return "LIBORFIXINGS";
            case FRA_MARKETDATA:
                return "FRAS";
			case FUTURES_MARKETDATA:
				return "FUTURES";
			case SWAP_MARKETDATA:
				return "SWAPS";
			case OIS_MARKETDATA:
				return "OIS";
			case OIS_FIXING_TABLE:
				return "OISFIXINGS";
			case LIBOR_OIS_BASISSPREAD_MARKETDATA:
				return "LIBOROISBASISSPREADS";
			case BASIS_SWAP_MARKETDATA:
				return "BASISSWAPS";
			case XCCY_SWAP_MARKETDATA:
				return "XCCYSWAPS";
			case FXFWD_MARKETDATA:
				return "FXFWDS";
            case FXFWD_BIDASK_MARKETDATA:
				return "FXFWDS_BIDASK";
			case FXSPOT_MARKETDATA:
				return "FXSPOTS";
            case FXSPOT_BIDASK_MARKETDATA:
				return "FXSPOTS_BIDASK";
			case FORWARD_ADJUSTMENTS_MARKETDATA:
				return "FORWARDADJUSTMENTS";
            default:
				throw ETradingException(  ( boost::format( "#Error: MarketData key '%i' not supported" ) % cmdEnum  ).str()  );
                break;
		}
	}

	 CurveMarketDataEnum toCurveMarketDataEnum( const std::string& incomingString )
    {
         std::string enumString = etrading::trim_to_upper( incomingString.c_str() );
		if ((enumString == "NONE") || (enumString == ""))
		{
			return NONE_MARKETDATA;
		}
		if( enumString == "MARKETDATAPROPERTIES" )
        {
            return MARKETDATA_PROPERTIES;
        }
        if( enumString == "LIBORFIXINGS" )
        {
            return LIBOR_FIXING_TABLE;
        }
		if( enumString == "FRAS" )
        {
            return FRA_MARKETDATA;
        }
		if( enumString == "FUTURES" )
        {
            return FUTURES_MARKETDATA;
        }
		if( enumString == "SWAPS" )
        {
            return SWAP_MARKETDATA;
        }
		if( enumString == "OIS" )
        {
            return OIS_MARKETDATA;
        }
		if( enumString == "OISFIXINGS" )
        {
            return OIS_FIXING_TABLE;
        }
		if( enumString == "LIBOROISBASISSPREADS" )
        {
            return LIBOR_OIS_BASISSPREAD_MARKETDATA;
        }
		if( enumString == "BASISSWAPS" )
        {
            return BASIS_SWAP_MARKETDATA;
        }
		if( enumString == "XCCYSWAPS" )
        {
            return XCCY_SWAP_MARKETDATA;
        }
		if( enumString == "FXFWDS" )
        {
            return FXFWD_MARKETDATA;
        }
        if( enumString == "FXFWDS_BIDASK" )
        {
            return FXFWD_BIDASK_MARKETDATA;
        }
		if( enumString == "FXSPOTS" )
        {
            return FXSPOT_MARKETDATA;
        }
        if( enumString == "FXSPOTS_BIDASK" )
        {
            return FXSPOT_BIDASK_MARKETDATA;
        }
		if (enumString == "FORWARDADJUSTMENTS")
		{
		 return FORWARD_ADJUSTMENTS_MARKETDATA;
		}
		if ( enumString.empty() )
        {
			throw ETradingException("#Error: No marketdata key provided. Available marketdata keys include: MARKETDATAPROPERTIES, LIBORFIXINGTABLE, FRAS, FUTURES, SWAPS, OIS, OISFIXINGS, LIBOROISBASISSPREADS, BASISSWAPS, XCCYSWAPS, FXFWDS, FXFWDS_BIDASK, FXSPOTS, FXSPOTS_BIDASK, FORWARDADJUSTMENTS.");
        }
		throw ETradingException(  ( boost::format( "#Error: Marketdata key '%s' not supported. Available marketdata keys include: MARKETDATAPROPERTIES, LIBORFIXINGTABLE, FRAS, FUTURES, SWAPS, OIS, OISFIXINGS, LIBOROISBASISSPREADS, BASISSWAPS, XCCYSWAPS, FXFWDS, FXFWDS_BIDASK, FXSPOTS, FXSPOTS_BIDASK, FORWARDADJUSTMENTS." ) % enumString ).str()  );
	}

	 std::string toString( const etrading::CreditModelEnum creditModelEnum )
    {
        switch( creditModelEnum )
        {
			case NONE_CREDITMODEL:
				return "NONE";
			case MODEL_PROPERTIES:
				return "MODEL_PROPERTIES";
			case CDS_MARKETDATA:
				return "CDS_MARKETDATA";
			case BOND_MARKETDATA:
				return "BOND_MARKETDATA";
			case CREDIT_MODELS:
				return "CREDIT_MODELS";
            default:
				throw ETradingException(  ( boost::format( "#Error: CreditModel key '%i' not supported" ) % creditModelEnum  ).str()  );
                break;
		}
	}

	 CreditModelEnum toCreditModelEnum( const std::string& incomingString )
    {
         std::string enumString = etrading::trim_to_upper( incomingString.c_str() );
		if ((enumString == "NONE") || (enumString == ""))
		{
			return NONE_CREDITMODEL;
		}
		if( enumString == "MODEL_PROPERTIES" )
        {
            return MODEL_PROPERTIES;
        }
		if( enumString == "CDS_MARKETDATA" )
        {
            return CDS_MARKETDATA;
        }
		if( enumString == "BOND_MARKETDATA" )
        {
            return BOND_MARKETDATA;
        }
		if( enumString == "CREDIT_MODELS" )
        {
            return CREDIT_MODELS;
        }
		if ( enumString.empty() )
        {
			throw ETradingException("#Error: No credit model key provided. Available credit model keys include: MODEL_PROPERTIES, CDS_MARKETDATA, BOND_MARKETDATA, CREDIT_MODELS.");
        }
		throw ETradingException(  ( boost::format( "#Error: Credit model key '%s' not supported. Available credit model keys include: MODEL_PROPERTIES, CDS_MARKETDATA, BOND_MARKETDATA, CREDIT_MODELS." ) % enumString ).str()  );
	}

	 std::string toString( const etrading::CreditBasketTypeEnum creditBasketTypeEnum )
    {
        switch( creditBasketTypeEnum )
        {
			case NONE_BASKET_TYPE:
				return "NONE";
			case HOMOGENEOUS_LOSS:
				return "HOMOGENEOUSLOSS";
			case INHOMOGENEOUS_LOSS:
				return "INHOMOGENEOUSLOSS";
            default:
				throw ETradingException(  ( boost::format( "#Error: CreditBasketType key '%i' not supported" ) % creditBasketTypeEnum  ).str()  );
                break;
		}
	}

	 CreditBasketTypeEnum toCreditBasketTypeEnum( const std::string& incomingString )
    {
         std::string enumString = etrading::trim_to_upper( incomingString.c_str() );
		if ((enumString == "NONE") || (enumString == ""))
		{
			return NONE_BASKET_TYPE;
		}
		if( enumString == "HOMOGENEOUSLOSS" )
        {
            return HOMOGENEOUS_LOSS;
        }
		if( enumString == "INHOMOGENEOUSLOSS" )
        {
            return INHOMOGENEOUS_LOSS;
        }
		if ( enumString.empty() )
        {
			throw ETradingException("#Error: No credit basket type provided. Available credit basket types include: HOMOGENEOUSLOSS, INHOMOGENEOUSLOSS.");
        }
		throw ETradingException(  ( boost::format( "#Error: Credit basket type '%s' not supported. Available credit basket types include: HOMOGENEOUSLOSS, INHOMOGENEOUSLOSS." ) % enumString ).str()  );
	}

	 std::string toString( const BondCurveEnum enumValue )
	{
	    switch( enumValue )
        {
			case BONDCURVE_NONE:
				return "NONE";
			case BONDCURVE_PROPERTIES:
				return "BONDCURVE_PROPERTIES";
			case BONDCURVE_MARKETDATA:
				return "BONDCURVE_MARKETDATA";
			case BONDSPREADCURVE_PROPERTIES:
				return "BONDSPREADCURVE_PROPERTIES";
            default:
				throw ETradingException(  ( boost::format( "#Error: BondCurveEnum key '%i' not supported" ) % enumValue  ).str()  );
                break;
		}		
	}

	 BondCurveEnum toBondCurveEnum( const std::string& enumString )
	{
	     std::string bcString = etrading::trim_to_upper( enumString.c_str() );
		if ((bcString == "NONE") || (bcString == ""))
		{
			return BONDCURVE_NONE;
		}
		if ( bcString == "BONDCURVE_PROPERTIES" )
        {
            return BONDCURVE_PROPERTIES;
        }
		if ( bcString == "BONDCURVE_MARKETDATA" )
        {
            return BONDCURVE_MARKETDATA;
        }
		if ( bcString == "BONDSPREADCURVE_PROPERTIES" )
        {
            return BONDSPREADCURVE_PROPERTIES;
        }

		if ( bcString.empty() )
        {
			throw ETradingException("#Error: No property key provided. Available property keys include: BONDCURVE_PROPERTIES, BONDCURVE_MARKETDATA.");
        }
		throw ETradingException(  ( boost::format( "#Error: Property key '%s' not supported. Available property keys include: NONE, BONDCURVE_PROPERTIES, BONDCURVE_MARKETDATA." ) % bcString ).str()  );	
	}

	 std::string toString( const BondGeneratorEnum enumValue )
	{
        switch( enumValue )
        {
			case NONE_BONDGENERATOR:
				return "NONE";
			case BOND_STATICDATA:
				return "BOND_STATICDATA";
			case BOND_SCHEDULE:
				return "BONDSCHEDULE";
            default:
				throw ETradingException(  ( boost::format( "#Error: BondGenerator key '%i' not supported" ) % enumValue  ).str()  );
                break;
		}		
	}

	 BondGeneratorEnum toBondGeneratorEnum( const std::string& bgString )
	{
	     std::string enumString = etrading::trim_to_upper( bgString.c_str() );
		if ((enumString == "NONE") || (enumString == ""))
		{
			return NONE_BONDGENERATOR;
		}
		if ( enumString == "BOND_STATICDATA" || enumString == "STATICDATA" || enumString == "STATIC" )
        {
            return BOND_STATICDATA;
        }
		if ( enumString == "BOND_SCHEDULE" || enumString == "SCHEDULE" )
        {
            return BOND_SCHEDULE;
        }

		if ( enumString.empty() )
        {
			throw ETradingException("#Error: No property key provided. Available property keys include: BOND_STATICDATA, BOND_SCHEDULE.");
        }
		throw ETradingException(  ( boost::format( "#Error: Property key '%s' not supported. Available property keys include: NONE, BOND_STATICDATA, BOND_SCHEDULE." ) % enumString ).str()  );
	 }

	 std::string toString( const InflationCurveEnum enumValue )
	 {
		 switch (enumValue)
		 {
		 case INFLATIONCURVE_NONE:
			 return "NONE";
		 case INFLATIONCURVE_PROPERTIES:
			 return "CURVE_PROPERTIES";
		 case INFLATIONCURVE_CPI_FIRST_YEAR:
			 return "CPI_FIRST_YEAR";
		 case INFLATIONCURVE_ZC_INFLATIONSWAPS:
			 return "ZC_INFLATIONSWAPS";
		 case INFLATIONCURVE_SEASONALITY:
			 return "SEASONALITY";
		 default:
			 throw ETradingException((boost::format("#Error: InflationCurveEnum key '%i' not supported") % enumValue).str());
			 break;
		 }
	 }

	 InflationCurveEnum toInflationCurveEnum( const std::string& enumString )
	 {
		 std::string icString = etrading::trim_to_upper(enumString.c_str());
		 if ((icString == "NONE") || (icString == ""))
		 {
			 return INFLATIONCURVE_NONE;
		 }
		 if (icString == "CURVE_PROPERTIES")
		 {
			 return INFLATIONCURVE_PROPERTIES;
		 }
		 if (icString == "CPI_FIRST_YEAR")
		 {
			 return INFLATIONCURVE_CPI_FIRST_YEAR;
		 }
		 if (icString == "ZC_INFLATIONSWAPS")
		 {
			 return INFLATIONCURVE_ZC_INFLATIONSWAPS;
		 }
		 if (icString == "SEASONALITY")
		 {
			 return INFLATIONCURVE_SEASONALITY;
		 }

		 if (icString.empty())
		 {
			 throw ETradingException("#Error: No property key provided. Available property keys include: CURVE_PROPERTIES, CPI_FIRST_YEAR, ZC_INFLATIONSWAPS, SEASONALITY.");
		 }
		 throw ETradingException((boost::format("#Error: Property key '%s' not supported. Available property keys include: NONE, CURVE_PROPERTIES, CPI_FIRST_YEAR, ZC_INFLATIONSWAPS, SEASONALITY.") % icString).str());
	 }

	 std::string toString( const InflationResetTypeEnum enumValue )
	 {
		 switch ( enumValue )
		 {
		 case INFLATION_RESET_TYPE_NONE:
			 return "NONE";
		 case INFLATION_RESET_TYPE_MONTHLY_INTERPOLATION:
			 return "MonthlyInterpolation";
		 case INFLATION_RESET_TYPE_DAILY_INTERPOLATION:
			 return "DailyInterpolation";
		 default:
			 throw ETradingException((boost::format("#Error: InflationResetTypeEnum key '%i' not supported") % enumValue).str());
			 break;
		 }
	 }

	 InflationResetTypeEnum toInflationResetTypeEnum( const std::string& enumString )
	 {
		 std::string ctString = etrading::trim_to_upper(enumString.c_str());
		 if ((ctString == "NONE") || (ctString == ""))
		 {
			 return INFLATION_RESET_TYPE_NONE;
		 }
		 if ( (ctString == "MONTHLYINTERPOLATION") || ( ctString == "MONTHLY" ) )
		 {
			 return INFLATION_RESET_TYPE_MONTHLY_INTERPOLATION;
		 }
		 if ( (ctString == "DAILYINTERPOLATION") || ( ctString == "DAILY") || ( ctString == "BUSINESS_DAYS") )
		 {
			 return INFLATION_RESET_TYPE_DAILY_INTERPOLATION;
		 }

		 if (ctString.empty())
		 {
			 throw ETradingException("#Error: No property key provided. Available property keys include: NONE, MONTHLYINTERPOLATION, DAILYINTERPOLATION.");
		 }
		 throw ETradingException((boost::format("#Error: Property key '%s' not supported. Available property keys include: NONE, MONTHLYINTERPOLATION, DAILYINTERPOLATION.") % ctString).str());
	 }

    std::string toString( const CCY ccyEnum )
    {
        switch( ccyEnum )
        {
			/* Null Case */
			case NO_CCY:	return "";		// Null Case

			/* Majors */
			case AUD:		return "AUD";	// Australia Dollar
			case EUR:		return "EUR";	// Europe Euro
			case GBP:		return "GBP";	// United Kingdom Pound
			case JPY:		return "JPY";	// Japan Yen
			case NZD:		return "NZD";	// New Zealand Dollar
			case USD:		return "USD";	// United States Dollar

			/* Minors */
			case AED:		return "AED";	// United Arab Emirates Dirham
			case ARS:		return "ARS";	// Argentina Peso
			case BRL:		return "BRL";	// Brazil Real
			case CAD:		return "CAD";	// Canada Dollar
			case CHF:		return "CHF";	// Swiss Franc
			case CNH:		return "CNH";	// China Yuan (HK Offshore)
			case CNY:		return "CNY";	// China Yuan (Onshore)
			case CLP:		return "CLP";	// Chile Peso
			case COP:		return "COP";	// Colombia Peso
			case CZK:		return "CZK";	// Czech Koruna
			case DKK:		return "DKK";	// Danish Krone
			case EGP:		return "EGP";	// Egyptian Pound
			case HKD:		return "HKD";	// Hong Kong Dollar
			case HRK:		return "HRK";	// Croatia Kuna
			case HUF:		return "HUF";	// Hungary Forint
			case INR:		return "INR";	// India Rupee
			case IDR:		return "IDR";	// Indonesia Rupiah
			case ILS:		return "ILS";	// Israel Shekel
			case KRW:		return "KRW";	// South Korea Won
			case MAD:		return "MAD";	// Morocco Dirham
			case MXN:		return "MXN";	// Mexico Peso
			case MYR:		return "MYR";	// Malaysia Ringgit
			case NGN:		return "NGN";	// Nigeria Naira
			case NOK:		return "NOK";	// Norway Krone
			case OMR:		return "OMR";	// Oman Rial
			case PHP:		return "PHP";	// Philippines Peso
			case PLN:		return "PLN";	// Poland Zloty
			case QAR:		return "QAR";	// Qatar Riyal
			case RON:		return "RON";	// Romania Leu
			case RSD:		return "RSD";	// Serbia Dinar
			case RUB:		return "RUB";	// Russia Ruble
			case SAR:		return "SAR";	// Saudi Arabia Riyal
			case SEK:		return "SEK";	// Sweden Krone
			case SGD:		return "SGD";	// Singapore Dollar
			case TWD:		return "TWD";	// Taiwan Dollar
			case THB:		return "THB";	// Thailand Baht
			case TRY:		return "TRY";	// Turkey Lira
			case UAH:		return "UAH";	// Ukraine Hryvnia
			case VND:		return "VND";	// Vietnam Dong
			case ZAR:		return "ZAR";	// South Africa Rand
			
            default:
				throw ETradingException( ( boost::format( "#Error: Currency '%i' is not supported" ) % ccyEnum  ).str()  );
                break;
        }
    }

    CCY toCCYEnum( const std::string& incomingString )
    {
        const std::string enumString = etrading::trim_to_upper( incomingString.c_str() );

		/* Null Case */
		if ( enumString == "" || enumString == "NONE" )	return NO_CCY;	// Null Case
	
		/* Majors */
		if ( enumString == "AUD" )		return AUD;		// Australia Dollar
		if ( enumString == "EUR" )		return EUR;		// Europe Euro
		if ( enumString == "GBP" )		return GBP;		// United Kingdom Pound
		if ( enumString == "JPY" )		return JPY;		// Japan Yen
		if ( enumString == "NZD" )		return NZD;		// New Zealand Dollar
		if ( enumString == "USD" )		return USD;		// United States Dollar

		/* Minors */
		if ( enumString == "AED" )		return AED;		// United Arab Emirates Dirham
		if ( enumString == "ARS" )		return ARS;		// Argentina Peso
		if ( enumString == "BRL" )		return BRL;		// Brazil Real
		if ( enumString == "CAD" )		return CAD;		// Canada Dollar
		if ( enumString == "CHF" )		return CHF;		// Swiss Franc
		if ( enumString == "CNH" )		return CNH;		// China Yuan (HK Offshore)
		if ( enumString == "CNY" )		return CNY;		// China Yuan (Onshore)
		if ( enumString == "CLP" )		return CLP;		// Chile Peso
		if ( enumString == "COP" )		return COP;		// Colombia Peso
		if ( enumString == "CZK" )		return CZK;		// Czech Koruna
		if ( enumString == "DKK" )		return DKK;		// Danish Krone
		if ( enumString == "EGP" )		return EGP;		// Egyptian Pound
		if ( enumString == "HKD" )		return HKD;		// Hong Kong Dollar
		if ( enumString == "HRK" )		return HRK;		// Croatia Kuna
		if ( enumString == "HUF" )		return HUF;		// Hungary Forint
		if ( enumString == "INR" )		return INR;		// India Rupee
		if ( enumString == "IDR" )		return IDR;		// Indonesia Rupiah
		if ( enumString == "ILS" )		return ILS;		// Israel Shekel
		if ( enumString == "KRW" )		return KRW;		// South Korea Won
		if ( enumString == "MAD" )		return MAD;		// Morocco Dirham
		if ( enumString == "MXN" )		return MXN;		// Mexico Peso
		if ( enumString == "MYR" )		return MYR;		// Malaysia Ringgit
		if ( enumString == "NGN" )		return NGN;		// Nigeria Naira
		if ( enumString == "NOK" )		return NOK;		// Norway Krone
		if ( enumString == "OMR" )		return OMR;		// Oman Rial
		if ( enumString == "PHP" )		return PHP;		// Philippines Peso
		if ( enumString == "PLN" )		return PLN;		// Poland Zloty
		if ( enumString == "QAR" )		return QAR;		// Qatar Riyal
		if ( enumString == "RON" )		return RON;		// Romania Leu
		if ( enumString == "RSD" )		return RSD;		// Serbia Dinar
		if ( enumString == "RUB" )		return RUB;		// Russia Ruble
		if ( enumString == "SAR" )		return SAR;		// Saudi Arabia Riyal
		if ( enumString == "SEK" )		return SEK;		// Sweden Krone
		if ( enumString == "SGD" )		return SGD;		// Singapore Dollar
		if ( enumString == "TWD" )		return TWD;		// Taiwan Dollar
		if ( enumString == "THB" )		return THB;		// Thailand Baht
		if ( enumString == "TRY" )		return TRY;		// Turkey Lira
		if ( enumString == "UAH" )		return UAH;		// Ukraine Hryvnia
		if ( enumString == "VND" )		return VND;		// Vietnam Dong
		if ( enumString == "ZAR" )		return ZAR;		// South Africa Rand

		throw ETradingException(  ( boost::format( "#Error: Currency '%i' is not supported" ) % enumString ).str()  );
    }

     std::string toString( const NotionalExchangeEnum enumValue )
    {
        switch( enumValue )
        {
            case NONE_NE:
                return "NONE";
                break;
            case START_NE:
                return "START";
                break;
            case END_NE:
                return "END";
                break;
            case START_AND_END_NE:
                return "START_AND_END";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: NotionalExchange value '%i' invalid, must be 'None', 'Start', 'End' or 'Start_And_End'." ) % enumValue ).str()  );
                break;
        }
    }

     NotionalExchangeEnum toNotionalExchangeEnum( const std::string& incomingString )
    {
         std::string enumString = etrading::trim_to_upper( incomingString.c_str() );
        if( enumString == "NONE" || enumString == "")
        {
            return NONE_NE;
        }
        if( enumString == "START" )
        {
            return START_NE;
        }
        if( enumString == "END" )
        {
            return END_NE;
        }
        if( enumString == "START_AND_END" || enumString == "STARTANDEND" || enumString == "START AND END")
        {
            return START_AND_END_NE;
        }
		throw ETradingException(  ( boost::format( "#Error: NotionalExchange '%s' invalid, must be 'NONE', 'START', 'END' or 'START_AND_END'." ) % enumString ).str()  );
    }


     std::string toString( const ScheduleTypeEnum enumValue )
    {
        switch( enumValue )
        {
            case NONE_SCHEDULE_TYPE:
				return "NONE";
				break;
			case FIXED_SCHEDULE_TYPE:
                return "FIXED";
                break;
            case FLOAT_SCHEDULE_TYPE:
                return "FLOAT";
                break;
            case FEE_SCHEDULE_TYPE:
                return "FEE";
                break;
            case FRA_SCHEDULE_TYPE:
                return "FRA";
                break;
			case PREMIUM_SCHEDULE_TYPE:
				return "PREMIUM";
			case PROTECTION_SCHEDULE_TYPE:
				return "PROTECTION";
			case SWAPSCHEDULE_FIXEDBOND: // Swap fixed leg as the fixed bond proxy
				return "FIXEDBOND"; 
			case SWAPSCHEDULE_FLOATBOND: // Swap float leg as the float bond proxy
				return "FLOATBOND"; 
			case BONDSCHEDULE_FIXEDBOND:
				return "BONDSCHEDULEFIXED";  //internal used for BOND
			case BONDSCHEDULE_FLOATBOND:
				return "BONDSCHEDULEFLOAT";
			case CMS_SCHEDULE_TYPE:
				return "CMS";
			case INFLATION_SCHEDULE_TYPE:
				return "INFLATION";
			default:
				throw ETradingException(  ( boost::format( "#Error: ScheduleType value '%i', invalid must be 'Fixed', Float', 'Fee', 'FRA', 'PREMIUM', 'PROTECTION', 'CMS', 'INFLATION', 'FIXEDBOND', 'FLOATBOND', 'BONDSCHEDULEFIXED', 'BONDSCHEDULEFLOAT'." ) % enumValue ).str() );
                break;
        }
    }

     ScheduleTypeEnum toScheduleTypeEnum( const std::string& incomingString )
    {
         std::string enumString = etrading::trim_to_upper( incomingString.c_str() );
		if( enumString == "NONE" || enumString == "")
        {
            return NONE_SCHEDULE_TYPE;
        }
        if( enumString  == "FIXED")
        {
            return FIXED_SCHEDULE_TYPE;
        }
        if( enumString  == "FLOAT")
        {
            return FLOAT_SCHEDULE_TYPE;
        }
        if( enumString  == "FEE")
        {
            return FEE_SCHEDULE_TYPE;
        }
        if( enumString  == "FRA")
        {
            return FRA_SCHEDULE_TYPE;
        }
		if( enumString  == "PREMIUM")
        {
            return PREMIUM_SCHEDULE_TYPE;
        }
		if( enumString  == "PROTECTION")
        {
            return PROTECTION_SCHEDULE_TYPE;
        }
		if (enumString == "FIXEDBOND") 
		{ 
			return SWAPSCHEDULE_FIXEDBOND; // Swap fixed leg as the fixed bond proxy
		}
		if (enumString == "FLOATBOND")
		{
			return SWAPSCHEDULE_FLOATBOND; // Swap float leg as the float bond proxy
		}
		if (enumString == "BONDSCHEDULEFIXED") 
		{
			return BONDSCHEDULE_FIXEDBOND; //internal used for BOND
		}
		if (enumString == "BONDSCHEDULEFLOAT") 
		{
			return BONDSCHEDULE_FLOATBOND;
		}
		if (enumString == "CMS")
		{
			return CMS_SCHEDULE_TYPE;
		}
		if (enumString == "INFLATION")
		{
			return INFLATION_SCHEDULE_TYPE;
		}
		throw ETradingException((boost::format("#Error: ScheduleType value '%i', invalid must be 'Fixed', Float', 'Fee', 'FRA', 'PREMIUM', 'PROTECTION', 'CMS', 'INFLATION', 'FIXEDBOND', 'FLOATBOND', 'BONDSCHEDULEFIXED', 'BONDSCHEDULEFLOAT'.") % enumString).str());
    }


	 std::string toString(const BespokeScheduleTypeEnum enumValue)
	{
		switch (enumValue)
		{
		case NONE_BESPOKE_SCHEDULE:
			return "NONE";
			break;
		case BESPOKE_SCHEDULE_WITH_PROPERTIES:
			return "BESPOKE_SCHEDULE_WITH_PROPERTIES";
		case BESPOKE_SCHEDULE:
			return "BESPOKE_SCHEDULE";
		default:
			throw ETradingException((boost::format("#Error: BespokeScheduleType value '%i', invalid must be 'BESPOKE_SCHEDULE_WITH_PROPERTIES', 'BESPOKE_SCHEDULE'.") % enumValue).str());
			break;
		}
	}

	 BespokeScheduleTypeEnum toBespokeScheduleTypeEnum(const std::string& incomingString)
	{
		 std::string enumString = etrading::trim_to_upper(incomingString.c_str());
		if (enumString == "NONE" || enumString == "")
		{
			return NONE_BESPOKE_SCHEDULE;
		}
		if (enumString == "BESPOKE_SCHEDULE_WITH_PROPERTIES")
		{
			return BESPOKE_SCHEDULE_WITH_PROPERTIES;
		}
		if (enumString == "BESPOKE_SCHEDULE")
		{
			return BESPOKE_SCHEDULE;
		}
		throw ETradingException((boost::format("#Error: BespokeScheduleType value '%i', invalid must be 'BESPOKE_SCHEDULE_WITH_PROPERTIES', 'BESPOKE_SCHEDULE'.") % enumString).str());
	}

     std::string toString( const SwapTypeEnum enumValue )
    {
        switch( enumValue )
        {
            case VANILLA_SWAP:
                return "VanillaSwap";
                break;
            case CROSS_CURRENCY_SWAP:
                return "CrossCurrencySwap";
                break;
            case ZERO_COUPON_SWAP:
                return "ZeroCouponSwap";
                break;
            case XCCY_ZERO_COUPON_SWAP:
                return "XccyZeroCouponSwap";
                break;
			case CREDIT_DEFAULT_SWAP:
				return "CreditDefaultSwap";
			case CONSTANT_MATURITY_SWAP:
				return "ConstantMaturitySwap";
			case TOTAL_RETURN_SWAP:
				return "TotalReturnSwap";
			case ZERO_COUPON_INFLATION_SWAP:
				return "ZeroCouponInflationSwap";
            default:
				throw ETradingException(  ( boost::format( "#Error: SwapType value '%i' invalid, must be 'VanillaSwap', 'CrossCurrencySwap', 'ZeroCouponSwap', 'XccyZeroCouponSwap', 'CreditDefaultSwap', 'ConstantMaturitySwap', 'TotalReturnSwap' or 'ZeroCouponInflationSwap'." ) % enumValue ).str()  );
                break;
        }
    }

     SwapTypeEnum toSwapTypeEnum( const std::string& incomingString )
    {
         std::string enumString = etrading::trim_to_upper( incomingString.c_str() );
        if( boost::iequals( enumString.c_str(), "VanillaSwap" ) )
        {
            return VANILLA_SWAP;
        }
        if( boost::iequals( enumString.c_str(), "CrossCurrencySwap" ) )
        {
            return CROSS_CURRENCY_SWAP;
        }
        if( boost::iequals( enumString.c_str(), "ZeroCouponSwap" ) )
        {
            return ZERO_COUPON_SWAP;
        }
        if( boost::iequals( enumString.c_str(), "XccyZeroCouponSwap" ) )
        {
            return XCCY_ZERO_COUPON_SWAP;
        }
		if( boost::iequals( enumString.c_str(), "CreditDefaultSwap" ) )
        {
            return CREDIT_DEFAULT_SWAP;
        }
		if( boost::iequals( enumString.c_str(), "ConstantMaturitySwap" ) )
        {
            return CONSTANT_MATURITY_SWAP;
        }
		if( boost::iequals( enumString.c_str(), "TotalReturnSwap" ) )
        {
            return TOTAL_RETURN_SWAP;
        }
		if (boost::iequals(enumString.c_str(), "ZeroCouponInflationSwap"))
		{
			return ZERO_COUPON_INFLATION_SWAP;
		}
		throw ETradingException(  ( boost::format( "#Error: SwapType '%s' invalid, must be 'VanillaSwap', 'CrossCurrencySwap', 'ZeroCouponSwap', 'XccyZeroCouponSwap', 'CreditDefaultSwap', 'ConstantMaturitySwap', 'TotalReturnSwap' or 'ZeroCouponInflationSwap'." ) % enumString ).str()  );
    }

	 std::string toString( const PaymentTriggerEnum enumValue )
    {
        switch( enumValue )
        {
            case PAY_ALWAYS:
                return "PayAlways";
                break;
            case PAY_ON_SURVIVAL:
                return "PayOnSurvival";
                break;
            case PAY_ON_DEFAULT:
                return "PayOnDefault";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: PaymentTrigger value '%i' invalid, must be 'PayAlways', 'PayOnSurvival', or 'PayOnDefault'." ) % enumValue ).str()  );
                break;
        }
	}

     PaymentTriggerEnum toPaymentTriggerEnum( const std::string& incomingString )
    {
         std::string enumString = etrading::trim_to_upper( incomingString.c_str() );
        if( boost::iequals( enumString.c_str(), "PayAlways" ) )
        {
            return PAY_ALWAYS;
        }
        if( boost::iequals( enumString.c_str(), "PayOnSurvival" ) )
        {
            return PAY_ON_SURVIVAL;
        }
        if( boost::iequals( enumString.c_str(), "PayOnDefault" ) )
        {
            return PAY_ON_DEFAULT;
        }
		throw ETradingException(  ( boost::format( "#Error: PaymentTrigger value '%s' invalid, must be 'PayAlways', 'PayOnSurvival', or 'PayOnDefault'." ) % enumString ).str()  );
    }

	 std::string toString( const TRSAssetPerformanceEnum enumValue )
    {
        switch( enumValue )
        {
            case NO_PERFORMANCE:
                return "NONE";
                break;
            case PAY_AT_MATURITY_PERFORMANCE:
                return "PayAtMaturity";
                break;
            case PAY_EACH_COUPON_PERFORMANCE:
                return "PayEachCoupon";
                break;
			case RESTRIKE_NOTIONAL_PERFORMANCE:
                return "RestrikeNotional";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: TRS PerformanceCalculation value '%i' invalid, must be 'None', 'PayAtMaturity', 'PayEachCoupon', or 'RestrikeNotional'." ) % enumValue ).str()  );
                break;
        }
	}

     TRSAssetPerformanceEnum toTRSAssetPerformanceEnum( const std::string& incomingString )
    {
         std::string enumString = etrading::trim_to_upper( incomingString.c_str() );

        if( boost::iequals( enumString.c_str(), "NONE" ) || enumString == "" )
        {
            return NO_PERFORMANCE;
        }
        if( boost::iequals( enumString.c_str(), "PayAtMaturity" ) )
        {
            return PAY_AT_MATURITY_PERFORMANCE;
        }
        if( boost::iequals( enumString.c_str(), "PayEachCoupon" ) )
        {
            return PAY_EACH_COUPON_PERFORMANCE;
        }
		if( boost::iequals( enumString.c_str(), "RestrikeNotional" ) )
        {
            return RESTRIKE_NOTIONAL_PERFORMANCE;
        }
		throw ETradingException(  ( boost::format( "#Error: TRS PerformanceCalculation value '%i' invalid, must be 'None', 'PayAtMaturity', 'PayEachCoupon', or 'RestrikeNotional'." ) % enumString ).str()  );
    }

     std::string toString( const FileTypeEnum enumValue )
    {
        switch( enumValue )
        {
            case JSON:
                return "JSON";
                break;
            case TEXT:
                return "TEXT";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: SwapType value '%i' invalid, must be 'JSON' or 'TEXT'." ) % enumValue ).str()  );
                break;
        }
    }

     FileTypeEnum toFileTypeEnum( const std::string& enumString )
    {
        std::string thisEnumString = etrading::trim_to_upper( enumString.c_str() );
        if( boost::iequals( thisEnumString.c_str(), "JSON" ) )
        {
            return JSON;
        }
        if( boost::iequals( thisEnumString.c_str(), "TEXT" ) )
        {
            return TEXT;
        }
        
		throw ETradingException(  ( boost::format( "#Error: SwapType '%s' invalid, must be 'JSON' or 'TEXT'." ) % thisEnumString ).str()  );
    }


     std::string toString( const FrequencyEnum enumValue )
    {
        switch( enumValue )
        {
            case NONE_FREQUENCY:
                return "NONE";
                break;
            case ANNUAL_FREQUENCY:
                return "ANNUAL";
                break;
            case SEMI_ANNUAL_FREQUENCY:
                return "SEMI-ANNUAL";
                break;
            case QUARTERLY_FREQUENCY:
                return "QUARTERLY";
                break;
            case MONTHLY_FREQUENCY:
                return "MONTHLY";
                break;
            case WEEKLY_FREQUENCY:
                return "WEEKLY";
                break;
            case DAILY_FREQUENCY:
                return "DAILY";
                break;
            case AT_MATURITY_FREQUENCY:
                return "AT_MATURITY";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: Frequency '%i' invalid, must be 'NONE', 'ANNUAL', 'SEMI_ANNUAL', 'QUARTERLY', 'MONTHLY', 'WEEKLY', 'DAILY' or 'AT_MATURITY'." ) % enumValue ).str()  );
                break;
        };
    }

     FrequencyEnum toFrequencyEnum( const std::string& enumString )
    {
         std::string compareString = trim_to_upper( enumString.c_str() );
		if( enumString == "NONE" || enumString == "")
        {
            return NONE_FREQUENCY;
        }
        if( compareString == "ANNUAL")
        {
            return ANNUAL_FREQUENCY;
        }
        if( compareString == "SEMI_ANNUAL"  || compareString == "SEMI-ANNUAL")
        {
            return SEMI_ANNUAL_FREQUENCY;
        }
        if( compareString == "QUARTERLY")
        {
            return QUARTERLY_FREQUENCY;
        }
        if( compareString == "MONTHLY" || compareString == "MONTH")
        {
            return MONTHLY_FREQUENCY;
        }
        if( compareString == "WEEKLY" || compareString == "WEEK")
        {
            return WEEKLY_FREQUENCY;
        }
        if( compareString == "DAILY" || compareString == "BUSINESS_DAYS" )
        {
            return DAILY_FREQUENCY;
        }
        if( compareString == "AT MATURITY" || compareString == "AT_MATURITY" || compareString == "ATMATURITY" || compareString == "MATURITY")
        {
            return AT_MATURITY_FREQUENCY;
        }
		throw ETradingException(  ( boost::format( "#Error: Frequency '%i' invalid, must be 'NONE', 'ANNUAL', 'SEMI_ANNUAL', 'QUARTERLY', 'MONTHLY', 'WEEKLY', 'DAILY' or 'AT_MATURITY'." ) % enumString ).str()  );
    }
  
    // Helper Method to Convert from FrequencyEnum to CurveTenorEnum, which are similar
     CurveTenorEnum fromFrequencyEnumtoCurveTenorEnum( const FrequencyEnum frequencyEnum )
    {
        switch( frequencyEnum )
        {
            case DAILY_FREQUENCY:
                return CURVE_TENOR_1D;
                break;
            case MONTHLY_FREQUENCY:
                return CURVE_TENOR_1M;
                break;
            case QUARTERLY_FREQUENCY:
                return CURVE_TENOR_3M;
                break;
            case SEMI_ANNUAL_FREQUENCY:
                return CURVE_TENOR_6M;
                break;
            case ANNUAL_FREQUENCY:
                return CURVE_TENOR_12M;
                break;
            case NONE_FREQUENCY:
                return NONE_CURVE_TENOR;
                break;
            case WEEKLY_FREQUENCY:
                throw ETradingException( "#Error: Invalid Curve Frequency Tenor"  );
                break;
            case AT_MATURITY_FREQUENCY:
                throw ETradingException( "#Error: Invalid Curve Frequency Tenor"  );
                break;
            default:
				throw ETradingException( "#Error: Invalid Curve Frequency Tenor"  );
                break;
        };
    }

     std::string toString( const ProductEnum enumValue )
    {
        switch( enumValue )
        {
            case NONE_PRODUCT:
                return "NONE";
                break;
            case SWAP_PRODUCT:
                return "SWAP";
                break;
            case BOND_PRODUCT:
                return "BOND";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error:Product '%i' invalid, must be 'NONE', 'SWAP' or 'BOND'." ) % enumValue ).str()  );
                break;
        };
    }

     ProductEnum toProductEnum( const std::string& enumString )
    {
         std::string compareString = trim_to_upper( enumString.c_str() );
        if( enumString == "NONE" )
        {
            return NONE_PRODUCT;
        }
        if( compareString == "SWAP" )
        {
            return SWAP_PRODUCT;
        }
        if( compareString == "BOND" )
        {
            return BOND_PRODUCT;
        }
		throw ETradingException(  ( boost::format( "#Error:Product '%i' invalid, must be 'NONE', 'SWAP' or 'BOND'." ) % enumString ).str()  );
    }

	 std::string toUpperString(const CashflowHeaderEnum enumValue)
	{
		return trim_to_upper(toString(enumValue).c_str());
	}

     std::string toString( const CashflowHeaderEnum enumValue )
    {
        switch( enumValue )
        {
            case FIXING_DATE_HEADER:
                return "FixingDate";
                break;
			case FIXING_END_DATE_HEADER:
				return "FixingEndDate";
				break;
			case ACCRUAL_START_HEADER:
                return "AccrualStart";
                break;
            case ACCRUAL_END_HEADER:
                return "AccrualEnd";
                break;
            case ACCRUAL_DAYS_HEADER:
                return "AccrualDays";
                break;
            case ACCRUAL_YEAR_FRACTIONS_HEADER:
                return "AccrualYearFractions";
                break;
            case TRUE_YIELD_YEAR_FRACTIONS_HEADER:
                return "TrueYieldYearFractions";
                break;
			case EX_DIVIDEND_DATE_HEADER:
				return "ExDividendDate";
				break;
			case PAYMENT_DATE_HEADER:
                return "PaymentDate";
                break;
            case NOTIONAL_HEADER:
                return "Notional";
                break;
            case NOTIONAL_EXCHANGE_HEADER:
                return "NotionalExchange";
                break;
            case LEVERAGE_HEADER:
                return "Leverage";
                break;
			case COUPON_MULTIPLIER_HEADER:
				return "CouponMultiplier";
				break;
            case FIXED_RATE_HEADER:
                return "FixedRate";
                break;
            case FLOAT_SPREAD_HEADER:
                return "FloatSpread";
                break;
            case FLOAT_RATE_HEADER:
                return "FloatRate";
                break;
			case CONVEXITY_HEADER:
				return "Convexity";
				break;
			case UNADJUSTED_FLOAT_RATE_HEADER:
				return "UnadjustedFloatRate";
				break;
			case FX_FIXING_DATE_HEADER:
                return "FxFixingDate";
                break;
            case FX_RATE_HEADER:
                return "FxRate";
                break;
            case COUPON_HEADER:
                return "Coupon";
                break;
            case DISCOUNT_FACTOR_HEADER:
                return "DiscountFactor";
                break;
            case COUPON_PV_HEADER:
                return "CouponPV";
                break;
            case AMOUNT_HEADER:
                return "Amount";
                break;
            case PAY_RECEIVE_HEADER:
                return "PayReceive";
                break;
            case STRIKE_RATE_HEADER:
                return "StrikeRate";
                break;
			case CDS_SPREAD_HEADER:
				return "CDSSpread";
				break;
			case SURVIVAL_PROBABILITY_HEADER:
				return "SurvivalProbability";
				break;
			case MARGINAL_DEFAULT_PROBABILITY_HEADER:
				return "MarginalDefaultProbability";
				break;
			case PREMIUM_COUPON_HEADER:
				return "PremiumCoupon";
				break;
			case RISKY_COUPON_HEADER:
				return "RiskyCoupon";
				break;
			case ACCRUAL_ON_DEFAULT_COUPON_HEADER:
				return "AccrualOnDefaultCoupon";
				break;
			case EXPIRY_DATE_HEADER:
				return "ExpiryDate";
				break;
			case STRIKE_HEADER:
				return "Strike";
				break;
			case VOL_HEADER:
				return "Vol";
				break;
			case ACCRUAL_DAYCOUNT_HEADER:
				return "AccrualDaycount";
				break;
			case FIXING_BUSINESSDAYADJUSTMENT_HEADER:
				return "FixingBusinessDayAdjustment";
				break;
			case FIXING_CALENDAR_HEADER:
				return "FixingCalendar";
				break;
			case FORECAST_CURVE_HEADER:
				return "ForecastCurve";
				break;
			default:
				throw ETradingException(  ( boost::format( "#Error:Column header '%i' invalid, must be 'FixingDate', 'AccrualStart', 'AccrualEnd', 'AccrualDays', 'AccrualYearFractions', 'TrueYieldYearFractions', 'ExDividendDate', 'PaymentDate', 'Notional', 'NotionalExchange', 'Leverage', 'FixedRate', 'FloatSpread', 'FloatRate', 'FxFixingDate', 'FxRate', 'Coupon', 'DiscountFactor', 'CouponPV', 'Amount', 'PayReceive', 'StrikeRate', 'ExpiryDate', 'Strike', 'Vol', 'AccrualDaycount', 'FixingBusinessDayAdjustment', 'FixingCalendar', 'ForecastCurve'." ) % enumValue ).str()  );
                break;
        };
    }

     CashflowHeaderEnum toCashflowHeaderEnum( const std::string& enumString )
    {
         std::string compareString = trim_to_upper( enumString.c_str() );

        if( compareString == "FIXINGDATE" || compareString == "FIXING DATE" || compareString == "FIXING_DATE" )
        {
            return FIXING_DATE_HEADER;
        }
		if (compareString == "FIXINGENDDATE")
		{
			return FIXING_END_DATE_HEADER;
		}
		if( compareString == "ACCRUALSTART" || compareString == "ACCRUAL START" || compareString == "ACCRUAL_START" )
        {
            return ACCRUAL_START_HEADER;
        }
        if( compareString == "ACCRUALEND" || compareString == "ACCRUAL END" || compareString == "ACCRUAL_END" )
        {
            return ACCRUAL_END_HEADER;
        }
        if( compareString == "ACCRUALDAYS" || compareString == "ACCRUALDAY" || compareString == "ACCRUAL DAYS" || compareString == "ACCRUAL_DAYS")
        {
            return ACCRUAL_DAYS_HEADER;
        }
        if( compareString == "ACCRUALEND" || compareString == "ACCRUAL END" || compareString == "ACCRUAL_END" )
        {
            return ACCRUAL_END_HEADER;
        }
        if( compareString == "ACCRUALYEARFRACTIONS" || compareString == "ACCRUALYEARFRACTION" || compareString == "ACCRUAL YEAR FRACTIONS" || compareString == "ACCRUAL_YEAR_FRACTIONS")
        {
            return ACCRUAL_YEAR_FRACTIONS_HEADER;
        }
        if( compareString == "TRUEYIELDYEARFRACTIONS" || compareString == "TRUEYIELDYEARFRACTION" || compareString == "TRUE YIELD YEAR FRACTIONS" || compareString == "TRUE_YIELD_YEAR_FRACTIONS" )
        {
            return TRUE_YIELD_YEAR_FRACTIONS_HEADER;
        }
        if( compareString == "EXDIVIDENDDATE" || compareString == "EXDIVIDEN DDATE" || compareString == "EX DIVIDEND DATE" || compareString == "EX_DIVIDEND_DATE")
        {
            return EX_DIVIDEND_DATE_HEADER;
        }
		if (compareString == "PAYMENTDATE" || compareString == "PAYMENT DATE" || compareString == "PAYMENT_DATE")
		{
			return PAYMENT_DATE_HEADER;
		}
		if( compareString == "NOTIONAL" )
        {
            return NOTIONAL_HEADER;
        }
        if( compareString == "NOTIONALEXCHANGE" || compareString == "NOTIONAL EXCHANGE" || compareString == "NOTIONAL_EXCHANGE" )
        {
            return NOTIONAL_EXCHANGE_HEADER;
        }
        if( compareString == "LEVERAGE" )
        {
            return LEVERAGE_HEADER;
        }
        if( compareString == "COUPONMULTIPLIER" )
        {
            return COUPON_MULTIPLIER_HEADER;
        }
        if( compareString == "FIXEDRATE" || compareString == "FIXED RATE" || compareString == "FIXED_RATE" )
        {
            return FIXED_RATE_HEADER;
        }
        if( compareString == "FLOATSPREAD" || compareString == "FLOAT SPREAD" || compareString == "FLOAT_SPREAD" )
        {
            return FLOAT_SPREAD_HEADER;
        }
        if( compareString == "FLOATRATE" || compareString == "FLOAT RATE" || compareString == "FLOAT_RATE" )
        {
            return FLOAT_RATE_HEADER;
        }
		if (compareString == "CONVEXITY" )
		{
			return CONVEXITY_HEADER;
		}
		if (compareString == "UNADJUSTEDFLOATRATE") 
		{
			return UNADJUSTED_FLOAT_RATE_HEADER;
		}
		if( compareString == "FXFIXINGDATE" || compareString == "FX FIXING DATE" || compareString == "FX_FIXING_DATE" )
        {
            return FX_FIXING_DATE_HEADER;
        }
        if( compareString == "FXRATE" || compareString == "FX RATE" || compareString == "FX_RATE" )
        {
            return FX_RATE_HEADER;
        }
        if( compareString == "COUPON" )
        {
            return COUPON_HEADER;
        }
        if( compareString == "DISCOUNTFACTOR" || compareString == "DISCOUNT FACTOR"|| compareString == "DISCOUNT_FACTOR" )
        {
            return DISCOUNT_FACTOR_HEADER;
        }
        if( compareString == "PV" || compareString == "COUPONPV" || compareString == "COUPON PV" || compareString == "COUPON_PV" )
        {
            return COUPON_PV_HEADER;
        }
        if( compareString == "AMOUNT")
        {
            return AMOUNT_HEADER;
        }
        if( compareString == "PAYRECEIVE" || compareString == "PAY RECEIVE" || compareString == "PAY_RECEIVE" )
        {
            return PAY_RECEIVE_HEADER;
        }
        if( compareString == "STRIKERATE" || compareString == "STRIK ERATE" || compareString == "STRIK_ERATE" )
        {
            return STRIKE_RATE_HEADER;
        }
		if ( compareString == "CDSSPREAD" )
		{
			return CDS_SPREAD_HEADER;
		}
		if ( compareString == "SURVIVALPROBABILITY" )
		{
			return SURVIVAL_PROBABILITY_HEADER;
		}
		if ( compareString == "MARGINALDEFAULTPROBABILITY" )
		{
			return MARGINAL_DEFAULT_PROBABILITY_HEADER;
		}
		if ( compareString == "PREMIUMCOUPON" )
		{
			return PREMIUM_COUPON_HEADER;
		}
		if ( compareString == "RISKYCOUPON" )
		{
			return RISKY_COUPON_HEADER;
		}
		if ( compareString == "ACCRUALONDEFAULTCOUPON" )
		{
			return ACCRUAL_ON_DEFAULT_COUPON_HEADER;
		}
		if (compareString == "EXPIRYDATE" || compareString == "EXPIRY DATE" || compareString == "EXPIRY_DATE")
		{
			return EXPIRY_DATE_HEADER;
		}
		if (compareString == "STRIKE")
		{
			return STRIKE_HEADER;
		}
		if (compareString == "VOL" || compareString == "VOLATILITY")
		{
			return VOL_HEADER;
		}
		if (compareString == "ACCRUALDAYCOUNT")
		{
			return ACCRUAL_DAYCOUNT_HEADER;
		}
		if (compareString == "FIXINGBUSINESSDAYADJUSTMENT")
		{
			return FIXING_BUSINESSDAYADJUSTMENT_HEADER;
		}
		if (compareString == "FIXINGCALENDAR")
		{
			return FIXING_CALENDAR_HEADER;
		}
		if (compareString == "FORECASTCURVE")
		{
			return FORECAST_CURVE_HEADER;
		}
		throw ETradingException(  ( boost::format( "#Error:Column header '%i' invalid, must be 'FixingDate', 'AccrualStart', 'AccrualEnd', 'AccrualDays', 'AccrualYearFractions', 'TrueYieldYearFractions', 'ExDividendDate', 'PaymentDate', 'Notional', 'NotionalExchange', 'Leverage', 'CouponMultiplier', 'FixedRate', 'FloatSpread', 'FloatRate', 'FxFixingDate', 'FxRate', 'Coupon', 'DiscountFactor', 'CouponPV', 'Amount', 'PayReceive','StrikeRate', 'ExpiryDate', 'Strike', 'Vol', 'CDSSpread', 'SurvivalProbability', 'MarginalDefaultProbability', 'PremiumCoupon', 'AccrualOnDefaultCoupon', 'AccrualDaycount', 'FixingBusinessDayAdjustment', 'FixingCalendar', 'ForecastCurve'." ) % enumString ).str()  );
    }

     std::unordered_set<CashflowHeaderEnum,EnumClassHash> toCashflowHeaderEnumSet( const std::vector<std::string>& enumStrings )
    {
        std::unordered_set<CashflowHeaderEnum,EnumClassHash> enumSet;

        for (size_t i = 0; i < enumStrings.size(); ++i)
        {
            enumSet.insert(toCashflowHeaderEnum(enumStrings[i]));
        }
        return enumSet;
    }


     std::string toString( const CallOrPutEnum enumValue )
    {
        switch( enumValue )
        {
			case NONE_OPTION:
				return "NONE";
				break;
			case CALL_OPTION:
                return "CALL";
                break;
            case PUT_OPTION:
                return "PUT";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error: Invalid CallOrPut flag '%i', must be 'CALL' or 'PUT." ) % enumValue ).str()  );
                break;
        }
    }

     CallOrPutEnum toCallOrPutEnum( const std::string& enumString )
    {
         std::string compareString = trim_to_upper( enumString.c_str() );
		if (enumString == "NONE" || enumString == "")
		{
			return NONE_OPTION;
		}
		if( compareString == "CALL" )
        {
            return CALL_OPTION;
        }
        if( compareString == "PUT" )
        {
            return PUT_OPTION;
        }
		throw ETradingException(  ( boost::format( "#Error: Invalid CallOrPut flag '%s', must be 'CALL' or 'PUT." ) % enumString ).str()  );
    }

	 std::string toString(const CapFloorEnum enumValue)
	{
		switch (enumValue)
		{
		case CAP_OPTION:
			return "CAP";
			break;
		case FLOOR_OPTION:
			return "FLOOR";
			break;
		default:
			throw ETradingException((boost::format("#Error: Invalid CapOrFloor flag '%i', must be 'CAP' or 'FLOOR'.") % enumValue).str());
			break;
		}
	}

	 CapFloorEnum toCapFloorEnum(const std::string& enumString)
	{
		 std::string compareString = trim_to_upper(enumString.c_str());
		if (compareString == "CAP")
		{
			return CAP_OPTION;
		}
		if (compareString == "FLOOR")
		{
			return FLOOR_OPTION;
		}
		throw ETradingException((boost::format("#Error: Invalid CapOrFloor flag '%s', must be 'CAP' or 'FLOOR'.") % enumString).str());
	}

     std::string toString(const CapletFloorletEnum enumValue)
	{
		switch (enumValue)
		{
		case CAPLET_OPTION:
			return "CAPLET";
			break;
		case FLOOR_OPTION:
			return "FLOORLET";
			break;
		default:
			throw ETradingException((boost::format("#Error: Invalid CapletOrFloorlet flag '%i', must be 'CAPLET' or 'FLOORLET'.") % enumValue).str());
			break;
		}
	}

	 CapletFloorletEnum toCapletFloorletEnum(const std::string& enumString)
	{
		 std::string compareString = trim_to_upper(enumString.c_str());
		if (compareString == "CAPLET")
		{
			return CAPLET_OPTION;
		}
		if (compareString == "FLOORLET")
		{
			return FLOORLET_OPTION;
		}
		throw ETradingException((boost::format("#Error: Invalid CapletOrFloorlet flag '%s', must be 'CAPLET' or 'FLOORLET'.") % enumString).str());
	}

	 std::string toString( const FXPriceEnum enumValue )
    {
        switch( enumValue )
        {
            case NONE_FXPRICE:
                return "None";
                break;
            case OUTRIGHT_FXPRICE:
                return "Outright";
                break;
            case POINTS_FXPRICE:
                return "Points";
                break;
            case BID_FXPRICE:
                return "Bid";
                break;
            case ASK_FXPRICE:
                return "Ask";
                break;
            case MID_FXPRICE:
                return "Mid";
                break;
            case BID_OUTRIGHT_FXPRICE:
                return "BidOutright";
                break;
            case ASK_OUTRIGHT_FXPRICE:
                return "AskOutright";
                break;
            case MID_OUTRIGHT_FXPRICE:
                return "MidOutright";
                break;
            case BID_POINTS_FXPRICE:
                return "BidPoints";
                break;
            case ASK_POINTS_FXPRICE:
                return "AskPoints";
                break;
            case MID_POINTS_FXPRICE:
                return "MidPoints";
                break;
            default:
				throw ETradingException(  ( boost::format( "#Error:Product '%i' invalid, must be 'None', 'Outright' , 'Points', 'Bid', 'Ask', 'Mid', 'BidOutright', 'AskOutright', 'MidOutright', 'BidPoints', 'AskPoints' or 'MidPoints'." ) % enumValue ).str()  );
                break;
        };
    }

     FXPriceEnum toFXPriceEnum( const std::string& enumString )
    {
         std::string compareString = trim_to_upper( enumString.c_str() );
        if( enumString == "NONE" || enumString == "" )
        {
            return NONE_FXPRICE;
        }
        if( compareString == "OUTRIGHT" )
        {
            return OUTRIGHT_FXPRICE;
        }
        if( compareString == "POINTS")
        {
            return POINTS_FXPRICE;
        }
        if( compareString == "BID")
        {
            return BID_FXPRICE;
        }
        if( compareString == "ASK")
        {
            return ASK_FXPRICE;
        }
        if( compareString == "MID")
        {
            return MID_FXPRICE;
        }
        if( compareString == "BIDOUTRIGHT" ||  compareString == "BID OUTRIGHT"  || compareString == "BID_OUTRIGHT" || compareString == "BID-OUTRIGHT")
        {
            return BID_OUTRIGHT_FXPRICE;
        }
        if( compareString == "ASKOUTRIGHT" ||  compareString == "ASK OUTRIGHT"  || compareString == "ASK_OUTRIGHT" || compareString == "ASK-OUTRIGHT")
        {
            return ASK_OUTRIGHT_FXPRICE;
        }
        if( compareString == "BIDPOINTS" ||  compareString == "BID POINTS"  || compareString == "BID_POINTS" || compareString == "BID-POINTS")
        {
            return BID_POINTS_FXPRICE;
        }
        if( compareString == "ASKPOINTS" ||  compareString == "ASK POINTS"  || compareString == "ASK_POINTS" || compareString == "ASK-POINTS")
        {
            return ASK_POINTS_FXPRICE;
        }
        if( compareString == "MIDPOINTS" ||  compareString == "MID POINTS"  || compareString == "MID_POINTS" || compareString == "MID-POINTS")
        {
            return MID_POINTS_FXPRICE;
        }
        throw ETradingException(  ( boost::format( "#Error:Product '%i' invalid, must be 'None', 'Outright' , 'Points', 'Bid', 'Ask', 'Mid', 'BidOutright', 'AskOutright', 'MidOutright', 'BidPoints', 'AskPoints' or 'MidPoints'." ) % enumString ).str()  );
    }

	 std::unordered_set<FXPriceEnum, EnumClassHash> toFXPriceEnumSet( const std::vector<std::string>& enumStrings )
    {
        std::unordered_set<FXPriceEnum, EnumClassHash> enumSet;

        for (size_t i = 0; i < enumStrings.size(); ++i)
        {
            enumSet.insert(toFXPriceEnum(enumStrings[i]));
        }
        return enumSet;
    }

	 std::string toString( const BooleanEnum enumValue )
    {
        switch( enumValue )
        {
            case NONE_BOOL:
                return "NONE";
                break;
            case TRUE_BOOL:
                return "TRUE";
                break;
            case FALSE_BOOL:
                return "FALSE";
                break;
            default:
				throw ETradingException( "#Error: Invalid Boolean Value, must be 'NONE', 'TRUE', or 'FALSE'" );
                break;
        }
    }

    BooleanEnum toBooleanEnum( const std::string& enumString )
    {
        std::string uppercaseEnumString = etrading::trim_to_upper( enumString.c_str() );

		if( uppercaseEnumString == "" || uppercaseEnumString == "NONE")
        {
            return NONE_BOOL;
        }
		if( uppercaseEnumString == "TRUE")
        {
            return TRUE_BOOL;
        }
		if( uppercaseEnumString == "FALSE")
        {
            return FALSE_BOOL;
        }
		throw ETradingException( "#Error: Invalid Boolean Value, must be 'NONE', 'TRUE', or 'FALSE'" );
    }

	bool toBoolean( const BooleanEnum enumValue )
    {
        switch( enumValue )
        {
            case TRUE_BOOL:
                return true;
                break;
            case FALSE_BOOL:
                return false;
                break;
            default:
				throw ETradingException( "#Error: Invalid Boolean Value, must be 'TRUE' or 'FALSE'" );
                break;
        }
    }

     bool toBooleanFromString( const std::string& enumString )
    {
        std::string uppercaseEnumString = etrading::trim_to_upper( enumString.c_str() );

		if( uppercaseEnumString == "TRUE")
        {
            return true;
        }

        if( uppercaseEnumString == "FALSE")
        {
            return false;
        }
		
        throw ETradingException( "#Error: Invalid Boolean Value, must be 'TRUE' or 'FALSE'" );
    }

     BooleanEnum toBooleanEnumFromBool( const bool& boolValue )
    {
        if( boolValue == true )
        {
            return TRUE_BOOL;
        }

        if( boolValue == false )
        {
            return FALSE_BOOL;
        }
		
        throw ETradingException( "#Error: Invalid Boolean Value, must be 'TRUE' or 'FALSE'" );
    }

    StateVariableEnum toStateVariableEnum( const std::string& enumString )
    {
        std::string uppercaseEnumString = etrading::trim_to_upper( enumString.c_str() );

		if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		{
			return STATE_VARIABLE_NONE;
		}
		if( uppercaseEnumString == "ZERO_RATE" || uppercaseEnumString == "ZERORATE")
        {
            return STATE_VARIABLE_ZERO_RATE;
        }
		if( uppercaseEnumString == "ZERO_RATE_TIMES_TIME" || uppercaseEnumString == "RATETIME" || uppercaseEnumString == "ZERORATETIMESTIME")
        {
            return STATE_VARIABLE_ZERO_RATE_TIMES_TIME;
        }
		if( uppercaseEnumString == "LOG_DISCOUNT_FACTOR" || uppercaseEnumString == "LOGDISCOUNTFACTOR" || uppercaseEnumString == "LOG_DF" || uppercaseEnumString == "LOGDF")
        {
            return STATE_VARIABLE_LOG_DF;
        }
        if( uppercaseEnumString == "DISCOUNT_FACTOR" || uppercaseEnumString == "DISCOUNTFACTOR" || uppercaseEnumString == "DF" )
		 {
			 return STATE_VARIABLE_DF;
		 }
		 if (uppercaseEnumString == "FORWARD_RATE" || uppercaseEnumString == "FORWARDRATE")
		 {
			 return STATE_VARIABLE_FORWARD_RATE;
		 }
		 throw ETradingException("#Error: Invalid Yield Curve State Variable String, must be 'ZeroRate', 'ForwardRate', 'RateTime', 'LogDF' or 'DiscountFactor'");
	 }

	 std::string toString(const StateVariableEnum enumValue)
	 {
		 switch (enumValue)
		 {
		 case STATE_VARIABLE_NONE:
			 return std::string("NONE");
			 break;
		 case STATE_VARIABLE_ZERO_RATE:
			 return std::string("ZERO_RATE");
			 break;
		 case STATE_VARIABLE_ZERO_RATE_TIMES_TIME:
			 return std::string("ZERO_RATE_TIMES_TIME");
			 break;
		 case STATE_VARIABLE_FORWARD_RATE:
			 return std::string("FORWARD_RATE");
			 break;
		 case STATE_VARIABLE_LOG_DF:
			 return std::string("LOG_DISCOUNT_FACTOR");
                break;
            case STATE_VARIABLE_DF:
                return std::string("DISCOUNT_FACTOR");
                break;
            default:
				throw ETradingException("#Error: Invalid Yield Curve State Variable String, must be 'ZeroRate', 'ForwardRate', 'RateTime', 'LogDF' or 'DiscountFactor'");
                break;
        }
    }

	 FuturesTypeEnum toFuturesTypeEnum(const std::string& enumString)
	 {
		 std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		 if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		 {
			 return FUTURES_TYPE_NONE;
		 }
		 if (uppercaseEnumString == "SERIAL_BY_RATE" || uppercaseEnumString == "RATE" || uppercaseEnumString == "SERIAL_RATE" )
		 {
			 return SERIAL_FUTURES_BY_RATE;
		 }
		 if (uppercaseEnumString == "SERIAL_BY_DF" || uppercaseEnumString == "DF" || uppercaseEnumString == "SERIAL_DF"  )
		 {
			 return SERIAL_FUTURES_BY_DF;
		 }
		 if (uppercaseEnumString == "CONTIGUOUS")
		 {
			 return CONTIGUOUS_FUTURES;
		 }
		 throw ETradingException("#Error: Invalid Yield Curve Futures Type, must be 'CONTIGUOUS', 'SERIAL_BY_RATE' or 'SERIAL_BY_DF'");
	 }

	 std::string toString(const FuturesTypeEnum enumValue)
	 {
		 switch (enumValue)
		 {
		 case FUTURES_TYPE_NONE:
			 return std::string("NONE");
			 break;
		 case SERIAL_FUTURES_BY_RATE:
			 return std::string("SERIAL_BY_RATE");
			 break;
		 case SERIAL_FUTURES_BY_DF:
			 return std::string("SERIAL_BY_DF");
			 break;
		 case CONTIGUOUS_FUTURES:
			 return std::string("CONTIGUOUS");
			 break;
		 default:
			 throw ETradingException("#Error: Invalid Yield Curve Futures Type, must be 'CONTIGUOUS', 'SERIAL_BY_RATE' or 'SERIAL_BY_DF'");
			 break;
		 }
	 }

	 std::string toString( const PayReceiveEnum enumValue )
    {
        switch( enumValue )
        {
            case NONE_PAYRECEIVE_ENUM:
                return "NONE";
                break;
            case PAY_PAYRECEIVE_ENUM:
                return "PAY";
                break;
            case RECEIVE_PAYRECEIVE_ENUM:
                return "RECEIVE";
                break;
            default:
				throw ETradingException( "#Error: Invalid PayReceive Enum, must be 'PAY' or 'RECEIVE'" );
                break;
        }
    }

	 PayReceiveEnum toPayReceiveEnum( const std::string& enumString )
    {
        std::string uppercaseEnumString = etrading::trim_to_upper( enumString.c_str() );

		if( uppercaseEnumString == "" || uppercaseEnumString == "NONE")
        {
            return NONE_PAYRECEIVE_ENUM;
        }
		if( uppercaseEnumString == "PAY" || uppercaseEnumString == "PAYER" || uppercaseEnumString == "P")
        {
            return PAY_PAYRECEIVE_ENUM;
        }
		if( uppercaseEnumString == "RECEIVE" || uppercaseEnumString == "RECEIVER" || uppercaseEnumString == "REC" || uppercaseEnumString == "R")
        {
            return RECEIVE_PAYRECEIVE_ENUM;
        }
		throw ETradingException( "#Error: Invalid PayReceive Enum, must be 'PAY' or 'RECEIVE'" );
    }

	 PayReceiveEnum flipPayReceive(const PayReceiveEnum& payRec )
	{
		if( payRec == NONE_PAYRECEIVE_ENUM)
        {
            return NONE_PAYRECEIVE_ENUM;
        }
		if( payRec == PAY_PAYRECEIVE_ENUM)
        {
            return RECEIVE_PAYRECEIVE_ENUM;
        }
		else if( payRec == RECEIVE_PAYRECEIVE_ENUM)
        {
            return PAY_PAYRECEIVE_ENUM;
        }
		throw ETradingException( "#Error: Invalid PayReceive Enum, must be 'PAY' or 'RECEIVE'" );
	}


     std::string toString( const PayerReceiverSwaptionEnum enumValue )
    {
        switch ( enumValue )
        {
        case NONE_SWAPTION:
            return "NONE";
            break;
        case PAYER_SWAPTION:
            return "PAYER";
            break;
        case RECEIVER_SWAPTION:
            return "RECEIVER";
            break;
        default:
            throw ETradingException( "#Error: Invalid PayerReceiverSwaption Enum, must be 'PAYER' or 'RECEIVER'" );
            break;
        }
    }

     PayerReceiverSwaptionEnum toPayerReceiverSwaptionEnum( const std::string& enumString )
    {
        std::string uppercaseEnumString = etrading::trim_to_upper( enumString.c_str() );

        if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
        {
            return NONE_SWAPTION;
        }
        if ( uppercaseEnumString == "PAYER" )
        {
            return PAYER_SWAPTION;
        }
        if ( uppercaseEnumString == "RECEIVER" )
        {
            return RECEIVER_SWAPTION;
        }
        throw ETradingException( "#Error: Invalid PayerReceiverSwaption Enum, must be 'PAYER' or 'RECEIVER'" );
    }

     std::string toString( const OptionStyleEnum enumValue )
    {
        switch ( enumValue )
        {
        case NONE_STYLE_OPTION:
            return "NONE";
            break;
		case EUROPEAN_OPTION:
			return "EUROPEAN";
			break;
		case BERMUDAN_OPTION:
			return "BERMUDAN";
			break;
		case AMERICAN_OPTION:
			return "AMERICAN";
			break;
       default:
            throw ETradingException( "#Error: Invalid OptionStyle Enum, must be 'NONE', 'EUROPEAN', 'BERMUDAN' or 'AMERICAN'" );
            break;
		}
	}

	 OptionStyleEnum toOptionStyleEnum( const std::string& enumString )
    {
        std::string uppercaseEnumString = etrading::trim_to_upper( enumString.c_str() );

        if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
        {
            return NONE_STYLE_OPTION;
        }
		if ( uppercaseEnumString == "EUROPEAN" )
        {
            return EUROPEAN_OPTION;
        }
		if ( uppercaseEnumString == "BERMUDAN" )
        {
            return BERMUDAN_OPTION;
        }
		if ( uppercaseEnumString == "AMERICAN" )
        {
            return AMERICAN_OPTION;
        }
		throw ETradingException( "#Error: Invalid OptionStyle Enum, must be 'NONE', 'EUROPEAN', 'BERMUDAN' or 'AMERICAN'" );
	}

     std::string toString( const VolatilityTypeEnum enumValue )
    {
        switch ( enumValue )
        {
        case NONE_VOLATILITY:
            return "NONE";
            break;
        case LOGNORMAL_VOLATILITY:
            return "LOGNORMAL";
            break;
        case SHIFTED_LOGNORMAL_VOLATILITY:
            return "SHIFTED_LOGNORMAL";
            break;
        case NORMAL_VOLATILITY:
            return "NORMAL";
            break;
        default:
            throw ETradingException( "#Error: Invalid Volatility Type, must be 'LOGNORMAL' 'SHIFTED_LOGNORMAL' or 'NORMAL'" );
            break;
        }
    }

     VolatilityTypeEnum toVolatilityTypeEnum( const std::string& enumString )
    {
        std::string uppercaseEnumString = etrading::trim_to_upper( enumString.c_str() );

        if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
        {
            return NONE_VOLATILITY;
        }
        if ( uppercaseEnumString == "LOGNORMAL" )
        {
            return LOGNORMAL_VOLATILITY;
        }
        if ( uppercaseEnumString == "SHIFTED_LOGNORMAL" )
        {
            return SHIFTED_LOGNORMAL_VOLATILITY;
        }
        if ( uppercaseEnumString == "NORMAL" )
        {
            return NORMAL_VOLATILITY;
        }
        throw ETradingException( "#Error: Invalid Volatility Type, must be 'LOGNORMAL' 'SHIFTED_LOGNORMAL' or 'NORMAL'" );
    }

	 std::string toString(const ConvexityMethodEnum enumValue)
	{
		switch (enumValue)
		{
		case NONE_CONVEXITY:
			return "NONE";
			break;
		case HULL_APPROX_CONVEXITY:
			return "HULL_APPROX";
			break;
		case STANDARD_CONVEXITY:
			return "STANDARD_CONVEXITY";
			break;
		default:
			throw ETradingException("#Error: Invalid Volatility Type, must be 'NONE' 'HULL_APPROX' or 'STANDARD_CONVEXITY'");
			break;
		}
	}

	 ConvexityMethodEnum toConvexityMethodEnum(const std::string& enumString)
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "NONE")
		{
			return NONE_CONVEXITY;
		}
		if (uppercaseEnumString == "HULL_APPROX" || uppercaseEnumString == "HULL APPROX" || uppercaseEnumString == "HULLAPPROX")
		{
			return HULL_APPROX_CONVEXITY;
		}
		if (uppercaseEnumString == "STANDARD")
		{
			return STANDARD_CONVEXITY;
		}
		throw ETradingException("#Error: Invalid Volatility Type, must be 'NONE' 'HULL_APPROX' or 'STANDARD_CONVEXITY'");
	}

	 std::string toString(const PricingModelEnum enumValue)
	{
		switch (enumValue)
		{
		case NONE_MODEL:
			return "NONE";
			break;
		case BLACK_MODEL:
			return "BLACK";
			break;
		case BLACK_SCHOLES_MODEL:
			return "BLACK_SCHOLES";
			break;
		default:
			throw ETradingException("#Error: Invalid Pricing Model, must be 'BLACK' or 'BLACK_SCHOLES'");
			break;
		}
	}

	 PricingModelEnum toPricingModelEnum(const std::string& enumString)
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		{
			return NONE_MODEL;
		}
		if (uppercaseEnumString == "BLACK")
		{
			return BLACK_MODEL;
		}
		if (uppercaseEnumString == "BLACKSCHOLES" || uppercaseEnumString == "BLACK SCHOLES" || uppercaseEnumString == "BLACK_SCHOLES")
		{
			return BLACK_SCHOLES_MODEL;
		}
		throw ETradingException("#Error: Invalid Pricing Model, must be 'BLACK' or 'BLACK_SCHOLES'");
	}

	 SettlementTypeEnum toSettlementTypeEnum( const std::string& enumString )
    {
        std::string uppercaseEnumString = etrading::trim_to_upper( enumString.c_str() );

        if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
        {
            return NONE_SETTLEMENT;
        }
		if ( uppercaseEnumString == "CASH" )
        {
            return CASH_PRICE_SETTLEMENT;
        }
		if ( uppercaseEnumString == "PARYIELD" )
        {
            return CASH_PAR_YIELD_SETTLEMENT;
        }
		if ( uppercaseEnumString == "PHYSICAL" )
        {
            return PHYSICAL_SETTLEMENT;
        }
		throw ETradingException( "#Error: Invalid Settlement Type, must be 'NONE', 'CASH', 'PARYIELD', or 'PHYSICAL'" );
	}
	
	 std::string toString( const SettlementTypeEnum enumValue )
    {
        switch ( enumValue )
        {
        case NONE_SETTLEMENT:
            return "NONE";
            break;
        case CASH_PRICE_SETTLEMENT:
            return "PRICE";
            break;
        case CASH_PAR_YIELD_SETTLEMENT:
            return "PARYIELD";
            break;
	    case PHYSICAL_SETTLEMENT:
            return "PHYSICAL";
            break;
        default:
            throw ETradingException( "#Error: Invalid Settlement Type, must be 'NONE', 'CASH', 'PARYIELD', or 'PHYSICAL'" );
            break;
        }
    }

	 std::string toString(const FraStyleEnum enumValue)
	{
		switch (enumValue)
		{
		case NONE_FRA_STYLE:
			return "NONE";
			break;
		case REGULAR_FRA_STYLE:
			return "REGULAR";
			break;
		case BROKEN_DATED_FRA_STYLE:
			return "BROKEN_DATED";
			break;
		default:
			throw ETradingException("#Error: Invalid Fra Style, must be 'REGULAR' or 'BROKEN_DATED'");
			break;
		}
	}

	 FraStyleEnum toFraStyleEnum(const std::string& enumString)
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		{
			return NONE_FRA_STYLE;
		}
		if (uppercaseEnumString == "REGULAR" || uppercaseEnumString == "R")
		{
			return REGULAR_FRA_STYLE;
		}
		if (uppercaseEnumString == "BROKENDATED" || uppercaseEnumString == "BROKEN_DATED" || uppercaseEnumString == "BROKEN DATED" )
		{
			return BROKEN_DATED_FRA_STYLE;
		}
		throw ETradingException("#Error: Invalid Fra Style, must be 'REGULAR' or 'BROKEN_DATED'");
	}

	 std::string toString(const CashflowTypeEnum enumValue)
	{
		switch (enumValue)
		{
		case NORMAL_CASHFLOW_TYPE:
			return "NORMAL";
			break;
		case FIRST_NOTIONAL_EXCHANGE_CASHFLOW_TYPE:
			return "FIRST_NOTIONAL_EXCHANGE";
			break;
		case NORMAL_LAST_CASHFLOW_TYPE:
			return "NORMAL_LAST";
			break;
		default:
			throw ETradingException("#Error: Invalid CashflowType Enum, must be 'NORMAL', 'FIRST_NOTIONAL_EXCHANGE' or 'NORMAL_LAST'");
			break;
		}
	}

	 CashflowTypeEnum toCashflowTypeEnum(const std::string& enumString)
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "NORMAL")
		{
			return NORMAL_CASHFLOW_TYPE;
		}
		if (uppercaseEnumString == "FIRST_NOTIONAL_EXCHANGE")
		{
			return FIRST_NOTIONAL_EXCHANGE_CASHFLOW_TYPE;
		}
		if (uppercaseEnumString == "NORMAL_LAST")
		{
			return NORMAL_LAST_CASHFLOW_TYPE;
		}
		throw ETradingException("#Error: Invalid CashflowType Enum, must be 'NORMAL', 'FIRST_NOTIONAL_EXCHANGE' or 'NORMAL_LAST'");
	}

	 std::string toString(const GreekTypeEnum enumValue)
	{
		switch (enumValue)
		{
		case NONE_GREEK_TYPE:
			return "NONE";
			break;
		case ANALYTICAL_GREEK_TYPE:
			return "ANALYTICAL";
			break;
		case NUMERICAL_GREEK_TYPE:
			return "NUMERICAL";
			break;
		case AAD_GREEK_TYPE:
			return "AAD";
			break;
		default:
			throw ETradingException("#Error: Invalid CashflowType Enum, must be 'ANALYTICAL', 'NUMERICAL' or 'AAD'");
			break;
		}
	}

	 GreekTypeEnum toGreekTypeEnum(const std::string& enumString)
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		{
			return NONE_GREEK_TYPE;
		}
		if (uppercaseEnumString == "ANALYTICAL")
		{
			return ANALYTICAL_GREEK_TYPE;
		}
		if (uppercaseEnumString == "NUMERICAL")
		{
			return NUMERICAL_GREEK_TYPE;
		}
		if (uppercaseEnumString == "AAD")
		{
			return AAD_GREEK_TYPE;
		}
		throw ETradingException("#Error: Invalid CashflowType Enum, must be 'ANALYTICAL', 'NUMERICAL' or 'AAD'");
	}

	 std::string toString(const VolatilityDataSourceEnum enumValue)
    {
        switch (enumValue)
		{
		case NONE_VOLDATA:
			return "NONE";
			break;
		case SWAPTION_VOLDATA:
			return "SWAPTION";
			break;
		case CAP_VOLDATA:
			return "CAP";
			break;
		case FLOOR_VOLDATA:
			return "FLOOR";
			break;
        case HISTORICAL_VOLDATA:
			return "HISTORICAL";
			break;
        case MANUALINPUT_VOLDATA:
			return "MANUALINPUT";
			break;
		default:
			throw ETradingException("#Error: Invalid Volatility Data Source, must be 'SWAPTION', 'CAP', 'FLOOR', 'HISTORICAL' or 'MANUALINPUT'");
			break;
		}
    }
	
     VolatilityDataSourceEnum toVolatilityDataSourceEnum(const std::string& enumString)
    {
        std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		{
			return NONE_VOLDATA;
		}
		if (uppercaseEnumString == "SWAPTION")
		{
			return SWAPTION_VOLDATA;
		}
		if (uppercaseEnumString == "CAP")
		{
			return CAP_VOLDATA;
		}
		if (uppercaseEnumString == "FLOOR")
		{
			return FLOOR_VOLDATA;
		}
        if (uppercaseEnumString == "HISTORICAL")
		{
			return HISTORICAL_VOLDATA;
		}
        if (uppercaseEnumString == "MANUALINPUT")
		{
			return MANUALINPUT_VOLDATA;
		}
		throw ETradingException("#Error: Invalid Volatility Data Source, must be 'SWAPTION', 'CAP', 'FLOOR', 'HISTORICAL' or 'MANUALINPUT'");
	}
    
	 std::string toString(const OptionTradeTypeEnum enumValue)
    {
        switch (enumValue)
		{
		case NONE_TRADE_TYPE:
			return "NONE";
			break;
		case CAP_FLOOR_TRADE:
			return "CAPFLOOR";
			break;
		case EUROPEAN_SWAPTION_TRADE:
			return "EUROPEAN_SWAPTION";
			break;
        case BOND_OPTION_TRADE:
            return "BONDOPTION";
            break;
		default:
			throw ETradingException("#Error: Invalid Trade Type, must be 'CAPFLOOR', 'EUROPEAN_SWAPTION' or 'BONDOPTION' ");
			break;
		}
    }

	 OptionTradeTypeEnum toOptionTradeTypeEnum(const std::string& enumString)
    {
        std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		{
			return NONE_TRADE_TYPE;
		}
		if ( uppercaseEnumString == "CAPFLOOR" )
		{
			return CAP_FLOOR_TRADE;
		}
		if (uppercaseEnumString == "EUROPEAN_SWAPTION" )
		{
			return EUROPEAN_SWAPTION_TRADE;
		}
        if (uppercaseEnumString == "BONDOPTION" )
		{
			return BOND_OPTION_TRADE;
		}
		throw ETradingException("#Error: Invalid Trade Type, must be 'CAPFLOOR', 'EUROPEAN_SWAPTION' or 'BONDOPTION' ");
    }

	 std::string toString(const LongShortPositionEnum enumValue)
    {
        switch (enumValue)
		{
		case NONE_POSITION:
			return "NONE";
			break;
		case LONG_POSITION:
			return "LONG";
			break;
		case SHORT_POSITION:
			return "SHORT";
			break;
		default:
			throw ETradingException("#Error: Invalid Long / Short parameter must be 'LONG' or 'SHORT'");
			break;
		}
    }

	 LongShortPositionEnum toLongShortPositionEnum(const std::string& enumString)
    {
        std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		{
			return NONE_POSITION;
		}
		if ( uppercaseEnumString == "LONG" )
		{
			return LONG_POSITION;
		}
		if (uppercaseEnumString == "SHORT" )
		{
			return SHORT_POSITION;
		}
		throw ETradingException("#Error: Invalid Long / Short parameter must be 'LONG' or 'SHORT'");
    }

	 std::string toString(const RandomNumberGeneratorEnum enumValue)
	{
	    switch (enumValue)
		{
		case NONE_GENERATOR:
			return "NONE";
			break;
		case MERSENNE_TWISTER_GENERATOR:
			return "MERSENNETWISTER";
			break;
		case SOBOL_GENERATOR:
			return "SOBOL";
			break;
		default:
			throw ETradingException("#Error: Invalid RandomNumberGenerator parameter must be 'MERSENNETWISTER' or 'SOBOL'");
			break;
		}
	}

	 RandomNumberGeneratorEnum toRandomNumberGeneratorEnum(const std::string& enumString)
	{
	    std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		{
			return NONE_GENERATOR;
		}
		if ( uppercaseEnumString == "MERSENNETWISTER" )
		{
			return MERSENNE_TWISTER_GENERATOR;
		}
		if ( uppercaseEnumString == "SOBOL" )
		{
			return SOBOL_GENERATOR;
		}
		throw ETradingException("#Error: Invalid RandomNumberGenerator parameter must be 'MERSENNETWISTER' or 'SOBOL'");
	}

	 std::string toString(const DistributionEnum enumValue)
	{
		switch (enumValue)
		{
		case NONE_DISTRIBUTION:
			return "NONE";
			break;
		case UNIFORM_DISTRIBUTION:
			return "UNIFORM";
			break;
		case NORMAL_DISTRIBUTION:
			return "NORMAL";
			break;
		default:
			throw ETradingException("#Error: Invalid Distribution parameter must be 'UNIFORM' or 'NORMAL'");
			break;
		}
	}

	 DistributionEnum toDistributionEnum(const std::string& enumString)
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		{
			return NONE_DISTRIBUTION;
		}
		if ( uppercaseEnumString == "UNIFORM" )
		{
			return UNIFORM_DISTRIBUTION;
		}
		if ( uppercaseEnumString == "NORMAL" )
		{
			return NORMAL_DISTRIBUTION;
		}
		throw ETradingException("#Error: Invalid Distribution parameter must be 'UNIFORM' or 'NORMAL'");

	}


	 std::string toString(const FixingTypeEnum enumValue)
	{
		switch (enumValue)
		{
		case IN_ADVANCE_FIXING:
			return "ADVANCE";
			break;
		case IN_ARREARS_FIXING:
			return "ARREARS";
			break;
		default:
			throw ETradingException("#Error: Invalid Input, the 'fixingAdvanceOrArrears' parameter must be set to 'advance', 'arrears', 'adv', 'arr' or left blank.");
			break;
		}
	}

	 FixingTypeEnum toFixingTypeEnum(const std::string& enumString)
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "" || uppercaseEnumString == "ADVANCE" || uppercaseEnumString == "ADV")
		{
			return IN_ADVANCE_FIXING;
		}
		if (uppercaseEnumString == "ARREARS" || uppercaseEnumString == "ARR" || uppercaseEnumString == "ARREAR")
		{
			return IN_ARREARS_FIXING;
		}
		throw ETradingException("#Error: Invalid Input, the 'fixingAdvanceOrArrears' parameter must be set to 'advance', 'arrears', 'adv', 'arr' or left blank.");
	}

	 std::string toString(const FixingTableEnum enumValue)
	 {
		 switch ( enumValue )
		 {
		 case INTEREST_RATE_FIXING_TABLE:
			 return "INTERESTRATE";
			 break;
			 
		 case INFLATION_FIXING_TABLE:
			 return "INFLATION";
			 break;
		 
		 case FX_FIXING_TABLE:
			 return "FX";
			 break;
			 
		 default:
			 throw ETradingException("#Error: Invalid Input, the 'FixingTableType' parameter must be set to INTERESTRATE, INFLATION, or FX" );
			 break;
		 }

	 }

	 FixingTableEnum toFixingTableEnum(const std::string& enumString)
	 {
		 const std::string uppercaseEnumString = etrading::trim_to_upper( enumString.c_str() );
		 if ( uppercaseEnumString == "INTERESTRATE" )
		 {
			 return INTEREST_RATE_FIXING_TABLE;
		 }
		 if (uppercaseEnumString == "INFLATION")
		 {
			 return INFLATION_FIXING_TABLE;
		 }
		 if (uppercaseEnumString == "FX")
		 {
			 return FX_FIXING_TABLE;
		 }

		 throw ETradingException("#Error: Invalid Input, the 'FixingTableType' parameter must be set to INTERESTRATE, INFLATION, or FX" );
	 }

     std::string toString(const OISLongTermInstrumentsEnum enumValue)
    {
		switch (enumValue)
		{
		case NONE_OIS_LONGTERM_INSTRUMENTS:
			return "NONE";
			break;
		case LIBOROIS_OIS_LONGTERM_INSTRUMENTS:
			return "LOBASIS";
			break;
		default:
			AQ_THROW( "Invalid Input, the 'OISLongTermInstrumentsEnum' parameter must be set to 'NONE' or 'LOBASIS'" )
			break;
		}
	}

	 OISLongTermInstrumentsEnum toOISLongTermInstrumentsEnum(const std::string& enumString)
    {
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		{
			return NONE_OIS_LONGTERM_INSTRUMENTS;
		}
		if (uppercaseEnumString == "LOBASIS" || uppercaseEnumString == "LIBOROIS" || uppercaseEnumString == "LIBOR-OIS" )
		{
			return LIBOROIS_OIS_LONGTERM_INSTRUMENTS;
		}
		AQ_THROW( "Invalid Input, the 'OISLongTermInstrumentsEnum' parameter must be set to 'NONE' or 'LOBASIS'" )
	}


	 std::string toString(const OISBasisInstrumentSwapTypeEnum enumValue)
	 {
		 switch (enumValue)
		 {
		 case NONE_OISBASIS_INSTRUMENT_SWAP_TYPE:
			 return "NONE";
			 break;
		 case OIS_OISBASIS_INSTRUMENT_SWAP_TYPE:
			 return "OIS";
			 break;
		 case LIBOR_OISBASIS_INSTRUMENT_SWAP_TYPE:
			 return "LIBOR";
			 break;
		 default:
			 AQ_THROW("Invalid Input, the 'OISBasisInstrumentSpreadTypeEnum' parameter must be set to 'NONE' or 'OIS' or 'LIBOR'")
				 break;
		 }
	 }

	 OISBasisInstrumentSwapTypeEnum toOISBasisInstrumentSwapTypeEnum(const std::string& enumString)
	 {
		 std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		 if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		 {
			 return NONE_OISBASIS_INSTRUMENT_SWAP_TYPE;
		 }
		 if (uppercaseEnumString == "OIS")
		 {
			 return OIS_OISBASIS_INSTRUMENT_SWAP_TYPE;
		 }
		 if (uppercaseEnumString == "LIBOR")
		 {
			 return LIBOR_OISBASIS_INSTRUMENT_SWAP_TYPE;
		 }

		 AQ_THROW("Invalid Input, the 'OISBasisInstrumentSpreadType' parameter must be set to 'NONE' or 'OIS' or 'LIBOR'. A value of '" + enumString + "' has been provided.")
	 }

	 
	 std::string toString(const OISLongTermCompoundingEnum enumValue)
	 {
		 switch (enumValue)
		 {
		 case NONE_OIS_LONGTERM_COMPOUNDING:
			 return "NONE";
			 break;
		 case DAILY_AVERAGES_OIS_LONGTERM_COMPOUNDING:
			 return "DAILYAVERAGING";
			 break;
		 default:
			 AQ_THROW("Invalid Input, the 'OISLongTermCompoundingEnum' parameter must be set to 'NONE' or 'DAILYAVERAGING'")
			 break;
		 }
	 }

	 OISLongTermCompoundingEnum toOISLongTermCompoundingEnum(const std::string& enumString)
	 {
		 std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		 if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		 {
			 return NONE_OIS_LONGTERM_COMPOUNDING;
		 }
		 if (uppercaseEnumString == "DAILYAVERAGING")
		 {
			 return DAILY_AVERAGES_OIS_LONGTERM_COMPOUNDING;
		 }
		 AQ_THROW("Invalid Input, the 'OISLongTermCompoundingEnum' parameter must be set to 'NONE' or 'DAILYAVERAGING'")
	 }

     std::string toString(const OISMidTermInstrumentsEnum enumValue)
    {
		switch (enumValue)
		{
		case NONE_OIS_SHORTTERM_INSTRUMENTS:
			return "NONE";
			break;
		case CENTRAL_BANK_SWAP:
			return "CentralBankSwap";
			break;
		case FED_FUND_FUTURE:
			return "FedFund";
			break;
        case ONE_MONTH_FUTURE:
            return "1MFuture";
		case THREE_MONTH_FUTURE:
			return "3MFuture";
			break;
		default:
			AQ_THROW( "Invalid Input, the 'OISMidTermInstrumentsEnum' parameter must be set to 'CentralBankSwap', 'FedFund', '1MFuture', '3MFuture'" )
			break;
		}
	}

	 OISMidTermInstrumentsEnum toOISMidTermInstrumentsEnum(const std::string& enumString)
	 {
		 std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		 if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		 {
			 return NONE_OIS_SHORTTERM_INSTRUMENTS;
		 }
		 if (uppercaseEnumString == "CENTRALBANKSWAP" )
		 {
			 return CENTRAL_BANK_SWAP;
		 }
		 if (uppercaseEnumString == "FEDFUND")
		 {
			 return FED_FUND_FUTURE;
		 }
		 if (uppercaseEnumString == "1MFUTURE")
		 {
			 return ONE_MONTH_FUTURE;
		 }
		 if (uppercaseEnumString == "3MFUTURE")
		 {
			 return THREE_MONTH_FUTURE;
		 }
		 AQ_THROW("Invalid Input, the 'OISMidTermInstrumentsEnum' parameter must be set to 'CentralBankSwap', 'FedFund', '1MFuture', '3MFuture'")
	 }

	 std::string toString(const OISCompoundingEnum enumValue)
	 {
		 switch (enumValue)
		 {
		 case NONE_OIS_COMPOUNDING:
			 return "NONE";
			 break;
		 case ARITHMETIC_OIS_COMPOUNDING:
			 return "ARITHMETIC";
			 break;
		 case GEOMETRIC_OIS_COMPOUNDING:
			 return "GEOMETRIC";
			 break;
		 default:
			 AQ_THROW("Invalid Input, the 'OISCompoundingEnum' parameter must be set to 'ARITHMETIC', 'GEOMETRIC'")
				 break;
		 }
	 }

	 OISCompoundingEnum toOISCompoundingEnum(const std::string& enumString)
    {
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if (uppercaseEnumString == "" || uppercaseEnumString == "NONE")
		{
			return NONE_OIS_COMPOUNDING;
		}
		if (uppercaseEnumString == "ARITHMETIC" || uppercaseEnumString == "ARITHMETICAVERAGE" || uppercaseEnumString == "ARITHMETIC_COMPOUNDING" || uppercaseEnumString == "ARITHMETICCOMPOUNDING" )
		{
			return ARITHMETIC_OIS_COMPOUNDING;
		}
        if (uppercaseEnumString == "GEOMETRIC" || uppercaseEnumString == "GEOMETRIC_COMPOUNDING" || uppercaseEnumString == "GEOMETRICCOMPOUNDING")
		{
			return GEOMETRIC_OIS_COMPOUNDING;
		}
		AQ_THROW( "Invalid Input, the 'OISCompoundingEnum' parameter must be set to 'ARITHMETIC', 'GEOMETRIC'" )
	}


	 std::string toString( const LoanTypeEnum enumValue )
	{
		switch ( enumValue )
		{
		case NONE_LOAN_TYPE:
			return "NONE";
			break;
		case AMORTIZING_LOAN_TYPE:
			return "AMORTIZING";
			break;
        case BULLET_LOAN_TYPE:
            return "BULLET";
            break;
		default:
			AQ_THROW( "Invalid Input, the 'LoanTypeEnum' parameter must be set to 'AMORTISING', 'BULLET'" );
			break;
		}
	
	}

	 LoanTypeEnum toLoanTypeEnum( const std::string& enumString )
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
		{
			return NONE_LOAN_TYPE;
		}
		if (uppercaseEnumString == "AMORTIZING" )
		{
			return AMORTIZING_LOAN_TYPE;
		}
        if ( uppercaseEnumString == "BULLET" )
		{
			return BULLET_LOAN_TYPE;
		}
		AQ_THROW( "Invalid Input, the 'LoanTypeEnum' parameter must be set to 'AMORTIZING', 'BULLET'" );
	}

	std::string toString(const SupervisoryTypeEnum enumValue)
	{
		switch ( enumValue )
		{
		case NONE_SUPERVISORY_TYPE:
			return "NONE";
			break;
		case LEGACY_SUPERVISORY_TYPE:
			return "LEGACY";
			break;
        case IRBA_SUPERVISORY_TYPE:
            return "IRBA";
            break;
		default:
			AQ_THROW( "Invalid Input, the 'SupervisoryTypeEnum' parameter must be set to 'LEGACY', 'IRBA'" );
			break;
		}
	}

	SupervisoryTypeEnum toSupervisoryTypeEnum(const std::string& enumString)
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
		{
			return NONE_SUPERVISORY_TYPE;
		}
		if (uppercaseEnumString == "LEGACY" )
		{
			return LEGACY_SUPERVISORY_TYPE;
		}
        if ( uppercaseEnumString == "IRBA" )
		{
			return IRBA_SUPERVISORY_TYPE;
		}
		AQ_THROW( "Invalid Input, the 'SupervisoryTypeEnum' parameter must be set to 'LEGACY', 'IRBA'" );
	}

	std::string toString(const PoolTypeEnum enumValue)
	{
		switch ( enumValue )
		{
		case NONE_POOL_TYPE:
			return "NONE";
			break;
		case WHOLESALE_POOL_TYPE:
			return "WHOLESALE";
			break;
        case RETAIL_POOL_TYPE:
            return "RETAIL";
            break;
		default:
			AQ_THROW( "Invalid Input, the 'PoolTypeEnum' parameter must be set to 'WHOLESALE', 'RETAIL'" );
			break;
		}
	}

	PoolTypeEnum toPoolTypeEnum( const std::string& enumString)
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
		{
			return NONE_POOL_TYPE;
		}
		if (uppercaseEnumString == "WHOLESALE" )
		{
			return WHOLESALE_POOL_TYPE;
		}
        if ( uppercaseEnumString == "RETAIL" )
		{
			return RETAIL_POOL_TYPE;
		}
		AQ_THROW( "Invalid Input, the 'PoolTypeEnum' parameter must be set to 'WHOLESALE', 'RETAIL'" );
	}

	std::string toString(const RankTypeEnum enumValue)
	{
		switch ( enumValue )
		{
		case NONE_RANK_TYPE:
			return "NONE";
			break;
		case SENIOR_RANK_TYPE:
			return "SENIOR";
			break;
        case NON_SENIOR_RANK_TYPE:
            return "NONSENIOR";
            break;
		default:
			AQ_THROW( "Invalid Input, the 'RankTypeEnum' parameter must be set to 'SENIOR', 'NONSENIOR'" );
			break;
		}
	}

	RankTypeEnum toRankTypeEnum(const std::string& enumString)
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
		{
			return NONE_RANK_TYPE;
		}
		if (uppercaseEnumString == "SENIOR" )
		{
			return SENIOR_RANK_TYPE;
		}
        if ( uppercaseEnumString == "NONSENIOR" )
		{
			return NON_SENIOR_RANK_TYPE;
		}
		AQ_THROW( "Invalid Input, the 'RankTypeEnum' parameter must be set to 'SENIOR', 'NONSENIOR'" );
	}

	std::string toString(const TrancheTypeEnum enumValue)
	{
		switch ( enumValue )
		{
		case NONE_TRANCHE_TYPE:
			return "NONE";
			break;
		case PRO_RATA_TRANCHE_TYPE:
			return "PRORATA";
			break;
        case SEQUENTIAL_TRANCHE_TYPE:
            return "SEQUENTIAL";
            break;
		default:
			AQ_THROW( "Invalid Input, the 'TramcheTypeEnum' parameter must be set to 'PRORATA', 'SEQUENTIAL'" );	
			break;
		}
	
	}

	TrancheTypeEnum toTranchTypeEnum(const std::string& enumString)
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
		{
			return NONE_TRANCHE_TYPE;
		}
		if (uppercaseEnumString == "PRORATA" )
		{
			return PRO_RATA_TRANCHE_TYPE;
		}
        if ( uppercaseEnumString == "SEQUENTIAL" )
		{
			return SEQUENTIAL_TRANCHE_TYPE;
		}
		AQ_THROW( "Invalid Input, the 'TramcheTypeEnum' parameter must be set to 'NONE', 'PRORATA', 'SEQUENTIAL'" );	
	}

	std::string toString( const TrancheCouponTypeEnum enumValue )
	{
		switch ( enumValue )
		{
		case NONE_COUPON_TYPE:
			return "NONE";
			break;
		case FIXED_COUPON_TYPE:
			return "FIXED";
			break;
		case FLOAT_COUPON_TYPE:
			return "FLOAT";
			break;
		default:
			AQ_THROW( "Invalid Input, the 'TrancheCouponTypeEnum' parameter must be set to 'NONE', 'FIXED', 'FLOAT'" );
			break;
		}
	}

	TrancheCouponTypeEnum toTrancheCouponTypeEnum( const std::string& enumString )
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
		{
			return NONE_COUPON_TYPE;
		}
		if ( uppercaseEnumString == "FIXED" )
		{
			return FIXED_COUPON_TYPE;
		}
		if ( uppercaseEnumString == "FLOAT" )
		{
			return FLOAT_COUPON_TYPE;
		}
		AQ_THROW("Invalid Input, the 'TrancheCouponTypeEnum' parameter must be set to 'NONE', 'FIXED', 'FLOAT'");
	}

	std::string toString(const SecIrDataProviderranularityTypeEnum enumValue)
	{
		switch ( enumValue )
		{
		case NONE_SPECIFIED_GRANULAR_TYPE:
			return "NONE";
			break;
		case GRANULAR_TYPE:
			return "GRANULAR";
			break;
		case NON_GRANULAR_TYPE:
			return "NONGRANULAR";
			break;
		default:
			AQ_THROW( "Invalid Input, the 'SecIrDataProviderranularityTypeEnum' parameter must be set to 'NONGRANULAR', 'GRANULAR'" );
			break;
		}	
	}

	SecIrDataProviderranularityTypeEnum toSecIrDataProviderranularityTypeEnum(const std::string& enumString)
	{
		std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());

		if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
		{
			return NONE_SPECIFIED_GRANULAR_TYPE;
		}
		if (uppercaseEnumString == "GRANULAR" )
		{
			return GRANULAR_TYPE;
		}
        if ( uppercaseEnumString == "NONGRANULAR" )
		{
			return NON_GRANULAR_TYPE;
		}
		AQ_THROW( "Invalid Input, the 'SecIrDataProviderranularityTypeEnum' parameter must be set to 'NONGRANULAR', 'GRANULAR'" );
	}

    std::string toString( const ShiftTypeEnum enumValue )
    {
        switch ( enumValue )
		{
		case NONE_SHIFT_TYPE:
			return "NONE";
			break;
		case FLAT_SHIFT_TYPE:
			return "FLAT_SHIFT";
			break;
		case PERTURBED_SHIFT_TYPE:
			return "PERTURBED";
			break;
		default:
			AQ_THROW( "Invalid Input, the 'ShiftTypeEnum' parameter must be set to 'NONE', 'FLAT_SHIFT' or 'PERTURBED'" );
			break;
		}
    }
    
    ShiftTypeEnum toShiftTypeEnum( const std::string & enumString )
    {
        std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());
        if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
		{
			return NONE_SHIFT_TYPE;
		}
		if (uppercaseEnumString == "FLAT_SHIFT" )
		{
			return FLAT_SHIFT_TYPE;
		}
        if ( uppercaseEnumString == "PERTURBED" )
		{
			return PERTURBED_SHIFT_TYPE;
		}
		AQ_THROW( "Invalid Input, the 'ShiftTypeEnum' parameter must be set to 'NONE', 'FLAT_SHIFT' or 'PERTURBED'" );
    }

    std::string toString( const RiskTypeEnum enumValue )
    {
        switch ( enumValue )
		{
		case NONE_RISK_TYPE:
			return "NONE";
			break;
		case DISCOUNT_FACTOR_RISK_TYPE:
			return "DISCOUNT_FACTORS";
			break;
		case FORWARD_RATE_RISK_TYPE:
			return "FORWARD_RATES";
			break;
        case COMPOUND_RATE_RISK_TYPE:
			return "COMPOUND_RATES";
			break;
		default:
			AQ_THROW( "Invalid Input, the 'RiskTypeEnum' parameter must be set to 'NONE', 'DISCOUNT_FACTORS', 'FORWARD_RATES' or 'COMPOUND_RATES'" );
			break;
		}
    }
    
    RiskTypeEnum toRiskTypeEnum( const std::string & enumString )
    {
         std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());
        if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
		{
			return NONE_RISK_TYPE;
		}
		if ( uppercaseEnumString == "DISCOUNT_FACTORS" )
		{
			return DISCOUNT_FACTOR_RISK_TYPE;
		}
        if ( uppercaseEnumString == "FORWARD_RATES" )
		{
			return FORWARD_RATE_RISK_TYPE;
		}
        if ( uppercaseEnumString == "COMPOUND_RATES" )
		{
			return COMPOUND_RATE_RISK_TYPE;
		}
		AQ_THROW( "Invalid Input, the 'RiskTypeEnum' parameter must be set to 'NONE', 'DISCOUNT_FACTORS', 'FORWARD_RATES' or 'COMPOUND_RATES'" );
    }


    std::string toString( const BuildFrequencyEnum enumValue )
    {
        switch ( enumValue )
		{
		case NEVER_BUILD_FREQUENCY:
			return "NEVER";
			break;
		case ALWAYS_BUILD_FREQUENCY:
			return "ALWAYS";
			break;
		case ONCE_BUILD_FREQUENCY:
			return "ONCE";
			break;
        case TOLERANCE_BUILD_FREQUENCY:
			return "TOLERANCE";
			break;
		default:
			AQ_THROW( "Invalid Input, the 'BuildFrequencyEnum' parameter must be set to 'NEVER', 'ALWAYS', 'ONCE' or 'TOLERANCE'" );
			break;
		}
    }
    
    BuildFrequencyEnum toBuildFrequencyEnum( const std::string & enumString )
    {
         std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());
        if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" || uppercaseEnumString == "NEVER" )
		{
			return NEVER_BUILD_FREQUENCY;
		}
		if ( uppercaseEnumString == "ALWAYS" )
		{
			return ALWAYS_BUILD_FREQUENCY;
		}
        if ( uppercaseEnumString == "ONCE" )
		{
			return ONCE_BUILD_FREQUENCY;
		}
        if ( uppercaseEnumString == "TOLERANCE" )
		{
			return TOLERANCE_BUILD_FREQUENCY;
		}
		AQ_THROW( "Invalid Input, the 'BuildFrequencyEnum' parameter must be set to 'NEVER', 'ALWAYS', 'ONCE' or 'TOLERANCE'" );
    }

	std::string toString( const TurnOfYearAdjustmentTypeEnum enumValue )
    {
        switch ( enumValue )
		{
		case NO_TURN_ADJUSTMENT:
			return "NONE";
			break;
		case RELATIVE_TURN_ADJUSTMENT:
			return "RELATIVE";
			break;
		case ABSOLUTE_TURN_ADJUSTMENT:
			return "ABSOLUTE";
			break;
		default:
			AQ_THROW( "Invalid Input, the 'TurnOfYearAdjustmentTypeEnum' parameter must be set to 'NONE', 'RELATIVE' or 'ABSOLUTE'" );
			break;
		}
    }
    
    TurnOfYearAdjustmentTypeEnum toTurnOfYearAdjustmentTypeEnum( const std::string & enumString )
    {
         std::string uppercaseEnumString = etrading::trim_to_upper(enumString.c_str());
        if ( uppercaseEnumString == "" || uppercaseEnumString == "NONE" )
		{
			return NO_TURN_ADJUSTMENT;
		}
		if ( uppercaseEnumString == "RELATIVE" )
		{
			return RELATIVE_TURN_ADJUSTMENT;
		}
        if ( uppercaseEnumString == "ABSOLUTE" )
		{
			return ABSOLUTE_TURN_ADJUSTMENT;
		}
		AQ_THROW( "Invalid Input, the 'TurnOfYearAdjustmentTypeEnum' parameter must be set to 'NONE', 'RELATIVE' or 'ABSOLUTE'" );
    }

	std::string toString(const etrading::SabrModelEnum enumValue)
	{
		switch (enumValue)
		{
		case SABR_MODEL_PROPERTIES:
			return "SABR_MODEL_PROPERTIES";
		case SABR_MODEL_PARAMETERS:
			return "SABR_MODEL_PARAMETERS";
		case SABR_MODEL_MKTDATA:
			return "SABR_MODEL_MKTDATA";
		case SABR_MODEL_VOL_MKTDATA:
			return "SABR_MODEL_VOL_MKTDATA";
		default:
			throw ETradingException((boost::format("#Error: SabrModel key '%i' not supported") % enumValue).str());
			break;
		}
	}

	SabrModelEnum toSabrModelEnum(const std::string& incomingString)
	{
		std::string enumString = etrading::trim_to_upper(incomingString.c_str());
		if (enumString == "SABR_MODEL_PROPERTIES")
		{
			return SABR_MODEL_PROPERTIES;
		}
		if (enumString == "SABR_MODEL_PARAMETERS")
		{
			return SABR_MODEL_PARAMETERS;
		}
		if (enumString == "SABR_MODEL_MKTDATA")
		{
			return SABR_MODEL_MKTDATA;
		}
		if (enumString == "SABR_MODEL_VOL_MKTDATA")
		{
			return SABR_MODEL_VOL_MKTDATA;
		}
		if (enumString.empty())
		{
			throw ETradingException("#Error: No Sabr model key provided. Available Sabr model keys include: SABR_MODEL_PROPERTIES,SABR_MODEL_PARAMETERS,SABR_MODEL_MKTDATA,SABR_MODEL_VOL_MKTDATA.");
		}
		throw ETradingException((boost::format("#Error: Sabr model key '%s' not supported. Available Sabr model keys include: SABR_MODEL_PROPERTIES,SABR_MODEL_PARAMETERS,SABR_MODEL_MKTDATA,SABR_MODEL_VOL_MKTDATA.") % enumString).str());
	}

	std::string toString(const etrading::SabrMarketDataEnum enumValue)
	{
		switch (enumValue)
		{
		case SABR_MKTDATA:
			return "SABR_MKTDATA";
		case SABR_MKTDATA_PROPERTIES:
			return "SABR_MKTDATA_PROPERTIES";
		default:
			throw ETradingException((boost::format("#Error: SabrMarketDataEnum key '%i' not supported") % enumValue).str());
			break;
		}
	}

	SabrMarketDataEnum toSabrMarketDataEnum(const std::string& incomingString)
	{
		std::string enumString = etrading::trim_to_upper(incomingString.c_str());
		if (enumString  == "SABR_MKTDATA_PROPERTIES")
		{
			return SABR_MKTDATA_PROPERTIES;
		}
		if (enumString == "SABR_MKTDATA")
		{
			return SABR_MKTDATA;
		}
		if (enumString.empty())
		{
			throw ETradingException("#Error: No SabrMarketDataEnum key provided. Available keys include: SABR_MKTDATA_PROPERTIES,SABR_MKTDATA.");
		}

		throw ETradingException((boost::format("#Error: SabrMarketDataEnum key '%s' not supported. Available keys include: SABR_MKTDATA_PROPERTIES,SABR_MKTDATA.") % enumString).str());
	}

	std::string toString(const SabrMarketDataTypeEnum enumValue)
	{
		switch (enumValue)
		{
		case SABR_MKTDATA_VOL:
			return "VOLATILITY";
		case SABR_MKTDATA_SWAPRATE:
			return "SWAPRATE";
		case SABR_MKTDATA_ANNUITY:
			return "ANNUITY";
		case SABR_MKTDATA_PARAM:
			return "PARAMETER";
		default:
			throw ETradingException((boost::format("#Error: SabrMarketDataTypeEnum key '%i' not supported") % enumValue).str());
			break;
		}
	}

	SabrMarketDataTypeEnum toSabrMarketDataTypeEnum(const std::string& incomingString)
	{
		std::string enumString = etrading::trim_to_upper(incomingString.c_str());
		if (enumString == "VOLATILITY" || enumString == "SABR_MKTDATA_VOL")
		{
			return SABR_MKTDATA_VOL;
		}
		if (enumString == "SWAPRATE" || enumString == "SABR_MKTDATA_SWAPRATE")
		{
			return SABR_MKTDATA_SWAPRATE;
		}
		if (enumString == "ANNUITY" || enumString == "SABR_MKTDATA_ANNUITY")
		{
			return SABR_MKTDATA_ANNUITY;
		}
		if (enumString == "PARAMETER" || enumString == "SABR_MKTDATA_PARAM")
		{
			return SABR_MKTDATA_PARAM;
		}
		if ( enumString.empty())
		{
			throw ETradingException("#Error: No SabrMarketDataTypeEnum key provided. Available keys include: VOLATILITY,SWAPRATE,ANNUITY,PARAMETER.");
		}

		throw ETradingException((boost::format("#Error: SabrMarketDataTypeEnum key '%s' not supported. Available keys include: VOLATILITY,SWAPRATE,ANNUITY,PARAMETER.") % enumString).str());
	}

	std::string toString(const etrading::SabrMarketDataValueTypeEnum enumValue)
	{
		switch (enumValue)
		{
		case ABSOLUTE_SABR_VALUETYPE:
			return "ABSOLUTE";
		case RELATIVE_SABR_VALUETYPE:
			return "RELATIVE";
		default:
			throw ETradingException((boost::format("#Error: SabrMarketDataValueTypeEnum key '%i' not supported") % enumValue).str());
			break;
		}
	}

	SabrMarketDataValueTypeEnum toSabrMarketDataValueTypeEnum(const std::string& incomingString)
	{
		std::string enumString = etrading::trim_to_upper(incomingString.c_str());
		if (enumString == "ABSOLUTE")
		{
			return ABSOLUTE_SABR_VALUETYPE;
		}
		if (enumString == "RELATIVE")
		{
			return RELATIVE_SABR_VALUETYPE;
		}
		throw ETradingException((boost::format("#Error: SabrMarketDataValueTypeEnum key '%s' not supported. Available Sabr model keys include: ABSOLUTE,RELATIVE.") % enumString).str());
	}

	std::string toString(const etrading::SabrParamEnum enumValue)
	{
		switch (enumValue)
		{
		case ALPHA_SABR:
			return "ALPHA";
		case BETA_SABR:
			return "BETA";
		case RHO_SABR:
			return "RHO";
		case NU_SABR:
			return "NU";
		default:
			throw ETradingException((boost::format("#Error: SabrParamEnum key '%i' not supported") % enumValue).str());
			break;
		}
	}

	SabrParamEnum toSabrParamEnum(const std::string& incomingString)
	{
		std::string enumString = etrading::trim_to_upper(incomingString.c_str());
		if (enumString == "ALPHA")
		{
			return ALPHA_SABR;
		}
		if (enumString == "BETA")
		{
			return BETA_SABR;
		}
		if (enumString == "RHO")
		{
			return RHO_SABR;
		}
		if (enumString == "NU")
		{
			return NU_SABR;
		}
		throw ETradingException((boost::format("#Error: SabrParamEnum key '%s' not supported. Available keys include: ALPHA,BETA,RHO,NU.") % enumString).str());
	}

	std::string toString(const etrading::SABRCalibrationTargetEnum enumValue)
	{
		switch (enumValue)
		{
		case SABR_CALIB_VOLATILITY:
			return "VOLATILITY";
		case SABR_CALIB_PREMIUM:
			return "PREMIUM";
		default:
			throw ETradingException((boost::format("#Error: SABRCalibrationTarget key '%i' not supported") % enumValue).str());
			break;
		}
	}

	SABRCalibrationTargetEnum toSABRCalibrationTargetEnum(const std::string& incomingString)
	{
		std::string enumString = etrading::trim_to_upper(incomingString.c_str());
		if (enumString == "VOLATILITY")
		{
			return SABR_CALIB_VOLATILITY;
		}
		if (enumString == "PREMIUM")
		{
			return SABR_CALIB_PREMIUM;
		}
		throw ETradingException((boost::format("#Error: Sabr model key '%s' not supported. Available keys include: VOLATILITY,PREMIUM.") % enumString).str());
	}

	std::string toString(const etrading::SABRApproxMethodEnum enumValue)
	{
		switch (enumValue)
		{
		case HAGAN_APPROX_METHOD:
			return "HAGAN";
		case ANTONOV_APPROX_METHOD:
			return "ANTONOV";
		default:
			throw ETradingException((boost::format("#Error: SABRApproxMethodEnum key '%i' not supported") % enumValue).str());
			break;
		}
	}

	SABRApproxMethodEnum toSABRApproxMethodEnum(const std::string& incomingString)
	{
		std::string enumString = etrading::trim_to_upper(incomingString.c_str());
		if (enumString == "HAGAN")
		{
			return HAGAN_APPROX_METHOD;
		}
		if (enumString == "ANTONOV")
		{
			return ANTONOV_APPROX_METHOD;
		}
		throw ETradingException((boost::format("#Error: SABRApproxMethodEnum key '%s' not supported. Available keys include: HAGAN,ANTONOV.") % enumString).str());
	}


}
