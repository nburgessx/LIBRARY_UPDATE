// TestAqDateFuturesContract.cpp
#include "FuturesDates.h"

// Include: Google Test Library
#include <gTest/gTest.h>



TEST( TestAqDateFuturesContract, UNIT_CheckFuturesContractDates )
{
    // Check Dec-19 futures contract = 18-Dec-2019
    // Note: we can enter the futures contract as Z9, Z19 or Z2019
    
    // Dont test Z9 because this is a dynamic ticker format that rolls on expiry
    //EXPECT_EQ( AQLDate("20191218"), etrading::futureStartDate("Z9") );
    
    EXPECT_EQ( AQLDate("20191218"), etrading::futureStartDate("Z19") );
    EXPECT_EQ( AQLDate("20191218"), etrading::futureStartDate("Z2019") );

    // Check othter futures contracts
    EXPECT_EQ( AQLDate("20200115"), etrading::futureStartDate("F20") );
    EXPECT_EQ( AQLDate("20200219"), etrading::futureStartDate("G20") );
    EXPECT_EQ( AQLDate("20200318"), etrading::futureStartDate("H20") );
    EXPECT_EQ( AQLDate("20200415"), etrading::futureStartDate("J20") );
    EXPECT_EQ( AQLDate("20200520"), etrading::futureStartDate("K20") );
    EXPECT_EQ( AQLDate("20200617"), etrading::futureStartDate("M20") );
    EXPECT_EQ( AQLDate("20200715"), etrading::futureStartDate("N20") );
    EXPECT_EQ( AQLDate("20200819"), etrading::futureStartDate("Q20") );
    EXPECT_EQ( AQLDate("20200916"), etrading::futureStartDate("U20") );
    EXPECT_EQ( AQLDate("20201021"), etrading::futureStartDate("V20") );
    EXPECT_EQ( AQLDate("20201118"), etrading::futureStartDate("X20") );
    EXPECT_EQ( AQLDate("20201216"), etrading::futureStartDate("Z20") );
    
    EXPECT_EQ( AQLDate("20200115"), etrading::futureStartDate("F2020") );
    EXPECT_EQ( AQLDate("20200219"), etrading::futureStartDate("G2020") );
    EXPECT_EQ( AQLDate("20200318"), etrading::futureStartDate("H2020") );
    EXPECT_EQ( AQLDate("20200415"), etrading::futureStartDate("J2020") );
    EXPECT_EQ( AQLDate("20200520"), etrading::futureStartDate("K2020") );
    EXPECT_EQ( AQLDate("20200617"), etrading::futureStartDate("M2020") );
    EXPECT_EQ( AQLDate("20200715"), etrading::futureStartDate("N2020") );
    EXPECT_EQ( AQLDate("20200819"), etrading::futureStartDate("Q2020") );
    EXPECT_EQ( AQLDate("20200916"), etrading::futureStartDate("U2020") );
    EXPECT_EQ( AQLDate("20201021"), etrading::futureStartDate("V2020") );
    EXPECT_EQ( AQLDate("20201118"), etrading::futureStartDate("X2020") );
    EXPECT_EQ( AQLDate("20201216"), etrading::futureStartDate("Z2020") );

    // Check futures contract dates more than 10 years away
    EXPECT_EQ( AQLDate("20291219"), etrading::futureStartDate("Z29") );
    EXPECT_EQ( AQLDate("20291219"), etrading::futureStartDate("Z2029") );
    
    EXPECT_EQ( AQLDate("20391221"), etrading::futureStartDate("Z39") );
    EXPECT_EQ( AQLDate("20391221"), etrading::futureStartDate("Z2039") );
    
    EXPECT_EQ( AQLDate("20491215"), etrading::futureStartDate("Z49") );
    EXPECT_EQ( AQLDate("20491215"), etrading::futureStartDate("Z2049") );
    
    EXPECT_EQ( AQLDate("20591217"), etrading::futureStartDate("Z59") );
    EXPECT_EQ( AQLDate("20591217"), etrading::futureStartDate("Z2059") );
}
