/*! @file
    @brief LALinearRatesVolatility manager class 
*/
//  2007, AlgoQuantHub.
#ifndef LALinearRatesVolatility_h
#define LALinearRatesVolatility_h

#ifndef PRICING_DATA_VOLATILITYDIRECTINPUT
#define PRICING_DATA_VOLATILITYDIRECTINPUT		"VolatilityDirectInput"		//  data name of VolatilityDirectInput
#endif
#ifndef PRICING_DATA_VOLATILITYDIRECTINPUTS
#define PRICING_DATA_VOLATILITYDIRECTINPUTS		"VolatilityDirectInputs"		//  data name of VolatilityDirectInputs
#endif


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LALinearRatesVolatility.h
//
//  DESCRIPTION :       LALinearRatesVolatility 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif



class AQLString;
class LALinearRatesVolatility;
class AQLObject;
class AQLDataProvider;
//===================== Class Declare LALinearRatesVolatility==================================
/*! 
    @brief plain vanilla volatility set upper class
*/
class LALinearRatesVolatility
{
public:
	// constructor
	explicit LALinearRatesVolatility(void);
	// destructor
	virtual ~LALinearRatesVolatility(void);
	//set up volatility
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

class LAPricePlainVolatilityFromDirectInput : public LALinearRatesVolatility
{
public:
	// constructor
	explicit LAPricePlainVolatilityFromDirectInput(void);
	// destructor
	virtual ~LAPricePlainVolatilityFromDirectInput(void);
	//set up volatility
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

class LAPricePlainVolatilityFromDirectInputOfCashlets : public LALinearRatesVolatility
{
public:
	// constructor
	explicit LAPricePlainVolatilityFromDirectInputOfCashlets(void);
	// destructor
	virtual ~LAPricePlainVolatilityFromDirectInputOfCashlets(void);
	//set up volatility
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

class LAPriceFXStrangleSolverVolatility : public LALinearRatesVolatility
{
public:
	// constructor
	explicit LAPriceFXStrangleSolverVolatility(void);
	// destructor
	virtual ~LAPriceFXStrangleSolverVolatility(void);
	//set up volatility
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

class LAPriceFXStrangleSolverATMVolatility : public LALinearRatesVolatility
{
public:
	// constructor
	explicit LAPriceFXStrangleSolverATMVolatility(void);
	// destructor
	virtual ~LAPriceFXStrangleSolverATMVolatility(void);
	//set up volatility	
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

//hishida vannavolga
class LAPriceFXVannaVolgaVolatility : public LALinearRatesVolatility
{
public:
	// constructor
	explicit LAPriceFXVannaVolgaVolatility(void);
	// destructor
	virtual ~LAPriceFXVannaVolgaVolatility(void);
	//set up volatility
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

class LAPriceIRSABRVolatility : public LALinearRatesVolatility
{
public:
	// constructor
	explicit LAPriceIRSABRVolatility(void);
	// destructor
	virtual ~LAPriceIRSABRVolatility(void);
	//set up volatility	
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};


class LAPricePlainVolatilityPVVolMatrixUse : public LALinearRatesVolatility
{
public:
	// constructor
	explicit LAPricePlainVolatilityPVVolMatrixUse(void);
	// destructor
	virtual ~LAPricePlainVolatilityPVVolMatrixUse(void);
	//set up volatility	
	virtual void setVolatility(AQLDataProvider* dp, AQLObject& object, AQLString model);
private:
};

#endif
