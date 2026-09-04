/*! @file
    @brief AQLLinearRatesVolatility manager class 
*/
#ifndef AQLLinearRatesVolatility_h
#define AQLLinearRatesVolatility_h

#ifndef PRICING_DATA_VOLATILITYDIRECTINPUT
#define PRICING_DATA_VOLATILITYDIRECTINPUT		"VolatilityDirectInput"		//  data name of VolatilityDirectInput
#endif
#ifndef PRICING_DATA_VOLATILITYDIRECTINPUTS
#define PRICING_DATA_VOLATILITYDIRECTINPUTS		"VolatilityDirectInputs"		//  data name of VolatilityDirectInputs
#endif


#ifdef __GNUG__
#pragma interface
#endif



class AQLString;
class AQLLinearRatesVolatility;
class AQLObject;
class AQLDataProvider;
//===================== Class Declare AQLLinearRatesVolatility==================================
/*! 
    @brief plain vanilla volatility set upper class
*/
class AQLLinearRatesVolatility
{
public:
	// constructor
	explicit AQLLinearRatesVolatility(void);
	// destructor
	virtual ~AQLLinearRatesVolatility(void);
	//set up volatility
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

class AQLPricePlainVolatilityFromDirectInput : public AQLLinearRatesVolatility
{
public:
	// constructor
	explicit AQLPricePlainVolatilityFromDirectInput(void);
	// destructor
	virtual ~AQLPricePlainVolatilityFromDirectInput(void);
	//set up volatility
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

class AQLPricePlainVolatilityFromDirectInputOfCashlets : public AQLLinearRatesVolatility
{
public:
	// constructor
	explicit AQLPricePlainVolatilityFromDirectInputOfCashlets(void);
	// destructor
	virtual ~AQLPricePlainVolatilityFromDirectInputOfCashlets(void);
	//set up volatility
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

class AQLPriceFXStrangleSolverVolatility : public AQLLinearRatesVolatility
{
public:
	// constructor
	explicit AQLPriceFXStrangleSolverVolatility(void);
	// destructor
	virtual ~AQLPriceFXStrangleSolverVolatility(void);
	//set up volatility
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

class AQLPriceFXStrangleSolverATMVolatility : public AQLLinearRatesVolatility
{
public:
	// constructor
	explicit AQLPriceFXStrangleSolverATMVolatility(void);
	// destructor
	virtual ~AQLPriceFXStrangleSolverATMVolatility(void);
	//set up volatility	
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

//hishida vannavolga
class AQLPriceFXVannaVolgaVolatility : public AQLLinearRatesVolatility
{
public:
	// constructor
	explicit AQLPriceFXVannaVolgaVolatility(void);
	// destructor
	virtual ~AQLPriceFXVannaVolgaVolatility(void);
	//set up volatility
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

class AQLPriceIRSABRVolatility : public AQLLinearRatesVolatility
{
public:
	// constructor
	explicit AQLPriceIRSABRVolatility(void);
	// destructor
	virtual ~AQLPriceIRSABRVolatility(void);
	//set up volatility	
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};


class AQLPricePlainVolatilityPVVolMatrixUse : public AQLLinearRatesVolatility
{
public:
	// constructor
	explicit AQLPricePlainVolatilityPVVolMatrixUse(void);
	// destructor
	virtual ~AQLPricePlainVolatilityPVVolMatrixUse(void);
	//set up volatility	
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

#endif
