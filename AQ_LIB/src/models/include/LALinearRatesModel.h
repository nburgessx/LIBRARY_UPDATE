/*! @file
    @brief LALinearRatesModel manager class 
*/
//  2011, Mizuho International London.
#ifndef LALinearRatesModel_h
#define LALinearRatesModel_h


////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LALinearRatesModel.h
//
//  DESCRIPTION :       LALinearRatesModel 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif



class LAString;
class LAObject;
class LADataProvider;
class LADataValuation;
//===================== Class Declare LALinearRatesModel==================================
/*! 
    @brief plain vanilla volatility set upper class
*/
class LALinearRatesModel
{
public:
	// constructor
	explicit LALinearRatesModel(void);
	// destructor
	virtual ~LALinearRatesModel(void);
	//set up volatility
	virtual double calcValue(const LADataValuation& att, LADataProvider* dp, LAObject& object, LAString productname) = 0;
private:
};

class LAPriceBSValueModel : public LALinearRatesModel
{
public:
	// constructor
	explicit LAPriceBSValueModel(void);
	// destructor
	virtual ~LAPriceBSValueModel(void);
	//set up volatility	
	virtual double calcValue(const LADataValuation& att, LADataProvider* dp, LAObject& object, LAString productname);
private:
};

class LAPriceVVValueModel : public LALinearRatesModel
{
public:
	// constructor
	explicit LAPriceVVValueModel(void);
	// destructor
	virtual ~LAPriceVVValueModel(void);
	//set up volatility
	virtual double calcValue(const LADataValuation& att, LADataProvider* dp, LAObject& object, LAString productname);
private:
};

#endif
