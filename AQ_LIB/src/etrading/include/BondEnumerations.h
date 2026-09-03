// @File            BondEnumerations.h
//
// @Description:    This file as has all the essential enums for Bond Calculation Types
// @Created:        17th January 2017
// @Author:         Nicholas Burgess
// @Department:     Quant Research & Analytics
//
//                  The copyright to the computer program(s) herein is the property of Mizuho International.

#pragma once

#include <string>


namespace etrading
{
    enum BondCalculationTypeEnum
    {
        // These types are the same as defined by Bloomberg
        NO_CALCULATION_TYPE                    = 0,
        TYPE1_STREET_CONVENTION                = 1,
        TYPE2_US_TREASURIES                    = 2,
        TYPE5_ZERO_COUPON_BONDS                = 5,
        TYPE6_DISCOUNTED_COMMERCIAL_PAPER      = 6,
        TYPE13_STREET_CONVENTION_FIXED         = 13,
		TYPE21_FLOATING_RATE_NOTE              = 21,
        TYPE51_JAPANESE_GOVERNMENT_BONDS       = 51,   // Regular or short last coupons
        TYPE235_JAPANESE_GOVERNMENT_BONDS      = 235,  // Long last coupons
        TYPE60_GERMAN_FIXED_RATE_BONDS         = 60,
        TYPE89_FRENCH_COMPOUND_METHOD          = 89,   //French OATs
		TYPE102_STREET_CONVENTION			   = 102, 
		TYPE129_ISMA_CONVENTION                = 129,  //French Government BTAN securities
        TYPE523_ITALY_TRSY_BONDS               = 523,  //Italy Treasury Bonds: BTPS
        TYPE527_ITALY_TRSY_BILL                = 527,  //Italy Treasury 3M, 6M, 12M Bill: BOTS
        TYPE529_ITALY_2Y_TRSY_BILL             = 529,  //Italy Treasury 2 Years Bill: CTZS
		TYPE23_AUSTRALIAN_GOVERNMENT_BONDS	   = 23,   //Australian Government Bonds
		TYPE26_UK_GILT						   = 26,    //UK Gilt
		TYPE1029_SPAIN_GOVERNMENT_BONDS		   = 1029,	//Spain Govt Bonds
		TYPE730_SPAIN_T_BILL				   = 730	//Spain T-Bills

        // TODO: Complete the list as and when new types are needed. There are hundreds, no point setting-up methods
        // if they are not required by the bond's business
    };
     std::string toString( const BondCalculationTypeEnum enumValue );
     BondCalculationTypeEnum toBondCalculationTypeEnum( const std::string& enumString );

    enum BondTypeEnum
    {
        FIXED_BOND,
        FLOATER_BOND,
        PERPETUAL_BOND,
        CALLABLE_BOND,
        INFLATION_LINKED_BOND,
        CONVERTIBLE_BOND,
        STRUCTURED_BOND
    };
     std::string toString( const BondTypeEnum enumValue );
     BondTypeEnum toBondTypeEnum( const std::string& enumString );

    enum YieldCalculationTypeEnum
    {
        NONE_YIELD,
        ISMA_YIELD,
        TRUE_YIELD,
        SIMPLE_YIELD
    };
     std::string toString( const YieldCalculationTypeEnum enumValue );
     YieldCalculationTypeEnum toYieldCalculationTypeEnum( const std::string& enumString );


    enum YieldTypeEnum
    {
        YIELD_TO_MATURITY,
        YIELD_TO_WORST,
        YIELD_TO_CALL
    };
     std::string toString( const YieldTypeEnum enumValue );
     YieldTypeEnum toYieldTypeEnum( const std::string& enumString );

    enum BondQuoteConventionEnum
    {
        NO_BOND_QUOTE_CONVENTION,
        QUOTE_IN_32NDS,
        QUOTE_IN_64THS,
		QUOTE_TO_1_DECIMAL_PLACE,
        QUOTE_TO_2_DECIMAL_PLACES,
        QUOTE_TO_3_DECIMAL_PLACES,
        QUOTE_TO_4_DECIMAL_PLACES,
        QUOTE_TO_5_DECIMAL_PLACES
    };
     std::string toString( const BondQuoteConventionEnum enumValue );
     BondQuoteConventionEnum toBondQuoteConventionEnum( const std::string& enumString );

}