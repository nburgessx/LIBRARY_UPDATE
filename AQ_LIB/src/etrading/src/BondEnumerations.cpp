// @File            BondEnumerations.cpp
//
// @Description:    This file as has all the essential enums for Bond Calculation Types
// @Created:        17th January 2017
// @Author:         Nicholas Burgess
// @Department:     Quant Research & Analytics
//
//                  The copyright to the computer program(s) herein is the property of Mizuho International.

#include "BondEnumerations.h"
#include "ETradingException.h"
#include "ContainerUtilities.h"
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>

namespace etrading
{

	std::string toString( const BondCalculationTypeEnum enumValue )
	{
        switch( enumValue )
        {
            case NO_CALCULATION_TYPE:
                return "NO_CALCULATION_TYPE";
                break;
            case TYPE1_STREET_CONVENTION:
                return "TYPE1_STREET_CONVENTION";
                break;
            case TYPE2_US_TREASURIES:
                return "TYPE2_US_TREASURIES";
                break;
            case TYPE5_ZERO_COUPON_BONDS:
                return "TYPE5_ZERO_COUPON_BONDS";
                break;
            case TYPE6_DISCOUNTED_COMMERCIAL_PAPER:
                return "TYPE6_DISCOUNTED_COMMERCIAL_PAPER";
                break;
            case TYPE13_STREET_CONVENTION_FIXED:
                return "TYPE13_STREET_CONVENTION_FIXED";
                break;
			case TYPE21_FLOATING_RATE_NOTE:
                return "TYPE21_FLOATING_RATE_NOTE";
                break;
            case TYPE51_JAPANESE_GOVERNMENT_BONDS:
                return "TYPE51_JAPANESE_GOVERNMENT_BONDS";
                break;
            case TYPE235_JAPANESE_GOVERNMENT_BONDS:
                return "TYPE235_JAPANESE_GOVERNMENT_BONDS";
                break;
            case TYPE60_GERMAN_FIXED_RATE_BONDS:
                return "TYPE60_GERMAN_FIXED_RATE_BONDS";
                break;
            case TYPE89_FRENCH_COMPOUND_METHOD:
                return "TYPE89_FRENCH_COMPOUND_METHOD";
                break;
			case TYPE102_STREET_CONVENTION:
				return "TYPE102_STREET_CONVENTION";
				break;
            case TYPE129_ISMA_CONVENTION:
                return "TYPE129_ISMA_CONVENTION";
                break;
            case TYPE523_ITALY_TRSY_BONDS:
                return "TYPE523_ITALY_TRSY_BONDS";
                break;
            case TYPE527_ITALY_TRSY_BILL:
                return "TYPE527_ITALY_TRSY_BILL";
                break;
            case TYPE529_ITALY_2Y_TRSY_BILL:
                return "TYPE529_ITALY_2Y_TRSY_BILL";
                break;
			case TYPE23_AUSTRALIAN_GOVERNMENT_BONDS:
				return "TYPE23_AUSTRALIAN_GOVERNMENT_BONDS";
				break;
			case TYPE26_UK_GILT:
				return "TYPE26_UK_GILT";
				break;
			case TYPE1029_SPAIN_GOVERNMENT_BONDS:
				return "TYPE1029_SPAIN_GOVT";
				break; 
			case TYPE730_SPAIN_T_BILL:
				return "TYPE730_SPAIN_T_BILL";
				break; 
			default:
				throw ETradingException( "#Error: Invalid Bond Calculation Type. Must be 'TYPE1_STREET_CONVENTION', 'TYPE2_US_TREASURIES', 'TYPE5_ZERO_COUPON_BONDS', 'TYPE6_DISCOUNTED_COMMERCIAL_PAPER', 'TYPE13_STREET_CONVENTION_FIXED', 'TYPE51_JAPANESE_GOVERNMENT_BONDS', 'TYPE235_JAPANESE_GOVERNMENT_BONDS', 'TYPE60_GERMAN_FIXED_RATE_BONDS', 'TYPE89_FRENCH_COMPOUND_METHOD', 'TYPE102_STREET_CONVENTION', 'TYPE129_ISMA_CONVENTION', 'TYPE21_FLOATING_RATE_NOTE', 'TYPE523_ITALY_TRSY_BONDS', 'TYPE527_ITALY_TRSY_BILL', 'TYPE529_ITALY_2Y_TRSY_BILL','TYPE23_AUSTRALIAN_GOVERNMENT_BONDS','TYPE26_UK_GILT', 'TYPE1029_SPAIN_GOVERNMENT_BONDS','TYPE730_SPAIN_T_BILL'."  );
                break;
        };
	};

	BondCalculationTypeEnum toBondCalculationTypeEnum( const std::string& enumString )
	{
        const std::string uCaseString = etrading::trim_to_upper( enumString.c_str() );

        if( uCaseString == "NO_CALCULATION_TYPE" || uCaseString == "NONE"  )
        {
            return NO_CALCULATION_TYPE;
        }
        if( uCaseString == "TYPE1" || uCaseString == "STREET_CONVENTION" || uCaseString == "TYPE1_STREET_CONVENTION"  )
        {
            return TYPE1_STREET_CONVENTION;
        }
        if( uCaseString == "TYPE2" || uCaseString == "US_TREASURIES" || uCaseString == "TYPE2_US_TREASURIES"  )
        {
            return TYPE2_US_TREASURIES;
        }
        if( uCaseString == "TYPE5" || uCaseString == "ZERO_COUPON_BONDS" || uCaseString == "TYPE5_ZERO_COUPON_BONDS"  )
        {
            return TYPE5_ZERO_COUPON_BONDS;
        }
        if( uCaseString == "TYPE6" || uCaseString == "DISCOUNTED_COMMERCIAL_PAPER" || uCaseString == "TYPE6_DISCOUNTED_COMMERCIAL_PAPER"  )
        {
            return TYPE6_DISCOUNTED_COMMERCIAL_PAPER;
        }
        if( uCaseString == "TYPE13" || uCaseString == "STREET_CONVENTION_FIXED" || uCaseString == "TYPE13_STREET_CONVENTION_FIXED"  )
        {
            return TYPE13_STREET_CONVENTION_FIXED;
        }
		if( uCaseString == "TYPE21" || uCaseString == "TYPE21_FLOATER" || uCaseString == "TYPE21_FLOATING_RATE_NOTE"  )
        {
            return TYPE21_FLOATING_RATE_NOTE;
        }
        if( uCaseString == "TYPE51" || uCaseString == "JAPANESE_GOVERNMENT_BONDS" || uCaseString == "TYPE51_JAPANESE_GOVERNMENT_BONDS"  )
        {
            return TYPE51_JAPANESE_GOVERNMENT_BONDS;
        }
        if( uCaseString == "TYPE235" || uCaseString == "JAPANESE_GOVERNMENT_BONDS_LONG_LAST_COUPONS" || uCaseString == "TYPE235_JAPANESE_GOVERNMENT_BONDS"  )
        {
            return TYPE235_JAPANESE_GOVERNMENT_BONDS;
        }
        if( uCaseString == "TYPE60" || uCaseString == "GERMAN_FIXED_RATE_BONDS" || uCaseString == "TYPE60_GERMAN_FIXED_RATE_BONDS"  )
        {
            return TYPE60_GERMAN_FIXED_RATE_BONDS;
        }
        if( uCaseString == "TYPE89" || uCaseString == "FRENCH_COMPOUND_METHOD" || uCaseString == "TYPE89_FRENCH_COMPOUND_METHOD"  )
        {
            return TYPE89_FRENCH_COMPOUND_METHOD;
        }
		if (uCaseString == "TYPE102" || uCaseString == "TYPE102_STREET_CONVENTION")
		{
			return TYPE102_STREET_CONVENTION;
		}
        if( uCaseString == "TYPE129" || uCaseString == "ISMA_CONVENTION" || uCaseString == "TYPE129_ISMA_CONVENTION"  )
        {
            return TYPE129_ISMA_CONVENTION;
        }
        if( uCaseString == "TYPE523" || uCaseString == "ITALY_TRSY_BONDS" || uCaseString == "TYPE523_ITALY_TRSY_BONDS"  )
        {
            return TYPE523_ITALY_TRSY_BONDS;
        }
        if( uCaseString == "TYPE527" || uCaseString == "ITALY_TRSY_BILL" || uCaseString == "TYPE527_ITALY_TRSY_BILL"  )
        {
            return TYPE527_ITALY_TRSY_BILL;
        }
        if( uCaseString == "TYPE529" || uCaseString == "ITALY_2Y_TRSY_BILL" || uCaseString == "TYPE529_ITALY_2Y_TRSY_BILL"  )
        {
            return TYPE529_ITALY_2Y_TRSY_BILL;
        }
		if (uCaseString == "TYPE23" || uCaseString == "AUSTRALIAN_GOVERNMENT_BONDS" || uCaseString == "TYPE23_AUSTRALIAN_GOVERNMENT_BONDS")
		{
			return TYPE23_AUSTRALIAN_GOVERNMENT_BONDS;
		}
		if (uCaseString == "TYPE26" || uCaseString == "UK_GILT" || uCaseString == "TYPE26_UK_GILT")
		{
			return TYPE26_UK_GILT;
		}
		if (uCaseString == "TYPE1029" || uCaseString == "TYPE1029_SPAIN_GOVT" || uCaseString == "TYPE1029_SPAIN_GOVERNMENT_BONDS")
		{
			return TYPE1029_SPAIN_GOVERNMENT_BONDS;
		}
		if (uCaseString == "TYPE730" || uCaseString == "TYPE730_SPAIN_T_BILL" || uCaseString == "TYPE730 SPAIN T BILL")
		{
			return TYPE730_SPAIN_T_BILL;
		}
		
		throw ETradingException( "#Error: Invalid Bond Calculation Type. Must be 'TYPE1_STREET_CONVENTION', 'TYPE2_US_TREASURIES', 'TYPE5_ZERO_COUPON_BONDS', 'TYPE6_DISCOUNTED_COMMERCIAL_PAPER', 'TYPE13_STREET_CONVENTION_FIXED', 'TYPE51_JAPANESE_GOVERNMENT_BONDS', 'TYPE235_JAPANESE_GOVERNMENT_BONDS', 'TYPE60_GERMAN_FIXED_RATE_BONDS', 'TYPE89_FRENCH_COMPOUND_METHOD', 'TYPE102_STREET_CONVENTION', 'TYPE129_ISMA_CONVENTION', 'TYPE523_ITALY_TRSY_BONDS', 'TYPE527_ITALY_TRSY_BILL', 'TYPE529_ITALY_2Y_TRSY_BILL','TYPE23_AUSTRALIAN_GOVERNMENT_BONDS','TYPE26_UK_GILT', 'TYPE1029_SPAIN_GOVERNMENT_BONDS'.'TYPE730_SPAIN_T_BILL'."  );
	};

     std::string toString( const BondTypeEnum enumValue )
    {
        switch( enumValue )
        {
            case FIXED_BOND:
                return "FIXED";
                break;
            case FLOATER_BOND:
                return "FLOATER";
                break;
            case PERPETUAL_BOND:
                return "PERPETUAL";
                break;
            case CALLABLE_BOND:
                return "CALLABLE";
                break;
            case INFLATION_LINKED_BOND:
                return "INFLATION-LINKED";
                break;
            case CONVERTIBLE_BOND:
                return "CONVERTIBLE";
                break;
            case STRUCTURED_BOND:
                return "STRUCTURED";
                break;
            default:
				throw ETradingException( "#Error: Invalid BondType, must be'FIXED', 'FLOATER', 'PERPETUAL', 'CALLABLE', 'CALLABLE', 'INFLATION-LINKED', 'CONVERTIBLE' or 'STRUCTURED'." );
                break;
        }
    };

     BondTypeEnum toBondTypeEnum( const std::string& enumString )
    {
        std::string thisEnumString = etrading::trim_to_upper( enumString.c_str() );
        if( boost::iequals( thisEnumString.c_str(), "FIXED" ) )
        {
            return FIXED_BOND;
        }
        if( boost::iequals( thisEnumString.c_str(), "FLOATER" ) || boost::iequals(thisEnumString.c_str(), "FLOATING"))
        {
            return FLOATER_BOND;
        }
        if( boost::iequals( thisEnumString.c_str(), "PERPETUAL" ) )
        {
            return PERPETUAL_BOND;
        }
        if( boost::iequals( thisEnumString.c_str(), "CALLABLE" ) )
        {
            return CALLABLE_BOND;
        }
        if( boost::iequals( thisEnumString.c_str(), "INFLATION-LINKED" ) )
        {
            return INFLATION_LINKED_BOND;
        }
        if( boost::iequals( thisEnumString.c_str(), "CONVERTIBLE" ) )
        {
            return CONVERTIBLE_BOND;
        }
        if( boost::iequals( thisEnumString.c_str(), "STRUCTURED" ) )
        {
            return STRUCTURED_BOND;
        }
		throw ETradingException( "#Error: Invalid BondType, must be'FIXED', 'FLOATER', 'PERPETUAL', 'CALLABLE', 'CALLABLE', 'INFLATION-LINKED', 'CONVERTIBLE' or 'STRUCTURED'." );
    };


     std::string toString( const YieldCalculationTypeEnum enumValue )
    {
        switch( enumValue )
        {
            case NONE_YIELD:
                return "NONE";
                break;
            case ISMA_YIELD:
                return "ISMA";
                break;
            case TRUE_YIELD:
                return "TRUE";
                break;
            case SIMPLE_YIELD:
                return "SIMPLE";
                break;
            default:
				throw ETradingException( "#Error: Invalid BondYieldCalculationType, must be'ISMA', 'TRUE', 'SIMPLE'." );
                break;
        }
    };

     YieldCalculationTypeEnum toYieldCalculationTypeEnum( const std::string& enumString )
    {
         std::string uCaseString = etrading::trim_to_upper( enumString.c_str() );

        if( uCaseString == "" || uCaseString == "NONE" || uCaseString == "NONE_YIELD")
        {
            return NONE_YIELD;
        }
        if( uCaseString == "ISMA" || uCaseString == "ISMA_YIELD" )
        {
            return ISMA_YIELD;
        }
        if( uCaseString == "TRUE" || uCaseString == "TRUE_YIELD" )
        {
            return TRUE_YIELD;
        }
        if( uCaseString == "SIMPLE" || uCaseString == "SIMPLE_YIELD" )
        {
            return SIMPLE_YIELD;
        }
		throw ETradingException( "#Error: Invalid BondYieldCalculationType, must be'ISMA', 'TRUE', 'SIMPLE'." );
    };

     std::string toString( const YieldTypeEnum enumValue )
    {
        switch( enumValue )
        {
            case YIELD_TO_MATURITY:
                return "YIELD-TO-MATURITY";
                break;
            case YIELD_TO_WORST:
                return "YIELD-TO-WORST";
                break;
            case YIELD_TO_CALL:
                return "YIELD-TO-CALL";
                break;
            default:
				throw ETradingException( "#Error: Invalid YieldType, must be 'YIELD-TO-MATURITY', 'YIELD-TO-WORST' or 'YIELD-TO-CALL'." );
                break;
        }
    };

     YieldTypeEnum toYieldTypeEnum( const std::string& enumString )
    {
        std::string thisEnumString = etrading::trim_to_upper( enumString.c_str() );
        if( boost::iequals( thisEnumString.c_str(), "YIELD-TO-MATURITY" ) )
        {
            return YIELD_TO_MATURITY;
        }
        if( boost::iequals( thisEnumString.c_str(), "YTM" ) ) // ALIAS FOR YIELD-TO-MATURITY
        {
            return YIELD_TO_MATURITY;
        }
        if( boost::iequals( thisEnumString.c_str(), "YIELD-TO-WORST" ) )
        {
            return YIELD_TO_WORST;
        }
        if( boost::iequals( thisEnumString.c_str(), "YTW" ) ) // ALIAS FOR YIELD-TO-WORST
        {
            return YIELD_TO_WORST;
        }
        if( boost::iequals( thisEnumString.c_str(), "YIELD-TO-CALL" ) )
        {
            return YIELD_TO_CALL;
        }
        if( boost::iequals( thisEnumString.c_str(), "YTC" ) ) // ALIAS FOR YIELD-TO-CALL
        {
            return YIELD_TO_CALL;
        }

		throw ETradingException( "#Error: Invalid YieldType, must be 'YIELD-TO-MATURITY', 'YIELD-TO-WORST' or 'YIELD-TO-CALL'." );
    };

     std::string toString( const BondQuoteConventionEnum enumValue )
    {
        switch( enumValue )
        {
            case NO_BOND_QUOTE_CONVENTION:
                return "NONE";
                break;
            case QUOTE_IN_32NDS:
                return "QUOTE_IN_32NDS";
                break;
            case QUOTE_IN_64THS:
                return "QUOTE_IN_64THS";
                break;
			case QUOTE_TO_1_DECIMAL_PLACE:
				return "QUOTE_TO_1_DECIMAL_PLACE";
				break;
            case QUOTE_TO_2_DECIMAL_PLACES:
				return "QUOTE_TO_2_DECIMAL_PLACES";
				break;
            case QUOTE_TO_3_DECIMAL_PLACES:
				return "QUOTE_TO_3_DECIMAL_PLACES";
				break;
            case QUOTE_TO_4_DECIMAL_PLACES:
				return "QUOTE_TO_4_DECIMAL_PLACES";
				break;
            case QUOTE_TO_5_DECIMAL_PLACES:
				return "QUOTE_TO_5_DECIMAL_PLACES";
				break;
            default:
				throw ETradingException( "#Error: Invalid BondQuiteConvention, must be 'NONE', 'QUOTE_IN_32NDS', 'QUOTE_IN_64THS', 'QUOTE_TO_1_DECIMAL_PLACE', 'QUOTE_TO_2_DECIMAL_PLACES', 'QUOTE_TO_3_DECIMAL_PLACES', 'QUOTE_TO_4_DECIMAL_PLACES', or 'QUOTE_TO_5_DECIMAL_PLACES' " );
                break;
        }
    };

     BondQuoteConventionEnum toBondQuoteConventionEnum( const std::string& enumString )
    {
        std::string thisEnumString = etrading::trim_to_upper( enumString.c_str() );

        if(thisEnumString == "" || boost::iequals( thisEnumString.c_str(), "NONE" ) || boost::iequals( thisEnumString.c_str(), "NO_CONVENTION" ) || boost::iequals( thisEnumString.c_str(), "NO_BOND_QUOTE_CONVENTION" ) )
        {
            return NO_BOND_QUOTE_CONVENTION;
        }
        
        if( boost::iequals( thisEnumString.c_str(), "QUOTE_IN_32NDS" ) || boost::iequals( thisEnumString.c_str(), "32NDS" ) ) 
        {
            return QUOTE_IN_32NDS;
        }
		
        if( boost::iequals( thisEnumString.c_str(), "QUOTE_IN_64THS" ) || boost::iequals( thisEnumString.c_str(), "64THS" ) ) 
        {
            return QUOTE_IN_64THS;
        }

        if ( boost::iequals(thisEnumString.c_str(), "QUOTE_TO_1_DECIMAL_PLACE") || boost::iequals(thisEnumString.c_str(), "1_DECIMAL_PLACE") || boost::iequals(thisEnumString.c_str(), "1DP") )
		{
			return QUOTE_TO_1_DECIMAL_PLACE;
		}
        
        if ( boost::iequals(thisEnumString.c_str(), "QUOTE_TO_2_DECIMAL_PLACES") || boost::iequals(thisEnumString.c_str(), "2_DECIMAL_PLACES") || boost::iequals(thisEnumString.c_str(), "2DP") )
		{
			return QUOTE_TO_2_DECIMAL_PLACES;
		}
        
        if ( boost::iequals(thisEnumString.c_str(), "QUOTE_TO_3_DECIMAL_PLACES") || boost::iequals(thisEnumString.c_str(), "3_DECIMAL_PLACES") || boost::iequals(thisEnumString.c_str(), "3DP") )
		{
			return QUOTE_TO_3_DECIMAL_PLACES;
		}
        
        if ( boost::iequals(thisEnumString.c_str(), "QUOTE_TO_4_DECIMAL_PLACES") || boost::iequals(thisEnumString.c_str(), "4_DECIMAL_PLACES") || boost::iequals(thisEnumString.c_str(), "4DP") )
		{
			return QUOTE_TO_4_DECIMAL_PLACES;
		}
        
        if ( boost::iequals(thisEnumString.c_str(), "QUOTE_TO_5_DECIMAL_PLACES") || boost::iequals(thisEnumString.c_str(), "5_DECIMAL_PLACES") || boost::iequals(thisEnumString.c_str(), "5DP") )
		{
			return QUOTE_TO_5_DECIMAL_PLACES;
		}

		throw ETradingException( "#Error: Invalid BondQuiteConvention, must be 'NONE', 'QUOTE_IN_32NDS', 'QUOTE_IN_64THS', 'QUOTE_TO_1_DECIMAL_PLACE', 'QUOTE_TO_2_DECIMAL_PLACES', 'QUOTE_TO_3_DECIMAL_PLACES', 'QUOTE_TO_4_DECIMAL_PLACES', or 'QUOTE_TO_5_DECIMAL_PLACES' " );
    };

}