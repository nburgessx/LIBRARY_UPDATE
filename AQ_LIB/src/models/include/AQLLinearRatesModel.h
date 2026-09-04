/*! @file
    @brief AQLLinearRatesModel manager class 
*/
#ifndef AQLLinearRatesModel_h
#define AQLLinearRatesModel_h


#ifdef __GNUG__
#pragma interface
#endif



class AQLString;
class AQLObject;
class AQLDataProvider;
class AQLDataValuation;
//===================== Class Declare AQLLinearRatesModel==================================
/*! 
    @brief plain vanilla volatility set upper class
*/
class AQLLinearRatesModel
{
public:
	// constructor
	explicit AQLLinearRatesModel(void);
	// destructor
	virtual ~AQLLinearRatesModel(void);
	//set up volatility
	virtual double calcValue(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& object, AQLString productname) = 0;
private:
};

class AQLPriceBSValueModel : public AQLLinearRatesModel
{
public:
	// constructor
	explicit AQLPriceBSValueModel(void);
	// destructor
	virtual ~AQLPriceBSValueModel(void);
	//set up volatility	
	virtual double calcValue(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& object, AQLString productname);
private:
};

class AQLPriceVVValueModel : public AQLLinearRatesModel
{
public:
	// constructor
	explicit AQLPriceVVValueModel(void);
	// destructor
	virtual ~AQLPriceVVValueModel(void);
	//set up volatility
	virtual double calcValue(const AQLDataValuation& att, AQLDataProvider* dp, AQLObject& object, AQLString productname);
private:
};

#endif
