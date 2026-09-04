// TestAqMathBlackScholes.cpp
#include "BlackScholes.h"
#include "tryAqMathBlackScholes.h"

// Include: Google Test Library
#include <gTest/gTest.h>


const double testPrecision = 1e-4;


// Test Cases
// ------------------------------

TEST(TestAqMathBlackScholes, UNIT_TestAqMathBlackScholesPrices_dCall_dSpot)
{
	// Black-Scholes Base Case
	const etrading::CallOrPutEnum callOrPut = etrading::CALL_OPTION;
	const double spot = 100.0;
	const double strike = 100.0;
	const double volatility = 0.1;
	const double timeToExpiry = 1.0;
	const double rate = 0.01;
	const double carry = 0.0;

	// Declare Black-Scholes Option Classes
	etrading::BlackScholes bs50(callOrPut, 50, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs60(callOrPut, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs70(callOrPut, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs80(callOrPut, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs90(callOrPut, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bsSpot(callOrPut, spot, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs110(callOrPut, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs120(callOrPut, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs130(callOrPut, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs140(callOrPut, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs150(callOrPut, 150, strike, volatility, timeToExpiry, rate, carry);

	// ATM - At the Money
	// ---------------------
	EXPECT_NEAR(bsSpot.price(), 3.9480823, testPrecision);

	// OTM - Out the Money
	// ---------------------
	EXPECT_NEAR(bs50.price(), 0.0000000, testPrecision);
	EXPECT_NEAR(bs60.price(), 0.0000002, testPrecision);
	EXPECT_NEAR(bs70.price(), 0.0003703, testPrecision);
	EXPECT_NEAR(bs80.price(), 0.0395172, testPrecision);
	EXPECT_NEAR(bs90.price(), 0.7052926, testPrecision);


	// ITM - In the Money
	// ---------------------
	EXPECT_NEAR(bs110.price(), 10.8449538, testPrecision);
	EXPECT_NEAR(bs120.price(), 19.9468630, testPrecision);
	EXPECT_NEAR(bs130.price(), 29.7168016, testPrecision);
	EXPECT_NEAR(bs140.price(), 39.6031558, testPrecision);
	EXPECT_NEAR(bs150.price(), 49.5025595, testPrecision);
}


TEST(TestAqMathBlackScholes, UNIT_TestAqMathBlackScholesPrices_Sequential)
{
	// Black-Scholes Base Case
	const size_t numberOfTests = 1000;

	const std::vector< etrading::CallOrPutEnum > call(numberOfTests, etrading::CALL_OPTION);
	const std::vector< etrading::CallOrPutEnum > put(numberOfTests, etrading::PUT_OPTION);
	const std::vector< double > spot(numberOfTests, 100.0);
	const std::vector< double > strike(numberOfTests, 100.0);
	const std::vector< double > volatility(numberOfTests, 0.1);
	const std::vector< double > timeToExpiry(numberOfTests, 1.0);
	const std::vector< double > rate(numberOfTests, 0.01);
	const std::vector< double > carry(numberOfTests, 0.0);
	const std::vector< double > shift(numberOfTests, 0.0);

	// Calculate Sequentially
	const bool optimize = false;
	const std::vector< double > callResults = validation::tryAqMathBlackScholesPrices(call, spot, strike, volatility, timeToExpiry, rate, carry, shift, optimize);
	const std::vector< double > putResults = validation::tryAqMathBlackScholesPrices(put, spot, strike, volatility, timeToExpiry, rate, carry, shift, optimize);

	// End and Fail test of Call and Put vector results are not the same
	ASSERT_EQ(callResults.size(), putResults.size());

	// ATM Calls / Puts
	for (size_t i = 0; i < callResults.size(); ++i)
	{
		EXPECT_NEAR(callResults[i], 3.9480823, testPrecision);
		EXPECT_NEAR(putResults[i], 3.9480823, testPrecision);
	}
}

TEST(TestAqMathBlackScholes, UNIT_TestAqMathBlackScholesPrices_Parallel)
{
	// Black-Scholes Base Case
	const size_t numberOfTests = 1000;

	const std::vector< etrading::CallOrPutEnum > call(numberOfTests, etrading::CALL_OPTION);
	const std::vector< etrading::CallOrPutEnum > put(numberOfTests, etrading::PUT_OPTION);
	const std::vector< double > spot(numberOfTests, 100.0);
	const std::vector< double > strike(numberOfTests, 100.0);
	const std::vector< double > volatility(numberOfTests, 0.1);
	const std::vector< double > timeToExpiry(numberOfTests, 1.0);
	const std::vector< double > rate(numberOfTests, 0.01);
	const std::vector< double > carry(numberOfTests, 0.0);
	const std::vector< double > shift(numberOfTests, 0.0);

	// Calculate in Parallel using OMP
	const bool optimize = true;
	const std::vector< double > callResults = validation::tryAqMathBlackScholesPrices(call, spot, strike, volatility, timeToExpiry, rate, carry, shift, optimize);
	const std::vector< double > putResults = validation::tryAqMathBlackScholesPrices(put, spot, strike, volatility, timeToExpiry, rate, carry, shift, optimize);

	// End and Fail test of Call and Put vector results are not the same
	ASSERT_EQ(callResults.size(), putResults.size());

	// ATM Calls / Puts
	for (size_t i = 0; i < callResults.size(); ++i)
	{
		EXPECT_NEAR(callResults[i], 3.9480823, testPrecision);
		EXPECT_NEAR(putResults[i], 3.9480823, testPrecision);
	}
}

TEST(TestAqMathBlackScholes, UNIT_TestAqMathBlackScholesPrices_dPut_dSpot)
{
	// Black-Scholes Base Case
	const etrading::CallOrPutEnum callOrPut = etrading::PUT_OPTION;
	const double spot = 100.0;
	const double strike = 100.0;
	const double volatility = 0.1;
	const double timeToExpiry = 1.0;
	const double rate = 0.01;
	const double carry = 0.0;

	// Declare Black-Scholes Option Classes
	etrading::BlackScholes bs50(callOrPut, 50, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs60(callOrPut, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs70(callOrPut, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs80(callOrPut, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs90(callOrPut, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bsSpot(callOrPut, spot, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs110(callOrPut, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs120(callOrPut, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs130(callOrPut, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs140(callOrPut, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes bs150(callOrPut, 150, strike, volatility, timeToExpiry, rate, carry);

	// ATM - At the Money
	// ---------------------
	EXPECT_NEAR(bsSpot.price(), 3.9480823, testPrecision);


	// OTM - Out the Money
	// ---------------------
	EXPECT_NEAR(bs50.price(), 49.5024917, testPrecision);
	EXPECT_NEAR(bs60.price(), 39.6019936, testPrecision);
	EXPECT_NEAR(bs70.price(), 29.7018653, testPrecision);
	EXPECT_NEAR(bs80.price(), 19.8405139, testPrecision);
	EXPECT_NEAR(bs90.price(), 10.6057909, testPrecision);


	// ITM - In the Money
	// ---------------------
	EXPECT_NEAR(bs110.price(), 0.9444555, testPrecision);
	EXPECT_NEAR(bs120.price(), 0.1458663, testPrecision);
	EXPECT_NEAR(bs130.price(), 0.0153066, testPrecision);
	EXPECT_NEAR(bs140.price(), 0.0011625, testPrecision);
	EXPECT_NEAR(bs150.price(), 0.0000678, testPrecision);
}


TEST(TestAqMathBlackScholes, UNIT_TestAqMathBlackScholesImpliedVol)
{
	// Black-Scholes Base Case
	const etrading::CallOrPutEnum call = etrading::CALL_OPTION;
	const etrading::CallOrPutEnum put = etrading::PUT_OPTION;
	const double price = 3.9480823;
	const double spot = 100.0;
	const double strike = 100.0;
	const double initialGuessVol = 0.2;
	const double expectedVol = 0.1;
	const double timeToExpiry = 1.0;
	const double rate = 0.01;
	const double carry = 0.0;
	double impliedVol = 0.0;

	// Volatility Here is the Intial Guess for the Implied Vol Solver
	etrading::BlackScholes callOption(call, spot, strike, initialGuessVol, timeToExpiry, rate, carry);
	etrading::BlackScholes putOption(call, spot, strike, initialGuessVol, timeToExpiry, rate, carry);

	// Implied Vol Solver - For Call Option
	impliedVol = callOption.calculateImpliedVol(price);
	EXPECT_NEAR(impliedVol, expectedVol, testPrecision);

	// Implied Vol Solver - For Put Option
	impliedVol = putOption.calculateImpliedVol(price);
	EXPECT_NEAR(impliedVol, expectedVol, testPrecision);
}


TEST(TestAqMathBlackScholes, UNIT_TestAqMathBlackScholesImpliedVols_Sequential)
{
	// Black-Scholes Base Case
	const double vol = 0.1;
	const size_t numberOfTests = 1000;

	const std::vector< etrading::CallOrPutEnum > call(numberOfTests, etrading::CALL_OPTION);
	const std::vector< etrading::CallOrPutEnum > put(numberOfTests, etrading::PUT_OPTION);
	const std::vector< double > spot(numberOfTests, 100.0);
	const std::vector< double > strike(numberOfTests, 100.0);
	const std::vector< double > volatility(numberOfTests, vol);
	const std::vector< double > timeToExpiry(numberOfTests, 1.0);
	const std::vector< double > rate(numberOfTests, 0.01);
	const std::vector< double > carry(numberOfTests, 0.0);
	const std::vector< double > shift(numberOfTests, 0.0);

	// Calculate Prices Sequentially
	const bool optimize = false;
	const std::vector< double > callPrices = validation::tryAqMathBlackScholesPrices(call, spot, strike, volatility, timeToExpiry, rate, carry, shift, optimize);
	const std::vector< double > putPrices = validation::tryAqMathBlackScholesPrices(put, spot, strike, volatility, timeToExpiry, rate, carry, shift, optimize);

	// End and Fail test of Call and Put price results are not the same
	ASSERT_EQ(callPrices.size(), putPrices.size());

	// Calculate Implied Vols Sequentially
	const std::vector< double > callImpliedVols = validation::tryAqMathBlackScholesImpliedVols(callPrices, call, spot, strike, timeToExpiry, rate, carry, shift, optimize);
	const std::vector< double > putImpliedVols = validation::tryAqMathBlackScholesImpliedVols(putPrices, put, spot, strike, timeToExpiry, rate, carry, shift, optimize);

	// End and Fail test of Call and Put implied vols results are not the same
	ASSERT_EQ(callImpliedVols.size(), putImpliedVols.size());

	// ATM Calls / Put Implied Vols
	for (size_t i = 0; i < callImpliedVols.size(); ++i)
	{
		EXPECT_NEAR(callImpliedVols[i], vol, testPrecision);
		EXPECT_NEAR(putImpliedVols[i], vol, testPrecision);
	}
}

TEST(TestAqMathBlackScholes, UNIT_TestAqMathBlackScholesImpliedVols_Parallel)
{
	// Black-Scholes Base Case
	const double vol = 0.1;
	const size_t numberOfTests = 1000;

	const std::vector< etrading::CallOrPutEnum > call(numberOfTests, etrading::CALL_OPTION);
	const std::vector< etrading::CallOrPutEnum > put(numberOfTests, etrading::PUT_OPTION);
	const std::vector< double > spot(numberOfTests, 100.0);
	const std::vector< double > strike(numberOfTests, 100.0);
	const std::vector< double > volatility(numberOfTests, vol);
	const std::vector< double > timeToExpiry(numberOfTests, 1.0);
	const std::vector< double > rate(numberOfTests, 0.01);
	const std::vector< double > carry(numberOfTests, 0.0);
	const std::vector< double > shift(numberOfTests, 0.0);

	// Calculate Prices in Parallel using OMP
	const bool optimize = true;
	const std::vector< double > callPrices = validation::tryAqMathBlackScholesPrices(call, spot, strike, volatility, timeToExpiry, rate, carry, shift, optimize);
	const std::vector< double > putPrices = validation::tryAqMathBlackScholesPrices(put, spot, strike, volatility, timeToExpiry, rate, carry, shift, optimize);

	// End and Fail test of Call and Put price results are not the same
	ASSERT_EQ(callPrices.size(), putPrices.size());

	// Calculate Implied Vols in Parallel using OMP
	const std::vector< double > callImpliedVols = validation::tryAqMathBlackScholesImpliedVols(callPrices, call, spot, strike, timeToExpiry, rate, carry, shift, optimize);
	const std::vector< double > putImpliedVols = validation::tryAqMathBlackScholesImpliedVols(putPrices, put, spot, strike, timeToExpiry, rate, carry, shift, optimize);

	// End and Fail test of Call and Put implied vols results are not the same
	ASSERT_EQ(callImpliedVols.size(), putImpliedVols.size());

	// ATM Calls / Put Implied Vols
	for (size_t i = 0; i < callImpliedVols.size(); ++i)
	{
		EXPECT_NEAR(callImpliedVols[i], vol, testPrecision);
		EXPECT_NEAR(putImpliedVols[i], vol, testPrecision);
	}
}

// A quick sanity check of ATM greeks
TEST( TestAqMathBlackScholes, CONSISTENCY_TestAqMathBlackScholesATMGreeks_CompareAnalyticalVsNumericalAndAAD )
{
	// Black-Scholes Base Case
	const etrading::CallOrPutEnum call = etrading::CALL_OPTION;
	const etrading::CallOrPutEnum put = etrading::PUT_OPTION;
	const double spot = 100.0;
	const double strike = 100.0;
	const double volatility = 0.1;
	const double timeToExpiry = 1.0;
	const double rate = 0.01;
	const double carry = 0.0;

	// Additional test for the ATM call option: Verify that the method priceAndGreeksNumerical()
	// is consistent with priceAndGreeksAnalytical() and priceAndGreeksAAD()
	const double deltaBump = 1.0e-5;
	const double gammaBump = 1.0e-4;
	const double vegaBump  = 1.0e-5;
	const double rhoBump   = 1.0e-5;
	const double thetaBump = 1.0 / 365.25;

	etrading::BlackScholes call100(call, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholesGreeks numericalGreeks  = call100.priceAndGreeksNumerical( deltaBump, gammaBump, vegaBump, rhoBump, thetaBump );
	etrading::BlackScholesGreeks analyticalGreeks = call100.priceAndGreeksAnalytical();
	etrading::BlackScholesGreeks aadGreeks = call100.priceAndGreeksAAD();

	EXPECT_NEAR( call100.price(), numericalGreeks.price, testPrecision );
	EXPECT_NEAR( analyticalGreeks.price, numericalGreeks.price, testPrecision );
	EXPECT_NEAR( analyticalGreeks.price, aadGreeks.price, testPrecision );

	EXPECT_NEAR( call100.numericalDeltaSpot( deltaBump ), numericalGreeks.deltaSpot, testPrecision );
	EXPECT_NEAR( analyticalGreeks.deltaSpot, numericalGreeks.deltaSpot, testPrecision );
	EXPECT_NEAR( analyticalGreeks.deltaSpot, aadGreeks.deltaSpot, testPrecision );

	EXPECT_NEAR( call100.numericalDeltaForward( gammaBump ), numericalGreeks.deltaForward, testPrecision );
	EXPECT_NEAR( analyticalGreeks.deltaForward, numericalGreeks.deltaForward, testPrecision );
	EXPECT_NEAR( analyticalGreeks.deltaForward, aadGreeks.deltaForward, testPrecision );

	EXPECT_NEAR( call100.numericalVega( vegaBump ), numericalGreeks.vega, testPrecision );
	EXPECT_NEAR( analyticalGreeks.vega, numericalGreeks.vega, testPrecision );
	EXPECT_NEAR( analyticalGreeks.vega, aadGreeks.vega, testPrecision );

	EXPECT_NEAR( call100.numericalRho( rhoBump ), numericalGreeks.rho, testPrecision );
	EXPECT_NEAR( analyticalGreeks.rho, numericalGreeks.rho, testPrecision );
	EXPECT_NEAR( analyticalGreeks.rho, aadGreeks.rho, testPrecision );

	EXPECT_NEAR( call100.numericalTheta( thetaBump ), numericalGreeks.theta, testPrecision );
	EXPECT_NEAR( analyticalGreeks.theta, numericalGreeks.theta, testPrecision );
	EXPECT_NEAR( analyticalGreeks.theta, aadGreeks.theta, testPrecision );


	// Additional test for the ATM put option: Verify that the method priceAndGreeksNumerical()
	// is consistent with priceAndGreeksAnalytical() and priceAndGreeksAAD()
	etrading::BlackScholes put100(put, 100, strike, volatility, timeToExpiry, rate, carry);
	numericalGreeks  = put100.priceAndGreeksNumerical( deltaBump, gammaBump, vegaBump, rhoBump, thetaBump );
	analyticalGreeks = put100.priceAndGreeksAnalytical();
	aadGreeks        = put100.priceAndGreeksAAD();

	EXPECT_NEAR( put100.price(), numericalGreeks.price, testPrecision );
	EXPECT_NEAR( analyticalGreeks.theta, numericalGreeks.theta, testPrecision );
	EXPECT_NEAR( analyticalGreeks.theta, aadGreeks.theta, testPrecision );

	EXPECT_NEAR( put100.numericalDeltaSpot( deltaBump ), numericalGreeks.deltaSpot, testPrecision );
	EXPECT_NEAR( analyticalGreeks.deltaSpot, numericalGreeks.deltaSpot, testPrecision );
	EXPECT_NEAR( analyticalGreeks.deltaSpot, aadGreeks.deltaSpot, testPrecision );

	EXPECT_NEAR( put100.numericalDeltaForward( gammaBump ), numericalGreeks.deltaForward, testPrecision );
	EXPECT_NEAR( analyticalGreeks.deltaForward, numericalGreeks.deltaForward, testPrecision );
	EXPECT_NEAR( analyticalGreeks.deltaForward, aadGreeks.deltaForward, testPrecision );

	EXPECT_NEAR( put100.numericalVega( vegaBump ), numericalGreeks.vega, testPrecision );
	EXPECT_NEAR( analyticalGreeks.vega, numericalGreeks.vega, testPrecision );
	EXPECT_NEAR( analyticalGreeks.vega, aadGreeks.vega, testPrecision );

	EXPECT_NEAR( put100.numericalRho( rhoBump ), numericalGreeks.rho, testPrecision );
	EXPECT_NEAR( analyticalGreeks.rho, numericalGreeks.rho, testPrecision );
	EXPECT_NEAR( analyticalGreeks.rho, aadGreeks.rho, testPrecision );

	EXPECT_NEAR( put100.numericalTheta( thetaBump ), numericalGreeks.theta, testPrecision );
	EXPECT_NEAR( analyticalGreeks.theta, numericalGreeks.theta, testPrecision );
	EXPECT_NEAR( analyticalGreeks.theta, aadGreeks.theta, testPrecision );
}

TEST( TestAqMathBlackScholes, CONSISTENCY_TestAqMathBlackScholesDeltaSpot_CompareAnalyticalVsNumericalAndAAD )
{
	// Black-Scholes Base Case
	const etrading::CallOrPutEnum call = etrading::CALL_OPTION;
	const etrading::CallOrPutEnum put = etrading::PUT_OPTION;
	const double spot = 100.0;
	const double strike = 100.0;
	const double volatility = 0.1;
	const double timeToExpiry = 1.0;
	const double rate = 0.01;
	const double carry = 0.0;
	const double bump = 1e-5;
	double deltaSpotNumerical = 0.0;
	double deltaSpotAnalytical = 0.0;

	etrading::BlackScholesGreeks aadGreeks;

	// Declare Black-Scholes Option Classes
	etrading::BlackScholes call150(call, 150, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call140(call, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call130(call, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call120(call, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call110(call, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call100(call, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call90(call, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call80(call, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call70(call, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call60(call, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call50(call, 50, strike, volatility, timeToExpiry, rate, carry);

	etrading::BlackScholes put150(put, 150, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put140(put, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put130(put, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put120(put, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put110(put, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put100(put, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put90(put, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put80(put, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put70(put, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put60(put, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put50(put, 50, strike, volatility, timeToExpiry, rate, carry);

	// Compare Analytical and Numerical Risk for Call(s)
	// -------------------------------------------------

	// 1. ATM - At-the-Money Call
	deltaSpotNumerical = call100.numericalDeltaSpot(bump);
	deltaSpotAnalytical = call100.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);

	aadGreeks = call100.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );


	// 2. OTM - Out-of-Money Calls
	deltaSpotNumerical = call90.numericalDeltaSpot(bump);
	deltaSpotAnalytical = call90.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);

	aadGreeks = call90.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = call80.numericalDeltaSpot(bump);
	deltaSpotAnalytical = call80.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = call80.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = call70.numericalDeltaSpot(bump);
	deltaSpotAnalytical = call70.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
		
	aadGreeks = call70.priceAndGreeksAAD();;
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = call60.numericalDeltaSpot(bump);
	deltaSpotAnalytical = call60.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = call60.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = call50.numericalDeltaSpot(bump);
	deltaSpotAnalytical = call50.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = call50.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );


	// 3. ITM - In-the-Money Calls
	deltaSpotNumerical = call110.numericalDeltaSpot(bump);
	deltaSpotAnalytical = call110.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = call110.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = call120.numericalDeltaSpot(bump);
	deltaSpotAnalytical = call120.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);

	
	aadGreeks = call120.priceAndGreeksAAD();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);

	deltaSpotNumerical = call130.numericalDeltaSpot(bump);
	deltaSpotAnalytical = call130.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = call130.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = call140.numericalDeltaSpot(bump);
	deltaSpotAnalytical = call140.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = call140.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = call150.numericalDeltaSpot(bump);
	deltaSpotAnalytical = call150.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = call150.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );


	// 4. ATM - At-the-Money Put
	deltaSpotNumerical = put100.numericalDeltaSpot(bump);
	deltaSpotAnalytical = put100.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = put100.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );


	// 5. ITM - In-the-Money Put
	deltaSpotNumerical = put90.numericalDeltaSpot(bump);
	deltaSpotAnalytical = put90.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = put90.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = put80.numericalDeltaSpot(bump);
	deltaSpotAnalytical = put80.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = put80.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = put70.numericalDeltaSpot(bump);
	deltaSpotAnalytical = put70.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = put70.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = put60.numericalDeltaSpot(bump);
	deltaSpotAnalytical = put60.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = put60.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = put50.numericalDeltaSpot(bump);
	deltaSpotAnalytical = put50.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = put50.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );


	// 6. OTM - Out-of-Money Put
	deltaSpotNumerical = put110.numericalDeltaSpot(bump);
	deltaSpotAnalytical = put110.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = put110.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = put120.numericalDeltaSpot(bump);
	deltaSpotAnalytical = put120.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = put120.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = put130.numericalDeltaSpot(bump);
	deltaSpotAnalytical = put130.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = put130.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = put140.numericalDeltaSpot(bump);
	deltaSpotAnalytical = put140.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);
	
	aadGreeks = put140.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );

	deltaSpotNumerical = put150.numericalDeltaSpot(bump);
	deltaSpotAnalytical = put150.deltaSpot();
	EXPECT_NEAR(deltaSpotAnalytical, deltaSpotNumerical, testPrecision);

	aadGreeks = put150.priceAndGreeksAAD();
	EXPECT_NEAR( deltaSpotAnalytical, aadGreeks.deltaSpot, testPrecision );
}


TEST( TestAqMathBlackScholes, CONSISTENCY_TestAqMathBlackScholesDeltaForward_CompareAnalyticalVsNumerical )
{
	// Black-Scholes Base Case
	const etrading::CallOrPutEnum call = etrading::CALL_OPTION;
	const etrading::CallOrPutEnum put = etrading::PUT_OPTION;
	const double Forward = 100.0;
	const double strike = 100.0;
	const double volatility = 0.1;
	const double timeToExpiry = 1.0;
	const double rate = 0.01;
	const double carry = 0.0;
	const double bump = 1e-5;
	double deltaForwardNumerical = 0.0;
	double deltaForwardAnalytical = 0.0;

	// Declare Black-Scholes Option Classes
	etrading::BlackScholes call150(call, 150, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call140(call, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call130(call, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call120(call, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call110(call, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call100(call, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call90(call, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call80(call, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call70(call, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call60(call, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call50(call, 50, strike, volatility, timeToExpiry, rate, carry);

	etrading::BlackScholes put150(put, 150, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put140(put, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put130(put, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put120(put, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put110(put, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put100(put, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put90(put, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put80(put, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put70(put, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put60(put, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put50(put, 50, strike, volatility, timeToExpiry, rate, carry);

	// Compare Analytical and Numerical Risk for Call(s)
	// -------------------------------------------------

	// 1. ATM - At-the-Money Call
	deltaForwardNumerical = call100.numericalDeltaForward(bump);
	deltaForwardAnalytical = call100.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);


	// 2. OTM - Out-of-Money Calls
	deltaForwardNumerical = call90.numericalDeltaForward(bump);
	deltaForwardAnalytical = call90.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = call80.numericalDeltaForward(bump);
	deltaForwardAnalytical = call80.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = call70.numericalDeltaForward(bump);
	deltaForwardAnalytical = call70.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = call60.numericalDeltaForward(bump);
	deltaForwardAnalytical = call60.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = call50.numericalDeltaForward(bump);
	deltaForwardAnalytical = call50.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);


	// 3. ITM - Out-of-Money Calls
	deltaForwardNumerical = call110.numericalDeltaForward(bump);
	deltaForwardAnalytical = call110.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = call120.numericalDeltaForward(bump);
	deltaForwardAnalytical = call120.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = call130.numericalDeltaForward(bump);
	deltaForwardAnalytical = call130.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = call140.numericalDeltaForward(bump);
	deltaForwardAnalytical = call140.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = call150.numericalDeltaForward(bump);
	deltaForwardAnalytical = call150.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);


	// 4. ATM - At-the-Money Put
	deltaForwardNumerical = put100.numericalDeltaForward(bump);
	deltaForwardAnalytical = put100.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);


	// 5. ITM - Out-of-Money Puts
	deltaForwardNumerical = put90.numericalDeltaForward(bump);
	deltaForwardAnalytical = put90.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = put80.numericalDeltaForward(bump);
	deltaForwardAnalytical = put80.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = put70.numericalDeltaForward(bump);
	deltaForwardAnalytical = put70.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = put60.numericalDeltaForward(bump);
	deltaForwardAnalytical = put60.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = put50.numericalDeltaForward(bump);
	deltaForwardAnalytical = put50.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);


	// 6. OTM - Out-of-Money Puts
	deltaForwardNumerical = put110.numericalDeltaForward(bump);
	deltaForwardAnalytical = put110.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = put120.numericalDeltaForward(bump);
	deltaForwardAnalytical = put120.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = put130.numericalDeltaForward(bump);
	deltaForwardAnalytical = put130.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = put140.numericalDeltaForward(bump);
	deltaForwardAnalytical = put140.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);

	deltaForwardNumerical = put150.numericalDeltaForward(bump);
	deltaForwardAnalytical = put150.deltaForward();
	EXPECT_NEAR(deltaForwardAnalytical, deltaForwardNumerical, testPrecision);
}


TEST( TestAqMathBlackScholes, CONSISTENCY_TestAqMathBlackScholesGamma_CompareAnalyticalVsNumericalAndAAD )
{
	// Black-Scholes Base Case
	const etrading::CallOrPutEnum call = etrading::CALL_OPTION;
	const etrading::CallOrPutEnum put = etrading::PUT_OPTION;
	const double spot = 100.0;
	const double strike = 100.0;
	const double volatility = 0.1;
	const double timeToExpiry = 1.0;
	const double rate = 0.01;
	const double carry = 0.0;
	const double bump = 1e-4;
	double gammaNumerical = 0.0;
	double gammaAnalytical = 0.0;

	etrading::BlackScholesGreeks aadGreeks;

	// Declare Black-Scholes Option Classes
	etrading::BlackScholes call150(call, 150, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call140(call, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call130(call, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call120(call, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call110(call, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call100(call, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call90(call, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call80(call, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call70(call, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call60(call, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call50(call, 50, strike, volatility, timeToExpiry, rate, carry);

	etrading::BlackScholes put150(put, 150, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put140(put, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put130(put, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put120(put, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put110(put, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put100(put, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put90(put, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put80(put, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put70(put, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put60(put, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put50(put, 50, strike, volatility, timeToExpiry, rate, carry);

	// Compare Analytical and Numerical Risk for Call(s)
	// -------------------------------------------------

	// 1. ATM - At-the-Money Call
	gammaNumerical = call100.numericalGamma(bump);
	gammaAnalytical = call100.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = call100.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	// 2. OTM - Out-of-Money Calls
	gammaNumerical = call90.numericalGamma(bump);
	gammaAnalytical = call90.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = call90.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = call80.numericalGamma(bump);
	gammaAnalytical = call80.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = call80.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = call70.numericalGamma(bump);
	gammaAnalytical = call70.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = call70.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = call60.numericalGamma(bump);
	gammaAnalytical = call60.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = call60.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = call50.numericalGamma(bump);
	gammaAnalytical = call50.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = call50.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );


	// 3. ITM - Out-of-Money Calls
	gammaNumerical = call110.numericalGamma(bump);
	gammaAnalytical = call110.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = call110.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );;

	gammaNumerical = call120.numericalGamma(bump);
	gammaAnalytical = call120.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = call120.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = call130.numericalGamma(bump);
	gammaAnalytical = call130.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = call130.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = call140.numericalGamma(bump);
	gammaAnalytical = call140.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = call140.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = call150.numericalGamma(bump);
	gammaAnalytical = call150.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = call150.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	// 4. ATM - At-the-Money Put
	gammaNumerical = put100.numericalGamma(bump);
	gammaAnalytical = put100.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = put100.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	// 5. ITM - Out-of-Money Puts
	gammaNumerical = put90.numericalGamma(bump);
	gammaAnalytical = put90.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = put90.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = put80.numericalGamma(bump);
	gammaAnalytical = put80.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = put80.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = put70.numericalGamma(bump);
	gammaAnalytical = put70.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = put70.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = put60.numericalGamma(bump);
	gammaAnalytical = put60.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = put60.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = put50.numericalGamma(bump);
	gammaAnalytical = put50.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = put50.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	// 6. OTM - Out-of-Money Puts
	gammaNumerical = put110.numericalGamma(bump);
	gammaAnalytical = put110.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = put110.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = put120.numericalGamma(bump);
	gammaAnalytical = put120.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = put120.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = put130.numericalGamma(bump);
	gammaAnalytical = put130.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = put130.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = put140.numericalGamma(bump);
	gammaAnalytical = put140.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = put140.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );

	gammaNumerical = put150.numericalGamma(bump);
	gammaAnalytical = put150.gamma();
	EXPECT_NEAR(gammaAnalytical, gammaNumerical, testPrecision);

	aadGreeks = put150.priceAndGreeksAAD();
	EXPECT_NEAR( gammaAnalytical, aadGreeks.gamma, testPrecision );
}


TEST( TestAqMathBlackScholes, CONSISTENCY_TestAqMathBlackScholesVega_CompareAnalyticalVsNumericalAndAAD )
{
	// Black-Scholes Base Case
	const etrading::CallOrPutEnum call = etrading::CALL_OPTION;
	const etrading::CallOrPutEnum put = etrading::PUT_OPTION;
	const double spot = 100.0;
	const double strike = 100.0;
	const double volatility = 0.1;
	const double timeToExpiry = 1.0;
	const double rate = 0.01;
	const double carry = 0.0;
	const double bump = 1e-5;
	double vegaNumerical = 0.0;
	double vegaAnalytical = 0.0;

	etrading::BlackScholesGreeks aadGreeks;

	// Declare Black-Scholes Option Classes
	etrading::BlackScholes call150(call, 150, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call140(call, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call130(call, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call120(call, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call110(call, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call100(call, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call90(call, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call80(call, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call70(call, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call60(call, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call50(call, 50, strike, volatility, timeToExpiry, rate, carry);

	etrading::BlackScholes put150(put, 150, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put140(put, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put130(put, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put120(put, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put110(put, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put100(put, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put90(put, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put80(put, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put70(put, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put60(put, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put50(put, 50, strike, volatility, timeToExpiry, rate, carry);

	// Compare Analytical and Numerical Risk for Call(s)
	// -------------------------------------------------

	// 1. ATM - At-the-Money Call
	vegaNumerical = call100.numericalVega(bump);
	vegaAnalytical = call100.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = call100.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	// 2. OTM - Out-of-Money Calls
	vegaNumerical = call90.numericalVega(bump);
	vegaAnalytical = call90.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = call90.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = call80.numericalVega(bump);
	vegaAnalytical = call80.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = call80.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = call70.numericalVega(bump);
	vegaAnalytical = call70.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = call70.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = call60.numericalVega(bump);
	vegaAnalytical = call60.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = call60.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = call50.numericalVega(bump);
	vegaAnalytical = call50.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = call50.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );


	// 3. ITM - Out-of-Money Calls
	vegaNumerical = call110.numericalVega(bump);
	vegaAnalytical = call110.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = call110.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = call120.numericalVega(bump);
	vegaAnalytical = call120.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = call120.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = call130.numericalVega(bump);
	vegaAnalytical = call130.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = call130.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = call140.numericalVega(bump);
	vegaAnalytical = call140.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = call140.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = call150.numericalVega(bump);
	vegaAnalytical = call150.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = call150.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );


	// 4. ATM - At-the-Money Put
	vegaNumerical = put100.numericalVega(bump);
	vegaAnalytical = put100.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = put100.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	// 5. ITM - Out-of-Money Puts
	vegaNumerical = put90.numericalVega(bump);
	vegaAnalytical = put90.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = put90.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = put80.numericalVega(bump);
	vegaAnalytical = put80.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = put80.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = put70.numericalVega(bump);
	vegaAnalytical = put70.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = put70.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = put60.numericalVega(bump);
	vegaAnalytical = put60.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = put60.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = put50.numericalVega(bump);
	vegaAnalytical = put50.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = put50.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );


	// 6. OTM - Out-of-Money Puts
	vegaNumerical = put110.numericalVega(bump);
	vegaAnalytical = put110.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = put110.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = put120.numericalVega(bump);
	vegaAnalytical = put120.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = put120.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = put130.numericalVega(bump);
	vegaAnalytical = put130.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = put130.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = put140.numericalVega(bump);
	vegaAnalytical = put140.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = put140.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );

	vegaNumerical = put150.numericalVega(bump);
	vegaAnalytical = put150.vega();
	EXPECT_NEAR(vegaAnalytical, vegaNumerical, testPrecision);

	aadGreeks = put150.priceAndGreeksAAD();
	EXPECT_NEAR( vegaAnalytical, aadGreeks.vega, testPrecision );
}


TEST( TestAqMathBlackScholes, CONSISTENCY_TestAqMathBlackScholesTheta_CompareAnalyticalVsNumericalAndAAD )
{
	// Black-Scholes Base Case
	const etrading::CallOrPutEnum call = etrading::CALL_OPTION;
	const etrading::CallOrPutEnum put = etrading::PUT_OPTION;
	const double spot = 100.0;
	const double strike = 100.0;
	const double volatility = 0.1;
	const double timeToExpiry = 1.0;
	const double rate = 0.01;
	const double carry = 0.0;
	const double bump = 1 / 365.25;
	double thetaNumerical = 0.0;
	double thetaAnalytical = 0.0;

	etrading::BlackScholesGreeks aadGreeks;

	// Declare Black-Scholes Option Classes
	etrading::BlackScholes call150(call, 150, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call140(call, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call130(call, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call120(call, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call110(call, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call100(call, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call90(call, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call80(call, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call70(call, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call60(call, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call50(call, 50, strike, volatility, timeToExpiry, rate, carry);

	etrading::BlackScholes put150(put, 150, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put140(put, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put130(put, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put120(put, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put110(put, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put100(put, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put90(put, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put80(put, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put70(put, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put60(put, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put50(put, 50, strike, volatility, timeToExpiry, rate, carry);

	// Compare Analytical and Numerical Risk for Call(s)
	// -------------------------------------------------

	// 1. ATM - At-the-Money Call
	thetaNumerical = call100.numericalTheta(bump);
	thetaAnalytical = call100.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = call100.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	// 2. OTM - Out-of-Money Calls
	thetaNumerical = call90.numericalTheta(bump);
	thetaAnalytical = call90.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = call90.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = call80.numericalTheta(bump);
	thetaAnalytical = call80.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = call80.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = call70.numericalTheta(bump);
	thetaAnalytical = call70.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = call70.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = call60.numericalTheta(bump);
	thetaAnalytical = call60.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = call60.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = call50.numericalTheta(bump);
	thetaAnalytical = call50.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = call50.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );


	// 3. ITM - Out-of-Money Calls
	thetaNumerical = call110.numericalTheta(bump);
	thetaAnalytical = call110.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = call110.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = call120.numericalTheta(bump);
	thetaAnalytical = call120.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = call120.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = call130.numericalTheta(bump);
	thetaAnalytical = call130.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = call130.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = call140.numericalTheta(bump);
	thetaAnalytical = call140.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = call140.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = call150.numericalTheta(bump);
	thetaAnalytical = call150.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = call150.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );


	// 4. ATM - At-the-Money Put
	thetaNumerical = put100.numericalTheta(bump);
	thetaAnalytical = put100.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = put100.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	// 5. ITM - Out-of-Money Puts
	thetaNumerical = put90.numericalTheta(bump);
	thetaAnalytical = put90.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = put90.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = put80.numericalTheta(bump);
	thetaAnalytical = put80.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = put80.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = put70.numericalTheta(bump);
	thetaAnalytical = put70.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = put70.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = put60.numericalTheta(bump);
	thetaAnalytical = put60.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = put60.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = put50.numericalTheta(bump);
	thetaAnalytical = put50.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = put50.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	// 6. OTM - Out-of-Money Puts
	thetaNumerical = put110.numericalTheta(bump);
	thetaAnalytical = put110.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = put110.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = put120.numericalTheta(bump);
	thetaAnalytical = put120.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = put120.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = put130.numericalTheta(bump);
	thetaAnalytical = put130.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = put130.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = put140.numericalTheta(bump);
	thetaAnalytical = put140.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = put140.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );

	thetaNumerical = put150.numericalTheta(bump);
	thetaAnalytical = put150.theta();
	EXPECT_NEAR(thetaAnalytical, thetaNumerical, testPrecision);

	aadGreeks = put150.priceAndGreeksAAD();
	EXPECT_NEAR( thetaAnalytical, aadGreeks.theta, testPrecision );
}


TEST( TestAqMathBlackScholes, CONSISTENCY_TestAqMathBlackScholesRho_CompareAnalyticalVsNumericalAndAAD )
{
	// Black-Scholes Base Case
	const etrading::CallOrPutEnum call = etrading::CALL_OPTION;
	const etrading::CallOrPutEnum put = etrading::PUT_OPTION;
	const double spot = 100.0;
	const double strike = 100.0;
	const double volatility = 0.1;
	const double timeToExpiry = 1.0;
	const double rate = 0.01;
	const double carry = 0.0;
	const double bump = 1e-5;
	double rhoNumerical = 0.0;
	double rhoAnalytical = 0.0;

	etrading::BlackScholesGreeks aadGreeks;

	// Declare Black-Scholes Option Classes
	etrading::BlackScholes call150(call, 150, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call140(call, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call130(call, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call120(call, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call110(call, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call100(call, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call90(call, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call80(call, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call70(call, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call60(call, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes call50(call, 50, strike, volatility, timeToExpiry, rate, carry);

	etrading::BlackScholes put150(put, 150, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put140(put, 140, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put130(put, 130, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put120(put, 120, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put110(put, 110, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put100(put, 100, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put90(put, 90, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put80(put, 80, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put70(put, 70, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put60(put, 60, strike, volatility, timeToExpiry, rate, carry);
	etrading::BlackScholes put50(put, 50, strike, volatility, timeToExpiry, rate, carry);

	// Compare Analytical and Numerical Risk for Call(s)
	// -------------------------------------------------

	// 1. ATM - At-the-Money Call
	rhoNumerical = call100.numericalRho(bump);
	rhoAnalytical = call100.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = call100.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	// 2. OTM - Out-of-Money Calls
	rhoNumerical = call90.numericalRho(bump);
	rhoAnalytical = call90.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = call90.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = call80.numericalRho(bump);
	rhoAnalytical = call80.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = call80.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = call70.numericalRho(bump);
	rhoAnalytical = call70.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = call70.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = call60.numericalRho(bump);
	rhoAnalytical = call60.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = call60.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = call50.numericalRho(bump);
	rhoAnalytical = call50.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = call50.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );


	// 3. ITM - Out-of-Money Calls
	rhoNumerical = call110.numericalRho(bump);
	rhoAnalytical = call110.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = call110.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = call120.numericalRho(bump);
	rhoAnalytical = call120.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = call120.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = call130.numericalRho(bump);
	rhoAnalytical = call130.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = call130.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = call140.numericalRho(bump);
	rhoAnalytical = call140.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = call140.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = call150.numericalRho(bump);
	rhoAnalytical = call150.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = call150.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	// 4. ATM - At-the-Money Put
	rhoNumerical = put100.numericalRho(bump);
	rhoAnalytical = put100.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = put100.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	// 5. ITM - Out-of-Money Puts
	rhoNumerical = put90.numericalRho(bump);
	rhoAnalytical = put90.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = put90.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = put80.numericalRho(bump);
	rhoAnalytical = put80.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = put80.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = put70.numericalRho(bump);
	rhoAnalytical = put70.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = put70.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = put60.numericalRho(bump);
	rhoAnalytical = put60.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = put60.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = put50.numericalRho(bump);
	rhoAnalytical = put50.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = put50.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	// 6. OTM - Out-of-Money Puts
	rhoNumerical = put110.numericalRho(bump);
	rhoAnalytical = put110.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = put110.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = put120.numericalRho(bump);
	rhoAnalytical = put120.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = put120.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = put130.numericalRho(bump);
	rhoAnalytical = put130.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = put130.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = put140.numericalRho(bump);
	rhoAnalytical = put140.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = put140.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );

	rhoNumerical = put150.numericalRho(bump);
	rhoAnalytical = put150.rho();
	EXPECT_NEAR(rhoAnalytical, rhoNumerical, testPrecision);

	aadGreeks = put150.priceAndGreeksAAD();
	EXPECT_NEAR( rhoAnalytical, aadGreeks.rho, testPrecision );
}
